#include "../pch.h"
#include "Box.h"

namespace Scuffed {

	//Box::Box(const Box& otherBox) {
	//	m_originalVertices		= otherBox.m_originalVertices;
	//	m_planes				= otherBox.m_planes;
	//	m_vertices				= otherBox.m_vertices;
	//	m_normals				= otherBox.m_normals;
	//	m_edges					= otherBox.m_edges;
	//	m_middle				= otherBox.m_middle;
	//	m_normalsNeedsUpdate	= otherBox.m_normalsNeedsUpdate;
	//	m_verticesNeedsUpdate	= otherBox.m_verticesNeedsUpdate;
	//	baseMatrix				= otherBox.baseMatrix;
	//	matrix					= otherBox.matrix;
	//}

	Box::Box(const glm::vec3& halfSize, const glm::vec3& origin) {
		init();

		m_originalMiddle = origin;
		setHalfSize(halfSize);
	}

	Box::Box(glm::vec3 planes[6], const glm::vec3& origin) {
		init();

		m_originalMiddle = origin;
		setPlanesFromOrigin(planes);
	}

	Box::~Box() {

	}

	void Box::init() {
		baseMatrix = glm::mat4(1.0f);
		matrix = glm::mat4(1.0f);
		m_originalMiddle = { 0.f, 0.f, 0.f };

		m_edges.resize(0);
		m_originalPlanes.resize(6);
		m_originalVertices.resize(8);
		m_normals.resize(6);
		m_vertices.resize(8);

		m_hasChanged = false;
	}

	void Box::updateVertices() {
		for (int i = 0; i < 8; i++) {
			m_vertices[i] = matrix * baseMatrix * glm::vec4(m_originalMiddle + m_originalVertices[i], 1.0f);
		}
	}

	void Box::updateNormals() {
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(matrix * baseMatrix)));
		for (int i = 0; i < 6; i++) {
			m_normals[i] = glm::normalize(glm::vec3(normalMatrix * m_originalPlanes[i]));
		}
	}

	void Box::setUpdatesNeeded() {
		m_normalsNeedsUpdate = true;
		m_verticesNeedsUpdate = true;
		m_middleNeedsUpdate = true;
		m_hasChanged = true;
	}

	void Box::setHalfSize(const glm::vec3& halfSize) {
		glm::vec3 planes[6] = {
				{halfSize.x, 0.f, 0.f},
				{-halfSize.x, 0.f, 0.f},
				{0.f, halfSize.y, 0.f},
				{0.f, -halfSize.y, 0.f},
				{0.f, 0.f, halfSize.z},
				{0.f, 0.f, -halfSize.z}
		};

		setPlanesFromOrigin(planes);
	}

	void Box::setOrigin(const glm::vec3& origin) {
		m_originalMiddle = origin;
		setUpdatesNeeded();
	}

	void Box::setTranslation(const glm::vec3& translation) {
		setBaseMatrix(glm::translate(glm::mat4(1.f), translation));
	}

	void Box::translate(const glm::vec3& translation) {
		setBaseMatrix(glm::translate(baseMatrix, translation));
	}

	void Box::setPlanesFromOrigin(glm::vec3 planes[6]) {
		for (int i = 0; i < 6; i++) {
			m_originalPlanes[i] = planes[i];
		}

		// Assumes orthogonal
		// Sort planes, oposite sides indexed next to each other
		for (unsigned int i = 0; i < 6; i += 2) {
			for (unsigned int j = i + 1; j < 6; j++) {
				if (glm::dot(m_originalPlanes[i], m_originalPlanes[j]) < -0.001f) {
					// Opposite found, swap j and i+1 if not already correct
					if (j != i + 1) {
						glm::vec3 tempVec = m_originalPlanes[i + 1];
						m_originalPlanes[i + 1] = m_originalPlanes[j];
						m_originalPlanes[j] = tempVec;
					}
				}
			}
		}

		// Find corners
		for (int i = 0; i < 2; i++) {
			for (int j = 0; j < 2; j++) {
				for (int k = 0; k < 2; k++) {
					m_originalVertices[k + j * 2 + i * 4] = m_originalPlanes[i] + m_originalPlanes[2 + j] + m_originalPlanes[4 + k];
				}
			}
		}

		setUpdatesNeeded();
	}

	void Box::setBaseMatrix(const glm::mat4& newBaseMatrix) {
		baseMatrix = newBaseMatrix;

		setUpdatesNeeded();
	}

	void Box::setMatrix(const glm::mat4& newMatrix) {
		matrix = newMatrix;

		setUpdatesNeeded();
	}

	std::vector<glm::vec3>& Box::getNormals() {
		if (m_normalsNeedsUpdate) {
			updateNormals();
			m_normalsNeedsUpdate = false;
		}
		return m_normals;
	}

	std::vector<glm::vec3>& Box::getEdges() {
		//return getNormals(); 
		return m_edges;
	}

	std::vector<glm::vec3>& Box::getVertices() {
		if (m_verticesNeedsUpdate) {
			updateVertices();
			m_verticesNeedsUpdate = false;
		}
		return m_vertices;
	}

	glm::vec3& Box::getMiddle() {
		if (m_middleNeedsUpdate) {
			m_middle = glm::vec3(matrix * baseMatrix * glm::vec4(m_originalMiddle, 1.0f));
		}
		return m_middle;
	}

	const bool Box::getChange() {
		bool theChange = m_hasChanged;
		m_hasChanged = false;
		return theChange;
	}
}