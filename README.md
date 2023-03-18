# Memory Manager Simulator
A simple library for simulating the features of a memory manager with paging and virtual memory.

## Sections
1. [Details](#details)
1. [GUI](#gui)
1. [Cloning](#cloning)
1. [Building](#building)
1. [Usage](#usage)
1. [Tests](#tests)
1. [Documentation](#documentation)
1. [Algorithms](#algorithms)

## Details
The memory manager is made up of jobs and physical pages. The physical pages are shared between all jobs. Each job has access to a set amount of virtual memory, split up into pages of a given size. When a memory access occurs, the memory manager checks to see where the virtual page is located in physical memory. If it is not allocated, it finds a place for it in physical memory. If physical memory is full, there are a few [algorithms](#algorithms) that can be used to free up physical memory pages in a given order. Once the virtual page is found in memory, an address calculation occurs to return a pointer to a physical address. 

There is no `free` or `malloc` command since the very act of creating a job gives the job access to all the memory it needs, and there is no reason to release memory since it will get released when the physical page is needed, or when the job is removed. All physical memory consumed by the job will also be released when the job is removed/ended. 

## GUI
There is a GUI for this simulator that was created using [dear imgui](https://github.com/ocornut/imgui/tree/master) with a [GLFW 3.3](https://github.com/glfw/glfw/tree/3.3-stable) platform and an OpenGL3 renderer. Because of this, it is inherently cross platform. There are also precompiled GLFW libs included in a seprate folder used for development with different types of Visual Studio on Windows. These are excluded from the final build. 

## Cloning
To clone using git, you must also clone in the submodules using the following command:
```
git clone https://github.com/ryanriccio1/MemoryManagerSimulator.git --recurse-submodules
cd MemoryManagerSimulator
```

## Building
### Binaries
If on macOS or Windows, there are precompiled binaries in the releases section already. <b><i>[Here's](https://github.com/ryanriccio1/MemoryManagerSimulator/releases/tag/stable)</i></b> the link to the latest release

### Prerequisites
To build, CMake must be in PATH and there must be a CMake compatible compiler in PATH as well. To build the entire GUI application, building from the main root directory is sufficient. To build just the memory manager simulator library, CMake should be ran from `./MemoryManagerSimulator/`. This will generate a static library, although an executable can be generated which will enter the code in `./MemoryManagerSimulator/driver.c`. See [Building Console App](#building-console-app) for more details.

### Linux
For the GUI application, there are some dependencies required to build GLFW.
On debian derivatives, `xorg-dev` is required. For more details on other distros, see [the GLFW Documentation](https://www.glfw.org/docs/3.3/compile.html#compile_deps_x11).

```bash
sudo apt install xorg-dev
```

### All Platforms
Once dependencies are met, to build application, from the root folder run:
```
cmake -B "build"
```
This will generate all of the build files in `./build/` needed for building the application. To switch the build type, use `-G "<build toolchain>"` to generate different build files. For example:
```
cmake -B "build" -G "MinGW Makefiles" 
```
Once build files have been generated, build the application using the following:
```
cmake --build "build"
```
This will put the executable in the root directory after building.

### Building Console App
To generate a simple console app rather than then entire GUI, an edit is needed to `./MemoryManagerSimulator/CMakeLists.txt`. 

Switch this line:
```cmake
add_library(MemoryManagerSimulator STATIC ${SRC_FILES})
```
To this line:
```cmake
add_executable(MemoryManagerSimulator ${SRC_FILES})
```
Then before running the build instructions from above, enter the `./MemoryManagerSimulator/` directory first.
```bash
cd ./MemoryManagerSimulator/
```

## Usage
### Initialize
To create a memory manager, allocate space for it, then initialize it with `setupMemoryManager()`. This function takes in the memory manager, the size per each page, the total amount of physical memory, and the amount of virtual memory per job.
```c
#include "MemoryManager.h"

int main()
{
    // create memory manager
    MemoryManager *memoryManager = malloc(sizeof(MemoryManager));
    memoryManager = setupMemoryManager(memoryManager, 16, 128, 1024);
}
```

### Creating Jobs
Creating your first job is easy. Using `createJob()`, you will allocate a job inside of the memory manager. No physical memory will be allocated until specified. `createJob()` takes in the memory manager to register the job with, the name of the job, and the ID of the job.
```c
Job* newJob = createJob(memoryManager, "Test Job 1", 4444);
```

### Accessing Virtual Memory
Once a job has been created, it can be accessed using `accessJob()`, which takes in the memory manager where the job is located, the ID of the job, the virtual memory address you are trying to access, and the algorithm to use in case a physical page needs to be freed.
```c
void* pointerToPhysicalMemory = accessJob(memoryManager, 4444, 0x0ff, LRU);
```

### Freeing Jobs
To free all memory used by a job, and to remove it from the job manager entirely, a job can be removed using `removeJob()`, which takes in the memory manager where the job is located, and the ID of the job.
```c
#include <stdbool.h>    // should be dragged in by 'MemoryManager.h`,
                        // but some linkers like to complain

bool jobRemoved = removeJob(memoryManager, 4444);
```

### Removing Memory Manager
To remove the memory manager, use `cleanupMemoryManager()` before freeing it.
```c
cleanupMemoryManager(memoryManager);
free(memoryManager);
```

### C++ Wrapper
There is a C++ wrapper for the MemoryManager struct in `./MemoryManagerSimulator.GUI/` that is setup to support RAII through smart pointers to abstract the calling of `setupMemoryManager()` and `cleanupMemoryManager()`. It currently assumes that any job name (`char *`) is allocated on the heap so that it is always in scope, and then frees it when the job is removed. 

## Tests
All tests were written in [GoogleTest](https://github.com/google/googletest) and can be built/ran through Visual Studio using the Test Explorer. They can be accessed by opening up `MemoryManagerSimulator.sln` in the main directory. Note: it also runs the tests found inside `./DataStructures`.

## Documentation
Docs can be generated using Doxygen. Make sure to exclude the folders `./MemoryManagerSimulator.GUI/fmt/`, `./MemoryManagerSimulator.GUI/imgui/`, and `./MemoryManagerSimulator.GUI/glfw/` if you don't want the documentation for those libraries. You can also exclude `./DataStructures` if you don't need that documentation as well. 

## Algorithms
There are three algorithms that can be used:

1. <b>LRU</b> - Remove the physical page that was accessed the longest time ago.
1. <b>LFU</b> - Remove the physical page that has the lowest reference count.
1. <b>FIFO</b> - Remove the physical page that was allocated the longest time ago.