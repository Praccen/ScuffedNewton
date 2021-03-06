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
		void continousCollisionUpdate(Entity* e, float& dt);
		bool handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt);
		void gatherCollisionInformation(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
		void updateVelocityVec(Entity* e, glm::vec3& velocity, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
		glm::vec3 surfaceFromCollision(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);
		void updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions);

	private:
		Octree* m_octree;
	};

}