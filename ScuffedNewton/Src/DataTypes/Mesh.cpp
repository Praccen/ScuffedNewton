#include "../pch.h"

#include "Mesh.h"

Mesh::Mesh() {
	m_data = nullptr;
	m_size = 0;
	m_vertexSize = 0;
	m_positionOffset = 0;
	m_positionSize = 0;
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
			position.x = (float) *(double*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.y = (float) *(double*)pos;
			pos = static_cast<void*>(static_cast<char*>(pos) + m_positionSize);
			position.z = (float) *(double*)pos;
		}
	}

	return position;
}

int Mesh::getNumberOfVertices() {
	return m_size / m_vertexSize;
}
