# Arena allocator
A lightweight, high-performance memory arena allocator library for C and C++ with zero runtime overhead.

## Features
- **Header Only**: Single file integration with no external dependencies
- **Dual Language Support**: Clean C API with optional C++ wrapper and STL integration
- **Memory Alignment**: Configurable alignment support for optimal performance
- **Linear Allocation**: Zero fragmentation with O(1) allocation, with O(n) destructor cleanup for C++ objects
- **Object Management**: Automatic destructor tracking and cleanup for C++ objects
- **STL Compatible**: Drop-in allocator for standard containers (vector, map, etc.)
- **Raw + Typed**: Supports both byte-level and type-safe object allocation

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

struct Foo {
    int _x;
    Foo(int x)
    {
        _x = x;
        std::cout << "ctor " << _x << std::endl;
    }

    ~Foo() {
        std::cout << "dtor " << _x << std::endl;
    }
};

int main(void)
{
    // vectors manage memory in their own way, not depending on our dtor intrusive linked lists.

    Arena arena(1024 * 2); // 2 KB

    ArenaAllocator<int> base_allocator(&arena);
    std::vector<int, ArenaAllocator<int>> vec1(base_allocator);

    vec1.push_back(42);
    vec1.push_back(100);

    for (const int &x : vec1) {
        std::cout << x << std::endl;
    }

    ArenaAllocator<Foo> foo_allocator(base_allocator); // rebinding to Foo
    std::vector<Foo, ArenaAllocator<Foo>> vec2(foo_allocator);
    vec2.reserve(2);
    
    vec2.emplace_back(10);
    vec2.emplace_back(20);

    return 0;
}
```

```c
#include <arena_alloc.h>
#include <stdio.h>

int main(void)
{
    struct mem_arena *arena = arena_new(1024 * 2); // 2 KB
    if (arena) {
        int *arr = (int *)arena_alloc_default(arena, sizeof(int) * 2);

        /* if on >=C11, use this:
        // int *arr = (int *)arena_alloc(arena, sizeof(int) * 2, _Alignof(int));
        */

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
