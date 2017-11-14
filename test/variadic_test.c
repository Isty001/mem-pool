#include <mem_pool/mem_pool.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include "../deps/minunit/minunit.h"
#include "../src/internals.h"


struct test {
    char str[6];
    uint16_t i;
};


#define item_size(t) mem_align(sizeof(size_t)) + mem_align(sizeof(t))
#define header_of(ptr, header_size) ((Header *)((char *)ptr - header_size))
#define assert_size_in_header(block_size, header_size, ptr)  mu_assert_int_eq(mem_align(block_size), header_of(ptr, header_size)->size);


static void assert_initials(const size_t header_size, int *num, struct test *structure, struct test *array)
{
    assert_size_in_header(sizeof(int), header_size, num)
    mu_assert_int_eq(100, *num);

    assert_size_in_header(sizeof(struct test), header_size, structure)
    mu_assert_int_eq(10, structure->i);
    mu_assert_int_eq(0, strcmp(structure->str, "Hello"));

    assert_size_in_header(2 * sizeof(struct test), header_size, array)
    mu_assert_int_eq(20, array[0].i);
    mu_assert_int_eq(0, strcmp(array[0].str, "Test"));

    mu_assert_int_eq(30, array[1].i);
    mu_assert_int_eq(0, strcmp(array[1].str, "C"));
}

/**
 * Initial Layout:
 *
 * Buffer 1: |h| int |h| struct test
 * Buffer 2: |h| struct test | struct test
 */
MU_TEST(test_pool)
{
    const size_t header_size = mem_align(sizeof(Header));

    VariadicMemPool *pool = pool_variadic_init(2 * item_size(Header) + item_size(int) + item_size(struct test), 10);

    int *num = pool_variadic_alloc(pool, sizeof(int));
    mu_assert_int_eq(MEM_BLOCK_ALLOCATED, pool_variadic_block_info(pool, num).state);
    *num = 100;

    struct test *structure = pool_variadic_alloc(pool, sizeof(struct test));

    structure->i = 10;
    memcpy(structure->str, "Hello", 5);
    structure->str[5] = '\0';

    struct test *array = pool_variadic_alloc(pool, 2 * sizeof(struct test));

    array[0].i = 20;
    array[1].i = 30;

    memcpy(array[0].str, "Test", 4);
    array[0].str[5] = '\0';

    memcpy(array[1].str, "C", 1);
    array[1].str[2] = '\0';

    assert_initials(header_size, num, structure, array);

}

void run_dynamic_pool_test(void)
{
    MU_RUN_TEST(test_pool);

    MU_REPORT();
}
