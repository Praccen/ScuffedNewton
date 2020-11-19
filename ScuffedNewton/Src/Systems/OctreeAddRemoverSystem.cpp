#include "../pch.h"
#include "OctreeAddRemoverSystem.h"

#include "../DataTypes/Entity.h"

#include "../components/Components.h"

namespace Scuffed {

	OctreeAddRemoverSystem::OctreeAddRemoverSystem() {
		requiredComponents["BoundingBoxComponent"] = true;
		requiredComponents["PhysicalBodyComponent"] = true;

		m_octree = nullptr;
	}

	OctreeAddRemoverSystem::~OctreeAddRemoverSystem() {

	}

	void OctreeAddRemoverSystem::provideOctree(Octree* octree) {
		m_octree = octree;
		m_octree->addEntities(&entities, 0.f);
	}

	bool OctreeAddRemoverSystem::addEntity(Entity* entity) {
		if (BaseSystem::addEntity(entity)) {
			if (m_octree) {
				m_octree->addEntity(entity, 0.f);
				return true;
			}
		}
		return false;
	}

	void OctreeAddRemoverSystem::removeEntity(Entity* entity) {
		BaseSystem::removeEntity(entity);

		if (m_octree) {
			m_octree->removeEntity(entity);
		}
	}


	void OctreeAddRemoverSystem::update(float dt) {
		m_octree->update(dt);
	}

}