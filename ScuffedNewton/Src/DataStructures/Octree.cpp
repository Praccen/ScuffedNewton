#include "../pch.h"

#include "Octree.h"
#include "../Calculations/Intersection.h"
#include "../DataTypes/Entity.h"
#include "../DataTypes/Mesh.h"
#include "../Components/Components.h"
#include "../Shapes/Box.h"
#include "../Shapes/Triangle.h"
#include "../Shapes/Shape.h"
#include "../Shapes/Ray.h"

#include "../Utils/Utils.h"

#include <algorithm>

namespace Scuffed {

	Octree::Octree(Model* boundingBoxModel) {
		// All entities in the octree must have a BoundingBoxComponent and a PhysicalBodyComponent (since OctreeAddRemoverSystem only accepts these entities)

		m_boundingBoxModel = boundingBoxModel;
		m_softLimitEntities = 10;
		m_minimumNodeHalfSize = 6.0f;

		m_baseNode.halfSize = glm::vec3(1000.0f, 1000.0f, 1000.0f);
		m_baseNode.nodeBB = SN_NEW Box(m_baseNode.halfSize, glm::vec3(0.0f));

		m_baseNode.parentNode = nullptr;
	}

	Octree::~Octree() {
		clean(&m_baseNode);
	}

	void Octree::expandBaseNode(glm::vec3 direction) {
		// Direction to expand in
		int x, y, z;
		x = direction.x >= 0.0f;
		y = direction.y >= 0.0f;
		z = direction.z >= 0.0f;

		Node newBaseNode;
		Box* baseNodeBB = m_baseNode.nodeBB;
		newBaseNode.nodeBB = SN_NEW Box(m_baseNode.halfSize * 2.0f, baseNodeBB->getMiddle() - m_baseNode.halfSize + glm::vec3(x * m_baseNode.halfSize.x * 2.0f, y * m_baseNode.halfSize.y * 2.0f, z * m_baseNode.halfSize.z * 2.0f));
		newBaseNode.halfSize = m_baseNode.halfSize * 2.0f;

		newBaseNode.parentNode = nullptr;

		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					Node tempChildNode;
					if (i != x && j != y && k != z) {
						tempChildNode = m_baseNode;
					}
					else {
						tempChildNode.nodeBB = SN_NEW Box(m_baseNode.halfSize, newBaseNode.nodeBB->getMiddle() - m_baseNode.halfSize + glm::vec3(m_baseNode.halfSize.x * 2.0f * i, m_baseNode.halfSize.y * 2.0f * j, m_baseNode.halfSize.z * 2.0f * k));
						tempChildNode.halfSize = m_baseNode.halfSize;
					}
					tempChildNode.parentNode = &newBaseNode;
					newBaseNode.childNodes.push_back(tempChildNode);
				}
			}
		}

		m_baseNode = newBaseNode;
	}


	glm::vec3 Octree::findCornerOutside(Entity* entity, Node* testNode, float dt) {
		// Find if any corner of a entity's bounding box is (or will be) outside of node. Returns a vector towards the outside corner if one is found. Otherwise a 0.0f vec is returned.
		glm::vec3 directionVec(0.0f, 0.0f, 0.0f);

		const std::vector<glm::vec3>& corners = entity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getVertices();
		glm::vec3& velocity = entity->getComponent<PhysicalBodyComponent>()->velocity;
		glm::vec3 testNodeHalfSize = testNode->halfSize;

		for (int i = 0; i < 8; i++) {
			glm::vec3 distanceVec = corners[i] - testNode->nodeBB->getMiddle();

			if (distanceVec.x < -testNodeHalfSize.x || distanceVec.x > testNodeHalfSize.x ||
				distanceVec.y < -testNodeHalfSize.y || distanceVec.y > testNodeHalfSize.y ||
				distanceVec.z < -testNodeHalfSize.z || distanceVec.z > testNodeHalfSize.z) {
				directionVec = distanceVec;
				i = 8;
			}
			else if (distanceVec.x + velocity.x * dt < -testNodeHalfSize.x || distanceVec.x + velocity.x * dt > testNodeHalfSize.x ||
				distanceVec.y + velocity.y * dt < -testNodeHalfSize.y || distanceVec.y + velocity.y * dt > testNodeHalfSize.y ||
				distanceVec.z + velocity.z * dt < -testNodeHalfSize.z || distanceVec.z + velocity.z * dt > testNodeHalfSize.z) {
				directionVec = distanceVec + velocity * dt;
				i = 8;
			}
		}

		return directionVec;
	}

	void Octree::addEntityRec(Entity* newEntity, Node* currentNode, float dt) {
		Box* nodeBoundingBox = currentNode->nodeBB;
		Box* entityBoundingBox = newEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
		glm::vec3& entityVel = newEntity->getComponent<PhysicalBodyComponent>()->velocity;

		float tempCollisionTime = Intersection::continousSAT(entityBoundingBox, nodeBoundingBox, entityVel, glm::vec3(0.f), dt);

		if (tempCollisionTime >= 0.f) { // See if entity will be in this node during this frame
			if (currentNode->childNodes.size() > 0) { // Not leaf node
				// Recursively call children
				for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
					addEntityRec(newEntity, &currentNode->childNodes[i], dt);
				}
			}
			else { // Is leaf node
				if (currentNode->entities.size() < m_softLimitEntities || currentNode->halfSize.x / 2.0f < m_minimumNodeHalfSize) { // Soft limit not reached or smaller nodes are not allowed
				// Add entity to this node
					currentNode->entities.push_back(newEntity);
					m_entityOccurances[newEntity].emplace_back(currentNode); // Save this node to the list of where this entity is
				}
				else {
					// Create more children
					for (int i = 0; i < 2; i++) {
						for (int j = 0; j < 2; j++) {
							for (int k = 0; k < 2; k++) {
								Box* currentNodeBB = currentNode->nodeBB;
								Node tempChildNode;
								tempChildNode.nodeBB = SN_NEW Box(currentNode->halfSize / 2.0f, currentNodeBB->getMiddle() - currentNode->halfSize / 2.0f + glm::vec3(currentNode->halfSize.x * i, currentNode->halfSize.y * j, currentNode->halfSize.z * k));
								tempChildNode.halfSize = currentNode->halfSize / 2.0f;
								tempChildNode.parentNode = currentNode;
								currentNode->childNodes.push_back(tempChildNode);
							}
						}
					}
					// Re add entities in this node so it will be placed in child nodes.
					for (size_t l = 0; l < currentNode->entities.size(); l++) {
						Entity* tempEntityPointer = currentNode->entities[l];
						addEntityRec(tempEntityPointer, currentNode, dt);

						// Remove node from the entity's occurences
						auto item = std::find(m_entityOccurances[tempEntityPointer].begin(), m_entityOccurances[tempEntityPointer].end(), currentNode);
						if (item != m_entityOccurances[tempEntityPointer].end()) {
							m_entityOccurances[tempEntityPointer].erase(item);
						}

					}

					currentNode->entities.clear();

					//Try to add the new entity to newly created child nodes.
					addEntityRec(newEntity, currentNode, dt);
				}
			}
		}
	}

	void Octree::removeEntityFromNode(Entity* entityToRemove, Node* node) {
		// This function does not remove the node from this entity's occurences, handle that outside of this (currently done in removeEntity for example).

		// Look for entity in this node
		for (size_t i = 0; i < node->entities.size(); i++) {
			if (node->entities[i]->getId() == entityToRemove->getId()) {
				// Entity found - Remove it
				node->entities[i] = node->entities.back();
				node->entities.pop_back();
				break;
			}
		}
	}

	int Octree::pruneTreeRec(Node* currentNode) {
		int returnValue = 0;

		if (currentNode->childNodes.size() > 0) { //Not a leaf node
			//Call for child nodes
			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				returnValue += pruneTreeRec(&currentNode->childNodes[i]);
			}

			if (returnValue == 0) {
				//No entities in any child - Prune the children
				for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
					delete currentNode->childNodes[i].nodeBB;
				}
				currentNode->childNodes.clear();
			}
		}

		returnValue += (int)currentNode->entities.size();

		return returnValue;
	}

	void Octree::clean(Node* currentNode) {
		for (size_t i = 0; i < currentNode->childNodes.size(); i++) {
			clean(&currentNode->childNodes[i]);
		}
		delete currentNode->nodeBB;
	}

	void Octree::addEntity(Entity* newEntity, float dt) {
		// See if the base node needs to be bigger
		glm::vec3 directionVec = findCornerOutside(newEntity, &m_baseNode, dt);

		if (glm::length(directionVec) != 0.0f) {
			// Entity is outside base node
			// Create bigger base node
			expandBaseNode(directionVec);
			// Recall this function to try to add the mesh again
			addEntity(newEntity, dt);
		}
		else {
			addEntityRec(newEntity, &m_baseNode, dt);
		}

	}

	void Octree::addEntities(std::vector<Entity*>* newEntities, float dt) {
		for (unsigned int i = 0; i < newEntities->size(); i++) {
			addEntity(newEntities->at(i), dt);
		}
	}

	void Octree::removeEntity(Entity* entityToRemove) {
		std::vector<Node*>& nodes = m_entityOccurances[entityToRemove];

		for (size_t i = 0; i < nodes.size(); i++) {
			removeEntityFromNode(entityToRemove, nodes[i]);
		}

		nodes.clear();
	}

	void Octree::removeEntities(std::vector<Entity*> entitiesToRemove) {
		for (unsigned int i = 0; i < entitiesToRemove.size(); i++) {
			removeEntity(entitiesToRemove[i]);
		}
	}

	void Octree::update(float dt) {
		std::for_each(m_entityOccurances.begin(), m_entityOccurances.end(), [&](std::pair<Entity*, std::vector<Node*> > element) {
			if (element.first->getComponent<BoundingBoxComponent>()->getBoundingBox()->getChange() || glm::length2(element.first->getComponent<PhysicalBodyComponent>()->velocity) > Utils::instance()->epsilon) {
				// Update this element by removing and re-adding
				removeEntity(element.first);
				addEntity(element.first, dt);
			}
			});

		pruneTreeRec(&m_baseNode);
	}

	void Octree::getNextContinousCollision(Entity* entity, const float& dt, std::vector<Intersection::CollisionTimeInfo>& outInfo) {
		float collisionTime = INFINITY;
		std::vector<Node*>& nodes = m_entityOccurances[entity];

		Box* entityBoundingBox = entity->getComponent<BoundingBoxComponent>()->getBoundingBox();
		PhysicalBodyComponent* physicalComp = entity->getComponent<PhysicalBodyComponent>();

		Intersection::CollisionTimeInfo tempTimeInfo;

		for (size_t i = 0; i < nodes.size(); i++) {
			// Check against entities
			for (auto& e : nodes[i]->entities) {
				// Don't let an entity collide with itself
				if (entity->getId() == e->getId()) {
					continue;
				}

				PhysicalBodyComponent* otherPhysicalComp = e->getComponent<PhysicalBodyComponent>();

				// Don't let two constraints collide as it will have no effect
				if (physicalComp->isConstraint && otherPhysicalComp->isConstraint) {
					continue;
				}

				Intersection::getCollisionTime(*entity, *e, std::min(collisionTime, dt), tempTimeInfo);

				if (tempTimeInfo.time <= dt) {
					if (tempTimeInfo.time > 0.f && tempTimeInfo.time < collisionTime) {
						collisionTime = tempTimeInfo.time;
						outInfo.clear();
						outInfo.emplace_back(tempTimeInfo);
					}
					else if (tempTimeInfo.time == collisionTime) {
						outInfo.emplace_back(tempTimeInfo);
					}
				}
			}
		}
	}
}