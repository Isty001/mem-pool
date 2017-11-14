#include <stdio.h>
#include "internals.h"
#include "../include/mem_pool.h"


typedef struct Block Block;


struct Block {
    Block *next;
};

struct FixedMemPool {
    size_t memb_size;
    size_t buff_size;
    Buffer *buff_head;
    Buffer *buff_last;
    Block *block_head;
    pthread_mutex_t mutex;
};


FixedMemPool *pool_fixed_init(size_t block_size, size_t increase_count)
{
    FixedMemPool *pool = malloc(sizeof(FixedMemPool));
    pool->memb_size = mem_align(block_size < sizeof(Block) ? sizeof(Block) : block_size);
    pool->buff_size = increase_count * pool->memb_size;
    pool->buff_head = buffer_new(pool->buff_size);
    pool->buff_last = pool->buff_head;
    pool->block_head = NULL;

    check(pthread_mutex_init(&pool->mutex, NULL));

    return pool;
}

static void *from_free_list(FixedMemPool *pool)
{
    Block *tmp = pool->block_head;
    pool->block_head = tmp->next;
    unlock(pool);

    return tmp;
}

static void *from_buffer(FixedMemPool *pool, Buffer *buff)
{
    void *ptr = buff->current;
    buff->current += pool->memb_size;
    unlock(pool);

    return ptr;
}

void *pool_fixed_alloc(FixedMemPool *pool)
{
    lock(pool);

    if (pool->block_head) {
        return from_free_list(pool);
    }
    Buffer *buff = pool->buff_last;

    if (buff->current == buff->end) {
        buff = buffer_new(pool->buff_size);
        pool->buff_last->next = buff;
        pool->buff_last = buff;
    }

    return from_buffer(pool, buff);
}

static bool in_free_list(FixedMemPool *pool, void *ptr)
{
    Block *block = pool->block_head;

    while (block) {
        if (block == ptr) {
            return true;
        }
        block = block->next;
    }

    return false;
}

MemBlockInfo pool_fixed_block_info(FixedMemPool *pool, void *ptr)
{
    MemBlockInfo info = {.size = pool->memb_size};

    lock(pool);

    if (in_free_list(pool, ptr)) {
        info.state = MEM_BLOCK_FREE;
    } else if (buffer_list_has(pool->buff_head, ptr)) {
        info.state = MEM_BLOCK_ALLOCATED;
    } else {
        info.state = MEM_BLOCK_UNKOWN;
    }
    unlock(pool);

    return info;
}

void pool_fixed_foreach(FixedMemPool *pool, PoolForeach callback)
{
    lock(pool);

    Buffer *buff = pool->buff_head;

    while (buff) {
        for (void *block = buff->start; block < buff->current; block += pool->memb_size) {
            if (0 != callback(block)) {
                break;
            }
        }
        buff = buff->next;
    }
    unlock(pool);
}

int pool_fixed_free(FixedMemPool *pool, void *ptr)
{
    lock(pool);

    if (!buffer_list_has(pool->buff_head, ptr)) {
        unlock(pool);
        return -1;
    }

    Block *new = (Block *) ptr;
    Block *tmp = pool->block_head;
    pool->block_head = new;
    new->next = tmp;
    unlock(pool);

    return 0;
}

void pool_fixed_destroy(FixedMemPool *pool)
{
    pool_destroy(pool);
}
