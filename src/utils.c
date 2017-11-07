#include <stdalign.h>
#include <stddef.h>


size_t mem_align(size_t size)
{
    size_t align = alignof(max_align_t);

    if (size % align) {
        return size + (align - size % align);
    }

    return size;
}

