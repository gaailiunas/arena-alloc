#ifndef ARENA_ALLOC_H
#define ARENA_ALLOC_H

#include <stddef.h>
#include <stdlib.h>

#define ARENA_DEFAULT_ALIGNMENT 8
#define ALIGN_UP(n, alignment) ((n + alignment - 1) & ~(alignment - 1))

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct mem_arena {
    char *mem;
    size_t sz;
    size_t offset;
    size_t alignment;
};

/* `initial_sz` in bytes */
static inline struct mem_arena *arena_new(size_t initial_sz, size_t alignment)
{
    struct mem_arena *arena = (struct mem_arena *)malloc(sizeof(*arena));
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

static inline void *arena_alloc(struct mem_arena *arena, size_t nbytes)
{
    arena->offset = ALIGN_UP(arena->offset, arena->alignment);
    if (arena->offset + nbytes > arena->sz)
        return NULL;
    
    void *ptr = arena->mem + arena->offset;
    arena->offset += nbytes;
    return ptr;
}

static inline void arena_reset(struct mem_arena *arena)
{
    arena->offset = 0;
}

static inline size_t arena_get_size(const struct mem_arena *arena)
{
    return arena->sz;
}

static inline void arena_free(struct mem_arena *arena)
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
#include <cstddef>
#include <memory>

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

        void *alloc_raw(std::size_t nbytes)
        {
            return arena_alloc(this->_arena, nbytes);
        }

        void reset()
        {
            this->destroy();
            arena_reset(this->_arena);
        }

        std::size_t size() const
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

        struct mem_arena *_arena;
        DestructorNode *_dtor_head;

        void destroy()
        {
            DestructorNode *cur = this->_dtor_head;
            while (cur) {
                cur->dtor(cur->obj, cur->count);
                cur = cur->next;
            }
            this->_dtor_head = nullptr;
        }
};

template <typename T>
class ArenaAllocator {
    public:
        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using const_pointer = const T *;

        explicit ArenaAllocator(Arena *arena) noexcept : _arena(arena) {}
        ArenaAllocator(const ArenaAllocator&) noexcept = default;
        ArenaAllocator& operator=(const ArenaAllocator&) noexcept = default;
        
        template <typename U>
        struct rebind {
            using other = ArenaAllocator<U>;
        };

        template <typename U>
        ArenaAllocator(const ArenaAllocator<U>& other) noexcept : _arena(other._arena) {}

        T *allocate(size_type n)
        {
            void *ptr = this->_arena->alloc_raw(n * sizeof(T));
            if (!ptr)
                throw std::bad_alloc();
            return static_cast<T *>(ptr);
        }

        void deallocate(T *p, size_type n) noexcept
        {
            // this is an arena allocator and we don't have to deallocate every object individually
        }
        
        template <typename U, typename ...Args>
        void construct(U *p, Args&&... args)
        {
            new(p) U(std::forward<Args>(args)...);
        }

        template <typename U>
        void destroy(U *p)
        {
            p->~U();
        }

        template<typename U>
        bool operator==(const ArenaAllocator<U>& other) const noexcept
        {
            return _arena == other._arena;
        }

        template<typename U>
        bool operator!=(const ArenaAllocator<U>& other) const noexcept
        {
            return !(*this == other);
        }

        template<typename U> friend class ArenaAllocator;
        Arena *get_arena() const { return _arena; }

    private:
        Arena *_arena;
};

} // namespace arena
#endif // __cplusplus

#endif // ARENA_ALLOC_H
