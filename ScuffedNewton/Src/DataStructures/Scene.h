#pragma once

#include <unordered_map>
#include "../DataTypes/Entity.h"
#include "../DataTypes/Mesh.h"

class Octree;

class Scene {
public:
	Scene();
	~Scene();

	int addEntity();

	Entity* getEntity(int entityId);

	void update(float dt);

private:
	std::unordered_map<int, Entity> m_entities;
	//TODO: implement systems

	Octree *m_octree;
};