#pragma once

// Macro to easier track down memory leaks
#ifdef _DEBUG
#define SN_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define SN_NEW new
#endif

namespace Scuffed {

	class Utils {
	public:
		Utils();
		~Utils();

		static Utils* instance();

		int GetEntityIdCounter(bool increase = false);
	private:
		int m_entityIdCounter;
	};

}