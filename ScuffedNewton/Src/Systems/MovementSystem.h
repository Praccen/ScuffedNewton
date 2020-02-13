#pragma once
#include "BaseSystem.h"

namespace Scuffed {

	class MovementSystem : public BaseSystem {
	public:
		MovementSystem();
		~MovementSystem() = default;

		void update(float dt);
	};

}