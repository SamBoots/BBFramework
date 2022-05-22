#pragma once
#include <cstdint>

namespace BB
{
	enum framework_resource_type : uint32_t
	{
		FRAMEWORK_RESOURCE_EMPTY = 0,
		FRAMEWORK_RESOURCE_WINDOW = 1
	};

	union framework_handle
	{
		framework_handle(framework_resource_type a_Type, uint32_t a_Index)
		{
			type = a_Type;
			index = a_Index;
		};
		struct
		{
			framework_resource_type type;
			//The handle index, the index is tracked by the resource_type system.
			uint32_t index;
		};

		uint64_t handle;

		inline bool operator ==(framework_handle a_Rhs) const { return handle == a_Rhs.handle; }
		inline bool operator !=(framework_handle a_Rhs) const { return handle != a_Rhs.handle; }
	};

	#define FRAMEWORK_NULL_HANDLE BB::framework_handle( BB::FRAMEWORK_RESOURCE_EMPTY, 0);
}