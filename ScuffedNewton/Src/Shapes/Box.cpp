#include "../pch.h"
#include "Box.h"

Scuffed::Box::Box(const glm::vec3& halfSize, const glm::vec3& middle) {
	init();

	glm::vec3 planes[6] = {
			{halfSize.x, 0.f, 0.f},
			{-halfSize.x, 0.f, 0.f},
			{0.f, halfSize.y, 0.f},
			{0.f, -halfSize.y, 0.f},
			{0.f, 0.f, halfSize.z},
			{0.f, 0.f, -halfSize.z}
	};

	setData(planes, middle);
}

Scuffed::Box::Box(glm::vec3 planes[6], const glm::vec3& middle) { //Assumes Orthogonal
	init();

	setData(planes, middle);
}

Scuffed::Box::~Box() {

}

void Scuffed::Box::init() {
	m_edges.resize(0);
	m_planes.resize(6);
	m_originalVertices.resize(8);
	m_normals.resize(6);
	m_vertices.resize(8);
}

void Scuffed::Box::setData(glm::vec3 planes[6], const glm::vec3& middle) {
	matrix = glm::mat4(1.f);
	m_middle = middle;

	for (int i = 0; i < 6; i++) {
		m_planes[i] = planes[i];
	}

	// Sort planes, oposite sides indexed next to each other
	for (unsigned int i = 0; i < 6; i += 2) {
		for (unsigned int j = i + 1; j < 6; j++) {
			if (glm::dot(m_planes[i], m_planes[j]) < -0.001f) {
				// Opposite found, swap j and i+1 if not already correct
				if (j != i + 1) {
					glm::vec3 tempVec = m_planes[i + 1];
					m_planes[i + 1] = m_planes[j];
					m_planes[j] = tempVec;
				}
			}
		}
	}

	// Find corners
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 2; j++) {
			for (int k = 0; k < 2; k++) {
				m_originalVertices[k + j * 2 + i * 4] = m_middle + m_planes[i] + m_planes[2 + j] + m_planes[4 + k];
			}
		}
	}

	m_normalsNeedsUpdate = true;
	m_verticesNeedsUpdate = true;
}

void Scuffed::Box::setMatrix(const glm::mat4& newMatrix) {
	// Bring back to no transform
	glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
	for (int i = 0; i < 6; i++) {
		m_planes[i] = glm::vec3(glm::inverse(normalMatrix) * m_planes[i]);
	}

	m_middle = glm::vec3(glm::inverse(matrix) * glm::vec4(m_middle, 1.0f));

	matrix = newMatrix;

	normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix)));
	// Apply new transform
	for (int i = 0; i < 6; i++) {
		m_planes[i] = glm::vec3(normalMatrix * m_planes[i]);
	}

	m_middle = glm::vec3(matrix * glm::vec4(m_middle, 1.0f));

	m_normalsNeedsUpdate = true;
	m_verticesNeedsUpdate = true;
}

std::vector<glm::vec3>& Scuffed::Box::getNormals() {
	if (m_normalsNeedsUpdate) {
		// Save normalized vectors
		for (int i = 0; i < 6; i++) {
			m_normals[i] = glm::normalize(m_planes[i]);
		}
		m_normalsNeedsUpdate = false;
	}
	return m_normals;
}

std::vector<glm::vec3>& Scuffed::Box::getEdges() {
	return m_edges; 
}

std::vector<glm::vec3>& Scuffed::Box::getVertices() {
	if (m_verticesNeedsUpdate) {
		// Find corners
		for (int i = 0; i < 8; i++) {
			m_vertices[i] = matrix * glm::vec4(m_originalVertices[i], 1.0f);
		}
		m_verticesNeedsUpdate = false;
	}
	return m_vertices;
}

glm::vec3& Scuffed::Box::getMiddle() {
	return m_middle;
}
