#include "pch.h"
#include "OSDevice.h"
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

using namespace BB;

static OSDevice osDevice;

const OSDevice& BB::AppOSDevice()
{
	return osDevice;
}

OSDevice::OSDevice()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	virtualMemoryPageSize = t_SystemInfo.dwAllocationGranularity;
}

OSDevice::~OSDevice()
{

}

const uint32_t OSDevice::LatestOSError() const
{
	DWORD t_LatestError = GetLastError();
	if (t_LatestError != 0x00)
	{
		printf("OSDevice Error:");
		printf("%u", t_LatestError);
	}

	return static_cast<uint32_t>(t_LatestError);
}

void OSDevice::ExitApp() const
{
	exit(EXIT_FAILURE);
}