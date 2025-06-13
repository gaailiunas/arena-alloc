#ifndef _ARENA_ALLOC_H
#define _ARENA_ALLOC_H

#include <stddef.h>
#include <stdlib.h>

#define ALIGNMENT sizeof(int)
#define ALIGN_UP(n, alignment) ((n + alignment - 1) & ~(alignment - 1))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct arena_s {
    char *mem;
    size_t sz;
    size_t offset;
} arena_t;

/* `initial_sz` in bytes */
arena_t *arena_new(size_t initial_sz)
{
    arena_t *arena = (arena_t *)malloc(sizeof(*arena));
    if (arena) {
        arena->mem = (char *)malloc(initial_sz);
        if (!arena->mem) {
            free(arena);
            return NULL;
        }
        arena->sz = initial_sz;
        arena->offset = 0;
    }
    return arena;
}

void *arena_alloc(arena_t *arena, size_t nbytes)
{
    arena->offset = ALIGN_UP(arena->offset, ALIGNMENT);
    if (arena->offset + nbytes > arena->sz)
        return NULL;
    
    void *ptr = arena->mem + arena->offset;
    arena->offset += nbytes;
    return ptr;
}

void arena_reset(arena_t *arena)
{
    arena->offset = 0;
}

size_t arena_get_size(arena_t *arena)
{
    return arena->sz;
}

void arena_free(arena_t *arena)
{
    if (arena->mem)
        free(arena->mem);
    free(arena); 
}

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

#include <stdexcept>

namespace arena {
class Arena {
    public:
        /* `initial_sz` in bytes */
        Arena(const std::size_t initial_sz)
        {
            this->_arena = arena_new(initial_sz);
            if (!this->_arena)
                throw std::runtime_error("Failed to allocate arena");
        }

        ~Arena()
        {
            arena_free(this->_arena);
        }

        template <typename T>
        T *alloc(std::size_t count = 1) 
        {
            void *ptr = arena_alloc(this->_arena, sizeof(T) * count);
            return ptr ? static_cast<T *>(ptr) : nullptr;
        }

        void reset()
        {
            arena_reset(this->_arena);
        }

        std::size_t size()
        {
            return arena_get_size(this->_arena);
        }

    private:
        arena_t *_arena;
};

} // namespace arena
#endif // __cplusplus

#endif // _ARENA_ALLOC_H
