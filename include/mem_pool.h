#ifndef MEM_POOL_H
#define MEM_POOL_H


#include <stdlib.h>
#include <stdbool.h>


typedef struct FixedMemPool FixedMemPool;

typedef struct VariadicMemPool VariadicMemPool;

typedef int (*PoolForeach)(void *block);


/**
 * @return a new MemPool, with the given block size. If it runs out of space,
 * it'll create a new internal Buffer with increase_count * block_size size
 */
FixedMemPool *pool_fixed_init(size_t block_size, size_t increase_count);

void *pool_fixed_alloc(FixedMemPool *pool);

bool pool_fixed_is_allocated(FixedMemPool *pool, void *ptr);

/**
 * Iterates through all the blocks allocated with the given pool
 */
void pool_fixed_foreach(FixedMemPool *pool, PoolForeach callback);

/**
 * The memory block is not actually freed, just given back to the pool to reuse it
 *
 * @return -1 if the pointer is not known by the pool, 0 otherwise
 */
int pool_fixed_free(FixedMemPool *pool, void *ptr);

void pool_fixed_destroy(FixedMemPool *pool);

/**
 *
 */
VariadicMemPool *pool_variadic_init(size_t grow_size);

void *pool_variadic_alloc(VariadicMemPool *pool, size_t size);

bool pool_variadic_has(VariadicMemPool *pool, void *ptr);

int pool_variadic_free(VariadicMemPool *pool, void *ptr);


size_t mem_align(size_t size);


#endif
