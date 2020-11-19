#pragma once
#include "BaseSystem.h"

namespace Scuffed {

	class Octree;
	class Camera;

	class OctreeAddRemoverSystem : public BaseSystem {
	public:
		OctreeAddRemoverSystem();
		~OctreeAddRemoverSystem();

		void provideOctree(Octree* octree);

		bool addEntity(Entity* entity) override;

		void removeEntity(Entity* entity) override;

		void update(float dt) override;

	private:
		Octree* m_octree;
	};

}