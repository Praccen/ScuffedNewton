#include "Ray.h"

namespace Scuffed {

	Ray::Ray(const glm::vec3& start, const glm::vec3& direction) {
		m_vertices.resize(1);
		m_normals.resize(1);
		m_edges.resize(0);

		m_vertices[0] = start;
		m_normals[0] = glm::normalize(direction);
		
		baseMatrix = glm::mat4(1.0f);
		matrix = glm::mat4(1.0f);
	}

	Ray::~Ray() {

	}

	void Ray::setBaseMatrix(const glm::mat4& newBaseMatrix) {

	}

	void Ray::setMatrix(const glm::mat4& newMatrix) {
		// Bring back to base transform
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
		m_normals[0] = glm::vec3(glm::inverse(normalMatrix) * m_normals[0]);

		m_vertices[0] = glm::vec3(glm::inverse(matrix) * glm::vec4(m_vertices[0], 1.0f));

		matrix = newMatrix;

		// Apply new transform
		normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
		m_normals[0] = glm::vec3(normalMatrix * m_normals[0]);

		m_vertices[0] = glm::vec3(matrix * glm::vec4(m_vertices[0], 1.0f));
	}

	std::vector<glm::vec3>& Ray::getNormals() {
		return m_normals;
	}

	std::vector<glm::vec3>& Ray::getEdges() {
		return m_edges;
	}

	std::vector<glm::vec3>& Ray::getVertices() {
		return m_vertices;
	}

	glm::vec3& Ray::getMiddle() {
		return m_vertices[0];
	}

}