#include "pch.h"
#include "BackingAllocator.h"
#include "Utils/PointerUtils.h"
#include "Utils/Math.h"
#include "OS/OSDevice.h"

#include <sys/mman.h>

using namespace BB;

struct PageHeader
{
	size_t bytes_reserved;
	size_t bytes_commited;
	void* reserve_spot;
};

struct StartPageHeader
{
	PageHeader* header;
};

constexpr const size_t PREALLOC_PAGEHEADERS = 128 * 16;
constexpr const size_t PREALLOC_PAGEHEADERS_COMMIT_SIZE = PREALLOC_PAGEHEADERS * sizeof(PageHeader);

static PagePool pagePool{};

PagePool::PagePool()
{
	bufferStart = mmap(nullptr, PREALLOC_PAGEHEADERS_COMMIT_SIZE,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	BB_ASSERT(bufferStart, "mmap returned 0 on creating the pagePool, MAP_FAILED.");
	pool = reinterpret_cast<void**>(bufferStart);

	void** t_Pool = pool;
	for (size_t i = 0; i < PREALLOC_PAGEHEADERS - 1; i++)
	{
		*t_Pool = pointerutils::Add(t_Pool, sizeof(PageHeader));
		t_Pool = reinterpret_cast<void**>(*t_Pool);
	}
	*t_Pool = nullptr;
}

PagePool::~PagePool()
{
	BB_ASSERT(munmap(bufferStart, PREALLOC_PAGEHEADERS_COMMIT_SIZE) == 0, "munmap error on PagePool deconstructor.");
}

void* BB::mallocVirtual(void* a_Start, size_t a_Size, virtual_reserve_extra a_ReserveSize)
{
	size_t t_PageAdjustedSize = Math::RoundUp(a_Size + sizeof(StartPageHeader), AppOSDevice().virtual_memory_minimum_allocation);

	StartPageHeader* t_StartPageHeader = nullptr;
	PageHeader* t_PageHeader = nullptr;

	//Check the existing page header.
	if (a_Start != nullptr)
	{
		t_StartPageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Start, sizeof(StartPageHeader)));
		t_PageHeader = t_StartPageHeader->header;

		void* t_CurrentEnd = pointerutils::Add(t_PageHeader->reserve_spot, t_PageHeader->bytes_commited);

		//If the amount commited is not enough check if there is enough reserved, if not reserve more.
		if (t_PageHeader->bytes_reserved > t_PageAdjustedSize + t_PageHeader->bytes_commited)
		{
			t_PageHeader->bytes_commited += t_PageAdjustedSize;
			mprotect(t_PageHeader->reserve_spot,
				t_PageAdjustedSize + t_PageHeader->bytes_commited,
				PROT_READ | PROT_WRITE);
			BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Linux API error mprotect.");
			return t_CurrentEnd;
		}

		BB_ASSERT(false, "Going over reserved memory!")
	}

	size_t t_AdditionalReserve = t_PageAdjustedSize * static_cast<size_t>(a_ReserveSize);
	//New allocation so mmap some new memory on that nullptr.
	//The prot is PROT_NONE so that it cannot be accessed, this will reflect reserving memory like the VirtualAlloc call from windows.
	void* t_Address = mmap(a_Start, t_AdditionalReserve,
		PROT_NONE, 
		MAP_PRIVATE | MAP_ANONYMOUS, 
		0, 
		0);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Linux API error mmap.");
	//Instead of VirtualAlloc and commiting memory we will just remove the protection.
	mprotect(t_Address, t_PageAdjustedSize, PROT_READ | PROT_WRITE);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Linux API error mprotect.");
	//Set the header.
	t_StartPageHeader = reinterpret_cast<StartPageHeader*>(t_Address);
	PageHeader* t_NewHeader = reinterpret_cast<PageHeader*>(pagePool.AllocHeader());
	t_NewHeader->bytes_commited = t_PageAdjustedSize;
	t_NewHeader->bytes_reserved = t_AdditionalReserve;
	t_NewHeader->reserve_spot = t_Address;
	t_StartPageHeader->header = t_NewHeader; // Set the new header as the head.

	//Return the pointer that does not include the StartPageHeader
	return pointerutils::Add(t_Address, sizeof(StartPageHeader));
}

void BB::freeVirtual(void* a_Ptr)
{
	PageHeader* t_PageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Ptr, sizeof(StartPageHeader)))->header;
	munmap(t_PageHeader->reserve_spot,
		t_PageHeader->bytes_reserved);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Linux API error munmap.");
}