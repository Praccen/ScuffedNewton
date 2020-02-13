#include "Interface.h"

#include <iostream>

#include "DataTypes/Entity.h"
#include "Components/Components.h"
#include "DataTypes/Mesh.h"

namespace Scuffed {

	Interface::Interface() {
		m_scene = SN_NEW Scene();
	}

	Interface::~Interface() {
		delete m_scene;
	}

	void Interface::print() {
		std::cout << "Scuffed newton interface\n";
	}

	void Interface::update(float dt) {
		m_scene->update(dt);
	}

	int Interface::getNewObjectId() {
		return m_scene->addEntity();
	}

	void Interface::loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize) {
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

	void Interface::bindModelMatrix(int entityId, glm::mat4** matrix) {
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

	void Interface::bindPosition(int entityId, glm::vec3** positionVector) {
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

	Entity* Interface::getEntity(int entityId) {
		return m_scene->getEntity(entityId);
	}

	Scene* Interface::getScene() {
		return m_scene;
	}

}