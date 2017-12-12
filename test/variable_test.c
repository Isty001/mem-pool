#include <mem_pool/mem_pool.h>
#include <memory.h>
#include <stddef.h>
#include <stdint.h>
#include "minunit.h"
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
    assert_size_in_header(sizeof(int), header_size, num);
    mu_assert_int_eq(100, *num);

    assert_size_in_header(sizeof(struct test), header_size, structure)
    mu_assert_int_eq(10, structure->i);
    mu_assert_int_eq(0, strcmp(structure->str, "Hello"));

    mu_assert_int_eq(20, array[0].i);
    mu_assert_int_eq(0, strcmp(array[0].str, "Test"));

    mu_assert_int_eq(30, array[1].i);
    mu_assert_int_eq(0, strcmp(array[1].str, "C"));
}

static void test_defragmentation_in_first_buff(VariableMemPool *pool, int *num, struct test *structure)
{
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_free(pool, num));

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_free(pool, structure));

    // The two freed *aligned* blocks, including a Header should be enough for this
    struct test *arr;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, 5 * sizeof(struct test), (void **)&arr));

    mu_assert((void *)num == (void *)arr, "");
}

static void test_defragmentation_in_second_buff(VariableMemPool *pool, struct test *array)
{
    int *num_1, *num_2;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_free(pool, array));

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(int), (void **)&num_1));
    mu_assert((void *)num_1 == (void *)array, "");

    *num_1 = 100;

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(int), (void **)&num_2));
    *num_2 = 200;

    mu_assert_int_eq(100, *num_1);
    mu_assert_int_eq(200, *num_2);
}

/**
 * Initial Layout:
 *
 * Buffer 1: |h| int num |h| struct test structure
 * Buffer 2: |h| struct test array[2]
 */
MU_TEST(test_alloc)
{
    const size_t header_size = mem_align(sizeof(Header));

    VariableMemPool *pool; 
    mu_assert_int_eq(
            MEM_POOL_ERR_OK,
            pool_variable_init(&pool, 2 * item_size(Header) + item_size(int) + item_size(struct test), 10)
    );

    int *num;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(int), (void **)&num));
    mu_assert(pool_variable_is_associated(pool, num), "Should be known by the pool");
    *num = 100;

    struct test *structure;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(struct test), (void **)&structure));
    mu_assert(pool_variable_is_associated(pool, structure), "Should be known by the pool");

    structure->i = 10;
    memcpy(structure->str, "Hello", 5);
    structure->str[5] = '\0';

    struct test *array;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, 2 * sizeof(struct test), (void **)&array));
    mu_assert(pool_variable_is_associated(pool, array), "Should be known by the pool");

    array[0].i = 20;
    array[1].i = 30;

    memcpy(array[0].str, "Test", 4);
    array[0].str[5] = '\0';

    memcpy(array[1].str, "C", 1);
    array[1].str[2] = '\0';

    assert_initials(header_size, num, structure, array);

    test_defragmentation_in_first_buff(pool, num, structure);
    test_defragmentation_in_second_buff(pool, array);

    pool_variable_destroy(pool);
}

MU_TEST(test_complex_defragmentation)
{
    size_t header_size = mem_align(sizeof(Header));

    char *a, *b, *c;
    VariableMemPool *pool; 
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_init(&pool, 200, MEM_POOL_NO_BEST_FIT));

    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(char), (void **)&a));
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(char), (void **)&b));
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, sizeof(char), (void **)&c));

    *a = 'a';
    *b = 'b';
    *c = 'c';

    mu_assert_int_eq('a', *a);
    assert_size_in_header(sizeof(char), header_size, a);

    mu_assert_int_eq('b', *b);
    assert_size_in_header(sizeof(char), header_size, b);

    mu_assert_int_eq('c', *c);
    assert_size_in_header(sizeof(char), header_size, c);

    // These three should be merged into one block
    mu_assert_int_eq(0, pool_variable_free(pool, c));
    mu_assert_int_eq(0, pool_variable_free(pool, a));
    mu_assert_int_eq(0, pool_variable_free(pool, b));

    char *def;
    mu_assert_int_eq(MEM_POOL_ERR_OK, pool_variable_alloc(pool, 3 * sizeof(char), (void **)&def));

    /**
     * We set MEM_NO_BEST_FIT so we will retrieve the first free block which is was formed from
     * the previous, *separately* allocated three aligned blocks and their aligned headers
     * but it'll also keep space for its own new header
     */
    mu_assert_int_eq(
            2 * mem_align(sizeof(Header)) + 3 * mem_align(sizeof(char)),
            header_of(def, header_size)->size
    );
    mu_assert(def == a, "");

    pool_variable_destroy(pool);
}

void run_dynamic_pool_test(void)
{
    MU_RUN_TEST(test_alloc);
    MU_RUN_TEST(test_complex_defragmentation);

    MU_REPORT();
}
