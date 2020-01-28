#include "../pch.h"

#include "Entity.h"
#include "BoundingBox.h"
#include "Mesh.h"
#include "../DataStructures/Scene.h"

#include "../Utils/Utils.h"

Entity::Entity(Scene* scene) {
	m_id = Utils::instance()->GetEntityIdCounter(true);
	m_scene = scene;
}

Entity::~Entity() {
}

unsigned int Entity::getId() const {
	return m_id;
}

void Entity::addToSystems() {
	m_scene->addEntityToSystems(this);
}

void Entity::removeFromSystems() {
	m_scene->removeEntityFromSystems(this);
}

template<typename ComponentType, typename... Targs>
inline ComponentType* Entity::addComponent(Targs... args) {
	if (m_components[ComponentType::ID]) {
		SAIL_LOG_WARNING("Tried to add a duplicate component to an entity");
	}
	else {
		m_components[ComponentType::ID] = SN_NEW ComponentType(args...);

		// Place this entity within the correct systems if told to
		if (tryToAddToSystems) {
			addToSystems();
		}
	}

	// Return pointer to the component
	return static_cast<ComponentType*>(m_components[ComponentType::ID]);
}

template<typename ComponentType>
inline void Entity::removeComponent() {
	if (hasComponent<ComponentType>()) {
		delete m_components[ComponentType::ID];

		// Remove this entity from systems which required the removed component
		removeFromSystems();
	}
}

template<typename ComponentType>
inline ComponentType* Entity::getComponent() {
	if (hasComponent<ComponentType>()) {
		return static_cast<ComponentType*>(m_components[ComponentType::ID]);
	}
	return nullptr;
}

template<typename ComponentType>
inline bool Entity::hasComponent() const {
	return m_components[ComponentType::ID] != nullptr;
}