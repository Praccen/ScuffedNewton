#pragma once

#include <vector>
#include <glm/glm.hpp>

namespace Scuffed {

	class Shape {
	public:
		Shape();
		virtual ~Shape();

		virtual std::vector<glm::vec3>& getNormals() = 0;
		virtual std::vector<glm::vec3>& getEdges() = 0;
		virtual std::vector<glm::vec3>& getVertices() = 0;
		virtual glm::vec3& getMiddle() = 0;
	};

}