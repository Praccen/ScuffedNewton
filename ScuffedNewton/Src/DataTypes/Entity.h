#pragma once

class BoundingBox;
class Mesh;

class Entity {
public:
	Entity();
	virtual ~Entity();

	void setCollidable(bool status);
	void setHasModel(bool status);

	unsigned int getId() const;

	BoundingBox* getBoundingBox() const;

	Mesh* getMesh() const;

	bool hasModel() const;
	bool isCollidable() const;
	bool allowSimpleCollision() const;

	template<typename ComponentType, typename... Targs>
	ComponentType* addComponent(Targs... args);

	template<typename ComponentType>
	void removeComponent();

	template<typename ComponentType>
	ComponentType* getComponent();

	template<typename ComponentType>
	bool hasComponent() const;

private:
	unsigned int m_id;

	BoundingBox* m_boundingBox;
	Mesh* m_mesh;
	bool m_hasModel;
	bool m_allowSimpleCollision;
	bool m_collidable;
};

template<typename ComponentType, typename... Targs>
inline ComponentType* Entity::addComponent(Targs... args) {
	if (m_components[ComponentType::ID]) {
		SAIL_LOG_WARNING("Tried to add a duplicate component to an entity");
	}
	else {
		m_components[ComponentType::ID] = std::make_unique<ComponentType>(args...);

		m_componentTypes |= ComponentType::getBID();

		// Place this entity within the correct systems if told to
		if (tryToAddToSystems) {
			addToSystems();
		}
	}

	// Return pointer to the component
	return static_cast<ComponentType*>(m_components[ComponentType::ID].get());
}

template<typename ComponentType>
inline void Entity::removeComponent() {
	if (hasComponent<ComponentType>()) {
		m_components[ComponentType::ID].reset(nullptr);

		// Set the component type bit to 0 if it was 1
		std::bitset<MAX_NUM_COMPONENTS_TYPES> bits = 0;
		bits |= ComponentType::getBID();				// set a 1 to the type bit
		bits = ~bits;									// set a 0 to the type bit and rest to 1
		m_componentTypes = m_componentTypes & bits;		// keep the values of each not except the type bit which is now 0

		// Remove this entity from systems which required the removed component
		removeFromSystems();
	}
}

template<typename ComponentType>
inline ComponentType* Entity::getComponent() {
	if (hasComponent<ComponentType>()) {
		return static_cast<ComponentType*>(m_components[ComponentType::ID].get());
	}
	return nullptr;
}

template<typename ComponentType>
inline bool Entity::hasComponent() const {
	bool bitTrue = (m_componentTypes & ComponentType::getBID()).any();
	bool ptrTrue = m_components[ComponentType::ID] != nullptr;
	return bitTrue && ptrTrue;
}