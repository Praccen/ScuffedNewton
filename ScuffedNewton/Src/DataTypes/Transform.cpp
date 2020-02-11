#include "../pch.h"
#include "Transform.h"

Transform::Transform() {
	m_matrix = glm::mat4(1.0f);

	m_translation = { 0.f, 0.f, 0.f };
	m_rotation = { 0.f, 0.f, 0.f };
	m_scale = { 1.f, 1.f, 1.f };
	m_center = { 0.f, 0.f, 0.f };
	m_hasChanged = 2;
}

Transform::~Transform() {

}

void Transform::setTranslation(glm::vec3 translation) {
	m_translation = translation;
	m_hasChanged |= 1;
}

void Transform::setScale(glm::vec3 scale) {
	m_scale = scale;
	m_hasChanged |= 2;
}

void Transform::setCenter(glm::vec3 center) {
	m_center = center;
	m_hasChanged |= 2;
}

void Transform::translate(glm::vec3 translation) {
	m_translation += translation;
	m_hasChanged |= 1;
}

void Transform::rotate(glm::vec3 rotation) {
	m_rotation += rotation;
	m_hasChanged |= 2;
}

glm::mat4 Transform::getMatrixWithUpdate() {
	return glm::mat4();
}

glm::mat4 Transform::getMatrixWithoutUpdate() {
	return m_matrix;
}

glm::vec3 Transform::getTranslation() const {
	return m_translation;
}

void Transform::prepareUpdate() {
	//Call this in the beginning of each frame/tick
	m_hasChanged = 0;
}

void Transform::bindMatrixPointer(glm::mat4** matrix) {
	*matrix = &m_matrix;
}

void Transform::bindPositionPointer(glm::vec3** position) {
	*position = &m_translation;
}

void Transform::updateMatrix() {
	m_matrix = glm::mat4(1.0f);

	m_matrix = glm::translate(m_matrix, m_translation);

	m_matrix = glm::translate(m_matrix, m_center);
	//m_matrix *= glm::toMat4(m_rotation);
	m_matrix = glm::rotate(m_matrix, 1.f, m_rotation);
	m_matrix = glm::translate(m_matrix, -m_center);

	m_matrix = glm::scale(m_matrix, m_scale);
}

const int Transform::getChange() {
	return m_hasChanged;
}


