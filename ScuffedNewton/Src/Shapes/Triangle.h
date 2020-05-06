#pragma once

#include "Shape.h"

namespace Scuffed {

	class Triangle : public Shape {
	public:
		Triangle(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);
		virtual ~Triangle();

		virtual void setData(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3);

		virtual void setBaseMatrix(const glm::mat4& newBaseMatrix);
		virtual void setMatrix(const glm::mat4& newMatrix);
		virtual std::vector<glm::vec3>& getNormals();
		virtual std::vector<glm::vec3>& getEdges();
		virtual std::vector<glm::vec3>& getVertices();
		virtual glm::vec3& getMiddle();

	private:
		std::vector<glm::vec3> m_vertices;
		std::vector<glm::vec3> m_edges;
		std::vector<glm::vec3> m_normals;
		glm::vec3 m_middle;

		bool m_edgesNeedsUpdate;
		bool m_normalsNeedsUpdate;
		bool m_middleNeedsUpdate;
	};

}