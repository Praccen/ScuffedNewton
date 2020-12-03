#pragma once

#include "../Components/Components.h"

// Macro to easier track down memory leaks
#ifdef _DEBUG
#define SN_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
#define SN_NEW new
#endif

namespace Scuffed {

	namespace Components {
		enum types {
			BoundingBoxComponent,
			MeshComponent,
			PhysicalBodyComponent,
			TransformComponent
		}; // Change in switch cases in entity virtual functions if this changes
	}

	class Utils {
	public:
		Utils();
		~Utils();

		static Utils* instance();

		int GetEntityIdCounter(bool increase = false);

		float epsilon;
	private:
		int m_entityIdCounter;
	};

}