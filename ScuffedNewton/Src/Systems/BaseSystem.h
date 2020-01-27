#pragma once

#include <vector>
#include <unordered_map>

class Entity;

class BaseSystem {
public:
	BaseSystem();
	~BaseSystem();

protected:
	std::vector<Entity*> entities;
	std::unordered_map<std::string, bool> requiredComponents;
private:


};