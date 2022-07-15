# CMakeList.txt : CMake project for Allocator, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

#compile Definitions.
if (WIN32)
set(PLATFORM_NAME "_WIN")
elseif(UNIX)
set(PLATFORM_NAME "_LINUX")
include_directories(/usr/include/X11)
endif ()

#Add source to this project's executable.
add_library (Allocator 
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
"Framework/Utils/Utils.h"
"Framework/Utils/Hash.h"
"Framework/Utils/Slice.h"
"Framework/Allocators/BackingAllocator/BackingAllocator${PLATFORM_NAME}.cpp" 
"Framework/OS/OSDevice${PLATFORM_NAME}.cpp" )

#Include library
target_precompile_headers(Allocator PRIVATE "pch/pch.h")
include_directories(
"pch"
"Framework"
"Allocator"
"UnitTests"
)