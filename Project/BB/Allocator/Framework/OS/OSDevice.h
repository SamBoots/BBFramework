#pragma once
#include <cstdint>
#include "Common.h"

namespace BB
{
	class OSWindow;

	class OSDevice
	{
	public:
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

		framework_handle CreateOSWindow(int a_X, int a_Y, int a_Width, int a_Height, const char* a_WindowName);

		//Exits the application.
		void ExitApp() const;

		bool ProcessMessages() const;

	private:
		OSWindow* window;
	};

	OSDevice& AppOSDevice();
}