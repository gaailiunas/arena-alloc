#include <arena_alloc.hpp>

using namespace arena_alloc;

Arena::Arena(const std::size_t initial_sz) : sz(initial_sz), offset(0)
{
    this->mem = new char[initial_sz];
}

Arena::~Arena()
{
    delete[] this->mem;
}

void* Arena::alloc_bytes(std::size_t n)
{
    this->offset = this->align_up(offset, ALIGNMENT);
    if (this->offset + n > this->sz)
        return nullptr; // out of memory

    void *ptr = static_cast<char *>(this->mem) + this->offset;
    this->offset += n;
    return ptr;
}

