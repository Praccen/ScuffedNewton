#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "../Calculations/Intersection.h"

namespace Scuffed {

	class Box;
	class Model;
	class Entity;
	class Shape;
	class Ray;

	class Octree {
	private:
		struct Node {
			std::vector<Node> childNodes;
			Node* parentNode = nullptr;
			Box* nodeBB = nullptr;
			glm::vec3 halfSize = glm::vec3(0.f);
			std::vector<Entity*> entities;
		};

		Node m_baseNode;

		Model* m_boundingBoxModel;

		std::unordered_map<Entity*, std::vector<Node*>> m_entityOccurances; // Unordered map used to find which nodes an entity is in

		size_t m_softLimitEntities;
		float m_minimumNodeHalfSize;

		void expandBaseNode(glm::vec3 direction);
		glm::vec3 findCornerOutside(Entity* entity, Node* testNode, float dt);
		void addEntityRec(Entity* newEntity, Node* currentNode, float dt);
		void removeEntityFromNode(Entity* entityToRemove, Node* node);
				
		int pruneTreeRec(Node* currentNode);

		void clean(Node* currentNode);

	public:
		Octree(Model* boundingBoxModel = nullptr);
		virtual ~Octree();

		virtual void addEntity(Entity* newEntity, float dt);
		virtual void addEntities(std::vector<Entity*>* newEntities, float dt);

		virtual void removeEntity(Entity* entityToRemove);
		virtual void removeEntities(std::vector<Entity*> entitiesToRemove);

		virtual void update(float dt);

		virtual void getNextContinousCollision(Entity* entity, const float& dt, std::vector<Intersection::CollisionTimeInfo>& outInfo);
	};

}