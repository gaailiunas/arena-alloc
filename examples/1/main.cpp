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
    // ctors and dtors are called from both t1 and t2

    return 0;
}
