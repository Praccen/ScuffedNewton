#pragma once

#include "BaseSystem.h"
#include "../DataStructures/Octree.h"

namespace Scuffed {

	class CollisionSystem : public BaseSystem {
	public:
		CollisionSystem();
		~CollisionSystem();

		void provideOctree(Octree* octree);
		void update(float dt);

	private:
		float getNextCollisionTime(Entity* e, std::vector<Entity*>& collidingWith, const float dt);
		void handleCollisions(Entity* e1, Entity* e2, int recursionDepth = 0);
		//void updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);

	private:
		Octree* m_octree;
		
		struct UpcomingCollision {
			float collisionTime = INFINITY;
			Entity* entity1 = nullptr;
			Entity* entity2 = nullptr;
			bool operator<(const UpcomingCollision& other) {
				return collisionTime < other.collisionTime;
			}
		};
		
		std::vector<UpcomingCollision> m_collisionOrder;
	};

}