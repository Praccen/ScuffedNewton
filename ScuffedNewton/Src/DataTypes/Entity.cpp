#include "../pch.h"

#include "Entity.h"
#include "../DataStructures/Scene.h"

#include "../Components/Components.h"

namespace Scuffed {

	Entity::Entity(Scene* scene) {
		m_id = Utils::instance()->GetEntityIdCounter(true);
		m_scene = scene;
	}

	Entity::~Entity() {
		for (auto it : m_components) {
			delete it.second;
		}
	}

	unsigned int Entity::getId() const {
		return m_id;
	}

	Component* Entity::addComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return addComponent<BoundingBoxComponent>();
		case Components::MeshComponent: return addComponent<MeshComponent>();
		case Components::PhysicalBodyComponent: return addComponent<PhysicalBodyComponent>();
		case Components::TransformComponent: return addComponent<TransformComponent>();
		};
		return nullptr;
	}

	void Entity::removeComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return removeComponent<BoundingBoxComponent>();
		case Components::MeshComponent: return removeComponent<MeshComponent>();
		case Components::PhysicalBodyComponent: return removeComponent<PhysicalBodyComponent>();
		case Components::TransformComponent: return removeComponent<TransformComponent>();
		};
	}

	Component* Entity::getComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return getComponent<BoundingBoxComponent>();
		case Components::MeshComponent: return getComponent<MeshComponent>();
		case Components::PhysicalBodyComponent: return getComponent<PhysicalBodyComponent>();
		case Components::TransformComponent: return getComponent<TransformComponent>();
		};
		return nullptr;
	}

	bool Entity::hasComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return hasComponent<BoundingBoxComponent>();
		case Components::MeshComponent: return hasComponent<MeshComponent>();
		case Components::PhysicalBodyComponent: return hasComponent<PhysicalBodyComponent>();
		case Components::TransformComponent: return hasComponent<TransformComponent>();
		};
		return false;
	}

	bool Entity::hasComponents(const std::unordered_map<std::string, bool>& requiredComponents) {
		for (auto it = requiredComponents.begin(); it != requiredComponents.end(); it++) {
			if (m_components.count(it->first) <= 0) {
				return false;
			}
		}
		return true;
	}

	void Entity::addToSystems() {
		m_scene->addEntityToSystems(this);
	}

	void Entity::removeFromSystems() {
		m_scene->removeEntityFromSystems(this);
	}

}