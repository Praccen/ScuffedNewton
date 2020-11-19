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
		m_softLimitMeshes = 4;
		m_minimumNodeHalfSize = 4.0f;

		m_baseNode.halfSize = glm::vec3(20.0f, 20.0f, 20.0f);
		m_baseNode.nodeBB = SN_NEW Box(m_baseNode.halfSize, glm::vec3(0.0f));

		m_baseNode.parentNode = nullptr;
		m_baseNode.nrOfEntities = 0;
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

		newBaseNode.nrOfEntities = 0;
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
						tempChildNode.nrOfEntities = 0;
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
		if (currentNode->childNodes.size() > 0) { // Not leaf node
			// Recursively call children
			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				addEntityRec(newEntity, &currentNode->childNodes[i], dt);
			}
		}
		else { // Is leaf node

			Box* nodeBoundingBox = currentNode->nodeBB;
			Box* entityBoundingBox = newEntity->getComponent<BoundingBoxComponent>()->getBoundingBox();
			glm::vec3& entityVel = newEntity->getComponent<PhysicalBodyComponent>()->velocity;

			// See if entity will be in this node during this frame
			float tempCollisionTime = Intersection::continousSAT(entityBoundingBox, nodeBoundingBox, entityVel, glm::vec3(0.f), dt);
			glm::vec3 tempVec = findCornerOutside(newEntity, currentNode, dt);
			if (tempCollisionTime >= 0.f) {
				if (currentNode->nrOfEntities < m_softLimitMeshes || currentNode->halfSize.x / 2.0f < m_minimumNodeHalfSize) { // Soft limit not reached or smaller nodes are not allowed
				// Add entity to this node
					currentNode->entities.push_back(newEntity);
					currentNode->nrOfEntities++;
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
								tempChildNode.nrOfEntities = 0;
								tempChildNode.parentNode = currentNode;
								currentNode->childNodes.push_back(tempChildNode);
							}
						}
					}
					// Re add entities in this node so it will be placed in child nodes.
					for (int l = 0; l < currentNode->nrOfEntities; l++) {
						Entity* tempEntityPointer = currentNode->entities[l];
						addEntityRec(tempEntityPointer, currentNode, dt);

						// Remove node from the entity's occurences
						m_entityOccurances[tempEntityPointer].erase(std::find(m_entityOccurances[tempEntityPointer].begin(), m_entityOccurances[tempEntityPointer].end(), currentNode));
					}


					currentNode->entities.clear();
					currentNode->nrOfEntities = 0;


					//Try to add the new entity to newly created child nodes.
					addEntityRec(newEntity, currentNode, dt);
				}
			}
		}
	}

	void Octree::removeEntityFromNode(Entity* entityToRemove, Node* node) {
		// This function does not remove the node from this entity's occurences, handle that outside of this (currently done in removeEntity for example).

		// Look for entity in this node
		for (int i = 0; i < node->nrOfEntities; i++) {
			if (node->entities[i]->getId() == entityToRemove->getId()) {
				// Entity found - Remove it
				node->entities[i] = node->entities.back();
				node->entities.pop_back();
				node->nrOfEntities--;
				i = node->nrOfEntities;
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

		returnValue += currentNode->nrOfEntities;

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
			if (element.first->getComponent<BoundingBoxComponent>()->getBoundingBox()->getChange() || element.first->getComponent<PhysicalBodyComponent>()->velocity.length() != 0.f) {
				// Update this element by removing and re-adding
				removeEntity(element.first);
				addEntity(element.first, dt);
			}
			});

		pruneTreeRec(&m_baseNode);
	}

	void Octree::getNextContinousCollision(Entity* entity, std::vector<CollisionInfo>& outCollisionInfo, float& collisionTime, const float& dt) {
		std::vector<Node*>& nodes = m_entityOccurances[entity];

		Box* entityBoundingBox = entity->getComponent<BoundingBoxComponent>()->getBoundingBox();
		PhysicalBodyComponent* physicalComp = entity->getComponent<PhysicalBodyComponent>();

		float tempCollisionTime;

		for (size_t i = 0; i < nodes.size(); i++) {
			//Check against entities
			for (auto& e : nodes[i]->entities) {
				//Don't let an entity collide with itself
				if (entity->getId() == e->getId()) {
					continue;
				}

				Box* otherBoundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
				PhysicalBodyComponent* otherPhysicalComp = e->getComponent<PhysicalBodyComponent>();

				tempCollisionTime = Intersection::continousSAT(entityBoundingBox, otherBoundingBox, physicalComp->velocity, otherPhysicalComp->velocity, dt);

				if (tempCollisionTime >= 0.f && tempCollisionTime < collisionTime) {
					// Collide with bounding box
					collisionTime = tempCollisionTime;
					outCollisionInfo.clear();
					outCollisionInfo.emplace_back();
					outCollisionInfo.back().entity = e;
					outCollisionInfo.back().shape = std::make_shared<Box>(*otherBoundingBox);
				}
				else if (tempCollisionTime == collisionTime) {
					outCollisionInfo.emplace_back();
					outCollisionInfo.back().entity = e;
					outCollisionInfo.back().shape = std::make_shared<Box>(*otherBoundingBox);
				}

				// Mesh collisions
				//const MeshComponent* mesh = e->getComponent<MeshComponent>();
				//TransformComponent* transform = e->getComponent<TransformComponent>();
				//const CollidableComponent* collidable = e->getComponent<CollidableComponent>();

				//if (mesh && !(doSimpleCollisions && collidable->allowSimpleCollision)) {
				//	// Entity has a model. Check collision with meshes
				//	glm::mat4 transformMatrix(1.0f);
				//	if (transform) {
				//		transformMatrix = transform->getMatrixWithUpdate();
				//	}

				//	entityBoundingBox->setMatrix(glm::inverse(transformMatrix));

				//	//Convert velocities to local space for mesh
				//	glm::vec3 zeroPoint = glm::inverse(transformMatrix) * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
				//	glm::vec3 newEntityVel = glm::inverse(transformMatrix) * glm::vec4(entityVel, 1.0f);
				//	newEntityVel = newEntityVel - zeroPoint;
				//	otherEntityVel = glm::inverse(transformMatrix) * glm::vec4(otherEntityVel, 1.0f);
				//	otherEntityVel = otherEntityVel - zeroPoint;
				//	//glm::vec3 newEntityVel = entityVel;

				//	// Get triangles to test continous collision against from narrow phase octree in mesh
				//	std::vector<int> triangles;
				//	mesh->mesh->getTrianglesForContinousCollisionTesting(triangles, entityBoundingBox, newEntityVel, otherEntityVel, dt);

				//	// Triangle to set mesh data to avoid creating new shapes for each triangle in mesh
				//	Triangle triangle(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));

				//	//for (unsigned int j = 0; j < model->getModel()->getNumberOfMeshes(); j++) {
				//	int numTriangles = triangles.size();
				//	bool hasIndices = mesh->mesh->getNumberOfIndices() > 0;
				//	bool hasVertices = mesh->mesh->getNumberOfVertices() > 0;

				//	for (int j = 0; j < numTriangles; j++) {
				//		if (hasIndices) { // Has indices
				//			triangle.setData(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(triangles[j])), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(triangles[j] + 1)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(triangles[j] + 2)));
				//		}
				//		else if (hasVertices) {
				//			triangle.setData(mesh->mesh->getVertexPosition(triangles[j]), mesh->mesh->getVertexPosition(triangles[j] + 1), mesh->mesh->getVertexPosition(triangles[j] + 2));
				//		}
				//		//triangle.setBaseMatrix(transformMatrix);

				//		float time = Intersection::continousSAT(entityBoundingBox, &triangle, newEntityVel, otherEntityVel, dt);

				//		if (time > 0.f && time < collisionTime) {
				//			collisionInfo.clear();
				//			collisionTime = time;
				//			collisionInfo.emplace_back();
				//			collisionInfo.back().entity = e;
				//			collisionInfo.back().shape = std::make_shared<Triangle>(triangle);
				//			collisionInfo.back().shape->setBaseMatrix(transformMatrix);
				//		}
				//		else if (time == collisionTime) {
				//			collisionInfo.emplace_back();
				//			collisionInfo.back().entity = e;
				//			collisionInfo.back().shape = std::make_shared<Triangle>(triangle);
				//			collisionInfo.back().shape->setBaseMatrix(transformMatrix);
				//		}
				//		else if (time == 0.f) {
				//			zeroDistances.emplace_back();
				//			zeroDistances.back().entity = e;
				//			zeroDistances.back().shape = std::make_shared<Triangle>(triangle);
				//			zeroDistances.back().shape->setBaseMatrix(transformMatrix);
				//		}
				//	}
				//	//}

				//	entityBoundingBox->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
				//}
				//else { // No model or simple collision opportunity
				//	if (tempCollisionTime > 0.f && tempCollisionTime < collisionTime) {
				//		// Collide with bounding box
				//		collisionTime = tempCollisionTime;
				//		collisionInfo.clear();
				//		collisionInfo.emplace_back();
				//		collisionInfo.back().entity = e;
				//		collisionInfo.back().shape = std::make_shared<Box>(*otherBoundingBox);
				//	}
				//	else if (tempCollisionTime == collisionTime) {
				//		collisionInfo.emplace_back();
				//		collisionInfo.back().entity = e;
				//		collisionInfo.back().shape = std::make_shared<Box>(*otherBoundingBox);
				//	}
				//	else if (tempCollisionTime == 0.f) {
				//		zeroDistances.emplace_back();
				//		zeroDistances.back().entity = e;
				//		zeroDistances.back().shape = std::make_shared<Box>(*otherBoundingBox);
				//	}
				//}

			}
		}
	}
}