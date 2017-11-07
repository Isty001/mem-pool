#include "internals.h"
#include "../include/mem_pool.h"


#define item_size(size) mem_align(sizeof(size_t)) + mem_align(size)


typedef struct SizedBlock SizedBlock;

struct SizedBlock {
    size_t size;
    void *data;
};

struct VariadicMemPool {
    size_t buff_size;
    Buffer *buff_head;
    Buffer *buff_last;
    SizedBlock *block_head;
    pthread_mutex_t mutex;
};


VariadicMemPool *pool_variadic_init(size_t grow_size)
{
    VariadicMemPool *pool = malloc(sizeof(VariadicMemPool));
    pool->buff_size = grow_size;
    pool->buff_head = buffer_new(grow_size);
    pool->buff_last = pool->buff_head;
    pool->buff_head = NULL;

    return pool;
}

static Buffer *add_buffer(Buffer *prev, size_t size)
{
    prev->next = buffer_new(size);

    return prev->next;
}

static void *from_buffer(Buffer *buff, size_t size, size_t header_size, size_t block_size)
{
    size_t *from = buff->current;
    *from = size;
    buff->current += (header_size + block_size);

    return from + header_size;
}

void *pool_variadic_alloc(VariadicMemPool *pool, size_t size)
{
    lock(pool);

    Buffer *buff = pool->buff_last;
    size_t header_size = mem_align(sizeof(size_t));
    size_t block_size = mem_align(size);
    void *ptr = NULL;

    if (!buffer_has_space(buff, header_size + block_size)) {
        buff->next = buffer_new(header_size + max(pool->buff_size, block_size));
        buff = buff->next;
    }
    ptr = from_buffer(buff, size, header_size, block_size);
    unlock(pool);

    return ptr;
}

bool pool_variadic_has(VariadicMemPool *pool, void *ptr);

int pool_variadic_free(VariadicMemPool *pool, void *ptr);
