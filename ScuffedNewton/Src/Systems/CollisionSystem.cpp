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
		m_collisionOrder.clear();

		// 1. Handle resting contacts
		for (auto& e : entities) {
			PhysicalBodyComponent* physObj = e->getComponent<PhysicalBodyComponent>();
			for (size_t i = 0; i < physObj->restingContacts.size(); i++) {
				if (Intersection::isColliding(physObj->restingContacts[i])) {
					handleCollisions(physObj->restingContacts[i]);
				}
			}
		}

		// ----2. Find all upcoming collisions, save time until collision and the objects colliding in a list----
		std::vector<Intersection::CollisionTimeInfo> collisions;
		for (auto& e : entities) {
			collisions.clear();

			if (!e->getComponent<PhysicalBodyComponent>()->isConstraint) { // Don't check for constraints since these will be collided with anyway
				// Find and save upcoming collision time
				m_octree->getNextContinousCollision(e, dt, collisions);

				// Split collision info so that every colliding triangle pair etc have their own collision info
				if (collisions.size() > 0 && collisions[0].time <= dt) {
					for (size_t i = 0; i < collisions.size(); i++) {
						// TODO: add something to avoid duplicates
						if (collisions[i].triangleIndices.size() > 0) {
							for (size_t j = 0; j < collisions[i].triangleIndices.size(); j++) {
								m_collisionOrder.emplace_back(collisions[i]);
								std::pair<int, int> tempIndices = m_collisionOrder.back().triangleIndices[j];
								m_collisionOrder.back().triangleIndices.clear();
								m_collisionOrder.back().triangleIndices.emplace_back(tempIndices);
							}
						}
						else {
							m_collisionOrder.emplace_back(collisions[i]);
						}
					}
				}
			}
		}
		// ------------------------------------------------------------------------------------------------------

		float timeProcessed = 0.f;
		// 3. Sort the list by time
		std::sort(m_collisionOrder.begin(), m_collisionOrder.end());

		while (m_collisionOrder.size() > 0 && m_collisionOrder[0].time <= dt) {
			// 4. Advance all objects until collision
			for (auto& e : entities) {
				Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				PhysicalBodyComponent* phyicalComp = e->getComponent<PhysicalBodyComponent>();
				TransformComponent* transform = e->getComponent<TransformComponent>();

				transform->translate(phyicalComp->velocity * (m_collisionOrder[0].time - timeProcessed));
				boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
			}

			timeProcessed = m_collisionOrder[0].time;

			std::vector<Entity*> collidingEntities;
			// 5. Handle collision of all objects colliding at this time
			for (size_t i = 0; i < m_collisionOrder.size() && m_collisionOrder[i].time == timeProcessed; i++) {
				handleCollisions(m_collisionOrder[i]);

				// Save entities that are colliding currently (Only one time per entity)
				if (std::find(collidingEntities.begin(), collidingEntities.end(), m_collisionOrder[i].entity1) == collidingEntities.end()) {
					collidingEntities.emplace_back(m_collisionOrder[i].entity1);
				}
				if (std::find(collidingEntities.begin(), collidingEntities.end(), m_collisionOrder[i].entity2) == collidingEntities.end()) {
					collidingEntities.emplace_back(m_collisionOrder[i].entity2);
				}
			}

			// 6. Remove all collisions for these objects in the list
			for (size_t i = 0; i < m_collisionOrder.size(); i++) {
				for (size_t j = 0; j < collidingEntities.size(); j++) {
					if (m_collisionOrder[i].entity1 == collidingEntities[j] || m_collisionOrder[i].entity2 == collidingEntities[j]) {
						m_collisionOrder.erase(m_collisionOrder.begin() + i);
						i--;
						j = collidingEntities.size();
					}
				}
			}

			// 7. Recheck next collision for these objects and add to the list. TODO: Update the entities in the octree with the new velocities
			for (size_t i = 0; i < collidingEntities.size(); i++) {
				collisions.clear();

				// Find and save upcoming collision time
				m_octree->getNextContinousCollision(collidingEntities[i], dt, collisions);

				// Split collision info so that every colliding triangle pair etc have their own collision info
				if (collisions.size() > 0 && collisions[0].time + timeProcessed <= dt) {
					for (size_t j = 0; j < collisions.size(); j++) {
						collisions[j].time += timeProcessed;
						// TODO: add something to avoid duplicates
						if (collisions[j].triangleIndices.size() > 0) {
							for (size_t k = 0; k < collisions[j].triangleIndices.size(); k++) {
								m_collisionOrder.emplace_back(collisions[j]);
								std::pair<int, int> tempIndices = m_collisionOrder.back().triangleIndices[k];
								m_collisionOrder.back().triangleIndices.clear();
								m_collisionOrder.back().triangleIndices.emplace_back(tempIndices);
							}
						}
						else {
							m_collisionOrder.emplace_back(collisions[j]);
						}
					}
				}
			}

			std::sort(m_collisionOrder.begin(), m_collisionOrder.end());
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

	void CollisionSystem::handleCollisions(Intersection::CollisionTimeInfo& collisionInfo, int recursionDepth) {

		if (collisionInfo.entity1 == collisionInfo.entity2) { // Don't allow entity to collide with itself
			return;
		}

		PhysicalBodyComponent* physObj1 = collisionInfo.entity1->getComponent<PhysicalBodyComponent>();
		PhysicalBodyComponent* physObj2 = collisionInfo.entity2->getComponent<PhysicalBodyComponent>();

		// Assume these entities are colliding
		// Get intersection axis
		glm::vec3 intersectionAxis = glm::normalize(Intersection::getIntersectionAxis(collisionInfo));
		// Check that the entities are moving towards each other before going into collision response
		if (Intersection::dot(physObj2->velocity - physObj1->velocity, intersectionAxis) >= 0.0) {

			// Do collision response, effecting both entities. TODO: Add the possibility to have one object not effected (constraints)
			float v1Dot = Intersection::dot(physObj1->velocity, intersectionAxis);
			float v2Dot = Intersection::dot(physObj2->velocity, intersectionAxis);

			float collisionCoefficient = std::min(physObj1->collisionCoefficient, physObj2->collisionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			glm::vec3 eN = glm::cross(glm::cross(physObj1->velocity - physObj2->velocity, intersectionAxis), intersectionAxis);

			if (glm::length2(eN) > 0.00000001f) {
				eN = glm::normalize(eN);
			}

			float frictionCoefficient = std::min(physObj1->frictionCoefficient, physObj2->frictionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			if (!physObj1->isConstraint && !physObj2->isConstraint) {
				float u1Dot = ((physObj1->mass - collisionCoefficient * physObj2->mass) / (physObj1->mass + physObj2->mass)) * v1Dot + ((1.0f + collisionCoefficient) * physObj2->mass) / (physObj1->mass + physObj2->mass) * v2Dot;
				float u2Dot = ((physObj2->mass - collisionCoefficient * physObj1->mass) / (physObj2->mass + physObj1->mass)) * v2Dot + ((1.0f + collisionCoefficient) * physObj1->mass) / (physObj2->mass + physObj1->mass) * v1Dot;

				physObj1->velocity += (u1Dot - v1Dot) * (intersectionAxis + frictionCoefficient * eN);
				physObj2->velocity += (u2Dot - v2Dot) * (intersectionAxis + frictionCoefficient * eN);
			}
			else if (physObj1->isConstraint) {
				physObj2->velocity -= (v2Dot - v1Dot) * (1.0f + collisionCoefficient) * (intersectionAxis + frictionCoefficient * eN);
			}
			else if (physObj2->isConstraint) {
				physObj1->velocity -= (v1Dot - v2Dot) * (1.0f + collisionCoefficient) * (intersectionAxis + frictionCoefficient * eN);
			}
			else {
				assert(false); // This should not happen since the octree should not detect two constraints colliding
			}

			// Evaluate resting contacts
			if (recursionDepth < 3) {
				for (size_t i = 0; i < physObj1->restingContacts.size(); i++) {
					if (physObj1->restingContacts[i].entity1 != collisionInfo.entity2 && 
						physObj1->restingContacts[i].entity2 != collisionInfo.entity2 && 
						Intersection::isColliding(physObj1->restingContacts[i])) {
						handleCollisions(physObj1->restingContacts[i], recursionDepth + 1); // Recursively call all (other) resting contacts
					}
				}

				for (size_t i = 0; i < physObj2->restingContacts.size(); i++) {
					if (physObj2->restingContacts[i].entity1 != collisionInfo.entity1 && 
						physObj2->restingContacts[i].entity2 != collisionInfo.entity1 && 
						Intersection::isColliding(physObj2->restingContacts[i])) {
						handleCollisions(physObj2->restingContacts[i], recursionDepth + 1); // Recursively call all (other) resting contacts
					}
				}
			}
		}

		// ----Save resting contacts----
		int found1 = -1;
		int found2 = -1;

		for (size_t i = 0; i < physObj1->restingContacts.size(); i++) {
			if (physObj1->restingContacts[i].entity1 == collisionInfo.entity2) {
				if (physObj1->restingContacts[i].triangleIndices.size() == 0 && collisionInfo.triangleIndices.size() == 0) {
					found1 = i;
					break;
				}
				else if (physObj1->restingContacts[i].triangleIndices.size() > 0 && collisionInfo.triangleIndices.size() > 0) {
					if (physObj1->restingContacts[i].triangleIndices[0].first == collisionInfo.triangleIndices[0].second && physObj1->restingContacts[i].triangleIndices[0].second == physObj1->restingContacts[i].triangleIndices[0].first) {
						found1 = i;
						break;
					}
				}
			}
			else if (physObj1->restingContacts[i].entity2 == collisionInfo.entity2) {
				if (physObj1->restingContacts[i].triangleIndices.size() == 0 && collisionInfo.triangleIndices.size() == 0) {
					found1 = i;
					break;
				}
				else if (physObj1->restingContacts[i].triangleIndices.size() > 0 && collisionInfo.triangleIndices.size() > 0) {
					if (physObj1->restingContacts[i].triangleIndices[0].second == collisionInfo.triangleIndices[0].second && physObj1->restingContacts[i].triangleIndices[0].first == physObj1->restingContacts[i].triangleIndices[0].first) {
						found1 = i;
						break;
					}
				}
			}
		}

		for (size_t i = 0; i < physObj2->restingContacts.size(); i++) {
			if (physObj2->restingContacts[i].entity1 == collisionInfo.entity1) {
				if (physObj2->restingContacts[i].triangleIndices.size() == 0 && collisionInfo.triangleIndices.size() == 0) {
					found2 = i;
					break;
				}
				else if (physObj2->restingContacts[i].triangleIndices.size() > 0 && collisionInfo.triangleIndices.size() > 0) {
					if (physObj2->restingContacts[i].triangleIndices[0].first == collisionInfo.triangleIndices[0].first && physObj2->restingContacts[i].triangleIndices[0].second == physObj2->restingContacts[i].triangleIndices[0].second) {
						found2 = i;
						break;
					}
				}
			}
			else if (physObj2->restingContacts[i].entity2 == collisionInfo.entity1) {
				if (physObj2->restingContacts[i].triangleIndices.size() == 0 && collisionInfo.triangleIndices.size() == 0) {
					found2 = i;
					break;
				}
				else if (physObj2->restingContacts[i].triangleIndices.size() > 0 && collisionInfo.triangleIndices.size() > 0) {
					if (physObj2->restingContacts[i].triangleIndices[0].second == collisionInfo.triangleIndices[0].first && physObj2->restingContacts[i].triangleIndices[0].first == physObj2->restingContacts[i].triangleIndices[0].second) {
						found2 = i;
						break;
					}
				}
			}
		}

		float dotProd1 = Intersection::dot(physObj1->velocity, intersectionAxis);
		float dotProd2 = Intersection::dot(physObj2->velocity, intersectionAxis);

		if (std::abs(dotProd1 - dotProd2) < 0.1f) { // The two entities are staying close to each other.
			// "Stick" the entities to each other, i.e make their velocity along the intersection axis the same (as the heaviest one along the intersection axis)
			if (physObj1->isConstraint || (physObj1->mass > physObj2->mass)) {
				physObj2->velocity += (dotProd1 - dotProd2) * intersectionAxis;
			}
			else if (physObj2->isConstraint || (physObj1->mass < physObj2->mass)) {
				physObj1->velocity += (dotProd1 - dotProd2) * intersectionAxis;
			}
			else {
				// Same mass, use the slowest speed
				if (std::abs(dotProd1) < std::abs(dotProd2)) {
					physObj2->velocity += (dotProd1 - dotProd2) * intersectionAxis;
				}
				else {
					physObj1->velocity += (dotProd1 - dotProd2) * intersectionAxis;
				}
			}

			// Add this as a resting contact for both entities
			if (found1 == -1) {
				physObj1->restingContacts.emplace_back(collisionInfo);
			}

			if (found2 == -1) {
				physObj2->restingContacts.emplace_back(collisionInfo);
			}
		}
		else if (recursionDepth == 0) {
			// Remove from resting
			if (found1 >= 0) {
				physObj1->restingContacts.erase(physObj1->restingContacts.begin() + found1);
			}

			if (found2 >= 0) {
				physObj2->restingContacts.erase(physObj2->restingContacts.begin() + found2);
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