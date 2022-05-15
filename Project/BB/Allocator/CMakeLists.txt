# CMakeList.txt : CMake project for Allocator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
if (WIN32)
add_executable (Allocator "Main.cpp" "Framework/Allocators.cpp" "Framework/Allocators.h" "Framework/Allocators_UTEST.h" "Framework/BackingAllocator_WIN.cpp" "Framework/BackingAllocator.h" "Framework/BB_AllocTypes.h" "Framework/Storage/Dynamic_Array.h" "pch/pch.h" "pch/pch.cpp" "UnitTests/Utils/Math.h")
endif (WIN32)

if (UNIX)
add_executable (Allocator "Main.cpp" "Framework/Allocators.cpp" "Framework/Allocators.h" "Framework/Allocators_UTEST.h"  "Framework/BackingAllocator_LINUX.cpp" "Framework/BackingAllocator.h" "Framework/BB_AllocTypes.h" "Framework/Storage/Dynamic_Array.h" "pch/pch.h" "pch/pch.cpp" "UnitTests/Utils/Math.h")
endif (UNIX)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

target_link_libraries(
  Allocator
  gtest_main
)

include(GoogleTest)
# gtest_discover_tests(Allocator)

#Include library
target_precompile_headers(Allocator PRIVATE "pch/pch.h")
include_directories(
"pch"
"Framework"
"UnitTests"
)

# TODO: Add tests and install targets if needed.
