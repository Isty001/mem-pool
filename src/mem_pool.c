#include <stdio.h>
#include <stdalign.h>
#include <stddef.h>
#include <pthread.h>
#include "mem_pool.h"


#define check(res)                              \
    if (0 != res) {                             \
        fprintf(stderr, "pthread failure");     \
        exit(EXIT_FAILURE);                     \
    }

#define lock(pool) \
    check(pthread_mutex_lock(&pool->mutex));

#define unlock(pool)                                \
    check(pthread_cond_broadcast(&pool->cond));     \
    check(pthread_mutex_unlock(&pool->mutex));


typedef struct buffer Buffer;

typedef struct block Block;


struct buffer {
    void *start;
    void *current;
    void *end;
    Buffer *next;
};

struct block {
    Block *next;
};

struct mem_pool {
    size_t memb_size;
    size_t buff_size;
    Buffer *buff_head;
    Buffer *buff_last;
    Block *block_head;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};


static Buffer *new_buffer(MemPool *pool)
{
    Buffer *buff = malloc(sizeof(Buffer));
    buff->start = malloc(pool->buff_size);
    buff->current = buff->start;
    buff->next = NULL;
    buff->end = buff->current + pool->buff_size;

    return buff;
}

static size_t align(size_t block_size)
{
    block_size = block_size < sizeof(Block) ? sizeof(Block) : block_size;
    size_t align = alignof(max_align_t);

    if (block_size % align) {
        return block_size + (align - block_size % align);
    }

    return block_size;
}

MemPool *pool_init(size_t block_size, size_t increase_count)
{
    MemPool *pool = malloc(sizeof(MemPool));
    pool->memb_size = align(block_size);
    pool->buff_size = increase_count * pool->memb_size;
    pool->buff_head = new_buffer(pool);
    pool->buff_last = pool->buff_head;
    pool->block_head = NULL;

    check(pthread_mutex_init(&pool->mutex, NULL));
    check(pthread_cond_init(&pool->cond, NULL));

    return pool;
}

static void *from_free_list(MemPool *pool)
{
    Block *tmp = pool->block_head;
    pool->block_head = tmp->next;
    unlock(pool);

    return tmp;
}

static void *from_buffer(MemPool *pool, Buffer *buff)
{
    void *ptr = buff->current;
    buff->current += pool->memb_size;
    unlock(pool);

    return ptr;
}

void *pool_alloc(MemPool *pool)
{
    lock(pool);

    if (pool->block_head) {
        return from_free_list(pool);
    }
    Buffer *buff = pool->buff_last;

    if (buff->current == buff->end) {
        buff = new_buffer(pool);
        pool->buff_last->next = buff;
        pool->buff_last = buff;
    }

    return from_buffer(pool, buff);
}

bool pool_has_ptr(MemPool *pool, void *ptr)
{
    lock(pool);

    void *from;
    Buffer *buff = pool->buff_head;

    while (buff) {
        from = buff->end - pool->buff_size;

        if (ptr >= from && ptr <= buff->end) {
            unlock(pool);

            return true;
        }
        buff = buff->next;
    }
    unlock(pool);

    return false;
}

void pool_foreach(MemPool *pool, PoolForeach callback)
{
    lock(pool);

    Buffer *buff = pool->buff_head;

    while (buff) {
        for (void *block = buff->start; block < buff->current; block += pool->memb_size) {
            callback(block);
        }
        buff = buff->next;
    }
    unlock(pool);
}

int pool_free(MemPool *pool, void *ptr)
{
    if (!pool_has_ptr(pool, ptr)) {
        return -1;
    }

    lock(pool);

    Block *new = (Block *) ptr;
    Block *tmp = pool->block_head;
    pool->block_head = new;
    new->next = tmp;
    unlock(pool);

    return 0;
}

void pool_destroy(MemPool *pool)
{
    Buffer *buff, *head = pool->buff_head;

    while (head) {
        buff = head;
        head = head->next;
        free(buff->end - pool->buff_size);
        free(buff);
    }

    pthread_mutex_destroy(&pool->mutex);
    pthread_cond_destroy(&pool->cond);
    free(pool);
}
