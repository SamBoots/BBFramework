#include "pch.h"
#include "BackingAllocator.h"
#include "pointerUtils.h"

#include <Windows.h>
#include <memoryapi.h>
#include <sysinfoapi.h>

#include <cmath>


using namespace BB;

constexpr const size_t RESERVEMULTIPLICATION = 16;
static size_t PAGESIZE;

struct PageHeader
{
	size_t bytesCommited;
	size_t bytesUsed;
	size_t bytesReserved;
	PageHeader* next = nullptr;
};

BackingAllocator::BackingAllocator()
{
	SYSTEM_INFO t_SystemInfo;
	GetSystemInfo(&t_SystemInfo);
	PAGESIZE = t_SystemInfo.dwAllocationGranularity;
}


static size_t RoundUp(size_t a_NumToRound, size_t a_Multiple)
{
	BB_ASSERT(a_Multiple, "Multiple is 0!");
	return ((a_NumToRound + a_Multiple - 1) / a_Multiple) * a_Multiple;
}

void* BB::mallocVirtual(void* a_Start, size_t a_Size)
{
	//BB_WARNING(a_Size > PAGESIZE * 64, "Virtual Alloc is smaller then 4 MB, try to make allocators larger then 4 MB.");

	size_t t_PageAdjustedSize = RoundUp(a_Size + sizeof(PageHeader), PAGESIZE);
	PageHeader* t_PageHeader = nullptr;
	//Check the pageHeader
	if (a_Start != nullptr)
	{
		t_PageHeader = reinterpret_cast<PageHeader*>(pointerutils::Subtract(a_Start, sizeof(PageHeader)));

		while (t_PageHeader->next)
		{
			t_PageHeader = t_PageHeader->next;
		}

		void* t_ReturnAddress = pointerutils::Add(t_PageHeader, t_PageHeader->bytesUsed);
		//If the amount commited is enough just move the pointer and return it.
		if ((t_PageHeader->bytesCommited - t_PageHeader->bytesUsed) >= a_Size)
		{
			t_PageHeader->bytesUsed += a_Size;
			return t_ReturnAddress;
		}
		
		void* t_Address = pointerutils::Add(t_PageHeader, t_PageHeader->bytesCommited);

		//If the amount commited is not enough check if there is enough reserved, if not reserve more.
		if (t_PageHeader->bytesReserved > t_PageAdjustedSize)
		{
			t_PageHeader->bytesCommited += t_PageAdjustedSize;
			t_PageHeader->bytesUsed += a_Size;
			t_PageHeader->bytesReserved -= t_PageAdjustedSize;
			VirtualAlloc(t_ReturnAddress, t_PageAdjustedSize, MEM_COMMIT, PAGE_READWRITE);
			BB_ASSERT(GetLastError() == 0x0, "Windows API error commiting VirtualAlloc");
			return t_ReturnAddress;
		}
	}

	if (t_PageHeader) a_Start = pointerutils::Add(t_PageHeader, t_PageHeader->bytesReserved + t_PageHeader->bytesCommited);

	size_t t_AdditionalReserve = t_PageAdjustedSize * RESERVEMULTIPLICATION;
	void* t_Address = VirtualAlloc(a_Start, t_AdditionalReserve, MEM_RESERVE, PAGE_NOACCESS);
	BB_ASSERT(GetLastError() == 0x0, "Windows API error reserving VirtualAlloc");
	VirtualAlloc(t_Address, t_PageAdjustedSize, MEM_COMMIT, PAGE_READWRITE);
	BB_ASSERT(GetLastError() == 0x0, "Windows API error commiting right after a reserve VirtualAlloc");

	//Pageheader for new allocation.
	PageHeader* t_Header = reinterpret_cast<PageHeader*>(t_Address);
	t_Header->bytesCommited = t_PageAdjustedSize;
	t_Header->bytesUsed = a_Size + sizeof(PageHeader);
	t_Header->bytesReserved = t_AdditionalReserve - t_PageAdjustedSize;
	t_Header->next = nullptr;
	//If this is an extension, make this the next.
	if (t_PageHeader) t_PageHeader->next = t_Header;


	void* t_AdjustedAddress = pointerutils::Add(t_Address, sizeof(PageHeader));

	return t_AdjustedAddress;
}

void BB::freeVirtual(void* a_Ptr)
{
	PageHeader* t_PageHeader = reinterpret_cast<PageHeader*>(pointerutils::Subtract(a_Ptr, sizeof(PageHeader)));
	//free the extra pages.
	while (t_PageHeader)
	{
		PageHeader* t_NextHeader = t_PageHeader->next;
		VirtualFree(t_PageHeader, 0, MEM_RELEASE);
		BB_ASSERT(GetLastError() == 0x0, "Windows API error virtualFree");
		t_PageHeader = t_NextHeader;
	}
}

#pragma region Unit Test

