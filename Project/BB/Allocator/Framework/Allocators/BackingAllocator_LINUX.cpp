#include "pch.h"
#include "BackingAllocator.h"
#include "Utils/PointerUtils.h"
#include "OS/OSDevice.h"

#include <sys/mman.h>

using namespace BB;

struct PageHeader
{
	size_t bytesCommited;
	size_t bytesUsed;
	void* reserveSpot;
};

constexpr const size_t PREALLOC_PAGEHEADERS = 128 * 16;
constexpr const size_t PREALLOC_PAGEHEADERS_COMMIT_SIZE = PREALLOC_PAGEHEADERS * sizeof(PageHeader);

struct StartPageHeader
{
	PageHeader* head;
};

static size_t RoundUp(size_t a_NumToRound, size_t a_Multiple)
{
	BB_ASSERT(a_Multiple, "Multiple is 0!");
	return ((a_NumToRound + a_Multiple - 1) / a_Multiple) * a_Multiple;
}

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
	size_t t_OverCommitValue = RoundUp(a_Size, AppOSDevice().virtualMemoryPageSize) * static_cast<size_t>(a_ReserveSize);

	StartPageHeader* t_StartPageHeader = nullptr;
	PageHeader* t_PageHeader = nullptr;

	//Check the existing page header.
	if (a_Start != nullptr)
	{
		t_StartPageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Start, sizeof(StartPageHeader)));
		t_PageHeader = t_StartPageHeader->head;

		void* t_CurrentEnd = pointerutils::Add(t_PageHeader->reserveSpot, t_PageHeader->bytesUsed);


		//If the amount commited is enough just move the pointer and return it.
		if ((t_PageHeader->bytesCommited - t_PageHeader->bytesUsed) >= a_Size)
		{
			t_PageHeader->bytesUsed += a_Size;
			return t_CurrentEnd;
		}
	}

	//New allocation so mmap some new memory on that nullptr.
	void* t_VirtualAddress = mmap(a_Start, t_OverCommitValue,
		PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	BB_ASSERT(t_VirtualAddress != nullptr, "mmap returned 0 creating a new startheader mallocVirtual, MAP_FAILED.");

	//Set the header.
	t_StartPageHeader = reinterpret_cast<StartPageHeader*>(t_VirtualAddress);
	t_StartPageHeader->head = nullptr;

	PageHeader* t_NewHeader = reinterpret_cast<PageHeader*>(pagePool.AllocHeader());
	t_NewHeader->bytesCommited = t_OverCommitValue;
	t_NewHeader->bytesUsed = a_Size + sizeof(StartPageHeader);
	//Reserve spot is in front of the StartPageHeader.
	t_NewHeader->reserveSpot = pointerutils::Add(t_VirtualAddress, sizeof(StartPageHeader));
	t_StartPageHeader->head = t_NewHeader; // Set the new header as the head.

	//Return the pointer that does not include the StartPageHeader
	return t_StartPageHeader->head->reserveSpot;
}

void BB::freeVirtual(void* a_Ptr)
{
	PageHeader* t_PageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Ptr, sizeof(StartPageHeader)))->head;
	BB_ASSERT(munmap(pointerutils::Subtract(t_PageHeader->reserveSpot, sizeof(StartPageHeader)), t_PageHeader->bytesCommited) == 0, "munmap error on freeVirtual function.");
}