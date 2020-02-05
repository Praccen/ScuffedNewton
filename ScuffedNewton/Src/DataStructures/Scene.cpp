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

	for (auto e : m_entities) {
		delete e.second;
	}
}

int Scene::addEntity() {
	int testId = Utils::instance()->GetEntityIdCounter();
	if (m_entities.count(testId) > 0) {
		return -1;
	}
	else {
		// Create new entity
		m_entities[testId] = SN_NEW Entity(this);
	}

	return testId;
}

Entity* Scene::getEntity(int entityId) {
	if (m_entities.count(entityId) > 0) {
		return m_entities[entityId];
	}
	else {
		return nullptr;
	}
}

void Scene::createSystems() {
	m_systems.emplace_back();
	m_systems.back() = SN_NEW UpdateBoundingBoxSystem();

	m_systems.emplace_back();
	m_systems.back() = SN_NEW OctreeAddRemoverSystem();

	m_systems.emplace_back();
	m_systems.back() = SN_NEW MovementSystem();

	m_systems.emplace_back();
	m_systems.back() = SN_NEW CollisionSystem();

	m_systems.emplace_back();
	m_systems.back() = SN_NEW MovementPostCollisionSystem();

	m_systems.emplace_back();
	m_systems.back() = SN_NEW SpeedLimitSystem();


	/*
	m_systems.updateBoundingBoxSystem = SN_NEW UpdateBoundingBoxSystem();
	m_systems.octreeAddRemoverSystem = SN_NEW OctreeAddRemoverSystem();
	m_systems.movementSystem = SN_NEW MovementSystem();
	m_systems.collisionSystem = SN_NEW CollisionSystem();
	m_systems.movementPostCollisionSystem = SN_NEW MovementPostCollisionSystem();
	m_systems.speedLimitSystem = SN_NEW SpeedLimitSystem();
	*/
}

void Scene::update(float dt) {
	//m_octree->update();
	for (auto s : m_systems) {
		s->update(dt);
	}
}

void Scene::addEntityToSystems(Entity* entity) {
	// Check which systems this entity can be placed in
	for (auto sys : m_systems) {
		auto componentTypes = sys->getRequiredComponentTypes();

		// Add this entity to the system
		if (entity->hasComponents(componentTypes)) {
			sys->addEntity(entity);
		}
	}
}

void Scene::removeEntityFromSystems(Entity* entity) {
	for (auto sys : m_systems) {
		sys->removeEntity(entity);
	}
}

