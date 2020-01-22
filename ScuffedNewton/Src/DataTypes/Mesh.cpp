#include "../pch.h"

#include "Mesh.h"

Mesh::Mesh() {

}

Mesh::~Mesh() {

}

void Mesh::loadData(void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
	m_data = data;
	m_size = size;
	m_vertexSize = size;
	m_positionOffset = positionOffset;
	m_positionSize = positionSize;
}

void Mesh::setModelMatrixPointer(glm::mat4* modelMatrix) {
	m_modelMatrix = modelMatrix;
}

glm::vec3 Mesh::getVertexPosition(int vertexIndex) {
	if (vertexIndex < m_size / m_vertexSize) {
		glm::vec3 position(0.0);
		void* pos = static_cast<void*>(static_cast<char*>(m_data) + (m_vertexSize * vertexIndex) + m_positionOffset);

		if (m_positionSize == sizeof(float)) {
			position.x = *(float*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.y = *(float*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.z = *(float*)pos;
		}
		else if (m_positionSize == sizeof(double)) {
			position.x = *(double*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.y = *(double*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.z = *(double*)pos;
		}
		

		return position;
	}
}

int Mesh::getNumberOfVertices() {
	return m_size / m_vertexSize;
}
