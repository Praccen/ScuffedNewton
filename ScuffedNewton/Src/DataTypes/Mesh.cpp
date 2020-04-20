#include "../pch.h"

#include "Mesh.h"

namespace Scuffed {

	Mesh::Mesh() {
		m_data = nullptr;
		m_size = 0;
		m_vertexSize = 0;
		m_positionOffset = 0;
		m_positionSize = 0;

		m_indices = nullptr;
		m_nrOfIndices = 0;

		m_softLimitTriangles = 20;
	}

	Mesh::~Mesh() {

	}

	void Mesh::loadData(void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
		m_data = data;
		m_size = size;
		m_vertexSize = vertexSize;
		m_positionOffset = positionOffset;
		m_positionSize = positionSize;
	}

	void Mesh::loadIndices(int* indices, int nrOfIndices) {
		m_indices = indices;
		m_nrOfIndices = nrOfIndices;
	}

	glm::vec3 Mesh::getVertexPosition(int vertexIndex) {
		glm::vec3 position(0.0);
		if ((size_t)vertexIndex < m_size / m_vertexSize) {
			void* pos = static_cast<void*>(static_cast<char*>(m_data) + (m_vertexSize * vertexIndex) + m_positionOffset);

			if (m_positionSize == sizeof(float)) {
				position.x = *(float*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.y = *(float*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.z = *(float*)pos;
			}
			else if (m_positionSize == sizeof(double)) {
				position.x = (float) * (double*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.y = (float) * (double*)pos;
				pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
				position.z = (float) * (double*)pos;
			}
		}

		return position;
	}

	int Mesh::getVertexIndex(int indexIndex) {
		if (indexIndex < m_nrOfIndices) {
			return m_indices[indexIndex];
		}
		return -1;
	}

	int Mesh::getNumberOfVertices() {
		return m_size / m_vertexSize;
	}

	int Mesh::getNumberOfIndices() {
		return m_nrOfIndices;
	}

	void Mesh::setUpOctree() {
		glm::vec3 minVals{INFINITY};
		glm::vec3 maxVals{-INFINITY};

		if (m_nrOfIndices > 0) { // Has indices
			for (int i = 0; i < m_nrOfIndices; i++) {
				glm::vec3 position = getVertexPosition(m_indices[i]);
				for (int j = 0; j < 3; j++) {
					if (position[j] < minVals[j]) {
						minVals[j] = position[j];
					}
					if (position[j] > maxVals[j]) {
						maxVals[j] = position[j];
					}
				}
			}
		}
		else if (int numVertices = getNumberOfVertices() > 0) {
			for (int i = 0; i < numVertices; i++) {
				glm::vec3 position = getVertexPosition(i);
				for (int j = 0; j < 3; j++) {
					if (position[j] < minVals[j]) {
						minVals[j] = position[j];
					}
					if (position[j] > maxVals[j]) {
						maxVals[j] = position[j];
					}
				}
			}
		}

		m_baseNode.nodeBB = SN_NEW BoundingBox();
		m_baseNode.nodeBB->setHalfSize((maxVals - minVals) / 2.0f);
	}

}