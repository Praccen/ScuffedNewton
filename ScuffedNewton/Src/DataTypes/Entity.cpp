#include "../pch.h"

#include "Entity.h"
#include "BoundingBox.h"

#include "../Utils/Utils.h"

Entity::Entity() {
	m_id = Utils::sEntityIdCounter++;
	m_boundingBox = SN_NEW BoundingBox();
	m_hasModel = false;
	m_allowSimpleCollision = false;
}

Entity::~Entity() {
	delete m_boundingBox;
}

unsigned int Entity::getId() const {
	return m_id;
}

BoundingBox* Entity::getBoundingBox() const {
	return m_boundingBox;
}

bool Entity::hasModel() const{
	return m_hasModel;
}

bool Entity::allowSimpleCollision() const {
	return m_allowSimpleCollision;
}