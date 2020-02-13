#pragma once
#include "BaseSystem.h"

namespace Scuffed {

	class SpeedLimitSystem final : public BaseSystem {
	public:
		SpeedLimitSystem();
		~SpeedLimitSystem();

		void update(float dt);
	};

}