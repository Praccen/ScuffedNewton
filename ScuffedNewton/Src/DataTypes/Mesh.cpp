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
		glm::vec3 position;
		void* pos = static_cast<void*>(static_cast<char*>(m_data) + (m_vertexSize * vertexIndex) + m_positionOffset);
		//TODO: Add check for m_positionSize to know what glm::vec3 it should be. Instead of assuming float.
		position.x = *(float*)pos;
		pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
		position.y = *(float*)pos;
		pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
		position.z = *(float*)pos;

		return position;
	}
}

int Mesh::getNumberOfVertices() {
	return m_size / m_vertexSize;
}
