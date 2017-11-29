### Memory Pool


Dynamic memory pool implementation, for reusable memory blocks, using `pthread mutex` locks

#### Installation with [clib](https://github.com/clibs/clib)

```
$ clib install isty001/mem-pool
```


#### Usage:

Initialize a new MemPool, with the given `block_size` and `increase_count`. 
If it runs out of space, it'll create a new internal Buffer with `increase_count * block_size` size.

```c
#include "mem_pool.h"

... 

MemPool *pool = pool_new(10, 10);
```

Get a block:

```c
void *ptr = pool_alloc(pool);
```

You can iterate through all the blocks allocated with the given pool like this, if
the callback returns a non-zero value, the loop will be stopped:

```c
static void callback(void *item)
{
    //
    return 0;
}

pool_foreach(pool, callback);
```

To check if the block is allocated with the pool:

```c
if (pool_has_ptr(pool, ptr)) { /* */ }
```

When not needed anymore, give the pointer back to the pool, and make it reusable. This will return -1
if the pointer is not known by the pool.

```c
pool_free(pool, ptr);
```

To actually free all the memory allocated:

```c
pool_destroy(pool);
```

For a quick check, run `make test` or `make test-valgrind` 
