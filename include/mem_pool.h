#ifndef MEM_POOL_H
#define MEM_POOL_H


#include <stdlib.h>
#include <stdbool.h>


typedef struct FixedMemPool FixedMemPool;

typedef struct VariadicMemPool VariadicMemPool;

typedef int (*PoolForeach)(void *block);

/**
 * returns a new MemPool, with the given block size. If it runs out of space,
 * it'll create a new internal Buffer with increase_count * block_size size
 */
FixedMemPool *pool_fixed_init(size_t block_size, size_t increase_count);

void *pool_fixed_alloc(FixedMemPool *pool);

bool pool_fixed_is_associated(FixedMemPool *pool, void *ptr);

/**
 * Iterates through all the blocks allocated with the given pool
 */
void pool_fixed_foreach(FixedMemPool *pool, PoolForeach callback);

/**
 * The memory block is not actually freed, just given back to the pool to reuse it
 *
 * returns -1 if the pointer is not known by the pool, 0 otherwise
 */
int pool_fixed_free(FixedMemPool *pool, void *ptr);

void pool_fixed_destroy(FixedMemPool *pool);

/**
 * grow_size deremines the size of a new buffer required from malloc when no more free (fitting) space left
 * tolerance_percent is the maximum difference in percentage when looking for best fitting free blocks
 */
VariadicMemPool *pool_variadic_init(size_t grow_size, size_t tolerance_percent);

void *pool_variadic_alloc(VariadicMemPool *pool, size_t size);

bool pool_variadic_is_associated(VariadicMemPool *pool, void *ptr);

/*
 * Before appending to the free list, this function will attempt to merge neighbouring memory blocks (including the space used by their headers) in the given buffer.
 * Will return -1 if the pointer is not known by the pool.
 */
int pool_variadic_free(VariadicMemPool *pool, void *ptr);

void pool_variadic_destroy(VariadicMemPool *pool);


size_t mem_align(size_t size);


#endif
