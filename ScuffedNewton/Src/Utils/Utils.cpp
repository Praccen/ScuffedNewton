#include "../pch.h"

#include "Utils.h"

Utils::Utils() {
	m_entityIdCounter = 0;
}

Utils::~Utils() {

}

Utils* Utils::instance() {
	static Utils instance;
	return &instance;
}

int Utils::GetEntityIdCounter(bool increase) {
	if (increase) {
		return m_entityIdCounter++;
	}
	else {
		return m_entityIdCounter;
	}
}
