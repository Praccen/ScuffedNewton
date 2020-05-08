#pragma once

#include "Shape.h"

namespace Scuffed {

	class Box : public Shape {
	public:
		//Box(const Box& otherBox);
		Box(const glm::vec3& halfSize, const glm::vec3& origin);
		Box(glm::vec3 planes[6], const glm::vec3& origin); //Assumes Orthogonal
		virtual ~Box();

		virtual void setHalfSize(const glm::vec3& halfSize);
		virtual void setOrigin(const glm::vec3& origin);
		virtual void setTranslation(const glm::vec3& translation);
		virtual void setPlanesFromOrigin(glm::vec3 planes[6]);

		virtual void setBaseMatrix(const glm::mat4& newBaseMatrix);
		virtual void setMatrix(const glm::mat4& newMatrix);
		virtual std::vector<glm::vec3>& getNormals();
		virtual std::vector<glm::vec3>& getEdges();
		virtual std::vector<glm::vec3>& getVertices();
		virtual glm::vec3& getMiddle();

	private:
		void init();
		void updateVertices();
		void updateNormals();

	private:
		std::vector<glm::vec3> m_originalVertices; // Not effected by matrices
		std::vector<glm::vec3> m_originalPlanes; // Not effected by matrices
		glm::vec3 m_originalMiddle; // Not effected by matrices

		std::vector<glm::vec3> m_vertices;
		std::vector<glm::vec3> m_normals;
		glm::vec3 m_middle;

		std::vector<glm::vec3> m_edges;
		

		bool m_normalsNeedsUpdate;
		bool m_verticesNeedsUpdate;
		bool m_middleNeedsUpdate;
	};

}