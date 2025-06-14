#include <arena_alloc.h>
#include <stdio.h>

int main(void)
{
    struct mem_arena *arena = arena_new(1024 * 2, ARENA_DEFAULT_ALIGNMENT); // 2 KB
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

