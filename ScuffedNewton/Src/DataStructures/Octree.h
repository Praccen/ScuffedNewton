#pragma once

#include <glm/glm.hpp>
#include <vector>

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
				intersectionDepth = 0.f;
				//intersectionPosition = { 0.0f, 0.0f, 0.0f };
				entity = nullptr;
				shape = nullptr;
			};

			virtual ~CollisionInfo() {

			};

			glm::vec3 intersectionAxis;
			float intersectionDepth;
			//glm::vec3 intersectionPosition;
			Entity* entity;
			std::shared_ptr<Shape> shape;
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
		
		void getNextContinousCollisionRec(Entity* entity, Node* currentNode, std::vector<CollisionInfo>& collisionInfo, float& collisionTime, std::vector<CollisionInfo>& zeroDistances, const float& dt = INFINITY, const bool doSimpleCollisions = false, const bool checkBackfaces = false);

		void getIntersectionData(const glm::vec3& rayStart, const glm::vec3& rayDir, Entity* meshEntity, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, RayIntersectionInfo* outIntersectionData, float padding, const bool checkBackfaces = false);
		void getRayIntersectionRec(const glm::vec3& rayStart, const glm::vec3& rayDir, Node* currentNode, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis, float padding, const bool doSimpleIntersections, const bool checkBackfaces);
		
		int pruneTreeRec(Node* currentNode);
		//int frustumCulledDrawRec(const Frustum& frustum, Node* currentNode);

		void clean(Node* currentNode);

	public:
		Octree(Model* boundingBoxModel = nullptr);
		virtual ~Octree();

		virtual void addEntity(Entity* newEntity);
		virtual void addEntities(std::vector<Entity*>* newEntities);

		virtual void removeEntity(Entity* entityToRemove);
		virtual void removeEntities(std::vector<Entity*> entitiesToRemove);

		virtual void update();

		virtual void getCollisions(Entity* entity, BoundingBox* entityBoundingBox, std::vector<CollisionInfo>* outCollisionData, const bool doSimpleCollisions = false, const bool checkBackfaces = false);
		virtual void getNextContinousCollision(Entity* entity, std::vector<CollisionInfo>& outCollisionInfo, float& collisionTime, std::vector<CollisionInfo>& zeroDistances, const float& dt = INFINITY, const bool doSimpleCollisions = false, const bool checkBackfaces = false);
		virtual void getRayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDir, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis = nullptr, float padding = 0.0f, const bool doSimpleIntersections = false, const bool checkBackfaces = false);

		//int frustumCulledDraw(Camera& camera);
	};

}