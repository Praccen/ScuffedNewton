#pragma once

#include <vector>

class Entity;

class BaseSystem {
public:
	BaseSystem();
	~BaseSystem();

protected:
	std::vector<Entity*> entities;

private:


};