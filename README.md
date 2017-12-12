# Memory Pool

Dynamic memory pool implementation, for reusable fixed, or variable sized memory blocks, using `pthread mutex` locks.

### Installation

If you don't want to include the source in your project, you can install it as a dynamic library via `make install` and link against it with `-lmem_pool`

For a quick check, run `make test` or `make test-valgrind` 

### How it works

There are two main use cases when allocating:
* [Fixed sized objects](#fixed-pool)
    * This is useful when you want to store a large number of the same size of objects, this type of pool does a bit less work to allocate new blocks
* [Variable sized objects](#variable-pool)
    * This is basically a generic allocator, that is capable of allocating a **best fitting** block by storing some meta data:
```
    |                           Pool                            |
    |       Buffer                |       Buffer                |
    | H | B     | H | B | H | B   | H| B |      H| B            |
```
*H: Header 
B: Actual Block*

When allocating a given size of block, a header is also stored which contains the size of the block, and pointer to the previous header in the buffer. This is required to merge neighbouring blocks when the pointer is given back to the pool, so it can be defragmented. The stored size also helps us to find the best fitting block from the free list.

All the pointers given by the pools are pointing to *aligned* blocks.

## Ussage & API

To use the library you only need to `#include <mem_pool/mem_pool.h>`. Every function returns one of the `MemPoolError` enum values, thus making the error checking pretty simple.

```C
MemPoolErr err;

if (MEM_POOL_ERR_OK != (err = pool_*())) {
   //handle err
}
```

### <a name="fixed-pool">FixedMemPool</a>

Initialization:

```c
size_t block_size = sizeof(struct test);
size_t increase_count = 500;
FixedMemPool *pool

pool_fixed_init(&pool, block_size, increase_count);
```

The pool'll be able to provide `block_size` blocks, and when it runs out of memory, will *actually* allocate a new internal buffer `increase_count` * `block_size size`


Get a block:

```c
void *ptr;

pool_fixed_alloc(pool, (void **)&ptr);
```

It can make sense to iterate through a pool of objects when you know that all of them are of the same type:

```c
static MemPoolForeachStatus callback(void *item)
{
    if (should_stop(item)) {
         return MEM_POOL_FOREACH_STOP;
    }

    return MEM_POOL_FOREACH_CONTINUE;
}

pool_fixed_foreach(pool, callback);
```

To check if a pointer is from the pool:

```c
if (MEM_POOL_ERR_OK == pool_fixed_is_associated(pool, ptr)) { /* */ }
```
Won't check if the block is not used anymore.


When not needed anymore, give the pointer back to the pool's free list, and make it reusable. 

```c
pool_fixed_free(pool, ptr);
```

To actually `free` all the memory allocated:

```c
pool_fixed_destroy(pool);
```

### <a name="variable-pool">VariableMemPool</a>

This type of pool has a very similar API.

Initialization:

```c
size_t grow_size = 500; 
size_t tolerance_percent = 20;
VariableMemPool *pool;

pool_variable_init(&pool, grow_size, tolerance_percent);
```
`grow_size` deremines the size of a new buffer required from malloc when no more free (fitting) space left. When trying to figure out a realistic size for your use case, take into account the it also includes the (aligned) size of the headers too.

`tolerance_percent` is the maximum difference in percentage when looking for best fitting blocks from the free list. You can alternatively pas `MEM_POOL_NO_BEST_FIT` to skip this check.


Get a block:

```c
void *ptr;

pool_variable_alloc(pool, sizeof(some_type), (void **)&ptr);
```

To check if a pointer is from the pool:

```c
if (MEM_POOL_ERR_OK == pool_variable_is_associated(pool, ptr)) { /* */ }
```

In order to make the piece of memory reusable:

```c
pool_fixed_free(pool, ptr);
```
Before appending to the free list, this function will attempt to merge neighbouring free memory blocks (including the space used by their headers) in the given buffer.

To actually free all the memory allocated:

```c
pool_variable_destroy(pool);
```
