#include <stdio.h>
#include <stdalign.h>
#include <stddef.h>
#include "mem_pool.h"


typedef struct buffer Buffer;

typedef struct block Block;


struct buffer
{
    void *memory;
    void *end;
    Buffer *next;
};

struct block
{
    Block *next;
};

struct mem_pool
{
    size_t memb_size;
    size_t buff_size;
    Buffer *buff_head;
    Buffer *buff_last;
    Block *block_head;
};


static Buffer *new_buffer(MemPool *pool)
{
    Buffer *buff = malloc(sizeof(Buffer));
    buff->memory = malloc(pool->buff_size);
    buff->next = NULL;
    buff->end = buff->memory + pool->buff_size;

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

    return pool;
}

static void *from_free_list(MemPool *pool)
{
    Block *tmp = pool->block_head;
    pool->block_head = tmp->next;

    return tmp;
}

static void *from_buffer(MemPool *pool, Buffer *buff)
{
    void *ptr = buff->memory;
    buff->memory += pool->memb_size;

    return ptr;
}

void *pool_alloc(MemPool *pool)
{
    if (pool->block_head) {
        return from_free_list(pool);
    }
    Buffer *buff = pool->buff_last;

    if (buff->memory == buff->end) {
        buff = new_buffer(pool);
        pool->buff_last->next = buff;
        pool->buff_last = buff;
    }

    return from_buffer(pool, buff);
}

bool pool_has_ptr(MemPool *pool, void *ptr)
{
    void *from;
    Buffer *buff = pool->buff_head;

    while (buff) {
        from = buff->end - pool->buff_size;

        if (ptr >= from && ptr <= buff->end) {
            return true;
        }
        buff = buff->next;
    }
    return false;
}

int pool_free(MemPool *pool, void *ptr)
{
    if (!pool_has_ptr(pool, ptr)) {
        return -1;
    }

    Block *new = (Block *) ptr;
    Block *tmp = pool->block_head;
    pool->block_head = new;
    new->next = tmp;

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
    free(pool);
}
