#include "internals.h"


Buffer *buffer_new(size_t size)
{
    Buffer *buff = malloc(sizeof(Buffer));
    buff->start = malloc(size);
    buff->current = buff->start;
    buff->next = NULL;
    buff->end = buff->current + size;

    return buff;
}
