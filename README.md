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

MemPool *pool = pool_init(10, 10);
```

Get a block:

```c
void *ptr = pool_alloc(pool);
```

When not needed anymore, give it back to the pool, and make it reusable:

```c
pool_free(pool, ptr1);
```

If everything is finished, then actually free all the memory allocated:

```c
pool_destroy(pool);
```

For a quick check, run `make test` or `make test-valgrind` 
