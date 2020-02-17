#pragma once

#include <unordered_map>

namespace Scuffed {

	class Octree;
	class Entity;
	class BaseSystem;

	class Scene {
	public:
		Scene();
		virtual ~Scene();

		virtual int addEntity();
		virtual Entity* getEntity(int entityId);
		virtual Octree* getOctree();

		virtual void createSystems();
		virtual void deleteSystems();
		virtual void update(float dt);

		virtual void addEntityToSystems(Entity* entity);
		virtual void removeEntityFromSystems(Entity* entity);

	private:
		std::unordered_map<int, Entity*> m_entities;
		std::vector<BaseSystem*> m_systems;

		Octree* m_octree;
	};

}