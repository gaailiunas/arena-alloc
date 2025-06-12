# Arena allocator
A simple and fast arena allocator designed to be lightweight with memory alignment in mind. It does not support allocation of objects with constructors or destructors, so RAII is not applicable.

## Usage 
```cpp
#include <arena_alloc.hpp>
#include <iostream>

using namespace arena_alloc;

int main(void)
{
    Arena arena(1024 * 2); // 2 KB
    int *arr = arena.alloc<int>(2);
    arr[0] = 1337;
    arr[1] = 2000;
    std::cout << arr[0] << " " << arr[1] << std::endl;    
    return 0;
}
```

## Building
```bash
git clone https://github.com/gaailiunas/arena-alloc.git
cd arena-alloc
mkdir build && cd build
cmake ..
cmake --build .
```

## Installing
```bash
sudo cmake --install .
```
