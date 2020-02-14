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
			CollidableComponent,
			CollisionComponent,
			MovementComponent,
			MeshComponent,
			RagdollComponent,
			SpeedLimitComponent,
			TransformComponent
		}; // Change in switch cases in entity virtual functions if this changes
	}

	class Utils {
	public:
		Utils();
		~Utils();

		static Utils* instance();

		template<typename ComponentType>
		ComponentType getTypeFromInt(int componentNumber);

		int GetEntityIdCounter(bool increase = false);
	private:
		int m_entityIdCounter;
	};

}