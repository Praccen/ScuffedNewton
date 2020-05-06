#include "Shape.h"

namespace Scuffed {

	Shape::Shape() {
		matrix = glm::mat4(1.0f);
		baseMatrix = glm::mat4(1.0f);
	}

	Shape::~Shape() {

	}

}