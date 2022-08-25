# memory_studies
Memory studies scaled up to allow for cross-platform virtual memory allocation and a small framework to have memory & OS operation abstractions for a later game engine project.

Most code is unit tested using google test as the testing library. These unit tests are found as _UTEST.h files.

Will only work in C++ 17 and above.

## Memory
### Virtual Memory Backing Allocator
All the allocators and dynamic memory allocators come from a Backing Allocator. The backing allocator allocates virtual address space using the virtual allocation API's from specific OS's. **VirtualAlloc(WIN32) & MMAP(Linux)** This way the memory is always page bound, the allocators are end of page and resizing allocators can be done cheaply since the backing allocator will reserve more memory for when a resize event happens. 

**[BackingAllocator.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/BackingAllocator/BackingAllocator.h), [BackingAllocator_WIN.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/BackingAllocator/BackingAllocator_WIN.cpp), [BackingAllocator_LINUX.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/BackingAllocator/BackingAllocator_LINUX.cpp)**

### Allocators & Memory Arenas
This framework currently has 3 Allocators, a linear allocator, a freelist and a power-of-two freelist allocator. All these allocators get their memory from the virtual backing allocator and support resizing. All the allocators are unit tested for allocating, freeing and resizing.

In addition, these memory allocators are inside a Memory Arena, this way on debug mode boundry checking and memory tracking can be done while not disturbing release builds with debugging tools. 

**[Allocators.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/Allocators.h), [Allocators.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/Allocators.cpp), [MemoryArena.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/MemoryArena.h), [MemoryArena.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/MemoryArena.cpp)**

### Storage Containers
The framework also has containers similiar to the stl, however, these use the allocators found in the framework and require one if you want to use a container. 
These containers also use utils that replace common std operations like std::copy.

All containers support POD and non-POD types and try to optimize for each use.

The hashmap has an unordered hashmap and a open addressed linear probing hashmap.

**[Array.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Array.h), [Hashmap.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Hashmap.h), [Slotmap.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Slotmap.h), [BBString.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/BBString.h)**

### TODO
Memory Arena: Make system and temporary allocator distinction. 

Memory Arena: Use own hashmap for storing boundry checks.

Utils: Make own memory operations like memcpy, memmove, memcmp and string operations.

## Cross-platform

### OS Abstraction
One of the main ideas of this framework is to support cross-platform as best as possible. One of the ways it does this is by abstracting common OS operations. It is the first system in this framework that works with the custom allocators. It supports creating an OS window, checking for OS errors and storing some data specific for that OS. *e.g, see how big a virtual page is, what the minimum virtual allocation requirement is*

This will be expanded for file loading and more.

**[OSDevice.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/OS/OSDevice.h), [OSDevice_WIN.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/OS/OSDevice_WIN.cpp), [OSDevice_LINUX.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/OS/OSDevice_LINUX.cpp)**
