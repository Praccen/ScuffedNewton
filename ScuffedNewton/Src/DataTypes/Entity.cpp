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
		case Components::CollidableComponent: return addComponent<CollidableComponent>();
		case Components::CollisionComponent: return addComponent<CollisionComponent>();
		case Components::MovementComponent: return addComponent<MovementComponent>();
		case Components::MeshComponent: return addComponent<MeshComponent>();
		case Components::TransformComponent: return addComponent<TransformComponent>();
		};
		return nullptr;
	}

	void Entity::removeComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return removeComponent<BoundingBoxComponent>();
		case Components::CollidableComponent: return removeComponent<CollidableComponent>();
		case Components::CollisionComponent: return removeComponent<CollisionComponent>();
		case Components::MovementComponent: return removeComponent<MovementComponent>();
		case Components::MeshComponent: return removeComponent<MeshComponent>();
		case Components::TransformComponent: return removeComponent<TransformComponent>();
		};
	}

	Component* Entity::getComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return getComponent<BoundingBoxComponent>();
		case Components::CollidableComponent: return getComponent<CollidableComponent>();
		case Components::CollisionComponent: return getComponent<CollisionComponent>();
		case Components::MovementComponent: return getComponent<MovementComponent>();
		case Components::MeshComponent: return getComponent<MeshComponent>();
		case Components::TransformComponent: return getComponent<TransformComponent>();
		};
		return nullptr;
	}

	bool Entity::hasComponent(int componentNumber) {
		switch (componentNumber) {
		case Components::BoundingBoxComponent: return hasComponent<BoundingBoxComponent>();
		case Components::CollidableComponent: return hasComponent<CollidableComponent>();
		case Components::CollisionComponent: return hasComponent<CollisionComponent>();
		case Components::MovementComponent: return hasComponent<MovementComponent>();
		case Components::MeshComponent: return hasComponent<MeshComponent>();
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