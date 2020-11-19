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
		float getNextCollisionTime(Entity* e, std::vector<Octree::CollisionInfo>& collidingWith, const float dt);
		void handleCollisions(Entity* e, Octree::CollisionInfo& collision, const float dt);
		glm::vec3 surfaceFromCollision(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);
		void updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);

	private:
		Octree* m_octree;
	private:
		struct UpcomingCollision {
			float collisionTime = INFINITY;
			Entity* entity = nullptr;
			Octree::CollisionInfo collisionInfo;
			bool operator<(const UpcomingCollision& other) {
				return collisionTime < other.collisionTime;
			}
		};
		
	};

}