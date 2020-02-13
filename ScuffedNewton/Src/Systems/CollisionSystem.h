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
		bool collisionUpdatePart(float dt);
		bool surfaceFromCollisionPart(float dt);
		bool rayCastCollisionPart(float dt);

		const bool rayCastCheck(Entity* e, const BoundingBox* boundingBox, const glm::vec3& velocity, const float& dt) const;
		void rayCastUpdate(Entity* e, BoundingBox* boundingBox, float& dt);
		void rayCastRagdollUpdate(Entity* e, float& dt);
		void collisionUpdate(Entity* e, const float dt);
		const bool handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt);
		const bool handleRagdollCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, bool calculateMomentum, const float dt);
		void gatherCollisionInformation(Entity* e, const BoundingBox* boundingBox, std::vector<Octree::CollisionInfo>& collisions, std::vector<Octree::CollisionInfo>& trueCollisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
		void updateVelocityVec(Entity* e, glm::vec3& velocity, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt);
		glm::vec3 surfaceFromCollision(Entity* e, BoundingBox* boundingBox, std::vector<Octree::CollisionInfo>& collisions);
		void surfaceFromRagdollCollision(Entity* e, std::vector<Octree::CollisionInfo>& collisions);

		glm::vec3 getAngularVelocity(Entity* e, const glm::vec3& offset, const glm::vec3& centerOfMass);

	private:
		Octree* m_octree;
	};

}