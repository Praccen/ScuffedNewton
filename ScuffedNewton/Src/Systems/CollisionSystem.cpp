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
		continousCollisions(dt);
	}

	void CollisionSystem::continousCollisions(float dt) {
		// ======================== Collision Update ======================================
		m_collisionOrder.clear();

		std::vector<Entity*> temp;
		// 1. Handle resting contacts
		for (auto& e : entities) {
			PhysicalBodyComponent* physObj = e->getComponent<PhysicalBodyComponent>();
			for (size_t i = 0; i < physObj->restingContacts.size(); i++) {
				if (Intersection::isColliding(physObj->restingContacts[i])) { // TODO: This fails sometimes
					handleCollisions(physObj->restingContacts[i], temp);
				}
				else {
					physObj->restingContacts.erase(physObj->restingContacts.begin() + i);
				}
			}
		}

		// ----2. Find all upcoming collisions, save time until collision and the objects colliding in a list----
		std::vector<Intersection::CollisionTimeInfo> collisions;
		for (auto& e : entities) {
			collisions.clear();
			PhysicalBodyComponent* physComp = e->getComponent<PhysicalBodyComponent>();

			if (!physComp->isConstraint && glm::length2(physComp->velocity) > Utils::instance()->epsilon) { // Don't check for constraints since these will be collided with anyway && only check if moving
				// Find and save upcoming collision time
				m_octree->getNextContinousCollision(e, dt, collisions);

				// Split collision info so that every colliding triangle pair etc have their own collision info
				for (size_t i = 0; i < collisions.size(); i++) {
					if (collisions[i].triangleIndices.size() > 0) {
						for (size_t j = 0; j < collisions[i].triangleIndices.size(); j++) {
							Scuffed::Intersection::CollisionTimeInfo temp;
							temp = collisions[i];
							std::pair<int, int> tempIndices = collisions[i].triangleIndices[j];
							temp.triangleIndices.clear();
							temp.triangleIndices.emplace_back(tempIndices);
							if (findMatchingCollision(m_collisionOrder, temp) == -1) { // Avoid duplicates
								m_collisionOrder.emplace_back(temp);
							}
						}
					}
					else {
						if (findMatchingCollision(m_collisionOrder, collisions[i]) == -1) { // Avoid duplicates
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
				PhysicalBodyComponent* physicalComp = e->getComponent<PhysicalBodyComponent>();
				moveObject(e, physicalComp->velocity, (m_collisionOrder[0].time - timeProcessed));
			}

			timeProcessed = m_collisionOrder[0].time;

			std::vector<Entity*> collidingEntities;
			glm::vec3 previousVelocities[2];
			// 5. Handle collision of all objects colliding at this time
			for (size_t i = 0; i < m_collisionOrder.size() && m_collisionOrder[i].time == timeProcessed; i++) {
				previousVelocities[0] = m_collisionOrder[i].entity1->getComponent<PhysicalBodyComponent>()->velocity;
				previousVelocities[1] = m_collisionOrder[i].entity2->getComponent<PhysicalBodyComponent>()->velocity;

				for (int j = 0; j < 2; j++) {
					if (glm::length2(previousVelocities[j]) > Utils::instance()->epsilon) {
						previousVelocities[j] = glm::normalize(previousVelocities[j]);
					}
				}

				// Move a bit longer to force detection of correct intersection axis
				moveObject(m_collisionOrder[i].entity1, previousVelocities[0], 0.001f);
				moveObject(m_collisionOrder[i].entity2, previousVelocities[1], 0.001f);

				handleCollisions(m_collisionOrder[i], collidingEntities);

				// Move back again
				moveObject(m_collisionOrder[i].entity1, previousVelocities[0], -0.001f);
				moveObject(m_collisionOrder[i].entity2, previousVelocities[1], -0.001f);
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

			// Update the entities in the octree with the new velocities
			m_octree->update(dt - timeProcessed);

			// 7. Recheck next collision for these objects and add to the list.
			for (size_t i = 0; i < collidingEntities.size(); i++) {
				PhysicalBodyComponent* physComp = collidingEntities[i]->getComponent<PhysicalBodyComponent>();
				if (!physComp->isConstraint && glm::length2(physComp->velocity) > Utils::instance()->epsilon) { // Don't check for constraints since these will be collided with anyway && only check if moving
					collisions.clear();

					// Find and save upcoming collision time
					m_octree->getNextContinousCollision(collidingEntities[i], (dt - timeProcessed), collisions);

					// Split collision info so that every colliding triangle pair etc have their own collision info
					for (size_t j = 0; j < collisions.size(); j++) {
						collisions[j].time += timeProcessed;
						if (collisions[j].triangleIndices.size() > 0) {
							for (size_t k = 0; k < collisions[j].triangleIndices.size(); k++) {
								Intersection::CollisionTimeInfo temp;
								temp = collisions[j];
								std::pair<int, int> tempIndices = collisions[j].triangleIndices[k];
								temp.triangleIndices.clear();
								temp.triangleIndices.emplace_back(tempIndices);
								if (findMatchingCollision(m_collisionOrder, temp) == -1) { // Avoid duplicates
									m_collisionOrder.emplace_back(temp);
								}
							}
						}
						else {
							if (findMatchingCollision(m_collisionOrder, collisions[j]) == -1) { // Avoid duplicates
								m_collisionOrder.emplace_back(collisions[j]);
							}
						}
					}
				}
			}

			std::sort(m_collisionOrder.begin(), m_collisionOrder.end());
			// 8. Repeat 3-7 until no time or no collisions remain
		}

		// 9. Advance all objects if dt has not been reached
		for (auto& e : entities) {
			PhysicalBodyComponent* physicalComp = e->getComponent<PhysicalBodyComponent>();
			moveObject(e, physicalComp->velocity, (dt - timeProcessed));
		}
	}

	void CollisionSystem::handleCollisions(Intersection::CollisionTimeInfo& collisionInfo, std::vector<Entity*>& collidingEntities) {

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
			// Do collision response, effecting both entities.

			glm::vec3 eN = glm::cross(glm::cross(physObj1->velocity - physObj2->velocity, intersectionAxis), intersectionAxis);

			if (glm::length2(eN) > Utils::instance()->epsilon) {
				eN = glm::normalize(eN);
			}

			float collisionCoefficient = std::min(physObj1->collisionCoefficient, physObj2->collisionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future
			float frictionCoefficient = std::max(physObj1->frictionCoefficient, physObj2->frictionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			if (!physObj1->isConstraint && !physObj2->isConstraint) {
				float v1Dot = Intersection::dot(physObj1->velocity, intersectionAxis);
				float v2Dot = Intersection::dot(physObj2->velocity, intersectionAxis);
				float tangentVel1 = Intersection::dot(physObj1->velocity, eN);
				float tangentVel2 = Intersection::dot(physObj2->velocity, eN);
				float u1Dot = ((physObj1->mass - collisionCoefficient * physObj2->mass) / (physObj1->mass + physObj2->mass)) * v1Dot + ((1.0f + collisionCoefficient) * physObj2->mass) / (physObj1->mass + physObj2->mass) * v2Dot;
				float u2Dot = ((physObj2->mass - collisionCoefficient * physObj1->mass) / (physObj2->mass + physObj1->mass)) * v2Dot + ((1.0f + collisionCoefficient) * physObj1->mass) / (physObj2->mass + physObj1->mass) * v1Dot;

				float frictionMagnitude1 = -1.0f * (((u1Dot - v1Dot) * frictionCoefficient) < 0.0f) * std::min(std::abs(tangentVel1), std::abs((u1Dot - v1Dot) * frictionCoefficient));
				float frictionMagnitude2 = -1.0f * (((u2Dot - v2Dot) * frictionCoefficient) < 0.0f) * std::min(std::abs(tangentVel2), std::abs((u2Dot - v2Dot) * frictionCoefficient));

				physObj1->velocity += (u1Dot - v1Dot) * (intersectionAxis) + frictionMagnitude1 * eN;
				physObj2->velocity += (u2Dot - v2Dot) * (intersectionAxis) + frictionMagnitude2 * eN;
			}
			else if (physObj1->isConstraint) {
				float v2Dot = Intersection::dot(physObj2->velocity - physObj1->velocity, intersectionAxis);
				float relativeTangentVel = Intersection::dot(physObj2->velocity - physObj1->velocity, eN);
				float frictionMagnitude = std::min(relativeTangentVel * frictionCoefficient, v2Dot * (1.0f + collisionCoefficient) + frictionCoefficient);
				//float uDot = v2Dot * (1.0f + collisionCoefficient) + frictionCoefficient;
				physObj2->velocity -= v2Dot * (1.0f + collisionCoefficient) * (intersectionAxis) + frictionMagnitude * eN;
			}
			else if (physObj2->isConstraint) {
				float v1Dot = Intersection::dot(physObj1->velocity - physObj2->velocity, intersectionAxis);
				float relativeTangentVel = Intersection::dot(physObj1->velocity - physObj2->velocity, eN);
				float frictionMagnitude = std::min(relativeTangentVel * frictionCoefficient, v1Dot * (1.0f + collisionCoefficient) + frictionCoefficient);
				//float uDot = v1Dot * (1.0f + collisionCoefficient) + frictionCoefficient;
				physObj1->velocity -= v1Dot * (1.0f + collisionCoefficient) * (intersectionAxis) + frictionMagnitude * eN;
			}
			else {
				std::cout << "Two constraints colliding were detected. Something is wrong. \n";
				assert(false); // This should not happen since the octree should not detect two constraints colliding
			}

			glm::vec3 velocityAfterInitialCollision1 = physObj1->velocity;
			glm::vec3 velocityAfterInitialCollision2 = physObj2->velocity;

			// Evaluate resting contacts
			if (!physObj1->isConstraint) {
				for (size_t i = 0; i < physObj1->restingContacts.size(); i++) {
					if (physObj1->restingContacts[i].entity1 != collisionInfo.entity2 &&
						physObj1->restingContacts[i].entity2 != collisionInfo.entity2) {
						handleRecursiveCollision(physObj1->restingContacts[i], collidingEntities, collisionInfo.entity1); // Recursively call all (other) resting contacts
					}
				}
			}

			if (!physObj2->isConstraint) {
				for (size_t i = 0; i < physObj2->restingContacts.size(); i++) {
					if (physObj2->restingContacts[i].entity1 != collisionInfo.entity1 &&
						physObj2->restingContacts[i].entity2 != collisionInfo.entity1) {
						handleRecursiveCollision(physObj2->restingContacts[i], collidingEntities, collisionInfo.entity2); // Recursively call all (other) resting contacts
					}
				}
			}


			if (std::find(collidingEntities.begin(), collidingEntities.end(), collisionInfo.entity1) == collidingEntities.end()) {
				collidingEntities.emplace_back(collisionInfo.entity1);
			}
			if (std::find(collidingEntities.begin(), collidingEntities.end(), collisionInfo.entity2) == collidingEntities.end()) {
				collidingEntities.emplace_back(collisionInfo.entity2);
			}

			// ----Handle resting contacts----
			float dotProd1 = Intersection::dot(physObj1->velocity, intersectionAxis);
			float dotProd2 = Intersection::dot(physObj2->velocity, intersectionAxis);

			if ((dotProd1 - dotProd2) < 0.1f) {
				bool addToRestingContacts = false;
				if (physObj1->isConstraint && !physObj2->isConstraint) {
					physObj2->velocity += (dotProd1 - dotProd2) * intersectionAxis;
					addToRestingContacts = true;
				}
				else if (physObj2->isConstraint && !physObj1->isConstraint) {
					physObj1->velocity += (dotProd1 - dotProd2) * intersectionAxis;
					addToRestingContacts = true;
				}
				else {
					if ((dotProd1 - dotProd2) >= 0.0f) { // The two entities are staying close to each other but moving slighlty apart
						// "Stick" the entities to each other, i.e make their velocity along the intersection axis the same (weighted average based on mass)
						float totMass = physObj1->mass + physObj2->mass;
						physObj1->velocity += (dotProd2 - dotProd1) * (physObj2->mass / totMass) * intersectionAxis;
						physObj2->velocity += (dotProd1 - dotProd2) * (physObj1->mass / totMass) * intersectionAxis;
						addToRestingContacts = true;
					}
					else {
						// The objects are moving into each other, don't let this happen

						physObj1->velocity = glm::vec3(0.0f);
						physObj2->velocity = glm::vec3(0.0f);
						addToRestingContacts = true;

						//bool obj1HasMeaningfullRestingCollision = glm::dot(velocityAfterInitialCollision1, physObj1->velocity) < 1.0f;
						//bool obj2HasMeaningfullRestingCollision = glm::dot(velocityAfterInitialCollision2, physObj2->velocity) < 1.0f;

						//if (obj1HasMeaningfullRestingCollision && !obj2HasMeaningfullRestingCollision) {
						//	// Make obj2 change velocity
						//	physObj2->velocity += (dotProd1 - dotProd2) * (1.0f + collisionCoefficient) * intersectionAxis;
						//}
						//else if (obj2HasMeaningfullRestingCollision && !obj1HasMeaningfullRestingCollision) {
						//	// Make obj1 change velocity
						//	physObj1->velocity += (dotProd2 - dotProd1) * (1.0f + collisionCoefficient) * intersectionAxis;
						//}
						//else if (!obj1HasMeaningfullRestingCollision && !obj2HasMeaningfullRestingCollision) {
						//	// None had meaningfull, must be sliding and stuff. Make both the average velocity
						//	physObj1->velocity += (dotProd2 - dotProd1) * 0.5f * intersectionAxis;
						//	physObj2->velocity += (dotProd1 - dotProd2) * 0.5f * intersectionAxis;

						//	addToRestingContacts = true;
						//}
						//else {
						//	// Both had meaningfull resting contacts, stop both completely
						//	
						//}

						
					}
				}				

				if (addToRestingContacts) {
					int found1 = findMatchingCollision(physObj1->restingContacts, collisionInfo);
					int found2 = findMatchingCollision(physObj2->restingContacts, collisionInfo);

					// Add this as a resting contact for both entities
					if (found1 == -1) {
						physObj1->restingContacts.emplace_back(collisionInfo);
					}

					if (found2 == -1) {
						physObj2->restingContacts.emplace_back(collisionInfo);
					}
				}
			}
			// -------------------------------
		}
	}

	void CollisionSystem::handleRecursiveCollision(Intersection::CollisionTimeInfo& collisionInfo, std::vector<Entity*>& collidingEntities, Entity* startingEntity) {

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
			// Do collision response, effecting both entities.

			glm::vec3 eN = glm::cross(glm::cross(physObj1->velocity - physObj2->velocity, intersectionAxis), intersectionAxis);

			if (glm::length2(eN) > Utils::instance()->epsilon) {
				eN = glm::normalize(eN);
			}

			float collisionCoefficient = std::min(physObj1->collisionCoefficient, physObj2->collisionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future
			float frictionCoefficient = std::max(physObj1->frictionCoefficient, physObj2->frictionCoefficient); // TODO: This can be calculated differently, will be based on material abilities in the future

			// Treat other object as constraint
			if (collisionInfo.entity1 == startingEntity) {
				float v1Dot = Intersection::dot(physObj1->velocity - physObj2->velocity, intersectionAxis);
				float relativeTangentVel = Intersection::dot(physObj1->velocity - physObj2->velocity, eN);
				float frictionMagnitude = std::min(relativeTangentVel * frictionCoefficient, v1Dot * (1.0f + collisionCoefficient) + frictionCoefficient);
				//float uDot = v1Dot * (1.0f + collisionCoefficient) + frictionCoefficient;
				physObj1->velocity -= v1Dot * (1.0f + collisionCoefficient) * (intersectionAxis) + frictionMagnitude * eN;
			}
			else if (collisionInfo.entity2 == startingEntity) {
				float v2Dot = Intersection::dot(physObj2->velocity - physObj1->velocity, intersectionAxis);
				float relativeTangentVel = Intersection::dot(physObj2->velocity - physObj1->velocity, eN);
				float frictionMagnitude = std::min(relativeTangentVel * frictionCoefficient, v2Dot * (1.0f + collisionCoefficient) + frictionCoefficient);
				//float uDot = v2Dot * (1.0f + collisionCoefficient) + frictionCoefficient;
				physObj2->velocity -= v2Dot * (1.0f + collisionCoefficient) * (intersectionAxis) + frictionMagnitude * eN;
			}
			

			if (std::find(collidingEntities.begin(), collidingEntities.end(), collisionInfo.entity1) == collidingEntities.end()) {
				collidingEntities.emplace_back(collisionInfo.entity1);
			}
			if (std::find(collidingEntities.begin(), collidingEntities.end(), collisionInfo.entity2) == collidingEntities.end()) {
				collidingEntities.emplace_back(collisionInfo.entity2);
			}
		}
	}

	void CollisionSystem::moveObject(Entity* e, glm::vec3& velocity, float dt) {
		if (glm::length2(velocity) > Utils::instance()->epsilon) { // Do not update transform if static, as updating the transform will promt a resorting of the octree
			Box* boundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
			TransformComponent* transform = e->getComponent<TransformComponent>();
			transform->translate(velocity * dt);
			boundingBox->setBaseMatrix(transform->getMatrixWithUpdate());
		}
	}

	int CollisionSystem::findMatchingCollision(std::vector<Intersection::CollisionTimeInfo>& list, Intersection::CollisionTimeInfo& search) {
		for (size_t i = 0; i < list.size(); i++) {
			if (list[i].entity1 == search.entity2 && list[i].entity2 == search.entity1) {
				if (list[i].triangleIndices.size() == 0 && search.triangleIndices.size() == 0) {
					return i;
				}
				else if (list[i].triangleIndices.size() > 0 && search.triangleIndices.size() > 0) {
					if (list[i].triangleIndices[0].first == search.triangleIndices[0].second && list[i].triangleIndices[0].second == search.triangleIndices[0].first) {
						return i;
					}
				}
			}
			else if (list[i].entity1 == search.entity1 && list[i].entity2 == search.entity2) {
				if (list[i].triangleIndices.size() == 0 && search.triangleIndices.size() == 0) {
					return i;
				}
				else if (list[i].triangleIndices.size() > 0 && search.triangleIndices.size() > 0) {
					if (list[i].triangleIndices[0].first == search.triangleIndices[0].first && list[i].triangleIndices[0].second == search.triangleIndices[0].second) {
						return i;
					}
				}
			}
		}
		return -1;
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