#pragma once

#include <glm/glm.hpp>

namespace Scuffed {

	class Transform {
	public:
		Transform();
		~Transform();

		void setTranslation(glm::vec3 translation);
		void setScale(glm::vec3 scale);
		void setCenter(glm::vec3 center);

		void translate(glm::vec3 translation);
		void rotate(glm::vec3 rotation);

		glm::mat4 getMatrixWithUpdate();
		glm::mat4 getMatrixWithoutUpdate();

		glm::vec3 getTranslation() const;

		void prepareUpdate();

		void bindMatrixPointer(glm::mat4** matrix);
		void bindPositionPointer(glm::vec3** position);

	private:
		void updateMatrix();

	private:
		glm::mat4 m_matrix;
		glm::vec3 m_translation;
		glm::vec3 m_rotation;
		glm::vec3 m_scale;

		glm::vec3 m_center;

		int m_hasChanged;

	private:
		friend class UpdateBoundingBoxSystem;
		const int getChange(); //Only access this from UpdateBoundingBoxSystem::update()
	};

}