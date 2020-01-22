#include "../pch.h"

#include "Scene.h"
#include "Octree.h"
#include "../DataTypes/Entity.h"
#include "../Utils/Utils.h"

Scene::Scene() {
	m_octree = SN_NEW Octree();
}

Scene::~Scene() {

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

void Scene::update(float dt) {
	m_octree->update();
}