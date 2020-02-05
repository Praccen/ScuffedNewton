#include "../pch.h"

#include <unordered_map>

#include "Entity.h"
#include "../DataStructures/Scene.h"


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

bool Entity::hasComponents(const std::unordered_map<std::string, bool>& requiredComponents) {
	for (auto it = requiredComponents.begin(); it != requiredComponents.end(); it++) {
		if (m_components.count(it->first) <= 0) {
			return false;
		}
	}
}

void Entity::addToSystems() {
	m_scene->addEntityToSystems(this);
}

void Entity::removeFromSystems() {
	m_scene->removeEntityFromSystems(this);
}