#pragma once

#include <glm/vec3.hpp>

namespace Scuffed {
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

	public:
		struct OctNode {
			std::vector<OctNode> childNodes;
			OctNode* parentNode = nullptr;
			BoundingBox* nodeBB = nullptr;
			int nrOfTriangles = 0;
			std::vector<size_t> triangleStarts;
		};

	private:

		void* m_data;
		size_t m_size;
		size_t m_vertexSize;
		size_t m_positionOffset;
		size_t m_positionSize;

		int* m_indices;
		int m_nrOfIndices;

		OctNode m_baseNode;
		int m_softLimitTriangles;

	private:
		void setUpOctree();
	};
}