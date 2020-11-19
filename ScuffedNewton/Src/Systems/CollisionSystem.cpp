#include "../pch.h"

#include "CollisionSystem.h"

#include <memory>
#include <algorithm>
#include <map>

#include "../Components/Components.h"
#include "../DataTypes/Entity.h"
#include "../Calculations/Intersection.h"
#include "../DataStructures/Octree.h"
#include "../Shapes/Box.h"

namespace Scuffed {

	CollisionSystem::CollisionSystem() {
		requiredComponents["BoundingBoxComponent"] = true;
		requiredComponents["PhysicalBodyComponent"] = true;
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
		std::vector<UpcomingCollision> collisionOrder;

		// ----1. Find all upcoming collisions, save time until collision and the objects colliding in a list----
		float upcomingTime;
		std::vector<Octree::CollisionInfo> collisionInfo;
		for (auto& e : entities) {
			collisionInfo.clear();

			// Find and save upcoming collision time
			upcomingTime = getNextCollisionTime(e, collisionInfo, INFINITY);

			if (upcomingTime < dt) {
				for (size_t i = 0; i < collisionInfo.size(); i++) {
					collisionOrder.emplace_back();
					collisionOrder.back().collisionTime = upcomingTime;
					collisionOrder.back().entity = e;
					collisionOrder.back().collisionInfo = collisionInfo[i];
				}
			}
		}
		// ------------------------------------------------------------------------------------------------------

		float timeProcessed = 0.f;
		// 2. Sort the list by time
		std::sort(collisionOrder.begin(), collisionOrder.end());

		while (collisionOrder.size() > 0 && collisionOrder[0].collisionTime <= dt) {
			// 3. Advance all objects until collision
			for (auto& e : entities) {
				Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				PhysicalBodyComponent* phyicalComp = e->getComponent<PhysicalBodyComponent>();
				TransformComponent* transform = e->getComponent<TransformComponent>();

				transform->translate(phyicalComp->velocity * (collisionOrder[0].collisionTime - timeProcessed));
				boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
			}

			timeProcessed = collisionOrder[0].collisionTime;

			// 4. Handle collision of the two objects
			handleCollisions(collisionOrder[0].entity, collisionOrder[0].collisionInfo, 0.0f);

			// 5. Remove all collisions for these objects in the list
			Entity* entities[2];
			entities[0] = collisionOrder[0].entity;
			entities[1] = collisionOrder[0].collisionInfo.entity;

			collisionOrder.erase(std::remove_if(collisionOrder.begin(),
				collisionOrder.end(),
				[&](UpcomingCollision x) {return x.entity == entities[0] || x.entity == entities[1] || x.collisionInfo.entity == entities[0] || x.collisionInfo.entity == entities[1]; }),
				collisionOrder.end());

			// 6. Recheck next collision for these objects and add to the list
			for (int i = 0; i < 2; i++) {
				collisionInfo.clear();

				// Find and save upcoming collision time
				float upcomingTime = getNextCollisionTime(entities[i], collisionInfo, dt) + timeProcessed;

				if (upcomingTime <= dt && upcomingTime > timeProcessed) { // TODO: Solve multiple collisions happening the same frame (upcomingTime > timeProcessed should not be needed)
					for (size_t i = 0; i < collisionInfo.size(); i++) {
						collisionOrder.emplace_back();
						collisionOrder.back().collisionTime = upcomingTime;
						collisionOrder.back().entity = entities[i];
						collisionOrder.back().collisionInfo = collisionInfo[i];
					}
				}
			}

			std::sort(collisionOrder.begin(), collisionOrder.end());
			// 7. Repeat 2-6 until no time or no collisions remain
		}

		// 8. Advance all objects if dt has not been reached
		for (auto& e : entities) {
			Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
			PhysicalBodyComponent* phyicalComp = e->getComponent<PhysicalBodyComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();

			transform->translate(phyicalComp->velocity * (dt - timeProcessed));
			boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
		}


		/*for (auto& e : entities) {
			CollisionComponent* collision = e->getComponent<CollisionComponent>();

			surfaceFromCollision(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);

			updateManifolds(e, e->getComponent<BoundingBoxComponent>()->getBoundingBox(), collision->collisions);
		}*/
	}

