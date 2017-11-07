#ifndef MEM_POOL_INTERNALS_H
#define MEM_POOL_INTERNALS_H


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#define check(res)                              \
    if (0 != res) {                             \
        fprintf(stderr, "pthread failure");     \
        exit(EXIT_FAILURE);                     \
    }

#define lock(pool) \
    check(pthread_mutex_lock(&pool->mutex));

#define unlock(pool)                                \
    check(pthread_mutex_unlock(&pool->mutex));


typedef struct Buffer Buffer;

struct Buffer {
    void *start;
    void *current;
    void *end;
    Buffer *next;
};


static inline size_t max(size_t a, size_t b)
{
    return a > b ? a : b;
}


Buffer *buffer_new(size_t size);

static inline bool buffer_has_space(Buffer *buff, size_t size)
{
    return buff->end - buff->current >= (long)size;
}


#endif
