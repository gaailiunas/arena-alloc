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
