# CMakeList.txt : CMake project for Allocator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
if (WIN32)
add_executable (Allocator 
"Main.cpp" 
"pch/pch.h" 
"pch/pch.cpp" 
"Framework/Common.h"
"Framework/Framework.h"
"Framework/Framework.cpp"
"Framework/Allocators/Allocators.cpp" 
"Framework/Allocators/Allocators.h" 
"Framework/Allocators/Allocators_UTEST.h" 
"Framework/Allocators/BackingAllocator_WIN.cpp" 
"Framework/Allocators/BackingAllocator.h" 
"Framework/Allocators/BB_AllocTypes.h" 
"Framework/Storage/Dynamic_Array.h" 
"Framework/Storage/Pool.h" 
"Framework/Storage/Pool_UTEST.h" 
"Framework/OS/OSDevice.h" 
"Framework/OS/OSDevice_WIN.cpp" 
"Framework/Utils/PointerUtils"
"Framework/Utils/Math.h"
 "Framework/Storage/Dynamic_Array_UTEST.h")
add_compile_definitions(_WIN32)
endif (WIN32)

if (UNIX)
add_executable (Allocator 
"Main.cpp" 
"pch/pch.h" 
"pch/pch.cpp" 
"Framework/Common.h"
"Framework/Allocators/Allocators.cpp" 
"Framework/Allocators/Allocators.h" 
"Framework/Allocators/Allocators_UTEST.h" 
"Framework/Allocators/BackingAllocator_LINUX.cpp" 
"Framework/Allocators/BackingAllocator.h" 
"Framework/Allocators/BB_AllocTypes.h" 
"Framework/Storage/Dynamic_Array.h" 
"Framework/Storage/Pool.h" 
"Framework/Storage/Pool_UTEST.h" 
"Framework/OS/OSDevice.h" 
"Framework/OS/OSDevice_LINUX.cpp" 
"Framework/Utils/PointerUtils"
"Framework/Utils/Math.h"
)
add_compile_definitions(_UNIX)
endif (UNIX)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(_X64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_definitions(_X86)
endif()

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
