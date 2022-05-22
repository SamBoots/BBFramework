#include "pch.h"
#include "BackingAllocator.h"
#include "Utils/pointerUtils.h"
#include "Utils/Math.h"
#include "OS/OSDevice.h"

#include <Windows.h>
#include <memoryapi.h>

using namespace BB;

#ifdef _X64
constexpr const size_t RESERVEMULTIPLICATION = 128;
#endif _X64
#ifdef _X86
constexpr const size_t RESERVEMULTIPLICATION = 64;
#endif _X86

struct PageHeader
{
	size_t bytesCommited;
	size_t bytesUsed;
	size_t bytesReserved;
	void* reserveSpot;
	PageHeader* previous = nullptr;
};

constexpr const size_t PREALLOC_PAGEHEADERS = 128 * 16;
constexpr const size_t PREALLOC_PAGEHEADERS_COMMIT_SIZE = PREALLOC_PAGEHEADERS * sizeof(PageHeader);

struct StartPageHeader
{
	PageHeader* head;
};

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

void* BB::mallocVirtual(void* a_Start, size_t a_Size)
{
	//BB_WARNING(a_Size > PAGESIZE * 64, "Virtual Alloc is smaller then 4 MB, try to make allocators larger then 4 MB.");

	size_t t_PageAdjustedSize = Math::RoundUp(a_Size + sizeof(PageHeader), AppOSDevice().virtualMemoryPageSize);
	StartPageHeader* t_StartPageHeader = nullptr;
	PageHeader* t_PageHeader = nullptr;
	//Check the pageHeader
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

		//If the amount commited is not enough check if there is enough reserved, if not reserve more.
		if (t_PageHeader->bytesReserved > t_PageAdjustedSize)
		{
			t_PageHeader->bytesCommited += t_PageAdjustedSize;
			t_PageHeader->bytesUsed += a_Size;
			t_PageHeader->bytesReserved -= t_PageAdjustedSize;
			VirtualAlloc(t_PageHeader->reserveSpot, t_PageAdjustedSize + t_PageHeader->bytesUsed, MEM_COMMIT, PAGE_READWRITE);
			BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error commiting VirtualAlloc");
			return t_CurrentEnd;
		}

		//Not enough commited memory available to support the size, so just commit the remaining memory and reserve a new header.
		VirtualAlloc(t_PageHeader->reserveSpot, t_PageHeader->bytesReserved + t_PageHeader->bytesCommited, MEM_COMMIT, PAGE_READWRITE);
		t_PageHeader->bytesCommited += t_PageHeader->bytesReserved;
		t_PageHeader->bytesUsed = t_PageHeader->bytesCommited;
		t_PageHeader->bytesReserved = 0;

		//Adjust the start pointer to be the end pointer of the page header.
		a_Start = pointerutils::Add(t_PageHeader->reserveSpot, t_PageHeader->bytesCommited);
	}

	//When making a new header reserve a lot more then that is requested to support later resizes better.
	size_t t_AdditionalReserve = t_PageAdjustedSize * RESERVEMULTIPLICATION;
	void* t_Address = VirtualAlloc(a_Start, t_AdditionalReserve, MEM_RESERVE, PAGE_NOACCESS);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error reserving VirtualAlloc");
	VirtualAlloc(t_Address, t_PageAdjustedSize, MEM_COMMIT, PAGE_READWRITE);
	BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error commiting right after a reserve VirtualAlloc");

	//StartPageHeader when the allocation is new.
	if (!t_StartPageHeader)
	{
		t_StartPageHeader = reinterpret_cast<StartPageHeader*>(t_Address);
		t_StartPageHeader->head = nullptr;
		a_Size += sizeof(StartPageHeader);
	}
	PageHeader* t_NewHeader = reinterpret_cast<PageHeader*>(pagePool.AllocHeader());
	t_NewHeader->bytesCommited = t_PageAdjustedSize;
	t_NewHeader->bytesUsed = a_Size;
	t_NewHeader->bytesReserved = t_AdditionalReserve - t_PageAdjustedSize;
	t_NewHeader->reserveSpot = pointerutils::Add(t_Address, sizeof(StartPageHeader));
	t_NewHeader->previous = t_StartPageHeader->head; //Set the previous header, stored for when deletion is called or a resize event.
	t_StartPageHeader->head = t_NewHeader; // Set the new header as the head.

	//Return the pointer that does not include the StartPageHeader
	return t_StartPageHeader->head->reserveSpot;
}

