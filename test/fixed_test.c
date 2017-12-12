#include <stdio.h>
#include <stdalign.h>
#include <stddef.h>
#include <mem_pool/mem_pool.h>
#include "minunit.h"


MU_TEST(test_pool)
{
    int expected = 10;
    int align = alignof(max_align_t);

    expected = expected + (align - expected % align);

    FixedMemPool *pool;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_init(&pool, 10, 2));

    void *ptr1, *ptr2, *ptr3, *ptr4, *prev;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, &ptr1));
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, &ptr2));
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, &ptr3));

    mu_assert_int_eq(expected, ptr2 - ptr1);
    mu_assert(ptr3 - ptr2 > expected, "This should be in a new Buffer");

    mu_assert(pool_fixed_is_associated(pool, ptr1), "This should be allocated");
    mu_assert(pool_fixed_is_associated(pool, ptr2), "This should be allocated");

    prev = ptr1;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_free(pool, ptr1));

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, &ptr4));
    mu_assert(prev == ptr4, "The same pointer should be returned from the free list");

    void *unknown = NULL;
    mu_assert(false == pool_fixed_is_associated(pool, unknown), "This shouldn't be known by the Pool");
    mu_assert_int_eq(MEM_POOL_ERR_UNKNOWN_BLOCK, pool_fixed_free(pool, unknown));

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_destroy(pool));
}

static int ADDED = 0;

static MemPoolForeachStatus add_items(int *item)
{
    if (50 == *item) {
        return MEM_POOL_FOREACH_STOP;
    }
    ADDED += *item;

    return MEM_POOL_FOREACH_CONTINUE;
}

MU_TEST(test_foreach)
{
    FixedMemPool *pool;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_init(&pool, sizeof(int), 2));
    int *a, *b, *c;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, (void **)&a));
    *a = 100;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, (void **)&b));
    *b = 200;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_fixed_alloc(pool, (void **)&c));
    *c = 50;

    pool_fixed_foreach(pool, (FixedPoolForeach) add_items);

    mu_assert_int_eq(300, ADDED);

    pool_fixed_destroy(pool);
}

void run_fixed_pool_test(void)
{
    MU_RUN_TEST(test_pool);
    MU_RUN_TEST(test_foreach);

    MU_REPORT();
}