	float CollisionSystem::getNextCollisionTime(Entity* e, std::vector<Octree::CollisionInfo>& collidingWith, const float dt) {
		float time = INFINITY;

		m_octree->getNextContinousCollision(e, collidingWith, time, dt);

		return time;
	}

	void CollisionSystem::handleCollisions(Entity* e, Octree::CollisionInfo& collision, const float dt) {
		Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

		PhysicalBodyComponent* physObj1 = e->getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* physObj2 = collision.entity->getComponent<PhysicalBodyComponent>();

		// Assume these entities are colliding
		// Get intersection axis
		collision.intersectionAxis = glm::normalize(Intersection::getIntersectionAxis(boundingBox, collision.shape.get()));
		// Check that the entities are moving towards each other before going into collision response
		if (Intersection::dot(physObj2->velocity - physObj1->velocity, collision.intersectionAxis) > 0.0) {

			// Do collision response, effecting both entities. TODO: Add the possibility to have one object not effected
			float v1Dot = Intersection::dot(physObj1->velocity, collision.intersectionAxis);
			float v2Dot = Intersection::dot(physObj2->velocity, collision.intersectionAxis);

			float collisionCoefficient = std::min(physObj1->collisionCoefficient, physObj2->collisionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			float u1Dot = ((physObj1->weight - collisionCoefficient * physObj2->weight) / (physObj1->weight + physObj2->weight)) * v1Dot + ((1.0f + collisionCoefficient) * physObj2->weight) / (physObj1->weight + physObj2->weight) * v2Dot;
			float u2Dot = ((physObj2->weight - collisionCoefficient * physObj1->weight) / (physObj2->weight + physObj1->weight)) * v2Dot + ((1.0f + collisionCoefficient) * physObj1->weight) / (physObj2->weight + physObj1->weight) * v1Dot;

			glm::vec3 eN = glm::cross(glm::cross(physObj1->velocity - physObj2->velocity, collision.intersectionAxis), collision.intersectionAxis);

			if (glm::length2(eN) > 0.00000001) {
				eN = glm::normalize(eN);
			}

			float frictionCoefficient = std::min(physObj1->frictionCoefficient, physObj2->frictionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			physObj1->velocity += (u1Dot - v1Dot) * (collision.intersectionAxis + frictionCoefficient * eN);
			physObj2->velocity += (u2Dot - v2Dot) * (collision.intersectionAxis + frictionCoefficient * eN);
		}
	}

	glm::vec3 CollisionSystem::surfaceFromCollision(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
		assert(false); // Not implemented
		glm::vec3 distance(0.0f);
		//TransformComponent* transform = e->getComponent<TransformComponent>();

		//const size_t count = collisions.size();
		//for (size_t i = 0; i < count; i++) {
		//	const Octree::CollisionInfo& collisionInfo_i = collisions[i];
		//	float depth;
		//	glm::vec3 axis;

		//	if (Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), &axis, &depth)) {
		//		transform->translate(axis * depth);
		//		boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
		//		distance += axis * depth;
		//	}
		//}

		////transform->translate(distance);


		return distance;
	}

	void CollisionSystem::updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
		assert(false); // Not implemented
		//CollisionComponent* collision = e->getComponent<CollisionComponent>();
		//collision->manifolds.clear();

		//std::vector<glm::vec3> manifolds;

		//const size_t count = collisions.size();
		//for (size_t i = 0; i < count; i++) {
		//	const Octree::CollisionInfo& collisionInfo_i = collisions[i];
		//	manifolds.clear();
		//	Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), manifolds);
		//	collision->manifolds.insert(collision->manifolds.end(), manifolds.begin(), manifolds.end());
		//}

		//// Remove duplicate manifolds
		//for (size_t i = 0; i < collision->manifolds.size(); i++) {
		//	for (size_t j = i + 1; j < collision->manifolds.size(); j++) {
		//		if (glm::length2(collision->manifolds[i] - collision->manifolds[j]) < 0.001f) {
		//			collision->manifolds.erase(collision->manifolds.begin() + j);
		//			j--;
		//		}
		//	}
		//}
	}
}