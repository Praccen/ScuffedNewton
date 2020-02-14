#pragma once

#include <unordered_map>
#include <string>

#include "../Utils/Utils.h"

namespace Scuffed {

	class Component;
	class Scene;

	class Entity {
	public:
		Entity(Scene* scene);
		virtual ~Entity();

		virtual unsigned int getId() const;
		virtual Component* addComponent(int componentNumber);
		virtual void removeComponent(int componentNumber);
		virtual Component* getComponent(int componentNumber);
		virtual bool hasComponent(int componentNumber);
		virtual bool hasComponents(const std::unordered_map<std::string, bool>& requiredComponents);

	public:
		// ----Template functions----
		//Can only be used from inside ScuffedNewton if explicity linking
		template<typename ComponentType, typename... Targs>
		ComponentType* addComponent(Targs... args);

		template<typename ComponentType>
		void removeComponent();

		template<typename ComponentType>
		ComponentType* getComponent();

		template<typename ComponentType>
		bool hasComponent() const;
		// --------------------------

	private:
		void addToSystems();
		void removeFromSystems();

	private:
		unsigned int m_id;

		Scene* m_scene;

		std::unordered_map<std::string, Component*> m_components;
	};

	// ----Template functions----
	template<typename ComponentType, typename... Targs>
	inline ComponentType* Entity::addComponent(Targs... args) {
		if (m_components[ComponentType::ID]) {
			//SAIL_LOG_WARNING("Tried to add a duplicate component to an entity");
			std::cout << "Tried to add a duplicate component to an entity\n";
		}
		else {
			m_components[ComponentType::ID] = SN_NEW ComponentType(args...);

			// Place this entity within the correct systems if told to
			//if (tryToAddToSystems) {
			addToSystems();
			//}
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
		return m_components.find(ComponentType::ID) != m_components.end();
	}
	// --------------------------
}