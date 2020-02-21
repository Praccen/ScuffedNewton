#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "../DataTypes/CollisionShapes.h"

namespace Scuffed {

	class BoundingBox;
	class Model;
	class Entity;
	class Shape;

	class Octree {
	public:
		class CollisionInfo {
		public:
			CollisionInfo() {
				intersectionAxis = { 0.0f, 0.0f, 0.0f };
				intersectionPosition = { 0.0f, 0.0f, 0.0f };
				shape = nullptr;
				entity = nullptr;
			};

			~CollisionInfo() {
				if (shape != nullptr) {
					shape->keeperTracker--;
					if (shape->keeperTracker == 0) {
						delete shape;
					}
				}
			};


			CollisionInfo(const CollisionInfo& other) {
				this->intersectionAxis = other.intersectionAxis;
				this->intersectionPosition = other.intersectionPosition;
				other.shape->keeperTracker++;
				this->shape = other.shape;
				this->entity = other.entity;
			}
			CollisionInfo& operator=(const CollisionInfo& other) {
				this->intersectionAxis = other.intersectionAxis;
				this->intersectionPosition = other.intersectionPosition;
				other.shape->keeperTracker++;
				this->shape = other.shape;
				this->entity = other.entity;
				return *this;
			}

			glm::vec3 intersectionAxis;
			glm::vec3 intersectionPosition;
			CollisionShape* shape;
			Entity* entity;
		};

		struct RayIntersectionInfo {
			float closestHit = -1.0f;
			int closestHitIndex = -1;
			std::vector<Octree::CollisionInfo> info;
		};

	private:
		struct Node {
			std::vector<Node> childNodes;
			Node* parentNode = nullptr;
			BoundingBox* nodeBB = nullptr;
			int nrOfEntities = 0;
			std::vector<Entity*> entities;
		};

		Node m_baseNode;

		Model* m_boundingBoxModel;

		int m_softLimitMeshes;
		float m_minimumNodeHalfSize;

		void expandBaseNode(glm::vec3 direction);
		glm::vec3 findCornerOutside(Entity* entity, Node* testNode);
		bool addEntityRec(Entity* newEntity, Node* currentNode);
		bool removeEntityRec(Entity* entityToRemove, Node* currentNode);
		void updateRec(Node* currentNode, std::vector<Entity*>* entitiesToReAdd);
		void getCollisionData(BoundingBox* entityBoundingBox, Entity* meshEntity, Shape* shape, std::vector<Octree::CollisionInfo>* outCollisionData, const bool checkBackfaces = false);
		void getCollisionsRec(Entity* entity, BoundingBox* entityBoundingBox, Node* currentNode, std::vector<Octree::CollisionInfo>* outCollisionData, const bool doSimpleCollisions, const bool checkBackfaces = false);
		void getIntersectionData(const glm::vec3& rayStart, const glm::vec3& rayDir, Entity* meshEntity, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, RayIntersectionInfo* outIntersectionData, float padding, const bool checkBackfaces = false);
		void getRayIntersectionRec(const glm::vec3& rayStart, const glm::vec3& rayDir, Node* currentNode, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis, float padding, const bool doSimpleIntersections, const bool checkBackfaces);
		int pruneTreeRec(Node* currentNode);
		//int frustumCulledDrawRec(const Frustum& frustum, Node* currentNode);

		void clean(Node* currentNode);

	public:
		Octree(Model* boundingBoxModel = nullptr);
		~Octree();

		virtual void addEntity(Entity* newEntity);
		virtual void addEntities(std::vector<Entity*>* newEntities);

		virtual void removeEntity(Entity* entityToRemove);
		virtual void removeEntities(std::vector<Entity*> entitiesToRemove);

		virtual void update();

		virtual void getCollisions(Entity* entity, BoundingBox* entityBoundingBox, std::vector<CollisionInfo>* outCollisionData, const bool doSimpleCollisions = false, const bool checkBackfaces = false);
		virtual void getRayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDir, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis = nullptr, float padding = 0.0f, const bool doSimpleIntersections = false, const bool checkBackfaces = false);

		//int frustumCulledDraw(Camera& camera);
	};

}