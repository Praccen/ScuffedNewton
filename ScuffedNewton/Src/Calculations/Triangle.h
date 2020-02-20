#pragma once

#include "Shape.h"

namespace Scuffed {

	class Triangle : public Shape {
	public:
		Triangle(const glm::vec3 vertices[3]);
		virtual ~Triangle();

		virtual std::vector<glm::vec3>& getNormals();
		virtual std::vector<glm::vec3>& getEdges();
		virtual std::vector<glm::vec3>& getVertices();
		virtual glm::vec3& getMiddle();

	private:
		std::vector<glm::vec3> m_vertices;
		std::vector<glm::vec3> m_edges;
		std::vector<glm::vec3> m_normals;
		glm::vec3 m_middle;
	};

}