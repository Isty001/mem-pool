#include <stdio.h>
#include <stdalign.h>
#include <stddef.h>
#include "../deps/minunit/minunit.h"
#include "../src/mem_pool.h"


MU_TEST(test_pool)
{
    int expected = 10;
    int align = alignof(max_align_t);

    expected = expected + (align - expected % align);

    MemPool *pool = pool_init(10, 2);

    void *ptr1, *ptr2, *ptr3, *ptr4, *prev;

    ptr1 = pool_alloc(pool);
    ptr2 = pool_alloc(pool);
    ptr3 = pool_alloc(pool);

    mu_assert_int_eq(expected, ptr2 - ptr1);
    mu_assert(ptr3 - ptr2 > expected, "This should be in a new Buffer");

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

static int ADDED = 0;

static void add_items(int *item)
{
    ADDED += *item;
}

MU_TEST(test_foreach)
{
    MemPool *pool = pool_init(sizeof(int), 2);
    int *a, *b, *c;

    a = pool_alloc(pool);
    *a = 100;
    b = pool_alloc(pool);
    *b = 200;
    c = pool_alloc(pool);
    *c = 50;

    pool_foreach(pool, (PoolForeach) add_items);

    mu_assert_int_eq(350, ADDED);

    pool_destroy(pool);
}

int main(void)
{
    MU_RUN_TEST(test_pool);
    MU_RUN_TEST(test_foreach);

    MU_REPORT();

    return 0;
}
