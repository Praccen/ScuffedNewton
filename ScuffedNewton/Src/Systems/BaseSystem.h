#pragma once

#include <vector>
#include <unordered_map>

namespace Scuffed {

	class Entity;

	class BaseSystem {
	public:
		BaseSystem();
		~BaseSystem();

		virtual bool addEntity(Entity* entity);

		virtual void removeEntity(Entity* entity);

		virtual void update(float dt);

		virtual std::unordered_map<std::string, bool>& getRequiredComponentTypes();

	protected:
		std::vector<Entity*> entities;
		std::unordered_map<std::string, bool> requiredComponents;
	private:


	};

}