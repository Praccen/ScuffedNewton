#pragma once

#include "DataStructures/Scene.h"

namespace Scuffed {

	class Interface {
	public:
		Interface();
		virtual ~Interface();

		virtual void print();

		virtual void update(float dt);
		virtual int getNewEntityID();
		virtual void loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize);
		virtual void bindModelMatrix(int entityId, glm::mat4** matrix);
		virtual void bindPosition(int entityId, glm::vec3** positionVector);

		virtual Entity* getEntity(int entityId);
		virtual Scene* getScene();

	private:
		Scene* m_scene;
	};

}