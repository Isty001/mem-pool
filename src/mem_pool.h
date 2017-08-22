#ifndef MEM_POOL_H
#define MEM_POOL_H


#include <stdlib.h>
#include <stdbool.h>


typedef struct mem_pool MemPool;

typedef int (*PoolForeach)(void *block);


/**
 * @return a new MemPool, with the given block size. If it runs out of space,
 * it1ll create a new internal Buffer with increase_count * block_size size
 */
MemPool *pool_init(size_t block_size, size_t increase_count);

/**
 * @return pointer to a new block
 */
void *pool_alloc(MemPool *pool);

/**
 * @return true if the block was allocated by the Pool
 */
bool pool_has_ptr(MemPool *pool, void *ptr);

/**
 * Iterates through all the blocks allocated with the given pool
 */
void pool_foreach(MemPool *pool, PoolForeach callback);

/**
 * Makes the pointer reusable
 */
int pool_free(MemPool *pool, void *ptr);

/**
 * frees all allocated memory
 */
void pool_destroy(MemPool *pool);


#endif
