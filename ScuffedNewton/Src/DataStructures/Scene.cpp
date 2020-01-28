#include "../pch.h"

#include "Scene.h"
#include "Octree.h"
#include "../DataTypes/Entity.h"
#include "../Systems/Systems.h"

Scene::Scene() {
	m_octree = SN_NEW Octree();
}

Scene::~Scene() {
	delete m_octree;
}

int Scene::addEntity() {
	int testId = Utils::instance()->GetEntityIdCounter();
	if (m_entities.count(testId) > 0) {
		return -1;
	}
	else {
		// Create new entity
		m_entities[testId];
		m_octree->addEntity(&m_entities[testId]);
 	}

	return testId;
}

Entity* Scene::getEntity(int entityId) {
	if (m_entities.count(entityId) > 0) {
		return &m_entities[entityId];
	}
}

void Scene::createSystems() {
	m_systems.emplace_back(); 
	m_systems.back() = SN_NEW CollisionSystem();

	/*m_systems.collisionSystem = SN_NEW CollisionSystem();
	m_systems.movementPostCollisionSystem = SN_NEW MovementPostCollisionSystem();
	m_systems.movementSystem = SN_NEW MovementSystem();
	m_systems.octreeAddRemoverSystem = SN_NEW OctreeAddRemoverSystem();
	m_systems.speedLimitSystem = SN_NEW SpeedLimitSystem();
	m_systems.updateBoundingBoxSystem = SN_NEW UpdateBoundingBoxSystem();*/
}

void Scene::update(float dt) {
	m_octree->update();
}

void Scene::addEntityToSystems(Entity* entity) {
	//SystemMap::iterator it = m_systems.begin();

	//// Check which systems this entity can be placed in
	//for (; it != m_systems.end(); ++it) {
	//	if (it->second) {
	//		auto componentTypes = it->second->getRequiredComponentTypes();

	//		// Add this entity to the system
	//		if (entity->hasComponents(componentTypes)) {
	//			it->second->addEntity(entity);
	//		}
	//	}
	//}
	assert(false); //Not implemented yet
}

void Scene::removeEntityFromSystems(Entity* entity) {
	assert(false); //Not implemented yet
}

