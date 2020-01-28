#pragma once

#include <vector>
#include <unordered_map>

class Entity;

class BaseSystem {
public:
	BaseSystem();
	~BaseSystem();

	virtual bool addEntity(Entity* entity);

	virtual void removeEntity(Entity* entity);

	virtual void update(float dt);

protected:
	std::vector<Entity*> entities;
	std::unordered_map<std::string, bool> requiredComponents;
private:


};