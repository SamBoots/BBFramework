# BBFramework (v0.4 does not support linux!)
//TODO, need to explain thread scheduler, BBImage, BBJson and more.

Memory studies scaled up to allow for cross-platform virtual memory allocation and a small framework to have memory & OS operation abstractions for a later game engine project.

In addition, the project also has code to create a window, load files and uses RAWINPUT from windows to handle input events.

Most code is unit tested using google test as the testing library. These unit tests are found as _UTEST.h files.

Will only work in C++ 17 and above.

## Memory
### Virtual Memory Backing Allocator
All the allocators and dynamic memory allocators come from a Backing Allocator. The backing allocator allocates virtual address space using the virtual allocation API's from specific OS's. **VirtualAlloc(WIN32) & MMAP(Linux)** This way the memory is always page bound, the allocators are end of page and resizing allocators can be done cheaply since the backing allocator will reserve more memory for when a resize event happens. 

**[BackingAllocator.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/BackingAllocator.h), [BackingAllocator.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/BackingAllocator.cpp)**

### Allocators & Memory Arenas
This framework currently has 4 Allocators, a linear allocator, fixed linear allocator, freelist and a power-of-two freelist allocator. All these allocators get their memory from the virtual backing allocator and support resizing. All the allocators are unit tested for allocating, freeing and resizing.

All system allocators inherit from BaseAllocator that handles potentional debugging such as logging allocations.

We also have a temporary linear allocator and a ring allocator that gets it's backing memory from a already existing allocator, this way you can quickly create and destroy an allocator without necessarily doing an operating system call to get virtual memory. 

In debug the allocators will allocate more memory to host a allocationLog that checks for boundry, file name and line number and how big it is. This is useful to see if you have a buffer overflow or a leak after you remove an allocator using RAII. You can find these under the BBMemory.h/cpp files.

**[Allocators.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/Allocators.h), 
[Allocators.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/Allocators.cpp), 
[TemporaryAllocator.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/TemporaryAllocator.h), 
[TemporaryAllocator.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/TemporaryAllocator.cpp),
[RingAllocator.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Allocators/RingAllocator.h), 
[RingAllocator.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/Allocators/RingAllocator.cpp),
[BBMemory.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/BBMemory.h),
[BBMemory.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/BBMemory.cpp)**

### Storage Containers
The framework also has containers similiar to the stl, however, these use the allocators found in the framework and require one if you want to use a container. 
These containers also use utils that replace common std operations like std::copy.

All containers support POD and non-POD types and try to optimize for each use.

The hashmap has an unordered hashmap and a open addressed linear probing hashmap.

**[Pool.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Pool.h), [Array.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Array.h), [Hashmap.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Hashmap.h), [Slotmap.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/Slotmap.h), [BBString.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/Storage/BBString.h)**

## Cross-platform

### OS Abstraction
One of the main ideas of this framework is to support cross-platform as best as possible. One of the ways it does this is by abstracting common OS operations. It is the first system in this framework that works with the custom allocators. It supports creating an OS window, checking for OS errors and storing some data specific for that OS. *e.g, see how big a virtual page is, what the minimum virtual allocation requirement is*

It also supports file loading and uses the RAWINPUT api from Windows to get input.

**[Program.h](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/include/OS/Program.h), [Program_WIN.cpp](https://github.com/SamBoots/memory_studies/blob/main/Project/BB/Framework/src/OS/Program_WIN.cpp)**
