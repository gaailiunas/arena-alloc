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
