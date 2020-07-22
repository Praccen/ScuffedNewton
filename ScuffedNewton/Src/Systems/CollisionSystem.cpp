#include "../pch.h"

#include "CollisionSystem.h"

#include <memory>

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
		for (auto& e: entities) {
			CollisionComponent* collision = e->getComponent<CollisionComponent>();
			MovementComponent* movement = e->getComponent<MovementComponent>();
			
			collision->collisions.clear();

			// Continous collisions
			movement->updateableDt = dt;
			continousCollisionUpdate(e, movement->updateableDt);
			movement->oldVelocity = movement->velocity;

			// Handle friction
			handleCollisions(e, collision->collisions, dt);

			surfaceFromCollision(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);

			updateManifolds(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);
		}
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

		if (handleCollisions(e, zeroDistances, 0.f)) {
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
			gatherCollisionInformation(e, boundingBox, collisions, sumVec, groundIndices, dt);

			if (groundIndices.size() > 0) {
				collision->onGround = true;
			}

			glm::vec3 preVel = movement->velocity;

			// Handle true collisions
			updateVelocityVec(e, movement->velocity, collisions, sumVec, groundIndices, dt);

			if (glm::length2(movement->velocity) > 0.0f && Intersection::dot(glm::normalize(preVel), glm::normalize(movement->velocity)) < 1.f) {
				// Collisions effected movement
				return true;
			}
		}

		// Collisions did not effect movement
		return false;
	}

	void CollisionSystem::gatherCollisionInformation(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt) {
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

					// Save ground collisions
					if (collisionInfo_i.intersectionAxis.y > 0.7f) {
						bool newGround = true;
						for (size_t j = 0; j < groundIndices.size(); j++) {
							if (collisionInfo_i.intersectionAxis == collisions[groundIndices[j]].intersectionAxis) {
								newGround = false;
							}
						}
						if (newGround) {
							// Save collision for friction calculation
							groundIndices.push_back((int)i);
						}
					}
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

	void CollisionSystem::updateVelocityVec(Entity* e, glm::vec3& velocity, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		const size_t collisionCount = collisions.size();

		// Loop through collisions and handle them
		for (size_t i = 0; i < collisionCount; i++) {
			const Octree::CollisionInfo& collisionInfo_i = collisions[i];

			// ----Velocity changes from collisions----

			// Stop movement towards triangle
			float projectionSize = glm::dot(velocity, -collisionInfo_i.intersectionAxis);

			if (projectionSize > 0.0f) { //Is pushing against wall
				velocity += collisionInfo_i.intersectionAxis * (projectionSize * (1.0f + collision->bounciness)); // Limit movement towards wall
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

		// ----Drag----
		if (collision->onGround) { // Ground drag
			size_t nrOfGroundCollisions = groundIndices.size();
			for (size_t i = 0; i < nrOfGroundCollisions; i++) {
				const Octree::CollisionInfo& collisionInfo_ground_i = collisions[groundIndices[i]];
				const glm::vec3 velAlongPlane = velocity - collisionInfo_ground_i.intersectionAxis * glm::dot(collisionInfo_ground_i.intersectionAxis, velocity);
				const float sizeOfVel = glm::length(velAlongPlane);
				if (sizeOfVel > 0.0f) {
					const float slowdown = glm::min((collision->drag / nrOfGroundCollisions) * dt, sizeOfVel);
					velocity -= slowdown * glm::normalize(velAlongPlane);
				}
			}
		}
		// ------------
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