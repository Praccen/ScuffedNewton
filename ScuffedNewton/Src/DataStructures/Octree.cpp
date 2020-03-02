#include "../pch.h"

#include "Octree.h"
#include "../Calculations/Intersection.h"
#include "../DataTypes/Entity.h"
#include "../DataTypes/Mesh.h"
#include "../Components/Components.h"
#include "../Shapes/Box.h"
#include "../Shapes/Triangle.h"
#include "../Shapes/Shape.h"

#include "../Utils/Utils.h"

namespace Scuffed {

	Octree::Octree(Model* boundingBoxModel) {

		m_boundingBoxModel = boundingBoxModel;
		m_softLimitMeshes = 4;
		m_minimumNodeHalfSize = 4.0f;

		m_baseNode.nodeBB = SN_NEW BoundingBox();
		m_baseNode.nodeBB->setPosition(glm::vec3(0.0f));
		m_baseNode.nodeBB->setHalfSize(glm::vec3(20.0f, 20.0f, 20.0f));

		m_baseNode.parentNode = nullptr;
		m_baseNode.nrOfEntities = 0;
	}

	Octree::~Octree() {
		clean(&m_baseNode);
	}

	void Octree::expandBaseNode(glm::vec3 direction) {
		//Direction to expand in
		int x, y, z;
		x = direction.x >= 0.0f;
		y = direction.y >= 0.0f;
		z = direction.z >= 0.0f;

		Node newBaseNode;
		const BoundingBox* baseNodeBB = m_baseNode.nodeBB;
		newBaseNode.nodeBB = SN_NEW BoundingBox();

		newBaseNode.nodeBB->setPosition(baseNodeBB->getPosition() - baseNodeBB->getHalfSize() + glm::vec3(x * baseNodeBB->getHalfSize().x * 2.0f, y * baseNodeBB->getHalfSize().y * 2.0f, z * baseNodeBB->getHalfSize().z * 2.0f));
		newBaseNode.nodeBB->setHalfSize(baseNodeBB->getHalfSize() * 2.0f);

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
						tempChildNode.nodeBB = SN_NEW BoundingBox();
						tempChildNode.nodeBB->setHalfSize(baseNodeBB->getHalfSize());
						tempChildNode.nodeBB->setPosition(newBaseNode.nodeBB->getPosition() - baseNodeBB->getHalfSize() + glm::vec3(tempChildNode.nodeBB->getHalfSize().x * 2.0f * i, tempChildNode.nodeBB->getHalfSize().y * 2.0f * j, tempChildNode.nodeBB->getHalfSize().z * 2.0f * k));
						tempChildNode.nrOfEntities = 0;
					}
					tempChildNode.parentNode = &newBaseNode;
					newBaseNode.childNodes.push_back(tempChildNode);
				}
			}
		}

		m_baseNode = newBaseNode;
	}


	glm::vec3 Octree::findCornerOutside(Entity* entity, Node* testNode) {
		//Find if any corner of a entity's bounding box is outside of node. Returns a vector towards the outside corner if one is found. Otherwise a 0.0f vec is returned.
		glm::vec3 directionVec(0.0f, 0.0f, 0.0f);

		const glm::vec3* corners = entity->getComponent<BoundingBoxComponent>()->getBoundingBox()->getCornersWithUpdate();
		glm::vec3 testNodeHalfSize = testNode->nodeBB->getHalfSize();

		for (int i = 0; i < 8; i++) {
			glm::vec3 distanceVec = corners[i] - testNode->nodeBB->getPosition();

			if (distanceVec.x <= -testNodeHalfSize.x || distanceVec.x >= testNodeHalfSize.x ||
				distanceVec.y <= -testNodeHalfSize.y || distanceVec.y >= testNodeHalfSize.y ||
				distanceVec.z <= -testNodeHalfSize.z || distanceVec.z >= testNodeHalfSize.z) {
				directionVec = distanceVec;
				i = 8;
			}
		}

		return directionVec;
	}

	bool Octree::addEntityRec(Entity* newEntity, Node* currentNode) {
		// TODO: Take movement into consideration

		bool entityAdded = false;

		glm::vec3 isInsideVec = findCornerOutside(newEntity, currentNode);
		if (glm::length(isInsideVec) < 1.0f) {
			//The current node does contain the whole mesh. Keep going deeper or add to this node if no smaller nodes are allowed

			if (currentNode->childNodes.size() > 0) { //Not leaf node
				//Recursively call children

				for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
					entityAdded = addEntityRec(newEntity, &currentNode->childNodes[i]);

					if (entityAdded) {
						//Mesh was added to child node. Break loop. No need to try the rest of the children
						i = (unsigned int)currentNode->childNodes.size();
					}
				}

				if (!entityAdded) { //Mesh did not fit in any child node
					//Add mesh to this node
					currentNode->entities.push_back(newEntity);
					currentNode->nrOfEntities++;
					entityAdded = true;
				}
			}
			else { //Is leaf node
				if (currentNode->nrOfEntities < m_softLimitMeshes || currentNode->nodeBB->getHalfSize().x / 2.0f < m_minimumNodeHalfSize) { //Soft limit not reached or smaller nodes are not allowed
					//Add mesh to this node
					currentNode->entities.push_back(newEntity);
					currentNode->nrOfEntities++;
					entityAdded = true;
				}
				else {
					//Create more children
					for (int i = 0; i < 2; i++) {
						for (int j = 0; j < 2; j++) {
							for (int k = 0; k < 2; k++) {
								const BoundingBox* currentNodeBB = currentNode->nodeBB;
								Node tempChildNode;
								tempChildNode.nodeBB = SN_NEW BoundingBox();
								BoundingBox* tempChildBoundingBox = tempChildNode.nodeBB;
								tempChildBoundingBox->setHalfSize(currentNodeBB->getHalfSize() / 2.0f);
								tempChildBoundingBox->setPosition(currentNodeBB->getPosition() - tempChildBoundingBox->getHalfSize() + glm::vec3(tempChildBoundingBox->getHalfSize().x * 2.0f * i, tempChildBoundingBox->getHalfSize().y * 2.0f * j, tempChildBoundingBox->getHalfSize().z * 2.0f * k));
								tempChildNode.nrOfEntities = 0;
								tempChildNode.parentNode = currentNode;
								currentNode->childNodes.push_back(tempChildNode);

								//Try to put meshes that was in this leaf node in the new child nodes.
								for (int l = 0; l < currentNode->nrOfEntities; l++) {
									if (addEntityRec(currentNode->entities[l], &currentNode->childNodes.back())) {
										//Mesh was successfully added to child. Remove it from this node.
										currentNode->entities.erase(currentNode->entities.begin() + l);
										currentNode->nrOfEntities--;
										l--;
									}
								}
							}
						}
					}

					//Try to add the mesh to newly created child nodes. It gets placed in current node within recursion if the children can not contain it.
					entityAdded = addEntityRec(newEntity, currentNode);
				}
			}
		}

		return entityAdded;
	}

	bool Octree::removeEntityRec(Entity* entityToRemove, Node* currentNode) {
		bool entityRemoved = false;

		//Look for mesh in this node
		for (int i = 0; i < currentNode->nrOfEntities; i++) {
			if (currentNode->entities[i]->getId() == entityToRemove->getId()) {
				//Mesh found - Remove it
				currentNode->entities[i] = currentNode->entities.back();
				currentNode->entities.pop_back();
				//currentNode->entities.erase(currentNode->entities.begin() + i);
				currentNode->nrOfEntities--;
				entityRemoved = true;
				i = currentNode->nrOfEntities;
			}
		}

		if (!entityRemoved) {
			//Mesh was not in this node. Recursively call this function for the children
			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				if (removeEntityRec(entityToRemove, &currentNode->childNodes[i])) {
					//Mesh was removed by one of the children, break the loop
					entityRemoved = true;
					i = (unsigned int)currentNode->childNodes.size();
				}
			}
		}

		return entityRemoved;
	}

	void Octree::updateRec(Node* currentNode, std::vector<Entity*>* entitiesToReAdd) {
		for (int i = 0; i < currentNode->nrOfEntities; i++) {
			if (currentNode->entities[i]->getComponent<BoundingBoxComponent>()->getBoundingBox()->getChange()) { //Entity has changed
				//Re-add the entity to get it in the right node
				Entity* tempEntity = currentNode->entities[i];
				//First remove the entity from this node to avoid duplicates
				bool removed = removeEntityRec(currentNode->entities[i], currentNode);

				if (removed) {
					i--;
					//Then store the entity to re-add it to the tree in the right node
					entitiesToReAdd->push_back(tempEntity);
				}
			}
		}

		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			updateRec(&currentNode->childNodes[i], entitiesToReAdd);
		}
	}

	void Octree::getCollisionData(BoundingBox* entityBoundingBox, Entity* meshEntity, Shape* shape, std::vector<CollisionInfo>* outCollisionData, const bool checkBackfaces) {
		if (Intersection::SAT(entityBoundingBox->getBox(), shape)) {
			outCollisionData->emplace_back();
			outCollisionData->back().entity = meshEntity;
			std::vector<glm::vec3>& verts = shape->getVertices();
			outCollisionData->back().shape = std::make_shared<Triangle>(verts[0], verts[1], verts[2]);
			outCollisionData->back().shape->setMatrix(meshEntity->getComponent<TransformComponent>()->getMatrixWithoutUpdate());
			// std::cout << "Collision detected with " + meshEntity->getId();
		}
	}

	// Shouldn't modify any components
	void Octree::getCollisionsRec(Entity* entity, BoundingBox* entityBoundingBox, Node* currentNode, std::vector<CollisionInfo>* outCollisionData, const bool doSimpleCollisions, const bool checkBackfaces) {
		BoundingBox* nodeBoundingBox = currentNode->nodeBB;

		// Early exit if Bounding box doesn't collide with the current node
		if (!Intersection::SAT(entityBoundingBox->getBox(), nodeBoundingBox->getBox())) {
			return;
		}
		//Check against entities
		for (auto& e: currentNode->entities) {
			//Don't let an entity collide with itself
			if (entity->getId() == e->getId()) {
				continue;
			}

			BoundingBox* otherBoundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();

			// continue if Bounding box doesn't collide with entity bounding box
			if (!Intersection::SAT(entityBoundingBox->getBox(), otherBoundingBox->getBox())) {
				continue;
			}

			// Get collision
			const MeshComponent* mesh = e->getComponent<MeshComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();
			const CollidableComponent* collidable = e->getComponent<CollidableComponent>();

			if (mesh && !(doSimpleCollisions && collidable->allowSimpleCollision)) {
				// Entity has a model. Check collision with meshes
				glm::mat4 transformMatrix(1.0f);
				if (transform) {
					transformMatrix = transform->getMatrixWithoutUpdate();
				}

				entityBoundingBox->getBox()->setMatrix(glm::inverse(transformMatrix));

				// Triangle to set mesh data to avoid creating new shapes for each triangle in mesh
				Triangle triangle(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));

				//for (unsigned int j = 0; j < model->getModel()->getNumberOfMeshes(); j++) {
				if (int numIndices = mesh->mesh->getNumberOfIndices() > 0) { // Has indices
					for (int j = 0; j < numIndices; j += 3) {
						triangle.setData(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 1)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 2)));
						getCollisionData(entityBoundingBox, e, &triangle, outCollisionData, checkBackfaces);
					}
				}
				else if (mesh->mesh->getNumberOfVertices() > 0) {
					int numVertices = mesh->mesh->getNumberOfVertices();
					for (int j = 0; j < numVertices; j += 3) {
						triangle.setData(mesh->mesh->getVertexPosition(j), mesh->mesh->getVertexPosition(j + 1), mesh->mesh->getVertexPosition(j + 2));
						getCollisionData(entityBoundingBox, e, &triangle, outCollisionData, checkBackfaces);
					}
				}
				//}

				entityBoundingBox->getBox()->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
			}
			else { // No model or simple collision opportunity
				// Collide with bounding box
				outCollisionData->emplace_back();
				outCollisionData->back().entity = e;
				outCollisionData->back().shape = std::make_shared<Box>(otherBoundingBox->getHalfSize(), otherBoundingBox->getPosition());
			}
		}

		//Check for children
		for (auto& it: currentNode->childNodes) {
			getCollisionsRec(entity, entityBoundingBox, &it, outCollisionData, doSimpleCollisions, checkBackfaces);
		}
	}

	void Octree::getNextContinousCollisionRec(Entity* entity, Node* currentNode, CollisionInfo& collisionInfo, float& collisionTime, const float& dt, const bool doSimpleCollisions, const bool checkBackfaces) {
		BoundingBox* nodeBoundingBox = currentNode->nodeBB;
		BoundingBox* entityBoundingBox = entity->getComponent<BoundingBoxComponent>()->getBoundingBox();
		MovementComponent* movComp = entity->getComponent<MovementComponent>();

		glm::vec3 entityVel(0.f);

		if (movComp) {
			entityVel = movComp->velocity;
		}

		// Early exit if Bounding box doesn't collide with the current node
		float tempCollisionTime = Intersection::continousSAT(entityBoundingBox->getBox(), nodeBoundingBox->getBox(), entityVel, glm::vec3(0.f), dt);
		if (tempCollisionTime < 0.f || tempCollisionTime > collisionTime ) {
			return;
		}

		glm::vec3 otherEntityVel;

		//Check against entities
		for (auto& e : currentNode->entities) {
			//Don't let an entity collide with itself
			if (entity->getId() == e->getId()) {
				continue;
			}

			BoundingBox* otherBoundingBox = e->getComponent<BoundingBoxComponent>()->getBoundingBox();
			MovementComponent* otherMovComp = e->getComponent<MovementComponent>();

			if (otherMovComp) {
				otherEntityVel = otherMovComp->velocity;
			}
			else {
				otherEntityVel = { 0.f, 0.f, 0.f };
			}

			tempCollisionTime = Intersection::continousSAT(entityBoundingBox->getBox(), otherBoundingBox->getBox(), entityVel, otherEntityVel, dt);
			// continue if Bounding box doesn't collide with entity bounding box
			if (tempCollisionTime < 0.f || tempCollisionTime > collisionTime) {
				continue;
			}

			// Get collision
			const MeshComponent* mesh = e->getComponent<MeshComponent>();
			TransformComponent* transform = e->getComponent<TransformComponent>();
			const CollidableComponent* collidable = e->getComponent<CollidableComponent>();

			if (mesh && !(doSimpleCollisions && collidable->allowSimpleCollision)) {
				// Entity has a model. Check collision with meshes
				glm::mat4 transformMatrix(1.0f);
				if (transform) {
					transformMatrix = transform->getMatrixWithoutUpdate();
				}

				entityBoundingBox->getBox()->setMatrix(glm::inverse(transformMatrix));

				glm::mat3 normalMatrix = glm::mat3(glm::transpose(transformMatrix));

				entityVel = normalMatrix * entityVel;
				otherEntityVel = normalMatrix * otherEntityVel;

				// Triangle to set mesh data to avoid creating new shapes for each triangle in mesh
				Triangle triangle(glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f));

				//for (unsigned int j = 0; j < model->getModel()->getNumberOfMeshes(); j++) {
				if (int numIndices = mesh->mesh->getNumberOfIndices() > 0) { // Has indices
					for (int j = 0; j < numIndices; j += 3) {
						triangle.setData(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 1)), mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 2)));
						// TODO: create function that does this, similar to getCollisionData
						float time = Intersection::continousSAT(entityBoundingBox->getBox(), &triangle, entityVel, otherEntityVel, dt);
						
						if (time > 0.f && time < collisionTime) {
							collisionTime = time;
							collisionInfo.entity = e;
							std::vector<glm::vec3>& verts = triangle.getVertices();
							collisionInfo.shape = std::make_shared<Triangle>(verts[0], verts[1], verts[2]);
							collisionInfo.shape->setMatrix(transformMatrix);
						}
					}
				}
				else if (mesh->mesh->getNumberOfVertices() > 0) {
					int numVertices = mesh->mesh->getNumberOfVertices();
					for (int j = 0; j < numVertices; j += 3) {
						triangle.setData(mesh->mesh->getVertexPosition(j), mesh->mesh->getVertexPosition(j + 1), mesh->mesh->getVertexPosition(j + 2));
						
						// TODO: create function that does this, similar to getCollisionData
						float time = Intersection::continousSAT(entityBoundingBox->getBox(), &triangle, entityVel, otherEntityVel, dt);

						if (time > 0.f && time < collisionTime) {
							collisionTime = time;
							collisionInfo.entity = e;
							std::vector<glm::vec3>& verts = triangle.getVertices();
							collisionInfo.shape = std::make_shared<Triangle>(verts[0], verts[1], verts[2]);
							collisionInfo.shape->setMatrix(transformMatrix);
						}
					}
				}
				//}

				entityBoundingBox->getBox()->setMatrix(glm::mat4(1.0f)); //Reset bounding box matrix to identity
			}
			else { // No model or simple collision opportunity
				if (tempCollisionTime > 0.f) {
					// Collide with bounding box
					collisionTime = tempCollisionTime;

					collisionInfo.entity = e;
					collisionInfo.shape = std::make_shared<Box>(otherBoundingBox->getHalfSize(), otherBoundingBox->getPosition());
				}
			}
		}

		//Check for children
		for (auto& it : currentNode->childNodes) {
			getNextContinousCollisionRec(entity, &it, collisionInfo, collisionTime, dt, doSimpleCollisions, checkBackfaces);
		}
	}

	void Octree::getIntersectionData(const glm::vec3& rayStart, const glm::vec3& rayDir, Entity* meshEntity, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, RayIntersectionInfo* outIntersectionData, float padding, const bool checkBackfaces) {
		float intersectionDistance = Intersection::RayWithPaddedTriangle(rayStart, rayDir, v1, v2, v3, padding, checkBackfaces);

		if (intersectionDistance >= 0.0f) {// && (intersectionDistance <= outIntersectionData->closestHit || outIntersectionData->closestHit < 0.0f)) {
			//Save closest hit

			if (intersectionDistance <= outIntersectionData->closestHit || outIntersectionData->closestHit < 0.0f) {
				outIntersectionData->closestHit = intersectionDistance;
				outIntersectionData->closestHitIndex = (int)outIntersectionData->info.size();
			}

			outIntersectionData->info.emplace_back();
			outIntersectionData->info.back().entity = meshEntity;
			outIntersectionData->info.back().shape = std::make_shared<Triangle>(v1, v2, v3);
			outIntersectionData->info.back().shape->setMatrix(meshEntity->getComponent<TransformComponent>()->getMatrixWithoutUpdate());
		}
	}

	void Octree::getRayIntersectionRec(const glm::vec3& rayStart, const glm::vec3& rayDir, Node* currentNode, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis, float padding, const bool doSimpleIntersections, const bool checkBackfaces) {

		const BoundingBox* nodeBoundingBox = currentNode->nodeBB;
		float nodeIntersectionDistance = Intersection::RayWithPaddedAabb(rayStart, rayDir, nodeBoundingBox->getPosition(), nodeBoundingBox->getHalfSize(), padding, nullptr);

		// Early exit if ray doesn't intersect with the current node closer than the closest hit
		if (nodeIntersectionDistance < 0.0f && (nodeIntersectionDistance < outIntersectionData->closestHit || outIntersectionData->closestHit < 0.0f)) {
			return;
		}

		//Check against entities
		for (int i = 0; i < currentNode->nrOfEntities; i++) {
			if (ignoreThis && currentNode->entities[i]->getId() == ignoreThis->getId()) {
				continue;
			}

			const BoundingBox* collidableBoundingBox = currentNode->entities[i]->getComponent<BoundingBoxComponent>()->getBoundingBox();
			glm::vec3 intersectionAxis;
			float entityIntersectionDistance = Intersection::RayWithPaddedAabb(rayStart, rayDir, collidableBoundingBox->getPosition(), collidableBoundingBox->getHalfSize(), padding, &intersectionAxis);

			// Continue if ray doesn't intersect the entity bounding box closer than the closest hit
			if (entityIntersectionDistance < 0.0f && (entityIntersectionDistance < outIntersectionData->closestHit || outIntersectionData->closestHit < 0.0f)) {
				continue;
			}

			//Get Intersection
			const MeshComponent* mesh = currentNode->entities[i]->getComponent<MeshComponent>();
			TransformComponent* transform = currentNode->entities[i]->getComponent<TransformComponent>();
			const CollidableComponent* collidable = currentNode->entities[i]->getComponent<CollidableComponent>();

			if (mesh && !(doSimpleIntersections && collidable->allowSimpleCollision)) {
				// Entity has a model. Check ray against meshes
				glm::mat4 transformMatrix(1.0f);
				if (transform) {
					transformMatrix = transform->getMatrixWithoutUpdate();
				}

				//for (unsigned int j = 0; j < model->getModel()->getNumberOfMeshes(); j++) {
				if (int numIndices = mesh->mesh->getNumberOfIndices() > 0) { // Has indices
					for (int j = 0; j < numIndices; j += 3) {
						glm::vec3 v0, v1, v2;
						v0 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j)), 1.0f));
						v1 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 1)), 1.0f));
						v2 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(mesh->mesh->getVertexIndex(j + 2)), 1.0f));
						getIntersectionData(rayStart, rayDir, currentNode->entities[i], v0, v1, v2, outIntersectionData, padding, checkBackfaces);
					}
				}
				else if (int numVertices = mesh->mesh->getNumberOfVertices() > 0) {
					for (int j = 0; j < mesh->mesh->getNumberOfVertices(); j += 3) {
						glm::vec3 v0, v1, v2;
						v0 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(j), 1.0f));
						v1 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(j + 1), 1.0f));
						v2 = glm::vec3(transformMatrix * glm::vec4(mesh->mesh->getVertexPosition(j + 2), 1.0f));
						getIntersectionData(rayStart, rayDir, currentNode->entities[i], v0, v1, v2, outIntersectionData, padding, checkBackfaces);
					}
				}
				//}
			}
			else { // No model or simple collision opportunity
			 //Intersect with bounding box

			 //Save closest hit
				if (entityIntersectionDistance <= outIntersectionData->closestHit || outIntersectionData->closestHit < 0.0f) {
					outIntersectionData->closestHit = entityIntersectionDistance;
					outIntersectionData->closestHitIndex = (int)outIntersectionData->info.size();
				}

				outIntersectionData->info.emplace_back();
				outIntersectionData->info.back().entity = currentNode->entities[i];
				outIntersectionData->info.back().shape = std::make_shared<Box>(collidableBoundingBox->getHalfSize(), collidableBoundingBox->getPosition());
			}
		}

		//Check for children
		for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
			getRayIntersectionRec(rayStart, rayDir, &currentNode->childNodes[i], outIntersectionData, ignoreThis, padding, doSimpleIntersections, checkBackfaces);
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

	/*int Octree::frustumCulledDrawRec(const Frustum& frustum, Node* currentNode) {
		int returnValue = 0;
		assert(false); // Not implemented yet

		// Check if node is in frustum
		BoundingBox* nodeBoundingBox = currentNode->bbEntity->getBoundingBox();
		if (Intersection::FrustumWithAabb(frustum, nodeBoundingBox->getCornersWithUpdate())) {
			// In frustum

			// Draw meshes in node
			for (int i = 0; i < currentNode->nrOfEntities; i++) {
				// Let the renderer know that this entity should be rendered.
				auto* cullComponent = currentNode->entities[i]->getComponent<CullingComponent>();
				if (cullComponent) {
					cullComponent->isVisible = true;
				}
				returnValue++;
			}

			// Call draw for all children
			for (unsigned int i = 0; i < currentNode->childNodes.size(); i++) {
				returnValue += frustumCulledDrawRec(frustum, &currentNode->childNodes[i]);
			}
		}


		return returnValue;
	}*/

	void Octree::addEntity(Entity* newEntity) {
		// See if the base node needs to be bigger
		glm::vec3 directionVec = findCornerOutside(newEntity, &m_baseNode);

		if (glm::length(directionVec) != 0.0f) {
			// Entity is outside base node
			// Create bigger base node
			expandBaseNode(directionVec);
			// Recall this function to try to add the mesh again
			addEntity(newEntity);
		}
		else {
			addEntityRec(newEntity, &m_baseNode);
		}
	}

	void Octree::addEntities(std::vector<Entity*>* newEntities) {
		for (unsigned int i = 0; i < newEntities->size(); i++) {
			addEntity(newEntities->at(i));
		}
	}

	void Octree::removeEntity(Entity* entityToRemove) {
		removeEntityRec(entityToRemove, &m_baseNode);
	}

	void Octree::removeEntities(std::vector<Entity*> entitiesToRemove) {
		for (unsigned int i = 0; i < entitiesToRemove.size(); i++) {
			removeEntity(entitiesToRemove[i]);
		}
	}

	void Octree::update() {
		std::vector<Entity*> entitiesToReAdd;
		updateRec(&m_baseNode, &entitiesToReAdd);

		addEntities(&entitiesToReAdd);

		entitiesToReAdd.clear();

		pruneTreeRec(&m_baseNode);
	}

	void Octree::getCollisions(Entity* entity, BoundingBox* entityBoundingBox, std::vector<CollisionInfo>* outCollisionData, const bool doSimpleCollisions, const bool checkBackfaces) {
		getCollisionsRec(entity, entityBoundingBox, &m_baseNode, outCollisionData, doSimpleCollisions, checkBackfaces);
	}

	void Octree::getNextContinousCollision(Entity* entity, CollisionInfo& outCollisionInfo, float& collisionTime, const float& dt, const bool doSimpleCollisions, const bool checkBackfaces) {
		getNextContinousCollisionRec(entity, &m_baseNode, outCollisionInfo, collisionTime, dt, doSimpleCollisions, checkBackfaces);
	}

	void Octree::getRayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDir, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis, float padding, const bool doSimpleIntersections, const bool checkBackfaces) {
		getRayIntersectionRec(rayStart, rayDir, &m_baseNode, outIntersectionData, ignoreThis, padding, doSimpleIntersections, checkBackfaces);
	}

	//int Octree::frustumCulledDraw(Camera& camera) {
	//	return frustumCulledDrawRec(camera.getFrustum(), &m_baseNode);
	//}

}