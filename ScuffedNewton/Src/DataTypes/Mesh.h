#pragma once

#include <glm/vec3.hpp>

namespace Scuffed {
	class Box;

	class Mesh {
	public:
		Mesh();
		~Mesh();

		virtual void loadData(void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize);
		virtual void loadIndices(int* indices, int nrOfIndices);

		virtual glm::vec3 getVertexPosition(int vertexIndex);
		virtual int getVertexIndex(int indexIndex);

		virtual int getNumberOfVertices();
		virtual int getNumberOfIndices();

	private:

		void* m_data;
		size_t m_size;
		size_t m_vertexSize;
		size_t m_positionOffset;
		size_t m_positionSize;

		int* m_indices;
		int m_nrOfIndices;

	public:
		struct OctNode {
			std::vector<OctNode> childNodes;
			OctNode* parentNode = nullptr;
			Box* nodeBB = nullptr;
			glm::vec3 halfSize = glm::vec3(0.f);
			int nrOfTriangles = 0;
			std::vector<int> triangles;
		};

		OctNode* getOctreeBaseNode();

		void getCollidingNodesContinous(std::vector<OctNode*>& nodes, Box* box, glm::vec3& boxVel, glm::vec3& meshVel, const float maxTime);

	private:
		// ----Narrow phase octree----
		OctNode m_baseNode;
		int m_softLimitTriangles;
		float m_minimumNodeHalfSize;
		
		void setUpOctree();
		void addTrianglesToOctree(std::vector<int> trianglesToAdd);
		void addTriangleRec(int triangle, OctNode* node);
		void expandBaseNode(glm::vec3 direction);
		bool isIntersecting(int triangle, OctNode* node);
		glm::vec3 findCornerOutside(int triangle, OctNode* testNode);
		void clean(OctNode* node);

		void getCollidingNodesContinousRec(std::vector<OctNode*>& nodes, Box* box, glm::vec3& boxVel, glm::vec3& meshVel, OctNode* node, const float maxTime);
		// ---------------------------
	};
}