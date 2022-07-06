# CMakeList.txt : CMake project for Allocator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra)
endif()

#compile Definitions.
if (WIN32)
set(PLATFORM_NAME "_WIN")
add_compile_definitions(_WIN32)
elseif(UNIX)
set(PLATFORM_NAME "_LINUX")
add_compile_definitions(_LINUX)
include_directories(/usr/include/X11)
endif ()

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_compile_definitions(_64BIT)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    add_compile_definitions(_32BIT)
endif()

#Add source to this project's executable.
add_executable (Allocator 
"Main.cpp" 
"pch/pch.h" 
"pch/pch.cpp" 
"Framework/Common.h"
"Framework/Framework.h"
"Framework/Framework.cpp"
"Framework/Allocators/Allocators.cpp" 
"Framework/Allocators/Allocators.h" 
"Framework/Allocators/BackingAllocator/BackingAllocator.h" 
"Framework/Allocators/AllocTypes.h" 
"Framework/Allocators/MemoryArena.h" 
"Framework/Allocators/MemoryArena.cpp" 
"Framework/Storage/Dynamic_Array.h" 
"Framework/Storage/Pool.h" 
"Framework/Storage/Hashmap.h"
"Framework/OS/OSDevice.h" 
"Framework/Utils/PointerUtils.h"
"Framework/Utils/Math.h"
"Framework/Utils/Hash.h"
"Framework/Allocators/BackingAllocator/BackingAllocator${PLATFORM_NAME}.cpp" 
"Framework/OS/OSDevice${PLATFORM_NAME}.cpp"

"Framework/Allocators/Allocators_UTEST.h"
"Framework/Storage/Dynamic_Array_UTEST.h"
"Framework/Storage/Pool_UTEST.h" 
"Framework/Storage/Hashmap_UTEST.h"
"Framework/Allocators/MemoryArena_UTEST.h")

#Include library
target_precompile_headers(Allocator PRIVATE "pch/pch.h")
include_directories(
"pch"
"Framework"
"Allocator"
"UnitTests"
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing ()

if(UNIX)
target_link_libraries(Allocator gtest_main X11)
elseif(WIN32)
target_link_libraries(Allocator gtest_main)
endif()

include(GoogleTest)
#gtest_discover_tests(Allocator)

# TODO: Add tests and install targets if needed.
