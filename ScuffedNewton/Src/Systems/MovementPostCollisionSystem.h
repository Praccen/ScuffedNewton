#pragma once
#include "BaseSystem.h"

namespace Scuffed {

	class MovementPostCollisionSystem : public BaseSystem {
	public:
		MovementPostCollisionSystem();
		~MovementPostCollisionSystem() = default;

		void update(float dt);
	};

}