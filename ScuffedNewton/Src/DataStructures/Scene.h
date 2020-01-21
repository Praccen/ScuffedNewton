#pragma once

#include <unordered_map>
#include "../DataTypes/Entity.h"

class Octree;

class Scene {
public:
	Scene();
	~Scene();

	int addEntity();

	void setCollidable(int entityId, bool status);

	void update(float dt);

private:
	std::unordered_map<int, Entity> m_entities;

	Octree *m_octree;
};