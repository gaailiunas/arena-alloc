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
