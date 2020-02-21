#include "../pch.h"

#include "CollisionSystem.h"

#include "../Components/Components.h"

#include "../DataStructures/Octree.h"
#include "../DataTypes/Entity.h"
#include "../DataTypes/BoundingBox.h"

#include "../Calculations/Intersection.h"

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
		// prepare matrixes and bounding boxes
		for (auto e : entities) {
			e->getComponent<BoundingBoxComponent>()->getBoundingBox()->prepareCorners();
		}

		// ======================== Collision Update ======================================

		for (size_t i = 0; i < entities.size(); ++i) {
			Entity* e = entities[i];

			CollisionComponent* collision = e->getComponent<CollisionComponent>();

			for (auto it : collision->collisions) {
				delete it.shape;
			}

			collision->collisions.clear();

			collisionUpdate(e, dt);
		}

		// ======================== Surface from collisions ======================================

		for (size_t i = 0; i < entities.size(); ++i) {
			Entity* e = entities[i];

			CollisionComponent* collision = e->getComponent<CollisionComponent>();

			if (m_octree) {
				surfaceFromCollision(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);
			}
		}
	}

	void CollisionSystem::collisionUpdate(Entity* e, const float dt) {
		// Update collision data
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		m_octree->getCollisions(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), &collision->collisions, collision->doSimpleCollisions);
		handleCollisions(e, collision->collisions, dt);
	}

	const bool CollisionSystem::handleCollisions(Entity* e, std::vector<Octree::CollisionInfo>& collisions, const float dt) {
		MovementComponent* movement = e->getComponent<MovementComponent>();
		CollisionComponent* collision = e->getComponent<CollisionComponent>();
		BoundingBox* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

		bool collisionFound = false;
		collision->onGround = false;

		const size_t collisionCount = collisions.size();

		if (collisionCount > 0) {
			std::vector<int> groundIndices;
			glm::vec3 sumVec(0.0f);

			// Gather info
			gatherCollisionInformation(e, boundingBox, collisions, sumVec, groundIndices, dt);

			if (collisions.size() > 0) {
				collisionFound = true;
			}

			if (groundIndices.size() > 0) {
				collision->onGround = true;
			}

			// Handle true collisions
			updateVelocityVec(e, movement->velocity, collisions, sumVec, groundIndices, dt);
		}

		return collisionFound;
	}

	void CollisionSystem::gatherCollisionInformation(Entity* e, BoundingBox* boundingBox, std::vector<Octree::CollisionInfo>& collisions, glm::vec3& sumVec, std::vector<int>& groundIndices, const float dt) {
		CollisionComponent* collision = e->getComponent<CollisionComponent>();

		size_t collisionCount = collisions.size();

		if (collisionCount > 0) {
			// Get the actual intersection axises
			for (size_t i = 0; i < collisionCount; i++) {
				Octree::CollisionInfo& collisionInfo_i = collisions[i];

				if (Intersection::SAT(collisionInfo_i.shape, boundingBox->getBox(), &collisionInfo_i.intersectionAxis, &collisionInfo_i.intersectionDepth)) {
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
					// False collision (should not happen)
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


	glm::vec3 CollisionSystem::surfaceFromCollision(Entity* e, BoundingBox* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
		glm::vec3 distance(0.0f);
		TransformComponent* transform = e->getComponent<TransformComponent>();

		const size_t count = collisions.size();
		for (size_t i = 0; i < count; i++) {
			const Octree::CollisionInfo& collisionInfo_i = collisions[i];
			float depth;
			glm::vec3 axis;

			if (Intersection::SAT(collisionInfo_i.shape, boundingBox->getBox(), &axis, &depth)) {
				boundingBox->setPosition(boundingBox->getPosition() + axis * (depth - 0.0001f));
				distance += axis * (depth - 0.0001f);
			}
		}

		transform->translate(distance);

		return distance;
	}

}