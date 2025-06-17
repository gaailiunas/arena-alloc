#ifndef ARENA_ALLOC_H
#define ARENA_ALLOC_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#ifdef __cplusplus
    #if __cplusplus >= 201103L
        #define ARENA_ALIGNOF(type) alignof(type)
    #elif defined(__GNUC__) || defined(__clang__)
        #define ARENA_ALIGNOF(type) __alignof__(type)
    #elif defined(_MSC_VER)
        #define ARENA_ALIGNOF(type) __alignof(type)
    #else
        #error "No alignment operator available for this C++ compiler"
    #endif
#else  // C language
    #if __STDC_VERSION__ >= 202311L
        #define ARENA_ALIGNOF(type) alignof(type)
    #elif __STDC_VERSION__ >= 201112L
        #define ARENA_ALIGNOF(type) _Alignof(type)
    #elif defined(__GNUC__) || defined(__clang__)
        #define ARENA_ALIGNOF(type) __alignof__(type)
    #elif defined(_MSC_VER)
        #define ARENA_ALIGNOF(type) __alignof(type)
    #else
        #error "No alignment operator available for this C compiler"
    #endif
#endif

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || \
    (defined(__cplusplus) && __cplusplus >= 201103L)
#define ARENA_DEFAULT_ALIGNMENT ARENA_ALIGNOF(max_align_t)
#else
#define ARENA_DEFAULT_ALIGNMENT 16
#endif

#define ALIGN_UP(ptr, alignment) ((void *)(((uintptr_t)(ptr) + (alignment) - 1) & ~((alignment) - 1)))

#define ARENA_ALLOC_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define ARENA_ALLOC_2(arena, type) \
    ((type *)arena_alloc((arena), sizeof(type), ARENA_ALIGNOF(type)))
#define ARENA_ALLOC_3(arena, type, count) \
    ((type *)arena_alloc((arena), sizeof(type) * (count), ARENA_ALIGNOF(type)))

#define ARENA_ALLOC(...) \
    ARENA_ALLOC_GET_MACRO(__VA_ARGS__, ARENA_ALLOC_3, ARENA_ALLOC_2)(__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct mem_arena {
    char *mem;
    size_t sz;
    size_t offset;

    // when cleaning up, we know whether to free `mem` or not
    unsigned char stack; // 0 or 1
};

/*
 * if `mem` is provided, its size must be `initial_sz` bytes
 */
static inline int arena_init(struct mem_arena *arena, char *mem, size_t initial_sz)
{
    if (!mem) {
        arena->mem = (char *)malloc(initial_sz);
        if (!arena->mem)
            return 1;
        arena->stack = 0;
    }
    else {
        arena->mem = mem;
        arena->stack = 1;
    }
    arena->sz = initial_sz;
    arena->offset = 0;
    return 0;
}

/*
 * Allocates a new memory space on the heap.
 * `initial_sz` in bytes
 */
static inline struct mem_arena *arena_new(size_t initial_sz)
{
    struct mem_arena *arena = (struct mem_arena *)malloc(sizeof(*arena));
    if (arena) {
        if (arena_init(arena, NULL, initial_sz) != 0) {
            free(arena);
            return NULL;
        }
    }
    return arena;
}

static inline void *arena_alloc(struct mem_arena *arena, size_t nbytes, size_t alignment)
{
    void *ptr = arena->mem + arena->offset;
    void *ptr_aligned = ALIGN_UP(ptr, alignment);
    size_t new_offset = (char *)ptr_aligned - arena->mem + nbytes;
    if (new_offset > arena->sz)
        return NULL;

    arena->offset = new_offset;
    return ptr_aligned;
}

static inline void *arena_alloc_default(struct mem_arena *arena, size_t nbytes)
{
    return arena_alloc(arena, nbytes, ARENA_DEFAULT_ALIGNMENT);
}

static inline void arena_reset(struct mem_arena *arena)
{
    arena->offset = 0;
}

static inline size_t arena_get_size(const struct mem_arena *arena)
{
    return arena->sz;
}

static inline void arena_cleanup(struct mem_arena *arena)
{
    if (arena->mem && arena->stack == 0)
        free(arena->mem);
    arena->offset = 0;
    arena->sz = 0;
}

static inline void arena_free(struct mem_arena *arena)
{
    arena_cleanup(arena);
    free(arena); 
}

#ifdef __cplusplus
}
#endif // __cplusplus

#ifdef __cplusplus

#include <stdexcept>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace arena {
class Arena {
    public:
        /* `initial_sz` in bytes */
        explicit Arena(const std::size_t initial_sz) : _dtor_head(nullptr)
        {
            if (arena_init(&this->_arena, nullptr, initial_sz) != 0)
                throw std::runtime_error("Failed to initialize arena");
        }

        /* `initial_sz` in bytes */
        explicit Arena(char *mem, const std::size_t initial_sz) : _dtor_head(nullptr)
        {
            if (arena_init(&this->_arena, mem, initial_sz) != 0)
                throw std::runtime_error("Failed to initialize arena");
        }

        ~Arena()
        {
            this->destroy();
            arena_cleanup(&this->_arena);
        }

        template <typename T, std::size_t N = 1, std::size_t alignment = ARENA_ALIGNOF(T), typename... Args>
        T *alloc(Args &&... args) 
        {
            void *ptr = arena_alloc(&this->_arena, sizeof(T) * N, alignment);
            if (!ptr)
                return nullptr;
          
            T *typed = static_cast<T *>(ptr);

            if constexpr (!std::is_trivially_constructible_v<T>) {
                for (std::size_t i = 0; i < N; i++)
                    new (typed + i) T(std::forward<Args>(args)...);
            }
            
            if constexpr (!std::is_trivially_destructible_v<T>) {
                DestructorNode *node = reinterpret_cast<DestructorNode *>(
                        arena_alloc(&this->_arena, sizeof(DestructorNode), alignof(DestructorNode)));
                if (!node)
                    return nullptr;

                node->next = this->_dtor_head;
                node->dtor = [](void *p, std::size_t n) {
                    T *tp = static_cast<T *>(p);
                    for (std::size_t i = 0; i < n; i++)
                        tp[i].~T();
                };
                node->obj = ptr;
                node->count = N;
                this->_dtor_head = node;
            } 

            return typed; 
        }

        void *alloc_raw(std::size_t nbytes, std::size_t alignment = ARENA_DEFAULT_ALIGNMENT)
        {
            return arena_alloc(&this->_arena, nbytes, alignment);
        }

        void reset()
        {
            this->destroy();
            arena_reset(&this->_arena);
        }

        std::size_t size() const
        {
            return arena_get_size(&this->_arena);
        }

    private:
        struct DestructorNode {
            DestructorNode *next;
            void (*dtor)(void *, std::size_t);
            void *obj;
            std::size_t count;
        };

        struct mem_arena _arena;
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
            void *ptr = this->_arena->alloc_raw(n * sizeof(T), ARENA_ALIGNOF(T));
            if (!ptr)
                throw std::bad_alloc();
            return static_cast<T *>(ptr);
        }

        /*
         * since stl containers track references of objects, that becomes a massive overhead.
         */
        void deallocate(T *p, size_type n) noexcept
        {
            // this is an arena allocator and we don't have to deallocate every object individually
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
