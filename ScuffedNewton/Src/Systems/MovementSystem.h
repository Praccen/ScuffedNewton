#pragma once
#include "BaseSystem.h"

class MovementSystem: public BaseSystem {
public:
	MovementSystem();
	~MovementSystem() = default;

	void update(float dt);
};