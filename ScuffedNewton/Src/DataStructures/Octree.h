#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Scuffed {

	class Box;
	class Model;
	class Entity;
	class Shape;
	class Ray;

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
			Entity* entity;
		};

	private:
		struct Node {
			std::vector<Node> childNodes;
			Node* parentNode = nullptr;
			Box* nodeBB = nullptr;
			glm::vec3 halfSize = glm::vec3(0.f);
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

		void getNextContinousCollisionRec(Entity* entity, Node* currentNode, std::vector<CollisionInfo>& collisionInfo, float& collisionTime, std::vector<CollisionInfo>& zeroDistances, const float& dt = INFINITY, const bool doSimpleCollisions = false, const bool checkBackfaces = false);

		void getRayIntersectionRec(Ray* ray, Node* currentNode, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis, const bool doSimpleIntersections, const bool checkBackfaces);
		
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

		virtual void getNextContinousCollision(Entity* entity, std::vector<CollisionInfo>& outCollisionInfo, float& collisionTime, std::vector<CollisionInfo>& zeroDistances, const float& dt = INFINITY, const bool doSimpleCollisions = false, const bool checkBackfaces = false);
		virtual void getRayIntersection(const glm::vec3& rayStart, const glm::vec3& rayDir, RayIntersectionInfo* outIntersectionData, Entity* ignoreThis = nullptr, const bool doSimpleIntersections = false, const bool checkBackfaces = false);

		//int frustumCulledDraw(Camera& camera);
	};

}