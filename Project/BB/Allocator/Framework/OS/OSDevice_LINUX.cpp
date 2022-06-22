#include "pch.h"
#include "OSDevice.h"

#include <unistd.h>

using namespace BB;

static OSDevice osDevice;

OSDevice& BB::AppOSDevice()
{
	return osDevice;
}

OSDevice::OSDevice()
{
	virtualMemoryPageSize = sysconf(_SC_PAGE_SIZE);
}

OSDevice::~OSDevice()
{

}

const uint32_t OSDevice::LatestOSError() const
{
	int t_LatestError = errno;
	if (errno != 0)
	{
		printf("OSDevice Error:");
		printf("%i", t_LatestError);
	}
	return static_cast<uint32_t>(t_LatestError);
}

void OSDevice::ExitApp() const
{
	exit(EXIT_FAILURE);
}