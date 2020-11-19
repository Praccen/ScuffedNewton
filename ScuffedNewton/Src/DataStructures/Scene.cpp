#include "../pch.h"

#include "Scene.h"
#include "Octree.h"
#include "../DataTypes/Entity.h"
#include "../Systems/Systems.h"

namespace Scuffed {

	Scene::Scene() {
		m_octree = SN_NEW Octree();

		createSystems();
	}

	Scene::~Scene() {
		for (auto e : m_entities) {
			delete e.second;
		}

		deleteSystems();

		delete m_octree;
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

	Octree* Scene::getOctree() {
		return m_octree;
	}

	void Scene::createSystems() {
		// System updates happens in this order
		m_systems.emplace_back();
		m_systems.back() = SN_NEW UpdateBoundingBoxSystem();

		m_systems.emplace_back();
		m_systems.back() = SN_NEW OctreeAddRemoverSystem();

		static_cast<OctreeAddRemoverSystem*>(m_systems.back())->provideOctree(m_octree);

		m_systems.emplace_back();
		m_systems.back() = SN_NEW MovementSystem();

		m_systems.emplace_back();
		m_systems.back() = SN_NEW CollisionSystem();

		static_cast<CollisionSystem*>(m_systems.back())->provideOctree(m_octree);
	}

	void Scene::deleteSystems() {
		for (auto s : m_systems) {
			delete s;
		}
	}

	void Scene::update(float dt) {
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

}