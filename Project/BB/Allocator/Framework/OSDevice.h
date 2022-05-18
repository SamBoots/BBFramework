#pragma once
#include <cstdint>

namespace BB
{
	struct OSDevice
	{
		OSDevice();
		~OSDevice();

		//just delete these for safety, copies might cause errors.
		OSDevice(const OSDevice&) = delete;
		OSDevice(const OSDevice&&) = delete;
		OSDevice& operator =(const OSDevice&) = delete;
		OSDevice& operator =(OSDevice&&) = delete;

		//The size of a virtual memory page on the OS or the ALlocationGranularity.
		size_t virtualMemoryPageSize;

		//Prints the latest OS error and returns the error code, if it has no error code it returns 0.
		const uint32_t LatestOSError() const;

		//Exits the application.
		void ExitApp() const;
	};

	const OSDevice& AppOSDevice();
}