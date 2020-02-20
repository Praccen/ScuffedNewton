#include "Triangle.h"

#include "Intersection.h"

namespace Scuffed {

	Triangle::Triangle(const glm::vec3 vertices[3]) {
		m_vertices.resize(3);

		m_vertices[0] = vertices[0];
		m_vertices[1] = vertices[1];
		m_vertices[2] = vertices[2];

		m_middle = (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.0f;
	}

	Triangle::~Triangle() {

	}

	std::vector<glm::vec3>& Triangle::getNormals() {
		if (m_normals.size() == 0) {
			m_normals.resize(1);
			getEdges(); // Make sure that edges has been generated
			m_normals[0] = glm::normalize(glm::cross(m_edges[0], m_edges[1]));
		}

		return m_normals;
	}

	std::vector<glm::vec3>& Triangle::getEdges() {
		if (m_edges.size() == 0) {
			m_edges.resize(3);
			m_edges = { glm::normalize(m_vertices[1] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[1]) };
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