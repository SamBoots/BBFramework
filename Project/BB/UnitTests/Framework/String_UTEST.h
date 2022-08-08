#pragma once
#include "../TestValues.h"
#include "Storage/String.h"

TEST(String_DataStructure, append_push_pop_copy_assignment)
{
	constexpr size_t StringReserve = 128;

	const size_t allocatorSize = BB::kbSize * 16;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::String t_String(t_Allocator);

	const char* t_AppendString = "First Unit test of the string is now being done.";

	//test single string and compare.
	{
		t_String.append(t_AppendString);

		EXPECT_EQ(strcmp(t_String.c_str(), t_AppendString), 0) << "Append(const char*) failed";
	}

	t_String.clear();

	//test break the string apart and use parts of it.
	{
		const char* t_AppendString = "First Unit test of the string is now being done.";
		t_String.append(t_AppendString, 10);

		EXPECT_EQ(strcmp(t_String.c_str(), "First Unit"), 0) << "Append(const char*, size_t) failed";

		t_String.append(t_AppendString + t_String.size(), 12);

		EXPECT_EQ(strcmp(t_String.c_str() + 10, " test of the"), 0) << "Append(const char*, size_t) failed second call";

		t_String.append(t_AppendString + t_String.size(), 26);

		EXPECT_EQ(strcmp(t_String.c_str(), t_AppendString), 0) << "Append(const char*, size_t) failed third call";
	}

	//Copy
	BB::String t_CopyString(t_String);

	EXPECT_EQ(strcmp(t_CopyString.data(), t_String.data()), 0) << "Copy constructor failed.";

	BB::String t_CopyOperatorString(t_Allocator);
	t_CopyOperatorString = t_CopyString;

	EXPECT_EQ(strcmp(t_CopyOperatorString.data(), t_CopyString.data()), 0) << "Copy operator failed.";

	//Assignment
	BB::String t_AssignmentString(std::move(t_CopyOperatorString));

	EXPECT_EQ(strcmp(t_AssignmentString.data(), t_String.data()), 0) << "Assignment constructor failed.";
	EXPECT_EQ(t_CopyOperatorString.data(), nullptr) << "Assignment constructor failed, the other string has still has data while it shouldn't have.";

	BB::String t_AssignmentOperatorString(t_Allocator);
	t_AssignmentOperatorString = std::move(t_AssignmentString);

	EXPECT_EQ(strcmp(t_AssignmentOperatorString.data(), t_String.data()), 0) << "Assignment operator failed.";
	EXPECT_EQ(t_AssignmentString.data(), nullptr) << "Assignment operator failed, the other string has still has data while it shouldn't have.";
}

TEST(String_DataStructure, reserve_shrink)
{
	constexpr size_t StringReserve = 128;

	const size_t allocatorSize = BB::kbSize * 16;
	BB::FreeListAllocator_t t_Allocator(allocatorSize);

	BB::String t_String(t_Allocator);

	const char* t_AppendString = "First Unit test of the string is now being done.";

	t_String.reserve(128);
	const size_t t_StringCapacity = t_String.capacity();
	const size_t t_ModifiedC_StringSize = BB::Math::RoundUp(strlen(t_AppendString) + 1, BB::String_Specs::multipleValue);

	ASSERT_LT(t_ModifiedC_StringSize, t_StringCapacity) << "Modified string size is bigger then the string capacity, test is unaccurate.";

	t_String.append(t_AppendString);

	EXPECT_EQ(strcmp(t_String.c_str(), t_AppendString), 0) << "Append(const char*) failed";

	t_String.shrink_to_fit();

	EXPECT_EQ(t_String.capacity(), t_ModifiedC_StringSize) << "String did not shrink down to the correct size";
}