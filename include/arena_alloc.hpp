#ifndef _ARENA_ALLOC_HPP
#define _ARENA_ALLOC_HPP

#include <cstddef>

namespace arena_alloc {

static constexpr int ALIGNMENT = sizeof(int);

class Arena {
    public:
        /* `initial_sz` in bytes */
        Arena(const std::size_t initial_sz);
        ~Arena();

        template <typename T>
        T *alloc(std::size_t count = 1) 
        {
            void *ptr = alloc_bytes(sizeof(T) * count);
            return ptr ? static_cast<T *>(ptr) : nullptr;
        }

        void reset()
        {
            this->offset = 0;
        }

        std::size_t size()
        {
            return this->sz;
        }

    private:
        char *mem;
        std::size_t sz;
        std::size_t offset;

        std::size_t align_up(std::size_t n, std::size_t alignment)
        {
            return (n + alignment - 1) & ~(alignment - 1);
        }

        void *alloc_bytes(std::size_t n);
};

} // namespace arena_alloc

#endif // _ARENA_ALLOC_HPP
