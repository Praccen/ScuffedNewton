#include "../pch.h"
#include "Box.h"

Scuffed::Box::Box(glm::vec3 planes[6], const glm::vec3& middle) { //Assumes Orthogonal
	m_normals.resize(6);
	m_edges.resize(6);

	// Sort planes, oposite sides indexed next to each other
	for (unsigned int i = 0; i < 6; i+=2) {
		for (unsigned int j = i + 1; j < 6; j++) {
			if (glm::dot(planes[i], planes[j]) < -0.001f) {
				// Opposite found, swap j and i+1 if not already correct
				if (j != i + 1) {
					glm::vec3 tempVec = planes[i + 1];
					planes[i + 1] = planes[j];
					planes[j] = tempVec;
				}
			}
		}
	}

	// Save normalized vectors
	for (int i = 0; i < 6; i++) {
		glm::vec3 norm = glm::normalize(planes[i]);
		m_normals[i] = norm;
		m_edges[i] = norm;
	}

	m_vertices.resize(8);

	// Find corners
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; i < 2; k++) {
				m_vertices[k + j * 2 + i * 4] = middle + planes[i] + planes[2 + j] + planes[4 + k];
			}
		}
	}

	m_middle = middle;
}

Scuffed::Box::~Box() {

}

std::vector<glm::vec3>& Scuffed::Box::getNormals() {
	return m_normals;
}

std::vector<glm::vec3>& Scuffed::Box::getEdges() {
	return m_edges;
}

std::vector<glm::vec3>& Scuffed::Box::getVertices() {
	return m_vertices;
}

glm::vec3& Scuffed::Box::getMiddle() {
	return m_middle;
}
