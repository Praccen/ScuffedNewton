#pragma once
#include "Component.h"

namespace Scuffed {

	class SpeedLimitComponent final : public Component {
	public:
		SpeedLimitComponent(float maxSpeed_ = 9999999.0f);
		~SpeedLimitComponent();

		float maxSpeed;
		float normalMaxSpeed;
		static std::string ID;
	};

}