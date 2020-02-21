#include "Triangle.h"

namespace Scuffed {

	Triangle::Triangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
		m_vertices.resize(3);
		m_normals.resize(1);
		m_edges.resize(3);

		setData(v1, v2, v3);
	}

	Triangle::~Triangle() {

	}

	void Triangle::setData(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
		matrix = glm::mat4(1.0);

		m_vertices[0] = v1;
		m_vertices[1] = v2;
		m_vertices[2] = v3;

		m_middle = (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.0f;

		m_normalsNeedsUpdate = true;
		m_edgesNeedsUpdate = true;
	}

	void Triangle::setMatrix(const glm::mat4& newMatrix) {
		for (int i = 0; i < 3; i++) {
			m_vertices[i] = glm::vec3(glm::inverse(matrix) * glm::vec4(m_vertices[i], 1.0f));
		}

		matrix = newMatrix;

		for (int i = 0; i < 3; i++) {
			m_vertices[i] = glm::vec3(matrix * glm::vec4(m_vertices[i], 1.0f));
		}

		m_middle = (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.0f;


		m_normalsNeedsUpdate = true;
		m_edgesNeedsUpdate = true;
	}

	std::vector<glm::vec3>& Triangle::getNormals() {
		if (m_normalsNeedsUpdate) {
			getEdges(); // Make sure that edges has been generated
			m_normals[0] = glm::normalize(glm::cross(m_edges[0], m_edges[1]));
			m_normalsNeedsUpdate = false;
		}

		return m_normals;
	}

	std::vector<glm::vec3>& Triangle::getEdges() {
		if (m_edgesNeedsUpdate) {
			m_edges = { glm::normalize(m_vertices[1] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[1]) };
			m_edgesNeedsUpdate = false;
		}

		return m_edges;
	}

	std::vector<glm::vec3>& Triangle::getVertices() {
		return m_vertices;
	}

	glm::vec3& Triangle::getMiddle() {
		return m_middle;
	}

}