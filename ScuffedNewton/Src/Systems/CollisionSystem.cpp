#include "../pch.h"

#include "CollisionSystem.h"

#include <memory>
#include <map>

#include "../Components/Components.h"
#include "../DataTypes/Entity.h"
#include "../Calculations/Intersection.h"
#include "../DataStructures/Octree.h"
#include "../Shapes/Box.h"

namespace Scuffed {

	CollisionSystem::CollisionSystem() {
		requiredComponents["CollisionComponent"] = true;
		requiredComponents["MovementComponent"] = true;
		requiredComponents["BoundingBoxComponent"] = true;
		requiredComponents["TransformComponent"] = true;

		m_octree = nullptr;
	}

	CollisionSystem::~CollisionSystem() {
	}

	void CollisionSystem::provideOctree(Octree* octree) {
		m_octree = octree;
	}

	void CollisionSystem::update(float dt) {
		// ======================== Collision Update ======================================
		std::vector<std::pair<float, std::pair<Entity*, std::vector<Octree::CollisionInfo>>>> collisionOrder(entities.size());
		int entityNum = 0;

		// ---- Take care of all current collisions and save next upcoming collision----
		for (auto& e : entities) {
			performCurrentCollisions(e);

			// Find and save upcoming collision time
			collisionOrder[entityNum].second.first = e;
			collisionOrder[entityNum].first = getNextCollisionTime(e, collisionOrder[entityNum].second.second, dt);
			entityNum++;
		}
		// -------------------------------------

		float accumulativeTime = 0.f;
		float nextCollision = INFINITY;
		std::vector<int> collidingEntityIndices;
		bool collisionHappening = false;

		for (int i = 0; i < collisionOrder.size(); i++) {
			if (collisionOrder[i].first < nextCollision) {
				collidingEntityIndices.clear();
				nextCollision = collisionOrder[i].first;
				collidingEntityIndices.emplace_back(i);
				collisionHappening = true;
			}
			else if (collisionOrder[i].first == nextCollision) {
				collidingEntityIndices.emplace_back(i);
			}
		}

		while (nextCollision < dt && collisionHappening) {
			// Advance all objects untill next collision
			//std::cout << nextCollision - accumulativeTime;

			for (auto& e : entities) {
				Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				CollisionComponent* collision = e->getComponent<CollisionComponent>();
				MovementComponent* movement = e->getComponent<MovementComponent>();
				TransformComponent* transform = e->getComponent<TransformComponent>();

				glm::vec3 additionalMovement(0.f);
				additionalMovement = glm::normalize(movement->velocity) * 0.0001f;
				transform->translate(movement->velocity * (nextCollision - accumulativeTime) + additionalMovement);
				boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
			}

			//Handle collision
			for (int i = 0; i < collidingEntityIndices.size(); i++) {
				handleCollisions(collisionOrder[collidingEntityIndices[i]].second.first, collisionOrder[collidingEntityIndices[i]].second.second, 0.0f);

				CollisionComponent* collision = collisionOrder[collidingEntityIndices[i]].second.first->getComponent<CollisionComponent>();

				// Save collisions to collision component
				collision->collisions.insert(collision->collisions.end(), collisionOrder[collidingEntityIndices[i]].second.second.begin(), collisionOrder[collidingEntityIndices[i]].second.second.end());

			}

			accumulativeTime = nextCollision;

			//Update next collision values for the colliding objects
			for (int i = 0; i < collidingEntityIndices.size(); i++) {
				collisionOrder[collidingEntityIndices[i]].second.second.clear();
				collisionOrder[collidingEntityIndices[i]].first = getNextCollisionTime(collisionOrder[collidingEntityIndices[i]].second.first, collisionOrder[collidingEntityIndices[i]].second.second, dt - accumulativeTime) + accumulativeTime;
			}

			float nextCollision = INFINITY;
			collidingEntityIndices.clear();
			collisionHappening = false;

			for (int i = 0; i < collisionOrder.size(); i++) {
				if (collisionOrder[i].first < nextCollision) {
					collidingEntityIndices.clear();
					nextCollision = collisionOrder[i].first;
					collidingEntityIndices.emplace_back(i);
					collisionHappening = true;
				}
				else if (collisionOrder[i].first == nextCollision) {
					collidingEntityIndices.emplace_back(i);
				}
			}
		}


		for (auto& e : entities) {
			CollisionComponent* collision = e->getComponent<CollisionComponent>();

			surfaceFromCollision(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);

			updateManifolds(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);
		}
	}

