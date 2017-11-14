#include <stdio.h>
#include <stdalign.h>
#include <stddef.h>
#include <mem_pool/mem_pool.h>
#include "../deps/minunit/minunit.h"


MU_TEST(test_pool)
{
    int expected = 10;
    int align = alignof(max_align_t);

    expected = expected + (align - expected % align);

    FixedMemPool *pool = pool_fixed_init(10, 2);

    void *ptr1, *ptr2, *ptr3, *ptr4, *prev;

    ptr1 = pool_fixed_alloc(pool);
    ptr2 = pool_fixed_alloc(pool);
    ptr3 = pool_fixed_alloc(pool);

    mu_assert_int_eq(expected, ptr2 - ptr1);
    mu_assert(ptr3 - ptr2 > expected, "This should be in a new Buffer");

    mu_assert(MEM_BLOCK_ALLOCATED == pool_fixed_block_info(pool, ptr1).state, "This should be allocated");
    mu_assert(MEM_BLOCK_ALLOCATED == pool_fixed_block_info(pool, ptr3).state, "This should be allocated");

    prev = ptr1;
    pool_fixed_free(pool, ptr1);
    mu_assert(MEM_BLOCK_FREE == pool_fixed_block_info(pool, ptr1).state, "This should be in the free list");

    ptr4 = pool_fixed_alloc(pool);
    mu_assert(prev == ptr4, "The same pointer should be returned from the free list");

    void *unknown = NULL;
    mu_assert(MEM_BLOCK_UNKOWN == pool_fixed_block_info(pool, unknown).state, "This shouldn't be known by the Pool");

    pool_fixed_destroy(pool);
}

static int ADDED = 0;

static int add_items(int *item)
{
    if (50 == *item) {
        return -1;
    }
    ADDED += *item;

    return 0;
}

MU_TEST(test_foreach)
{
    FixedMemPool *pool = pool_fixed_init(sizeof(int), 2);
    int *a, *b, *c;

    a = pool_fixed_alloc(pool);
    *a = 100;
    b = pool_fixed_alloc(pool);
    *b = 200;
    c = pool_fixed_alloc(pool);
    *c = 50;

    pool_fixed_foreach(pool, (PoolForeach) add_items);

    mu_assert_int_eq(300, ADDED);

    pool_fixed_destroy(pool);
}

void run_fixed_pool_test(void)
{
    MU_RUN_TEST(test_pool);
    MU_RUN_TEST(test_foreach);

    MU_REPORT();
}
