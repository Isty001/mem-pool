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

#define pool_destroy(pool)                  \
    buffer_list_destroy(pool->buff_head);   \
    pthread_mutex_destroy(&pool->mutex);    \
    free(pool);

#define buffer_list_has(head, ptr) (NULL != buffer_list_find(head, ptr))


typedef struct Buffer Buffer;

struct Buffer {
    void *start;
    void *current;
    void *end;
    Buffer *next;
};

typedef struct Header Header;

typedef struct SizedBlock SizedBlock;

struct Header {
    size_t size;
    SizedBlock *prev_in_buff;
};


static inline size_t max(size_t a, size_t b)
{
    return a > b ? a : b;
}


Buffer *buffer_new(size_t size);

void buffer_list_destroy(Buffer *head);

Buffer *buffer_list_find(Buffer *head, void *ptr);

static inline bool buffer_has_space(Buffer *buff, size_t size)
{
    return buff->end - buff->current >= (long)size;
}

static inline bool buffer_has(Buffer *buff, void *ptr)
{
    return ptr >= buff->start && ptr <= buff->end;
}


#endif
