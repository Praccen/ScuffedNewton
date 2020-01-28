#pragma once
#include "BaseSystem.h"

class MovementPostCollisionSystem : public BaseSystem {
public:
	MovementPostCollisionSystem();
	~MovementPostCollisionSystem() = default;

	void update(float dt);
private:

};
