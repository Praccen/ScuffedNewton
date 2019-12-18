#pragma once

// Macro to easier track down memory leaks
#ifdef _DEBUG
#define SN_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define SN_NEW new
#endif

namespace Utils {
	static int sEntityIdCounter = 0;
}