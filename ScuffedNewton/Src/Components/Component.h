#pragma once

#include <string>

class Component {
public:
	Component(){};
	virtual ~Component(){};

	std::string ID;
};