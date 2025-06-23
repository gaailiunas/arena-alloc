#include <arena_alloc.h>
#include <stdio.h>

int main(void)
{
    //struct mem_arena *arena = arena_new(1024 * 2, 0);

    // arena can can grow in size
    struct mem_arena *arena = arena_new(1024 * 2, ARENA_FL_GROW); // 2 KB
    
    if (arena) {
        //int *arr = (int *)arena_alloc_default(arena, sizeof(int) * 2);
        int *arr = ARENA_ALLOC(arena, int, 2); // manages alignment internally

        if (arr) {
            arr[0] = 1337;
            arr[1] = 2000;
            printf("%d %d\n", arr[0], arr[1]);
        }

        int *a = ARENA_ALLOC(arena, int); // manages alignment internally
        *a = 200;
        printf("%d\n", *a);

        arena_free(arena);
    }
    return 0;
}

