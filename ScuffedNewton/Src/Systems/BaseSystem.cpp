#include "BaseSystem.h"

BaseSystem::BaseSystem() {

}

BaseSystem::~BaseSystem() {

}

bool BaseSystem::addEntity(Entity* entity) {
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i] == entity) {
			return false;
		}
	}

	entities.push_back(entity);
}

void BaseSystem::removeEntity(Entity* entity) {
	for (size_t i = 0; i < entities.size(); i++) {
		if (entities[i] == entity) {
			entities.erase(entities.begin() + i);
			break;
		}
	}
}

void BaseSystem::update(float dt) {

}

std::unordered_map<std::string, bool>& BaseSystem::getRequiredComponentTypes() {
	return requiredComponents;
}
