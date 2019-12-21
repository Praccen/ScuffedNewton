#include "../pch.h"

#include "Scene.h"
#include "../DataTypes/Entity.h"
#include "../Utils/Utils.h"

Scene::Scene() {

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
 	}

	return testId;
}