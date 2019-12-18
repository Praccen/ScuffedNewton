//#include "../pch.h"

#include "Scene.h"

#include "../Utils/Utils.h"

Scene::Scene() {

}

Scene::~Scene() {

}

int Scene::addEntity() {
	int testId = Utils::sEntityIdCounter;
	auto pair = m_entities.try_emplace(Utils::sEntityIdCounter);
	if (pair.second) {
		// Created a new entity. Return the entity id
		return testId;
	}
	else {
		return -1;
	}
}