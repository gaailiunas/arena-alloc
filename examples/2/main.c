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

