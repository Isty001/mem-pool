#include <mem_pool/mem_pool.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include "../deps/minunit/minunit.h"


struct test {
    char str[7];
    uint16_t i;
};


#define item_size(t) mem_align(sizeof(size_t)) + mem_align(sizeof(t))


MU_TEST(test_pool)
{
    VariadicMemPool *pool = pool_variadic_init(item_size(int) + item_size(struct test));

    int *num = pool_variadic_alloc(pool, sizeof(int));
    *num = 100;


   // printf("%lu  -  %lu", sizeof(struct test), mem_align(sizeof(struct test)));exit(0);

    struct test *structure = pool_variadic_alloc(pool, sizeof(struct test));
    structure->i = 4;
    memcpy(structure->str, "Hel", 3);

    return;
 //   mu_assert_int_eq(item_size(int), (intptr_t)structure - (intptr_t)num);

    struct test *array = pool_variadic_alloc(pool, 2 * sizeof(struct test));
    mu_assert(NULL != array, "");
}

void run_dynamic_pool_test(void)
{
    MU_RUN_TEST(test_pool);

    MU_REPORT();
}
