#pragma once

#include <unordered_map>

class Octree;
class Entity;
class BaseSystem;

class Scene {
public:
	Scene();
	~Scene();

	int addEntity();
	Entity* getEntity(int entityId);

	void createSystems();
	void deleteSystems();
	void update(float dt);

	void addEntityToSystems(Entity* entity);
	void removeEntityFromSystems(Entity* entity);

private:
	std::unordered_map<int, Entity*> m_entities;
	std::vector<BaseSystem*> m_systems;

	Octree *m_octree;
};