#include <gtest/gtest.h>
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
		void* t_Address;
		currentSize += a_Size;

		if (currentSize <= maxSize)
			t_Address = buffer;
		else
		{
			size_t t_BufferIncrease{};
			if (maxSize > a_Size)
				t_BufferIncrease = RoundUp(a_Size, maxSize);
			else
				t_BufferIncrease = a_Size;

			t_Address = mallocVirtual(start, t_BufferIncrease);
			//using memset because the memory is NOT commited to ram unless it's accessed.
			memset(t_Address, 5215, t_BufferIncrease);
			maxSize += t_BufferIncrease;
		}

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
	MockAllocator t_Allocator(PAGESIZE / 4 - sizeof(PageHeader));
	ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on creating the MockAllocator.";

	PageHeader lastHeader = *reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));

	EXPECT_EQ(lastHeader.bytesUsed, t_Allocator.maxSize + sizeof(PageHeader)) << "Used amount is wrong.";
	EXPECT_EQ(lastHeader.bytesCommited, PAGESIZE) << "Commited amount is wrong.";
	EXPECT_EQ(lastHeader.bytesReserved, PAGESIZE * RESERVEMULTIPLICATION - lastHeader.bytesCommited) << "Reserved amount is wrong.";

	//Allocate memory equal to half a page, it should NOT reserve/commit more pages, just use more.
	t_Allocator.Alloc(PAGESIZE / 2);
	PageHeader newHeader = *reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));
	EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	EXPECT_EQ(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is changed, more was allocated while it shouldn't!";
	EXPECT_EQ(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved is changed, more was allocated while it shouldn't!";
	lastHeader = newHeader;

	//Allocate memory equal to an entire page, this should increase the commited amount of pages, but not reserved.
	t_Allocator.Alloc(PAGESIZE);
	ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on commiting more memory.";
	newHeader = *reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));
	EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	EXPECT_NE(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
	EXPECT_NE(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved is not changed, while it should change!";
	lastHeader = newHeader;

	//Allocate memory equal to it's previous reserved memory amount, this should increase the commited and reserved amount of pages.
	t_Allocator.Alloc(lastHeader.bytesReserved);
	ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on reserving more memory.";
	newHeader = *reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)))->next;
	EXPECT_NE(lastHeader.bytesUsed, newHeader.bytesUsed) << "Bytes used is not changed, while it should change!";
	EXPECT_NE(lastHeader.bytesCommited, newHeader.bytesCommited) << "Bytes commited is not changed, while it should change!";
	EXPECT_LT(lastHeader.bytesReserved, newHeader.bytesReserved) << "Bytes reserved has not been increased, this means that the backend allocator does not reserve more memory as it should.";
	lastHeader = newHeader;
}

TEST(MemoryAllocators_Backend_Windows, COMMIT_RESERVE_PAGES_RANDOM)
{
	constexpr const size_t RANDOM_ALLOCATORS_AMOUNT = 32;
	constexpr const size_t RANDOM_SAMPLE_AMOUNT = 512;
	constexpr const size_t MAX_RANDOM_VALUE = 2097152; //256 KB.
	constexpr const size_t MIN_RANDOM_VALUE = 8192; //4 KB.

	for (size_t i = 0; i < RANDOM_ALLOCATORS_AMOUNT; i++)
	{
		//Allocator size is equal to half a page, it will allocate an entire page in the background anyway.
		MockAllocator t_Allocator(PAGESIZE);
		ASSERT_EQ(GetLastError(), 0x0) << "Windows API error on creating the MockAllocator.";

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
		} t_LastMockHeader(*reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader))));
		PageHeader* t_LastHeader = reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));


		for (size_t i = 0; i < RANDOM_SAMPLE_AMOUNT; i++)
		{
			const size_t t_RandomAllocAmount = Utils::RandomUintMinMax(MIN_RANDOM_VALUE, MAX_RANDOM_VALUE);
			const size_t t_SpaceLeft = t_Allocator.SpaceLeft();

			t_Allocator.Alloc(t_RandomAllocAmount);
			PageHeader* t_NewHeader = reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));
			MockPageHeader t_NewMockHeader(*t_NewHeader);

			while (t_NewHeader->next)
			{
				t_NewHeader = t_NewHeader->next;
				t_NewMockHeader += *t_NewHeader;
			}

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
		PageHeader* t_NewHeader = reinterpret_cast<PageHeader*>(pointerutils::Subtract(t_Allocator.start, sizeof(PageHeader)));
		std::cout << "The COMMIT_RESERVE_PAGES_RANDOM test has these pageheaders: \n";

		size_t t_HeaderAmount = 0;
		while (t_NewHeader->next)
		{
			t_HeaderAmount++;
			std::cout << "PageHeader number: " << t_HeaderAmount << " it had reserved and/or commited enough memory (in bytes): " << t_NewHeader->bytesCommited + t_NewHeader->bytesReserved << "\n";
			t_NewHeader = t_NewHeader->next;
		}
	}
}

#pragma endregion