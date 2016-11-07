#include <stdio.h>
#include "../deps/minunit/minunit.h"
#include "../src/mem_pool.h"


MU_TEST(test_pool)
{
    MemPool *pool = pool_init(10, 2);

    void *ptr1, *ptr2, *ptr3, *ptr4, *prev;

    ptr1 = pool_alloc(pool);
    ptr2 = pool_alloc(pool);
    ptr3 = pool_alloc(pool);

    mu_assert_int_eq(10, ptr2 - ptr1);
    mu_assert(ptr3 - ptr2 > 10, "This should be in a new Buffer");

    mu_assert(pool_has_ptr(pool, ptr1), "This should be known by the pool");
    mu_assert(pool_has_ptr(pool, ptr3), "This should be known by the pool");

    prev = ptr1;
    pool_free(pool, ptr1);
    ptr4 = pool_alloc(pool);

    mu_assert(prev == ptr4, "The same pointer should be returned from the free list");

    void *unknown = NULL;
    mu_assert(false == pool_has_ptr(pool, unknown), "This shouldn't be known by the Pool");

    pool_destroy(pool);
}

int main(void)
{
    MU_RUN_TEST(test_pool);
    MU_REPORT();

    return 0;
}
