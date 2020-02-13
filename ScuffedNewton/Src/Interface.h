#pragma once

#include "DataStructures/Scene.h"

namespace Scuffed {
	
	namespace Components {
		enum types {
			BoundingBoxComponent,
			CollidableComponent,
			CollisionComponent,
			MeshComponent,
			MovementComponent,
			RagdollComponent,
			SpeedLimitComponent,
			TransformComponent
		};
	}

	class Interface {
	public:
		Interface();
		virtual ~Interface();

		virtual void print();

		virtual void update(float dt);
		virtual int getNewObjectId();
		virtual void loadMesh(int entityId, void* data, size_t size, size_t vertexSize, size_t positionOffset, size_t positionSize);
		virtual void bindModelMatrix(int entityId, glm::mat4** matrix);
		virtual void bindPosition(int entityId, glm::vec3** positionVector);
		virtual bool addComponentToEntity(int entityId, int compType);


	private:
		Scene* m_scene;
	};

}