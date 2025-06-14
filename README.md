# Arena allocator
A simple and fast arena allocator designed to be lightweight with memory alignment in mind.
C++ objects are managed via intrusive linked lists that store destructor function pointers directly within the arena memory.

## Usage 
```cpp
#include <arena_alloc.h>
#include <iostream>

using namespace arena;

struct Test1 {
    Test1() {
        std::cout << "ctor 1" << std::endl;
    }
    ~Test1() {
        std::cout << "dtor 1" << std::endl;
    }
};

struct Test2 {
    int _a, _b;
    Test2(int a, int b) {
        _a = a;
        _b = b;
        std::cout << "ctor 2: " << _a << " " << _b << std::endl;
    }
    ~Test2() {
        std::cout << "dtor 2: " << _a << " " << _b << std::endl;
    }
};

int main(void)
{
    Arena arena(1024 * 2); // 2 KB
    int *arr = arena.alloc<int, 2>();
    if (arr) {
        arr[0] = 1337;
        arr[1] = 2000;
        std::cout << arr[0] << " " << arr[1] << std::endl;    
    }

    Test1 *t1 = arena.alloc<Test1>();
    Test2 *t2 = arena.alloc<Test2>(100, 200);
    // ctors and dtors are called from both t1 and t2 (FILO order)

    return 0;
}
```

STL support:
```cpp
#include <arena_alloc.h>
#include <iostream>
#include <vector>

using namespace arena;

int main(void)
{
    Arena arena(1024 * 2); // 2 KB
    ArenaAllocator<int> allocator(&arena);
    std::vector<int, ArenaAllocator<int>> vec(allocator);

    vec.push_back(42);
    vec.push_back(100);

    for (const int &x : vec) {
        std::cout << x << std::endl;
    }

    return 0;
}
```

```c
#include <arena_alloc.h>
#include <stdio.h>

int main(void)
{
    arena_t *arena = arena_new(1024 * 2, ARENA_DEFAULT_ALIGNMENT); // 2 KB
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
Additionally, you can just paste [arena_alloc.h](include/arena_alloc.h) in your project structure and use it from there.
