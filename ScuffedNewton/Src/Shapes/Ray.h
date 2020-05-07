#pragma once

#include "Shape.h"

namespace Scuffed {

	class Ray : public Shape {
	public:
		Ray(const glm::vec3& start, const glm::vec3& direction);
		virtual ~Ray();

		virtual void setBaseMatrix(const glm::mat4& newBaseMatrix);
		virtual void setMatrix(const glm::mat4& newMatrix);
		virtual std::vector<glm::vec3>& getNormals();
		virtual std::vector<glm::vec3>& getEdges();
		virtual std::vector<glm::vec3>& getVertices();
		virtual glm::vec3& getMiddle();

	private:
		std::vector<glm::vec3> m_vertices;
		std::vector<glm::vec3> m_normals;
		std::vector<glm::vec3> m_edges;
	};

}