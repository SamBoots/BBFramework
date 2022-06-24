#pragma once
#include <cstdint>

namespace BB
{
	enum class FRAMEWORK_RESOURCE_TYPE : uint32_t
	{
		EMPTY = 0,
		WINDOW = EMPTY + 1
	};

	union framework_handle
	{
		framework_handle(FRAMEWORK_RESOURCE_TYPE a_Type, uint32_t a_Index)
		{
			type = a_Type;
			index = a_Index;
		};
		struct
		{
			FRAMEWORK_RESOURCE_TYPE type;
			//The handle index, the index is tracked by the resource_type system.
			uint32_t index;
		};

		uint64_t handle;

		inline bool operator ==(framework_handle a_Rhs) const { return handle == a_Rhs.handle; }
		inline bool operator !=(framework_handle a_Rhs) const { return handle != a_Rhs.handle; }
	};

	#define FRAMEWORK_NULL_HANDLE BB::framework_handle( BB::FRAMEWORK_RESOURCE_TYPE::EMPTY, 0);
}