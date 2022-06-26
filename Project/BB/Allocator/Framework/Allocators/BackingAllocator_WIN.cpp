#include "pch.h"
#include "BackingAllocator.h"
#include "Utils/pointerUtils.h"
#include "Utils/Math.h"
#include "OS/OSDevice.h"

#include <Windows.h>
#include <memoryapi.h>

using namespace BB;



struct PageHeader
{
	size_t bytes_commited;
	size_t bytes_reserved;
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
	
	bufferStart = VirtualAlloc(bufferStart, PREALLOC_PAGEHEADERS_COMMIT_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
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
	VirtualFree(bufferStart, 0, MEM_RELEASE);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error releasing page pool memory.");
}

void* BB::mallocVirtual(void* a_Start, size_t& a_Size, const virtual_reserve_extra a_ReserveSize)
{
	size_t t_PageAdjustedSize = Math::Max(a_Size + sizeof(StartPageHeader), AppOSDevice().VirtualMemoryMinimumAllocation());
	t_PageAdjustedSize = Math::RoundUp(t_PageAdjustedSize, AppOSDevice().VirtualMemoryPageSize());

	//Set the reference of a_Size so that the allocator has enough memory until the end of the page.
	a_Size = t_PageAdjustedSize - sizeof(StartPageHeader);

	StartPageHeader* t_StartPageHeader = nullptr;
	PageHeader* t_PageHeader = nullptr;
	//Check the pageHeader
	if (a_Start != nullptr)
	{
		t_StartPageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Start, sizeof(StartPageHeader)));
		t_PageHeader = t_StartPageHeader->header;
		
		void* t_CurrentEnd = pointerutils::Add(t_PageHeader->reserve_spot, t_PageHeader->bytes_commited);

		//If the amount commited is not enough check if there is enough reserved, if not reserve more.
		if (t_PageHeader->bytes_reserved > t_PageAdjustedSize + t_PageHeader->bytes_commited)
		{
			t_PageHeader->bytes_commited += t_PageAdjustedSize;
			VirtualAlloc(t_PageHeader->reserve_spot, 
				t_PageHeader->bytes_commited, 
				MEM_COMMIT, 
				PAGE_READWRITE);
			BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error commiting VirtualAlloc");
			return t_CurrentEnd;
		}

		BB_ASSERT(false, "Going over reserved memory!")
	}

	//When making a new header reserve a lot more then that is requested to support later resizes better.
	size_t t_AdditionalReserve = t_PageAdjustedSize * static_cast<size_t>(a_ReserveSize);
	void* t_Address = VirtualAlloc(a_Start, 
		t_AdditionalReserve, 
		MEM_RESERVE, 
		PAGE_NOACCESS);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error reserving VirtualAlloc");
	//Now commit a range of our reserved memory.
	VirtualAlloc(t_Address, 
		t_PageAdjustedSize, 
		MEM_COMMIT, 
		PAGE_READWRITE);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error commiting right after a reserve VirtualAlloc");

	//Set the header
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
	VirtualFree(t_PageHeader->reserve_spot,
		0, 
		MEM_RELEASE);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error virtualFree");
}

#pragma region Unit Test

#pragma warning (push, 0)
#include <gtest/gtest.h>
#pragma warning (pop)
#include "Utils/Math.h"

struct MockAllocator
{
	MockAllocator(size_t a_Size)
	{
		start = reinterpret_cast<uint8_t*>(mallocVirtual(start, a_Size));
		maxSize = a_Size;
		//using memset because the memory is NOT commited to ram unless it's accessed.
		memset(start, 5215, a_Size);
		buffer = start;
	}

	~MockAllocator()
	{
		freeVirtual(start);
	}

	void* Alloc(size_t a_Size)
	{
		void* t_Address = buffer;
		currentSize += a_Size;

		if (currentSize > maxSize)
		{
			size_t t_BufferIncrease{};
			if (maxSize > a_Size)
				t_BufferIncrease = Math::RoundUp(a_Size, maxSize);
			else
				t_BufferIncrease = a_Size;

			mallocVirtual(start, t_BufferIncrease);
			//using memset because the memory is NOT commited to ram unless it's accessed.
			maxSize += t_BufferIncrease;
		}
		memset(buffer, 16, a_Size);
		buffer = pointerutils::Add(buffer, a_Size);
		return t_Address;
	};

	const size_t SpaceLeft() const
	{
		return maxSize - currentSize;
	}
	//Not supporting free yet.
	//void free(size_t a_Size);

	size_t maxSize;
	size_t currentSize = 0;
	void* start = nullptr;
	void* buffer;
};

TEST(MemoryAllocators_Backend_Windows, COMMIT_RESERVE_PAGES)
{
	//Allocator size is equal to half a page, it will allocate an entire page in the background anyway.
	MockAllocator t_Allocator(AppOSDevice().VirtualMemoryMinimumAllocation());
	ASSERT_EQ(AppOSDevice().LatestOSError(), 0x0) << "Windows API error on creating the MockAllocator.";

	PageHeader lastHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->header;
	PageHeader newHeader;

	//Allocate memory equal to an entire page, this should increase the commited amount of pages, but not reserved.
	t_Allocator.Alloc(AppOSDevice().VirtualMemoryMinimumAllocation() * 3);
	ASSERT_EQ(AppOSDevice().LatestOSError(), 0x0) << "Windows API error on commiting more memory.";
	newHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->header;
	EXPECT_NE(lastHeader.bytes_commited, newHeader.bytes_commited) << "Bytes commited is not changed, while it should change!";
	//Reserved is now never changed.
	//EXPECT_NE(lastHeader.bytes_reserved, newHeader.bytes_reserved) << "Bytes reserved is not changed, while it should change!";
	lastHeader = newHeader;
}
#pragma endregion