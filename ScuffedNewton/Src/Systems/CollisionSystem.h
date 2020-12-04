#pragma once

#include "BaseSystem.h"
#include "../Calculations/Intersection.h"

namespace Scuffed {

	class Octree;

	class CollisionSystem : public BaseSystem {
	public:
		CollisionSystem();
		~CollisionSystem();

		void provideOctree(Octree* octree);
		void update(float dt);

	private:
		void handleCollisions(Intersection::CollisionTimeInfo& collisionInfo, std::vector<Entity*> &collidingEntities, int recursionDepth = 0);
		//void updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);

	private:
		Octree* m_octree;
		
		std::vector<Intersection::CollisionTimeInfo> m_collisionOrder;
	};

}