#ifndef _ARENA_ALLOC_H
#define _ARENA_ALLOC_H

#include <stddef.h>
#include <stdlib.h>

#define ARENA_DEFAULT_ALIGNMENT 8
#define ALIGN_UP(n, alignment) ((n + alignment - 1) & ~(alignment - 1))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct arena_s {
    char *mem;
    size_t sz;
    size_t offset;
    size_t alignment;
} arena_t;

/* `initial_sz` in bytes */
static inline arena_t *arena_new(size_t initial_sz, size_t alignment)
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
        arena->alignment = alignment;
    }
    return arena;
}

static inline void *arena_alloc(arena_t *arena, size_t nbytes)
{
    arena->offset = ALIGN_UP(arena->offset, arena->alignment);
    if (arena->offset + nbytes > arena->sz)
        return NULL;
    
    void *ptr = arena->mem + arena->offset;
    arena->offset += nbytes;
    return ptr;
}

static inline void arena_reset(arena_t *arena)
{
    arena->offset = 0;
}

static inline size_t arena_get_size(arena_t *arena)
{
    return arena->sz;
}

static inline void arena_free(arena_t *arena)
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
        Arena(const std::size_t initial_sz, const std::size_t alignment = ARENA_DEFAULT_ALIGNMENT) : _dtor_head(nullptr)
        {
            this->_arena = arena_new(initial_sz, alignment);
            if (!this->_arena)
                throw std::runtime_error("Failed to allocate arena");
        }

        ~Arena()
        {
            this->destroy();
            arena_free(this->_arena);
        }

        template <typename T, std::size_t N = 1, typename... Args>
        T *alloc(Args &&... args) 
        {
            void *ptr = arena_alloc(this->_arena, sizeof(T) * N + sizeof(DestructorNode));
            if (!ptr)
                return nullptr;

            T *typed = static_cast<T *>(ptr);
            DestructorNode *node = reinterpret_cast<DestructorNode *>(typed + N);

            for (std::size_t i = 0; i < N; i++)
                new (typed + i) T(std::forward<Args>(args)...);

            node->next = this->_dtor_head;
            node->dtor =
                [](void *p, std::size_t n) {
                    T *tp = static_cast<T *>(p);
                    for (std::size_t i = 0; i < n; i++)
                        tp[i].~T();
                };
            node->obj = ptr;
            node->count = N;
            this->_dtor_head = node;

            return typed; 
        }

        void reset()
        {
            this->destroy();
            arena_reset(this->_arena);
        }

        std::size_t size()
        {
            return arena_get_size(this->_arena);
        }

    private:
        struct DestructorNode {
            DestructorNode *next;
            void (*dtor)(void *, std::size_t);
            void *obj;
            std::size_t count;
        };

        arena_t *_arena;
        DestructorNode *_dtor_head;

        void destroy()
        {
            DestructorNode *cur = this->_dtor_head;
            while (cur) {
                cur->dtor(cur->obj, cur->count);
                cur = cur->next;
            }
        }
};

} // namespace arena
#endif // __cplusplus

#endif // _ARENA_ALLOC_H
