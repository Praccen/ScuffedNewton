#include "Interface.h"

#include <iostream>

#include "DataTypes/Entity.h"
#include "Components/Components.h"
#include "DataTypes/Mesh.h"

Scuffed::Interface::Interface() {
	m_scene = SN_NEW Scene();
}

Scuffed::Interface::~Interface() {
	delete m_scene;
}

void Scuffed::Interface::print() {
	std::cout << "Scuffed newton interface\n";
}

void Scuffed::Interface::update(float dt) {
	m_scene->update(dt);
}

int Scuffed::Interface::getNewObjectId() {
	return m_scene->addEntity();
}

void Scuffed::Interface::loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
	//Find entity
	Entity* e = m_scene->getEntity(entityId);
	if (e) {
		MeshComponent* comp = e->getComponent<MeshComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Load data
			comp->mesh->loadData(data, size, vertexSize, positionOffset, positionSize);
		}
		else {
			// Create component and load data
			comp = e->addComponent<MeshComponent>();
			comp->mesh->loadData(data, size, vertexSize, positionOffset, positionSize);
		}
	}
}

void Scuffed::Interface::bindModelMatrix(int entityId, glm::mat4** matrix) {
	//Find entity
	Entity* e = m_scene->getEntity(entityId);
	if (e) {
		TransformComponent* comp = e->getComponent<TransformComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Bind pointer
			comp->bindMatrixPointer(matrix);
		}
		else {
			// Create component and bind pointer
			comp = e->addComponent<TransformComponent>();
			comp->bindMatrixPointer(matrix);
		}
	}
}

void Scuffed::Interface::bindPosition(int entityId, glm::vec3** positionVector) {
	//Find entity
	Entity* e = m_scene->getEntity(entityId);
	if (e) {
		TransformComponent* comp = e->getComponent<TransformComponent>();
		// Check if entity has mesh component
		if (comp) {
			// Bind pointer
			comp->bindPositionPointer(positionVector);
		}
		else {
			// Create component and bind pointer
			comp = e->addComponent<TransformComponent>();
			comp->bindPositionPointer(positionVector);
		}
	}
}

bool Scuffed::Interface::addComponentToEntity(int entityId, int compType) {
	Entity* e = m_scene->getEntity(entityId);
	if (e) {
		switch (compType) {
		case Components::BoundingBoxComponent: e->addComponent<BoundingBoxComponent>();  break;
		case Components::CollidableComponent: e->addComponent<CollidableComponent>();  break;
		case Components::CollisionComponent: e->addComponent<CollisionComponent>();  break;
		case Components::MeshComponent: e->addComponent<MeshComponent>();  break;
		case Components::MovementComponent: e->addComponent<MovementComponent>();  break;
		case Components::RagdollComponent: e->addComponent<RagdollComponent>(); break;
		case Components::SpeedLimitComponent: e->addComponent<SpeedLimitComponent>();  break;
		case Components::TransformComponent: e->addComponent<TransformComponent>(); break;
		default: break;
		}
		return true;
	}
	return false;
}
