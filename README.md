# Arena allocator
A simple and fast arena allocator designed to be lightweight with memory alignment in mind.
In C++, the allocator does not support allocation of objects with constructors or destructors, so RAII is not applicable.

## Usage 
```cpp
#include <arena_alloc.h>
#include <iostream>

using namespace arena;

int main(void)
{
    Arena arena(1024 * 2); // 2 KB
    int *arr = arena.alloc<int>(2);
    if (arr) {
        arr[0] = 1337;
        arr[1] = 2000;
        std::cout << arr[0] << " " << arr[1] << std::endl;    
    }
    return 0;
}
```

```c
#include <arena_alloc.h>
#include <stdio.h>

int main(void)
{
    arena_t *arena = arena_new(1024 * 2); // 2 KB
    if (arena) {
        int *arr = (int *)arena_alloc(arena, sizeof(int) * 2);
        if (arr) {
            arr[0] = 1337;
            arr[1] = 2000;
            printf("%d %d\n", arr[0], arr[1]);
        }
        arena_free(arena);
    }
    return 0;
}
```

## Setup
```bash
git clone https://github.com/gaailiunas/arena-alloc.git
cd arena-alloc
mkdir build && cd build
cmake ..
```

## Installing
```bash
sudo cmake --install .
```