void BB::freeVirtual(void* a_Ptr)
{
	PageHeader* t_PageHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(a_Ptr, sizeof(StartPageHeader)))->head;
	//free the extra pages.
	while (t_PageHeader)
	{
		PageHeader* t_NextHeader = t_PageHeader->previous;
		VirtualFree(t_PageHeader->reserveSpot, 0, MEM_RELEASE);
		BB_ASSERT(AppOSDevice().LatestOSError() == 0x0, "Windows API error virtualFree");
		t_PageHeader = t_NextHeader;
	}
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
		maxSize = a_Size;
		start = reinterpret_cast<uint8_t*>(mallocVirtual(start, a_Size));
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
	MockAllocator t_Allocator(AppOSDevice().virtualMemoryPageSize / 4 - sizeof(StartPageHeader));
	ASSERT_EQ(AppOSDevice().LatestOSError(), 0x0) << "Windows API error on creating the MockAllocator.";

	PageHeader lastHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;

	EXPECT_EQ(lastHeader.bytesUsed, t_Allocator.maxSize + sizeof(StartPageHeader)) << "Used amount is wrong.";
	EXPECT_EQ(lastHeader.bytesCommited, AppOSDevice().virtualMemoryPageSize) << "Commited amount is wrong.";
	EXPECT_EQ(lastHeader.bytesReserved, AppOSDevice().virtualMemoryPageSize * RESERVEMULTIPLICATION - lastHeader.bytesCommited) << "Reserved amount is wrong.";

	//Allocate memory equal to half a page, it should NOT reserve/commit more pages, just use more.
	t_Allocator.Alloc(AppOSDevice().virtualMemoryPageSize / 2);
	PageHeader newHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;
	EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	EXPECT_EQ(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is changed, more was allocated while it shouldn't!";
	EXPECT_EQ(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved is changed, more was allocated while it shouldn't!";
	lastHeader = newHeader;

	//Allocate memory equal to an entire page, this should increase the commited amount of pages, but not reserved.
	t_Allocator.Alloc(AppOSDevice().virtualMemoryPageSize);
	ASSERT_EQ(AppOSDevice().LatestOSError(), 0x0) << "Windows API error on commiting more memory.";
	newHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;
	EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	EXPECT_NE(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
	EXPECT_NE(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved is not changed, while it should change!";
	lastHeader = newHeader;

	//disabled since extending address is very hard 
	////Allocate memory equal to it's previous reserved memory amount, this should increase the commited and reserved amount of pages.
	//t_Allocator.Alloc(lastHeader.bytesReserved);
	//ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on reserving more memory.";
	//newHeader = *reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;
	//EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	//EXPECT_NE(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
	//EXPECT_LT(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved has not been increased, this means that the backend allocator does not reserve more memory as it should.";
	//lastHeader = newHeader;
}

TEST(MemoryAllocators_Backend_Windows, COMMIT_RESERVE_PAGES_RANDOM)
{
	constexpr const size_t RANDOM_ALLOCATORS_AMOUNT = 8;
	constexpr const size_t RANDOM_SAMPLE_AMOUNT = 128;
	constexpr const size_t MAX_RANDOM_VALUE = 2097152; //256 KB.
	constexpr const size_t MIN_RANDOM_VALUE = 8192; //4 KB.

	constexpr const size_t ALLOCATOR_MULTIPLY_PAGE_SIZE = 128;

	for (size_t i = 0; i < RANDOM_ALLOCATORS_AMOUNT; i++)
	{
		MockAllocator t_Allocator(AppOSDevice().virtualMemoryPageSize * ALLOCATOR_MULTIPLY_PAGE_SIZE);
		ASSERT_EQ(AppOSDevice().LatestOSError(), 0x0) << "Windows API error on creating the MockAllocator.";

		struct MockPageHeader
		{
			size_t bytesCommited = 0;
			size_t bytesUsed = 0;
			size_t bytesReserved = 0;

			MockPageHeader(PageHeader& a_PageHeader) :
				bytesCommited(a_PageHeader.bytesCommited),
				bytesUsed(a_PageHeader.bytesUsed),
				bytesReserved(a_PageHeader.bytesReserved)
			{}

			void operator+=(PageHeader& a_PageHeader)
			{
				bytesCommited += a_PageHeader.bytesCommited;
				bytesUsed += a_PageHeader.bytesUsed;
				bytesReserved += a_PageHeader.bytesReserved;
			}
		} t_LastMockHeader(*reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head);
		PageHeader* t_LastHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;


		for (size_t i = 0; i < RANDOM_SAMPLE_AMOUNT; i++)
		{
			const size_t t_RandomAllocAmount = Utils::RandomUintMinMax(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
			const size_t t_SpaceLeft = t_Allocator.SpaceLeft();

			t_Allocator.Alloc(t_RandomAllocAmount);
			PageHeader* t_NewHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;
			MockPageHeader t_NewMockHeader(*t_NewHeader);

			//If the buffer had to be resized need to reserve more space record it.
			if (t_RandomAllocAmount > t_SpaceLeft)
			{
				EXPECT_NE(t_LastMockHeader.bytesUsed, t_NewMockHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
				if (t_LastMockHeader.bytesCommited < t_LastMockHeader.bytesUsed + t_RandomAllocAmount && t_NewHeader == t_LastHeader)
				{
					EXPECT_NE(t_LastMockHeader.bytesCommited, t_NewMockHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
					EXPECT_EQ(t_NewMockHeader.bytesReserved + t_NewMockHeader.bytesCommited, t_LastMockHeader.bytesReserved + t_LastMockHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
				}

				if (t_NewHeader != t_LastHeader)
				{
					EXPECT_LT(t_LastMockHeader.bytesReserved, t_NewMockHeader.bytesReserved) << "Bytes reserved has not been increased, this means that the backend allocator does not reserve more memory as it should.";
				}

			}

			t_LastHeader = t_NewHeader;
			t_LastMockHeader = t_NewMockHeader;
		}
		PageHeader* t_NewHeader = reinterpret_cast<StartPageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(StartPageHeader)))->head;
		std::cout << "The COMMIT_RESERVE_PAGES_RANDOM test has these pageheaders: \n";

		size_t t_HeaderAmount = 0;
		while (t_NewHeader->previous)
		{
			t_HeaderAmount++;
			std::cout << "PageHeader number: " << t_HeaderAmount << " it had reserved and/or commited enough memory (in bytes): " << t_NewHeader->bytesCommited + t_NewHeader->bytesReserved << "\n";
			t_NewHeader = t_NewHeader->previous;
		}
	}
}

#pragma endregion