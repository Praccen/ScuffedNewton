#include "Triangle.h"

namespace Scuffed {

	Triangle::Triangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
		init();

		setData(v1, v2, v3);
	}

	Triangle::~Triangle() {

	}

	void Triangle::init() {
		m_originalVertices.resize(3);
		m_vertices.resize(3);
		m_normals.resize(1);
		m_edges.resize(3);

		m_matricesHasChanged = false;
	}

	void Triangle::updateEdges() {
		getVertices();
		m_edges = { glm::normalize(m_vertices[1] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[0]), glm::normalize(m_vertices[2] - m_vertices[1]) };
	}

	void Triangle::updateNormals() {
		getEdges(); // Make sure that edges has been generated
		m_normals[0] = glm::normalize(glm::cross(m_edges[0], m_edges[1]));
	}

	void Triangle::updateMiddle() {
		getVertices();
		m_middle = (m_vertices[0] + m_vertices[1] + m_vertices[2]) / 3.0f;
	}

	void Triangle::updateVertices() {
		for (int i = 0; i < 3; i++) {
			if (m_matricesHasChanged) {
				m_vertices[i] = glm::vec3(matrix * baseMatrix * glm::vec4(m_originalVertices[i], 1.0f));
			}
			else {
				m_vertices[i] = m_originalVertices[i];
			}
		}
	}

	void Triangle::setData(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3) {
		m_originalVertices[0] = v1;
		m_originalVertices[1] = v2;
		m_originalVertices[2] = v3;

		m_normalsNeedsUpdate = true;
		m_edgesNeedsUpdate = true;
		m_middleNeedsUpdate = true;
		m_verticesNeedsUpdate = true;
	}

	void Triangle::setBaseMatrix(const glm::mat4& newBaseMatrix) {
		baseMatrix = newBaseMatrix;

		m_normalsNeedsUpdate = true;
		m_edgesNeedsUpdate = true;
		m_middleNeedsUpdate = true;
		m_verticesNeedsUpdate = true;
		m_matricesHasChanged = true;
	}

	void Triangle::setMatrix(const glm::mat4& newMatrix) {
		matrix = newMatrix;

		m_normalsNeedsUpdate = true;
		m_edgesNeedsUpdate = true;
		m_middleNeedsUpdate = true;
		m_verticesNeedsUpdate = true;
		m_matricesHasChanged = true;
	}

	std::vector<glm::vec3>& Triangle::getNormals() {
		if (m_normalsNeedsUpdate) {
			updateNormals();
			m_normalsNeedsUpdate = false;
		}

		return m_normals;
	}

	std::vector<glm::vec3>& Triangle::getEdges() {
		if (m_edgesNeedsUpdate) {
			updateEdges();
			m_edgesNeedsUpdate = false;
		}

		return m_edges;
	}

	std::vector<glm::vec3>& Triangle::getVertices() {
		if (m_verticesNeedsUpdate) {
			updateVertices();
			m_verticesNeedsUpdate = false;
		}
		return m_vertices;
	}

	glm::vec3& Triangle::getMiddle() {
		if (m_middleNeedsUpdate) {
			updateMiddle();
			m_middleNeedsUpdate = false;
		}
		return m_middle;
	}
}