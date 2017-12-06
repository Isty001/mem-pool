#include "internals.h"
#include "../include/mem_pool.h"


// The size of a block cannot be 0, so if we are looking 
#define SIZE_RESETED 0


struct SizedBlock {
    Header header;
    SizedBlock *next_in_free_list;
};

struct VariadicMemPool {
    size_t buff_size;
    size_t header_size;
    int16_t tolerance_percent;
    Buffer *buff_head;
    Buffer *buff_last;
    SizedBlock *block_head;
    pthread_mutex_t mutex;
};


VariadicMemPool *pool_variadic_init(size_t grow_size, int16_t tolerance_percent)
{
    VariadicMemPool *pool = malloc(sizeof(VariadicMemPool));
    pool->tolerance_percent = tolerance_percent;
    pool->header_size = mem_align(sizeof(Header));
    pool->buff_size = grow_size;
    pool->buff_head = buffer_new(grow_size);
    pool->buff_last = pool->buff_head;
    pool->block_head = NULL;

    check(pthread_mutex_init(&pool->mutex, NULL));

    return pool;
}

static void *from_buffer(Buffer *buff, size_t header_size, size_t block_size)
{
    Header *header = buff->curr_ptr;
    header->size = block_size;
    header->prev_in_buff = buff->prev_ptr;

    buff->prev_ptr = buff->curr_ptr;
    buff->curr_ptr += (header_size + block_size);

    return (char *)header + header_size;
}

static void *best_fit_from_free_list(VariadicMemPool *pool, size_t required_size)
{
    SizedBlock **curr = &pool->block_head;
    int64_t block_size, diff;
    int16_t diff_percent;

    while (*curr) {
        block_size = (*curr)->header.size;
        diff = labs(block_size - (long)required_size);
        diff_percent = (diff * 100) / ((block_size + required_size) / 2);

        if (MEM_NO_BEST_FIT == pool->tolerance_percent || diff_percent <= pool->tolerance_percent) {
            SizedBlock *block = *curr;
            *curr = (*curr)->next_in_free_list;

            return (char *)block + pool->header_size;
        }
        curr = &(*curr)->next_in_free_list;
    }

    return NULL;
}

void *pool_variadic_alloc(VariadicMemPool *pool, size_t size)
{
    lock(pool);

    Buffer *buff = pool->buff_last;
    size_t block_size = mem_align(size);
    void *ptr = NULL;

    if (pool->block_head && (ptr = best_fit_from_free_list(pool, block_size))) {
        unlock(pool);

        return ptr;
    }

    if (!buffer_has_space(buff, pool->header_size + block_size)) {
        buff->next = buffer_new(pool->header_size + max(pool->buff_size, block_size));
        buff = buff->next;
    }
    ptr = from_buffer(buff, pool->header_size, block_size);
    unlock(pool);

    return ptr;
}

static int delete_block_from_free_list(VariadicMemPool *pool, SizedBlock *block)
{
    SizedBlock **curr = &pool->block_head;

    while (*curr) {
        if ((*curr) == block) {
            *curr = block->next_in_free_list;
            return 1;
        }
        curr = &(*curr)->next_in_free_list;
    }

    return 0;
}

bool pool_variadic_is_associated(VariadicMemPool *pool, void *ptr)
{
    return buffer_list_has(pool->buff_head, ptr);
}

static SizedBlock *append(SizedBlock *to, SizedBlock *from, size_t header_size)
{
    to->header.size += from->header.size + header_size;

    return to;
}

static SizedBlock *merge_next_free_blocks(VariadicMemPool *pool, Buffer *buff, SizedBlock *block)
{
    SizedBlock *next = NULL;

    while (1) {
        next = (SizedBlock *)((char *)block + block->header.size + pool->header_size);

        if (buffer_has(buff, next) && delete_block_from_free_list(pool, next)) {
            block = append(block, next, pool->header_size);
        } else {
            break;
        }
    }

    return block;
}

static SizedBlock *merge_previous_free_blocks(VariadicMemPool *pool, SizedBlock *block)
{
    SizedBlock *prev = block->header.prev_in_buff;

    while (prev) {
        if (!delete_block_from_free_list(pool, prev)) {
            break;
        }
        block = append(prev, block, pool->header_size);
        prev = prev->header.prev_in_buff;
    }

    return block;
}

static SizedBlock *defragment(VariadicMemPool *pool, Buffer *buff, SizedBlock *block)
{
    block = merge_next_free_blocks(pool, buff, block);
    block = merge_previous_free_blocks(pool, block);

    return block;
}

int pool_variadic_free(VariadicMemPool *pool, void *ptr)
{
    lock(pool);

    Buffer *buff = buffer_list_find(pool->buff_head, ptr);
    SizedBlock *new = (SizedBlock *)((char *)ptr - pool->header_size);

    if (!buff) {
        unlock(pool);
        return -1;
    } else {
        new = defragment(pool, buff, new);
    }
    SizedBlock *tmp = pool->block_head;
    pool->block_head = new;
    new->next_in_free_list = tmp;

    unlock(pool);

    return 0;
}

void pool_variadic_destroy(VariadicMemPool *pool)
{
    pool_destroy(pool);
}
