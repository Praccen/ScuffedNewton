#pragma once

#include <glm/vec3.hpp>

class Mesh {
public:
	Mesh();
	~Mesh();

	void loadData(void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize);

	glm::vec3 getVertexPosition(int vertexIndex);

	int getNumberOfVertices();

private:
	void* m_data;
	size_t m_size;
	size_t m_vertexSize;
	size_t m_positionOffset;
	size_t m_positionSize;
};