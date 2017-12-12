#ifndef MEM_POOL_INTERNALS_H
#define MEM_POOL_INTERNALS_H


#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "../include/mem_pool.h"


#define check_pthread(res, err)     \
    if (0 != (res)) {               \
        return err;                 \
    }

#define mutex_init(pool)                                                            \
    check_pthread(pthread_mutex_init(&pool->mutex, NULL), MEM_POOL_ERR_MUTEX_INIT);

#define mutext_destroy(pool) \
    check_pthread(pthread_mutex_destroy(&pool->mutex), MEM_POOL_ERR_MUTEX_DESTROY);

#define lock(pool)                                                      \
    check_pthread(pthread_mutex_lock(&pool->mutex), MEM_POOL_ERR_LOCK);

#define unlock(pool)                                                        \
    check_pthread(pthread_mutex_unlock(&pool->mutex), MEM_POOL_ERR_UNLOCK);

#define pool_destroy(pool)                  \
    buffer_list_destroy(pool->buff_head);   \
    mutext_destroy(pool)                    \
    free(pool);

#define buffer_list_has(head, ptr) (NULL != buffer_list_find(head, ptr))


typedef struct Buffer Buffer;

struct Buffer {
    void *start;
    void *prev_ptr;
    void *curr_ptr; /* This is only tracked for variadic blocks */
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
    return (char *)buff->end - (char *)buff->curr_ptr >= (long)size;
}

static inline bool buffer_has(Buffer *buff, void *ptr)
{
    return ptr >= buff->start && ptr <= buff->end;
}


#endif
