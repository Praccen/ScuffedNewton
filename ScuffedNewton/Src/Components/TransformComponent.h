#pragma once

#include <glm/glm.hpp>

#include "Component.h"
#include "../DataTypes/Transform.h"

class TransformComponent : public Component, public Transform {
public:
	TransformComponent();
	~TransformComponent();

	static std::string ID;
};