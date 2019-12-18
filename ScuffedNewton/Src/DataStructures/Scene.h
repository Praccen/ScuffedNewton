#pragma once

#include <unordered_map>

class Entity;

class Scene {
public:
	Scene();
	~Scene();

	int addEntity();

private:
	std::unordered_map<int, Entity> m_entities;
};