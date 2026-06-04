#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "../src/ternary_spreadsheet.h"

static int tests_passed = 0;
static int tests_run    = 0;

#define TEST(name) printf("  %-45s", #name); tests_run++;
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL — %s\n", msg); } while(0)

/* ================================================================== */
/* Test 1: grid create / destroy                                       */
/* ================================================================== */
static void test_create_destroy(void)
{
    TEST(create_destroy);
    Grid g = grid_create(3, 4);
    assert(g.rows == 3);
    assert(g.cols == 4);
    assert(g.cells != NULL);
    grid_destroy(&g);
    assert(g.cells == NULL);
    PASS();
}

/* ================================================================== */
/* Test 2: set and get values                                          */
/* ================================================================== */
static void test_set_get(void)
{
    TEST(set_get_values);
    Grid g = grid_create(2, 2);
    grid_set(&g, 0, 0, TERNARY_NEG);
    grid_set(&g, 0, 1, TERNARY_ZERO);
    grid_set(&g, 1, 0, TERNARY_POS);
    assert(grid_get(&g, 0, 0) == TERNARY_NEG);
    assert(grid_get(&g, 0, 1) == TERNARY_ZERO);
    assert(grid_get(&g, 1, 0) == TERNARY_POS);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 3: values clamp to ternary range                               */
/* ================================================================== */
static void test_clamp(void)
{
    TEST(clamp_ternary);
    assert(clamp_ternary(5)   == TERNARY_POS);
    assert(clamp_ternary(-99) == TERNARY_NEG);
    assert(clamp_ternary(0)   == TERNARY_ZERO);
    assert(clamp_ternary(1)   == TERNARY_POS);
    assert(clamp_ternary(-1)  == TERNARY_NEG);
    PASS();
}

/* ================================================================== */
/* Test 4: set clamps out-of-range values                              */
/* ================================================================== */
static void test_set_clamps(void)
{
    TEST(set_clamps_large_values);
    Grid g = grid_create(1, 1);
    grid_set(&g, 0, 0, 42);
    assert(grid_get(&g, 0, 0) == TERNARY_POS);
    grid_set(&g, 0, 0, -7);
    assert(grid_get(&g, 0, 0) == TERNARY_NEG);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 5: SUM formula                                                 */
/* ================================================================== */
static void test_formula_sum(void)
{
    TEST(formula_SUM);
    Grid g = grid_create(3, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_POS);
    /* cell[2,0] = SUM(A0:A1) */
    Formula f = { FORMULA_SUM, 0, 0, 1, 0, 0 };
    grid_set_formula(&g, 2, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_POS); /* 1+1=2 → clamped to 1 */
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 6: SUM formula with zero result                                */
/* ================================================================== */
static void test_formula_sum_zero(void)
{
    TEST(formula_SUM_zero);
    Grid g = grid_create(3, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_NEG);
    Formula f = { FORMULA_SUM, 0, 0, 1, 0, 0 };
    grid_set_formula(&g, 2, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_ZERO);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 7: PRODUCT formula                                             */
/* ================================================================== */
static void test_formula_product(void)
{
    TEST(formula_PRODUCT);
    Grid g = grid_create(3, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_NEG);
    Formula f = { FORMULA_PRODUCT, 0, 0, 1, 0, 0 };
    grid_set_formula(&g, 2, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_NEG); /* 1 * -1 = -1 */
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 8: PRODUCT with zero                                           */
/* ================================================================== */
static void test_formula_product_zero(void)
{
    TEST(formula_PRODUCT_zero);
    Grid g = grid_create(3, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_ZERO);
    Formula f = { FORMULA_PRODUCT, 0, 0, 1, 0, 0 };
    grid_set_formula(&g, 2, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_ZERO);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 9: THRESHOLD formula (positive)                                */
/* ================================================================== */
static void test_formula_threshold_pos(void)
{
    TEST(formula_THRESHOLD_positive);
    Grid g = grid_create(4, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_POS);
    grid_set(&g, 2, 0, TERNARY_NEG);
    Formula f = { FORMULA_THRESHOLD, 0, 0, 2, 0, 2 }; /* threshold=2 */
    grid_set_formula(&g, 3, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 3, 0) == TERNARY_POS);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 10: THRESHOLD formula (negative)                               */
/* ================================================================== */
static void test_formula_threshold_neg(void)
{
    TEST(formula_THRESHOLD_negative);
    Grid g = grid_create(4, 1);
    grid_set(&g, 0, 0, TERNARY_NEG);
    grid_set(&g, 1, 0, TERNARY_NEG);
    grid_set(&g, 2, 0, TERNARY_POS);
    Formula f = { FORMULA_THRESHOLD, 0, 0, 2, 0, 2 };
    grid_set_formula(&g, 3, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 3, 0) == TERNARY_NEG);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 11: THRESHOLD formula (neither → zero)                         */
/* ================================================================== */
static void test_formula_threshold_zero(void)
{
    TEST(formula_THRESHOLD_neither);
    Grid g = grid_create(4, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_NEG);
    grid_set(&g, 2, 0, TERNARY_ZERO);
    Formula f = { FORMULA_THRESHOLD, 0, 0, 2, 0, 2 };
    grid_set_formula(&g, 3, 0, f);
    grid_evaluate(&g);
    assert(grid_get(&g, 3, 0) == TERNARY_ZERO);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 12: Chained formulas (topological order)                       */
/* ================================================================== */
static void test_chained_formulas(void)
{
    TEST(chained_formulas_topological);
    Grid g = grid_create(4, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_POS);
    /* cell[2] = SUM(0..1) = +1 */
    grid_set_formula(&g, 2, 0, (Formula){ FORMULA_SUM, 0, 0, 1, 0, 0 });
    /* cell[3] = SUM(2..2) = +1  (depends on cell[2]) */
    grid_set_formula(&g, 3, 0, (Formula){ FORMULA_SUM, 2, 0, 2, 0, 0 });
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_POS);
    assert(grid_get(&g, 3, 0) == TERNARY_POS);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 13: SortEngine sorts by fitness descending                     */
/* ================================================================== */
static void test_sort_engine(void)
{
    TEST(sort_engine_fitness_order);
    Grid g = grid_create(1, 4);
    grid_set(&g, 0, 0, TERNARY_NEG);
    grid_set(&g, 0, 1, TERNARY_POS);
    grid_set(&g, 0, 2, TERNARY_ZERO);
    grid_set(&g, 0, 3, TERNARY_NEG);

    sort_engine_sort(&g, 0, 0, 0, 3);

    /* Expected order: +1, 0, -1, -1 */
    assert(grid_get(&g, 0, 0) == TERNARY_POS);
    assert(grid_get(&g, 0, 1) == TERNARY_ZERO);
    assert(grid_get(&g, 0, 2) == TERNARY_NEG);
    assert(grid_get(&g, 0, 3) == TERNARY_NEG);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 14: SortEngine on 2D range                                     */
/* ================================================================== */
static void test_sort_2d(void)
{
    TEST(sort_engine_2D_range);
    Grid g = grid_create(2, 2);
    grid_set(&g, 0, 0, TERNARY_NEG);
    grid_set(&g, 0, 1, TERNARY_ZERO);
    grid_set(&g, 1, 0, TERNARY_POS);
    grid_set(&g, 1, 1, TERNARY_NEG);

    sort_engine_sort(&g, 0, 0, 1, 1);

    /* +1, 0, -1, -1 */
    assert(grid_get(&g, 0, 0) == TERNARY_POS);
    assert(grid_get(&g, 0, 1) == TERNARY_ZERO);
    assert(grid_get(&g, 1, 0) == TERNARY_NEG);
    assert(grid_get(&g, 1, 1) == TERNARY_NEG);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 15: AutofillEngine with 0% mutation (exact copy)               */
/* ================================================================== */
static void test_autofill_no_mutation(void)
{
    TEST(autofill_zero_mutation);
    Grid g = grid_create(2, 4);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 0, 1, TERNARY_NEG);
    grid_set(&g, 0, 2, TERNARY_ZERO);
    grid_set(&g, 0, 3, TERNARY_POS);

    autofill_engine_fill(&g, 0, 0, 0, 3, 1, 0, 1, 3, 0);

    assert(grid_get(&g, 1, 0) == TERNARY_POS);
    assert(grid_get(&g, 1, 1) == TERNARY_NEG);
    assert(grid_get(&g, 1, 2) == TERNARY_ZERO);
    assert(grid_get(&g, 1, 3) == TERNARY_POS);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 16: AutofillEngine with 100% mutation (all cells change)       */
/* ================================================================== */
static void test_autofill_full_mutation(void)
{
    TEST(autofill_full_mutation);
    Grid g = grid_create(2, 4);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 0, 1, TERNARY_POS);
    grid_set(&g, 0, 2, TERNARY_POS);
    grid_set(&g, 0, 3, TERNARY_POS);

    autofill_engine_fill(&g, 0, 0, 0, 3, 1, 0, 1, 3, 100);

    /* All should be mutated; POS can only go to ZERO (dir=-1) or stay POS */
    for (int c = 0; c < 4; c++) {
        ternary_t v = grid_get(&g, 1, c);
        assert(v == TERNARY_POS || v == TERNARY_ZERO || v == TERNARY_NEG);
    }
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 17: Larger grid evaluate                                       */
/* ================================================================== */
static void test_large_grid_evaluate(void)
{
    TEST(large_grid_evaluate);
    Grid g = grid_create(5, 5);
    /* Fill with +1 */
    for (int r = 0; r < 5; r++)
        for (int c = 0; c < 5; c++)
            grid_set(&g, r, c, TERNARY_POS);

    /* Bottom-right = PRODUCT of entire grid = 1^25 = 1 */
    grid_set_formula(&g, 4, 4, (Formula){ FORMULA_PRODUCT, 0, 0, 3, 3, 0 });
    grid_evaluate(&g);
    assert(grid_get(&g, 4, 4) == TERNARY_POS);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 18: Formula with all zeros → SUM = 0                          */
/* ================================================================== */
static void test_sum_all_zeros(void)
{
    TEST(formula_SUM_all_zeros);
    Grid g = grid_create(3, 3);
    for (int r = 0; r < 3; r++)
        for (int c = 0; c < 3; c++)
            grid_set(&g, r, c, TERNARY_ZERO);

    grid_set_formula(&g, 2, 2, (Formula){ FORMULA_SUM, 0, 0, 2, 1, 0 });
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 2) == TERNARY_ZERO);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 19: Re-evaluation updates correctly                            */
/* ================================================================== */
static void test_reevaluate(void)
{
    TEST(reevaluate_after_change);
    Grid g = grid_create(3, 1);
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 1, 0, TERNARY_POS);
    grid_set_formula(&g, 2, 0, (Formula){ FORMULA_SUM, 0, 0, 1, 0, 0 });

    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_POS);

    /* Change a dependency */
    grid_set(&g, 0, 0, TERNARY_NEG);
    grid_evaluate(&g);
    assert(grid_get(&g, 2, 0) == TERNARY_ZERO); /* -1 + 1 = 0 */

    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Test 20: Grid cell pointer access                                   */
/* ================================================================== */
static void test_cell_pointer(void)
{
    TEST(cell_pointer_access);
    Grid g = grid_create(2, 3);
    Cell *c = grid_cell(&g, 1, 2);
    assert(c != NULL);
    c->value = TERNARY_NEG;
    assert(grid_get(&g, 1, 2) == TERNARY_NEG);
    grid_destroy(&g);
    PASS();
}

/* ================================================================== */
/* Main                                                                */
/* ================================================================== */
int main(void)
{
    printf("\n=== Ternary Spreadsheet Tests ===\n\n");

    test_create_destroy();
    test_set_get();
    test_clamp();
    test_set_clamps();
    test_formula_sum();
    test_formula_sum_zero();
    test_formula_product();
    test_formula_product_zero();
    test_formula_threshold_pos();
    test_formula_threshold_neg();
    test_formula_threshold_zero();
    test_chained_formulas();
    test_sort_engine();
    test_sort_2d();
    test_autofill_no_mutation();
    test_autofill_full_mutation();
    test_large_grid_evaluate();
    test_sum_all_zeros();
    test_reevaluate();
    test_cell_pointer();

    printf("\n  %d / %d tests passed.\n\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
