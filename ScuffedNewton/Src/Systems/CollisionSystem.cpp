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

		// 1. Handle resting contacts
		for (auto& e : entities) {
			PhysicalBodyComponent* physObj = e->getComponent<PhysicalBodyComponent>();
			for (size_t i = 0; i < physObj->restingContacts.size(); i++) {
				if (Intersection::SAT(e->getComponent<BoundingBoxComponent>()->getBoundingBox(), physObj->restingContacts[i]->getComponent<BoundingBoxComponent>()->getBoundingBox())) {
					handleCollisions(e, physObj->restingContacts[i], 0);
				}
			}
		}

		// ----2. Find all upcoming collisions, save time until collision and the objects colliding in a list----
		float upcomingTime;
		std::vector<Entity*> hitEntities;
		for (auto& e : entities) {
			hitEntities.clear();

			// Find and save upcoming collision time
			upcomingTime = getNextCollisionTime(e, hitEntities, dt);

			if (upcomingTime <= dt) {
				for (size_t i = 0; i < hitEntities.size(); i++) {
					collisionOrder.emplace_back();
					collisionOrder.back().collisionTime = upcomingTime;
					collisionOrder.back().entity1 = e;
					collisionOrder.back().entity2 = hitEntities[i];
				}
			}
		}
		// ------------------------------------------------------------------------------------------------------

		float timeProcessed = 0.f;
		// 3. Sort the list by time
		std::sort(collisionOrder.begin(), collisionOrder.end());

		while (collisionOrder.size() > 0 && collisionOrder[0].collisionTime <= dt) {
			// 4. Advance all objects until collision
			for (auto& e : entities) {
				Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				PhysicalBodyComponent* phyicalComp = e->getComponent<PhysicalBodyComponent>();
				TransformComponent* transform = e->getComponent<TransformComponent>();

				transform->translate(phyicalComp->velocity * (collisionOrder[0].collisionTime - timeProcessed));
				boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
			}

			timeProcessed = collisionOrder[0].collisionTime;

			std::vector<Entity*> entities;
			// 5. Handle collision of all objects colliding at this time
			for (size_t i = 0; i < collisionOrder.size() && collisionOrder[i].collisionTime == timeProcessed; i++) {
				handleCollisions(collisionOrder[i].entity1, collisionOrder[i].entity2);

				// Save entities that are colliding currently (Only one time per entity)
				if (std::find(entities.begin(), entities.end(), collisionOrder[i].entity1) == entities.end()) {
					entities.emplace_back(collisionOrder[i].entity1);
				}
				if (std::find(entities.begin(), entities.end(), collisionOrder[i].entity2) == entities.end()) {
					entities.emplace_back(collisionOrder[i].entity2);
				}
			}

			// 6. Remove all collisions for these objects in the list
			collisionOrder.erase(std::remove_if(collisionOrder.begin(),
				collisionOrder.end(),
				[&](UpcomingCollision x) {
					for (size_t i = 0; i < entities.size(); i++) {
						if (x.entity1 == entities[i] || x.entity2 == entities[i]) {
							return true;
						}
					}
					return false;
				}),
				collisionOrder.end());

			// 7. Recheck next collision for these objects and add to the list. TODO: Update the entities in the octree with the new velocities
			for (size_t i = 0; i < entities.size(); i++) {
				hitEntities.clear();

				// Find and save upcoming collision time
				float upcomingTime = getNextCollisionTime(entities[i], hitEntities, dt) + timeProcessed;

				if (upcomingTime <= dt) { // TODO: Solve multiple collisions happening at the same time (upcomingTime > timeProcessed should not be needed)
					for (size_t i = 0; i < hitEntities.size(); i++) {
						collisionOrder.emplace_back();
						collisionOrder.back().collisionTime = upcomingTime;
						collisionOrder.back().entity1 = entities[i];
						collisionOrder.back().entity2 = hitEntities[i];
					}
				}
			}

			std::sort(collisionOrder.begin(), collisionOrder.end());
			// 8. Repeat 3-7 until no time or no collisions remain
		}

		// 9. Advance all objects if dt has not been reached
		for (auto& e : entities) {
			Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
			PhysicalBodyComponent* phyicalComp = e->getComponent<PhysicalBodyComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();

			transform->translate(phyicalComp->velocity * (dt - timeProcessed));
			boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
		}
	}

	float CollisionSystem::getNextCollisionTime(Entity* e, std::vector<Entity*>& collidingWith, const float dt) {
		float time = INFINITY;

		m_octree->getNextContinousCollision(e, collidingWith, time, dt);

		return time;
	}

	void CollisionSystem::handleCollisions(Entity* e1, Entity* e2, int recursionDepth) {
		Box* boundingBox1 = e1->getComponent<BoundingBoxComponent>()->getBoundingBox();
		Box* boundingBox2 = e2->getComponent<BoundingBoxComponent>()->getBoundingBox();

		PhysicalBodyComponent* physObj1 = e1->getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* physObj2 = e2->getComponent<PhysicalBodyComponent>();

		// Assume these entities are colliding
		// Get intersection axis
		glm::vec3 intersectionAxis = glm::normalize(Intersection::getIntersectionAxis(boundingBox1, boundingBox2));
		// Check that the entities are moving towards each other before going into collision response
		if (Intersection::dot(physObj2->velocity - physObj1->velocity, intersectionAxis) >= 0.0) {

			// Do collision response, effecting both entities. TODO: Add the possibility to have one object not effected (constraints)
			float v1Dot = Intersection::dot(physObj1->velocity, intersectionAxis);
			float v2Dot = Intersection::dot(physObj2->velocity, intersectionAxis);

			float collisionCoefficient = std::min(physObj1->collisionCoefficient, physObj2->collisionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			float u1Dot = ((physObj1->mass - collisionCoefficient * physObj2->mass) / (physObj1->mass + physObj2->mass)) * v1Dot + ((1.0f + collisionCoefficient) * physObj2->mass) / (physObj1->mass + physObj2->mass) * v2Dot;
			float u2Dot = ((physObj2->mass - collisionCoefficient * physObj1->mass) / (physObj2->mass + physObj1->mass)) * v2Dot + ((1.0f + collisionCoefficient) * physObj1->mass) / (physObj2->mass + physObj1->mass) * v1Dot;

			glm::vec3 eN = glm::cross(glm::cross(physObj1->velocity - physObj2->velocity, intersectionAxis), intersectionAxis);

			if (glm::length2(eN) > 0.00000001f) {
				eN = glm::normalize(eN);
			}

			float frictionCoefficient = std::min(physObj1->frictionCoefficient, physObj2->frictionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			physObj1->velocity += (u1Dot - v1Dot) * (intersectionAxis + frictionCoefficient * eN);
			physObj2->velocity += (u2Dot - v2Dot) * (intersectionAxis + frictionCoefficient * eN);

			// Evaluate resting contacts
			if (recursionDepth < 10) {
				for (size_t i = 0; i < physObj1->restingContacts.size(); i++) {
					if (physObj1->restingContacts[i] != e2 && Intersection::SAT(e1->getComponent<BoundingBoxComponent>()->getBoundingBox(), physObj1->restingContacts[i]->getComponent<BoundingBoxComponent>()->getBoundingBox())) {
						handleCollisions(e1, physObj1->restingContacts[i], recursionDepth + 1); // Recursively call all (other) resting contacts
					}
				}

				for (size_t i = 0; i < physObj2->restingContacts.size(); i++) {
					if (physObj2->restingContacts[i] != e1 && Intersection::SAT(e2->getComponent<BoundingBoxComponent>()->getBoundingBox(), physObj2->restingContacts[i]->getComponent<BoundingBoxComponent>()->getBoundingBox())) {
						handleCollisions(e2, physObj2->restingContacts[i], recursionDepth + 1); // Recursively call all (other) resting contacts
					}
				}
			}

		}

		// ----Save resting contacts----
		auto found1 = std::find(physObj1->restingContacts.begin(), physObj1->restingContacts.end(), e2);
		auto found2 = std::find(physObj2->restingContacts.begin(), physObj2->restingContacts.end(), e1);

		float dotProd1 = Intersection::dot(physObj1->velocity, intersectionAxis);
		float dotProd2 = Intersection::dot(physObj2->velocity, intersectionAxis);

		if (std::abs(dotProd1 - dotProd2) < 0.1f) { // The two entities are staying close to each other.
			// "Stick" the entities to each other, i.e make their velocity along the intersection axis the same (as the slowest one along the intersection axis)
			if (std::abs(dotProd1) < std::abs(dotProd2)) {
				physObj2->velocity += (dotProd1 - dotProd2) * intersectionAxis;
			}
			else {
				physObj1->velocity += (dotProd1 - dotProd2) * intersectionAxis;
			}

			// Add this as a resting contact for both entities
			if (found1 == physObj1->restingContacts.end()) {
				physObj1->restingContacts.emplace_back(e2);
			}

			if (found2 == physObj2->restingContacts.end()) {
				physObj2->restingContacts.emplace_back(e1);
			}
		}
		else if (recursionDepth == 0) {
			// Remove from resting
			if (found1 != physObj1->restingContacts.end()) {
				physObj1->restingContacts.erase(found1);
			}

			if (found2 != physObj2->restingContacts.end()) {
				physObj2->restingContacts.erase(found2);
			}
		}
		// -----------------------------
	}

	//void CollisionSystem::updateManifolds(Entity* e, Box* boundingBox, std::vector<Octree::CollisionInfo>& collisions) {
	//	CollisionComponent* collision = e->getComponent<CollisionComponent>();
	//	collision->manifolds.clear();

	//	std::vector<glm::vec3> manifolds;

	//	const size_t count = collisions.size();
	//	for (size_t i = 0; i < count; i++) {
	//		const Octree::CollisionInfo& collisionInfo_i = collisions[i];
	//		manifolds.clear();
	//		Intersection::SAT(boundingBox, collisionInfo_i.shape.get(), manifolds);
	//		collision->manifolds.insert(collision->manifolds.end(), manifolds.begin(), manifolds.end());
	//	}

	//	// Remove duplicate manifolds
	//	for (size_t i = 0; i < collision->manifolds.size(); i++) {
	//		for (size_t j = i + 1; j < collision->manifolds.size(); j++) {
	//			if (glm::length2(collision->manifolds[i] - collision->manifolds[j]) < 0.001f) {
	//				collision->manifolds.erase(collision->manifolds.begin() + j);
	//				j--;
	//			}
	//		}
	//	}
	//}
}