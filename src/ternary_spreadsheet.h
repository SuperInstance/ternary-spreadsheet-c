#ifndef TERNARY_SPREADSHEET_H
#define TERNARY_SPREADSHEET_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Ternary value: -1, 0, or +1 */
typedef int ternary_t;

#define TERNARY_NEG (-1)
#define TERNARY_ZERO 0
#define TERNARY_POS 1

/* Formula types */
typedef enum {
    FORMULA_NONE = 0,
    FORMULA_SUM,
    FORMULA_PRODUCT,
    FORMULA_THRESHOLD
} FormulaType;

/* A formula referencing a rectangular range */
typedef struct {
    FormulaType type;
    int r1, c1;  /* top-left of range */
    int r2, c2;  /* bottom-right of range */
    int threshold; /* only used for FORMULA_THRESHOLD */
} Formula;

/* A single cell */
typedef struct {
    ternary_t value;       /* direct value (-1, 0, +1) */
    Formula   formula;     /* optional formula */
    bool      has_formula;
    bool      computed;    /* already evaluated this cycle */
} Cell;

/* N x M grid */
typedef struct {
    Cell   *cells;         /* row-major [rows][cols] */
    int     rows;
    int     cols;
} Grid;

/* --- Grid lifecycle --- */
Grid  grid_create(int rows, int cols);
void  grid_destroy(Grid *g);

/* --- Cell access --- */
Cell *grid_cell(Grid *g, int r, int c);
ternary_t grid_get(Grid *g, int r, int c);
void  grid_set(Grid *g, int r, int c, ternary_t val);

/* --- Formula helpers --- */
void  grid_set_formula(Grid *g, int r, int c, Formula f);
ternary_t clamp_ternary(int v);

/* --- Evaluator --- */
/* Evaluate all formula cells in topological (dependency) order. */
void  grid_evaluate(Grid *g);

/* --- SortEngine (natural-selection / fitness-based sort) --- */
/* Sort a range by fitness: count of positive values desc, then negative asc. */
void  sort_engine_sort(Grid *g, int r1, int c1, int r2, int c2);

/* --- AutofillEngine (mutation-based fill) --- */
/* Fill destination range with mutated copies of the source range.
   mutation_rate 0..100 : percent chance each cell flips one step. */
void  autofill_engine_fill(Grid *g,
                           int sr1, int sc1, int sr2, int sc2,
                           int dr1, int dc1, int dr2, int dc2,
                           int mutation_rate);

#ifdef __cplusplus
}
#endif
#endif /* TERNARY_SPREADSHEET_H */