	void CollisionSystem::performCurrentCollisions(Entity* e) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		float time = INFINITY;

		std::vector<Octree::CollisionInfo> collisions;
		std::vector<Octree::CollisionInfo> zeroDistances;

		m_octree->getNextContinousCollision(e, collisions, time, zeroDistances, 0.f, collision->doSimpleCollisions);

		handleCollisions(e, zeroDistances, 0.f);
	}

	float CollisionSystem::getNextCollisionTime(Entity* e, std::vector<Octree::CollisionInfo>& collidingWith, const float dt) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		float time = INFINITY;

		std::vector<Octree::CollisionInfo> zeroDistances;

		m_octree->getNextContinousCollision(e, collidingWith, time, zeroDistances, dt, collision->doSimpleCollisions);

		return time;
	}

	void CollisionSystem::continousCollisionUpdate(Entity* e, float& dt) {
		Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();

		float time = INFINITY;

		std::vector<Octree::CollisionInfo> collisions;
		std::vector<Octree::CollisionInfo> zeroDistances;

		m_octree->getNextContinousCollision(e, collisions, time, zeroDistances, dt, collision->doSimpleCollisions);

		if (handleCollisions(e, zeroDistances, dt)) {
			// Clear
			time = INFINITY;
			zeroDistances.clear();
			collisions.clear();

			m_octree->getNextContinousCollision(e, collisions, time, zeroDistances, dt, collision->doSimpleCollisions);
		}

		// Save zeroes
		collision->collisions.insert(collision->collisions.end(), zeroDistances.begin(), zeroDistances.end());

		while (time <= dt && time > 0.f) {
			// Move entity to collision
			glm::vec3 additionalMovement(0.f);
			additionalMovement = glm::normalize(movement->velocity) * 0.0001f;
			transform->translate(movement->velocity * time + additionalMovement);
			boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
			//boundingBox->setTranslation(boundingBox->getMiddle() + movement->velocity * time);

			// Decrease time
			dt -= time;

			handleCollisions(e, collisions, 0.f);

			// Save collisions to collision component
			collision->collisions.insert(collision->collisions.end(), collisions.begin(), collisions.end());

			// Clear
			time = INFINITY;
			zeroDistances.clear();
			collisions.clear();

			// Check for next collision
			m_octree->getNextContinousCollision(e, collisions, time, zeroDistances, dt, collision->doSimpleCollisions);
		}
	}

	bool CollisionSystem::handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt) {
		MovementComponent* movement = e->getComponent<MovementComponent>();
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

		collision->onGround = false;

		const size_t collisionCount = collisions.size();

		if (collisionCount > 0) {
			std::vector<int> groundIndices;
			glm::vec3 sumVec(0.0f);

			// Gather info
			gatherCollisionInformation(e, boundingBox, collisions, sumVec);

			glm::vec3 preVel = movement->velocity;

			// Handle true collisions
			updateVelocityVec(e, movement->velocity, collisions, sumVec, dt);

			if (glm::length2(movement->velocity) > 0.0f && Intersection::dot(glm::normalize(preVel), glm::normalize(movement->velocity)) < 1.f) {
				// Collisions effected movement
				return true;
			}
		}

		// Collisions did not effect movement
		return false;
	}

	void CollisionSystem::gatherCollisionInformation(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec) {
		size_t collisionCount = collisions.size();

		if (collisionCount > 0) {
			// Get the actual intersection axises
			for (size_t i = 0; i < collisionCount; i++) {
				Octree::CollisionInfo& collisionInfo_i = collisions[i];

				if (Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), &collisionInfo_i.intersectionAxis, &collisionInfo_i.intersectionDepth)) {
					//if (collisionInfo_i.shape.get()->getVertices().size() == 3) {
					//	// Triangle, make sure collision is along normal
					//	if (glm::dot(collisionInfo_i.intersectionAxis, collisionInfo_i.shape.get()->getNormals()[0]) < 0.f) {
					//		// False collision
					//		collisions.erase(collisions.begin() + i);
					//		collisionCount--;
					//		i--;
					//		continue;
					//	}
					//}

					sumVec += collisionInfo_i.intersectionAxis;
				}
				else {
					// False collision
					collisions.erase(collisions.begin() + i);
					collisionCount--;
					i--;
				}
			}
		}
	}

	void CollisionSystem::updateVelocityVec(Entity* e, glm::vec3& velocity, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, const float dt) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		const size_t collisionCount = collisions.size();

		// Loop through collisions and handle them
		for (size_t i = 0; i < collisionCount; i++) {
			const Octree::CollisionInfo& collisionInfo_i = collisions[i];

			// ----Velocity changes from collisions----

			// Stop movement towards triangle
			float projectionSize = glm::dot(velocity, -collisionInfo_i.intersectionAxis);

			if (projectionSize > 0.0f) { //Is pushing against wall

				bool calcFriction = false;
				//glm::vec3 e_n = velocity + projectionSize * glm::normalize(collisionInfo_i.intersectionAxis);
				glm::vec3 e_n = glm::cross(glm::cross(velocity, collisionInfo_i.intersectionAxis), collisionInfo_i.intersectionAxis);

				if (glm::length2(e_n) > 0.00000001f) {
					e_n = glm::normalize(e_n);
					calcFriction = true;
				}
					
					//glm::normalize(glm::cross(glm::cross(velocity, collisionInfo_i.intersectionAxis), collisionInfo_i.intersectionAxis));

				velocity += (projectionSize * (1.0f + collision->bounciness)) * (collisionInfo_i.intersectionAxis); // +collision->drag * e_n); // Update velocity including bouncing and friction
				if (calcFriction) {
					velocity += projectionSize * collision->drag * e_n * dt;
				}
			}


			// Tight angle corner special case
			const float dotProduct = glm::dot(collisionInfo_i.intersectionAxis, glm::normalize(sumVec));
			if (dotProduct < 0.7072f && dotProduct > 0.0f) { // Colliding in a tight angle corner
				glm::vec3 normalToNormal = sumVec - glm::dot(sumVec, collisionInfo_i.intersectionAxis) * collisionInfo_i.intersectionAxis;
				normalToNormal = glm::normalize(normalToNormal);

				// Stop movement towards corner
				projectionSize = glm::dot(velocity, -normalToNormal);

				if (projectionSize > 0.0f) {
					velocity += normalToNormal * projectionSize * (1.0f + collision->bounciness);
				}
			}
			// ----------------------------------------
		}
	}


	glm::vec3 CollisionSystem::surfaceFromCollision(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
		glm::vec3 distance(0.0f);
		TransformComponent* transform = e->getComponent<TransformComponent>();

		const size_t count = collisions.size();
		for (size_t i = 0; i < count; i++) {
			const Octree::CollisionInfo& collisionInfo_i = collisions[i];
			float depth;
			glm::vec3 axis;

			if (Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), &axis, &depth)) {
				transform->translate(axis * depth);
				boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
				distance += axis * depth;
			}
		}

		//transform->translate(distance);


		return distance;
	}

	void CollisionSystem::updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		collision->manifolds.clear();

		std::vector<glm::vec3> manifolds;

		const size_t count = collisions.size();
		for (size_t i = 0; i < count; i++) {
			const Octree::CollisionInfo& collisionInfo_i = collisions[i];
			manifolds.clear();
			Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), manifolds);
			collision->manifolds.insert(collision->manifolds.end(), manifolds.begin(), manifolds.end());
		}

		// Remove duplicate manifolds
		for (size_t i = 0; i < collision->manifolds.size(); i++) {
			for (size_t j = i + 1; j < collision->manifolds.size(); j++) {
				if (glm::length2(collision->manifolds[i] - collision->manifolds[j]) < 0.001f) {
					collision->manifolds.erase(collision->manifolds.begin() + j);
					j--;
				}
			}
		}
	}
}