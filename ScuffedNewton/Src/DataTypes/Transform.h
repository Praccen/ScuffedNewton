#pragma once

#include <glm/glm.hpp>

namespace Scuffed {

	class Transform {
	public:
		Transform();
		~Transform();

		virtual void setTranslation(const glm::vec3& translation);
		virtual void setScale(const glm::vec3& scale);
		virtual void setCenter(const glm::vec3& center);

		virtual void translate(const glm::vec3& translation);
		virtual void rotate(const glm::vec3& rotation);

		virtual glm::mat4 getMatrixWithUpdate();
		virtual glm::mat4 getMatrixWithoutUpdate();

		virtual glm::vec3 getTranslation() const;
		virtual glm::vec3 getCenter() const;

		virtual void prepareUpdate();

		virtual void bindMatrixPointer(glm::mat4** matrix);
		virtual void bindPositionPointer(glm::vec3** position);

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