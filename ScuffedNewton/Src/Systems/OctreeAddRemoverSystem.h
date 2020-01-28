#pragma once
#include "BaseSystem.h"

class Octree;
class Camera;

class OctreeAddRemoverSystem: public BaseSystem {
public:
	OctreeAddRemoverSystem();
	~OctreeAddRemoverSystem();

	void provideOctree(Octree* octree);

	bool addEntity(Entity* entity) override;

	void removeEntity(Entity* entity) override;

	void update(float dt) override;
	void updatePerFrame(float dt);

	void setCulling(bool activated, Camera* camera);

private:
	Octree* m_octree;
	bool m_doCulling;
	Camera* m_cullCamera;
};