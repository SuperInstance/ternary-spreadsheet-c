#include "ternary_spreadsheet.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------ */
/* Utility                                                             */
/* ------------------------------------------------------------------ */

ternary_t clamp_ternary(int v)
{
    if (v > 0) return TERNARY_POS;
    if (v < 0) return TERNARY_NEG;
    return TERNARY_ZERO;
}

/* ------------------------------------------------------------------ */
/* Grid lifecycle                                                      */
/* ------------------------------------------------------------------ */

Grid grid_create(int rows, int cols)
{
    Grid g;
    g.rows = rows;
    g.cols = cols;
    g.cells = (Cell *)calloc((size_t)rows * cols, sizeof(Cell));
    return g;
}

void grid_destroy(Grid *g)
{
    free(g->cells);
    g->cells = NULL;
    g->rows = g->cols = 0;
}

/* ------------------------------------------------------------------ */
/* Cell access                                                         */
/* ------------------------------------------------------------------ */

static inline int idx(Grid *g, int r, int c) { return r * g->cols + c; }

Cell *grid_cell(Grid *g, int r, int c)
{
    return &g->cells[idx(g, r, c)];
}

ternary_t grid_get(Grid *g, int r, int c)
{
    return grid_cell(g, r, c)->value;
}

void grid_set(Grid *g, int r, int c, ternary_t val)
{
    Cell *cell = grid_cell(g, r, c);
    cell->value = clamp_ternary(val);
    cell->has_formula = false;
    cell->computed = false;
}

/* ------------------------------------------------------------------ */
/* Formula helpers                                                     */
/* ------------------------------------------------------------------ */

void grid_set_formula(Grid *g, int r, int c, Formula f)
{
    Cell *cell = grid_cell(g, r, c);
    cell->formula = f;
    cell->has_formula = true;
    cell->computed = false;
}

/* Evaluate one formula over the grid (cells must already be resolved). */
static ternary_t eval_formula(Grid *g, const Formula *f)
{
    long long sum = 0;
    long long prod = 1;
    int pos_count = 0, neg_count = 0;

    for (int r = f->r1; r <= f->r2; r++) {
        for (int c = f->c1; c <= f->c2; c++) {
            ternary_t v = grid_get(g, r, c);
            sum  += v;
            prod *= v;
            if (v > 0) pos_count++;
            if (v < 0) neg_count++;
        }
    }

    switch (f->type) {
        case FORMULA_SUM:
            return clamp_ternary((int)sum);
        case FORMULA_PRODUCT:
            return clamp_ternary((int)prod);
        case FORMULA_THRESHOLD:
            return (pos_count >= f->threshold) ? TERNARY_POS :
                   (neg_count >= f->threshold) ? TERNARY_NEG : TERNARY_ZERO;
        default:
            return TERNARY_ZERO;
    }
}

/* ------------------------------------------------------------------ */
/* Evaluator – topological (dependency order)                          */
/*                                                                     */
/* Simplified approach: multiple passes until stable. Each pass        */
/* evaluates formula cells whose dependencies are all computed.        */
/* ------------------------------------------------------------------ */

static bool deps_ready(Grid *g, const Formula *f)
{
    for (int r = f->r1; r <= f->r2; r++)
        for (int c = f->c1; c <= f->c2; c++) {
            Cell *dep = grid_cell(g, r, c);
            if (dep->has_formula && !dep->computed)
                return false;
        }
    return true;
}

void grid_evaluate(Grid *g)
{
    /* Reset computed flags */
    int n = g->rows * g->cols;
    for (int i = 0; i < n; i++)
        g->cells[i].computed = !g->cells[i].has_formula;

    /* Iterate until all resolved or stuck */
    for (int pass = 0; pass < n + 1; pass++) {
        bool progress = false;
        for (int i = 0; i < n; i++) {
            Cell *cell = &g->cells[i];
            if (cell->has_formula && !cell->computed && deps_ready(g, &cell->formula)) {
                cell->value = eval_formula(g, &cell->formula);
                cell->computed = true;
                progress = true;
            }
        }
        if (!progress) break;
    }
}

/* ------------------------------------------------------------------ */
/* SortEngine – fitness-based (natural selection) ordering             */
/*                                                                     */
/* Fitness of a cell: +1 → 2, 0 → 1, -1 → 0   (higher is fitter).    */
/* Selection sort by fitness descending (stable for equal fitness).    */
/* ------------------------------------------------------------------ */

static int cell_fitness(ternary_t v)
{
    return v + 1;  /* -1→0, 0→1, +1→2 */
}

void sort_engine_sort(Grid *g, int r1, int c1, int r2, int c2)
{
    /* Collect cells in range into a flat list */
    int count = (r2 - r1 + 1) * (c2 - c1 + 1);
    ternary_t *vals = (ternary_t *)malloc((size_t)count * sizeof(ternary_t));
    int k = 0;
    for (int r = r1; r <= r2; r++)
        for (int c = c1; c <= c2; c++)
            vals[k++] = grid_get(g, r, c);

    /* Selection sort descending by fitness */
    for (int i = 0; i < count - 1; i++) {
        int best = i;
        for (int j = i + 1; j < count; j++) {
            if (cell_fitness(vals[j]) > cell_fitness(vals[best]))
                best = j;
        }
        if (best != i) {
            ternary_t tmp = vals[i];
            vals[i] = vals[best];
            vals[best] = tmp;
        }
    }

    /* Write back */
    k = 0;
    for (int r = r1; r <= r2; r++)
        for (int c = c1; c <= c2; c++)
            grid_set(g, r, c, vals[k++]);

    free(vals);
}

/* ------------------------------------------------------------------ */
/* AutofillEngine – mutation-based fill                                */
/*                                                                     */
/* Tile source range into destination. Each cell has mutation_rate %   */
/* chance to shift by ±1 (clamped to ternary range).                   */
/* ------------------------------------------------------------------ */

static ternary_t mutate(ternary_t v, int mutation_rate)
{
    /* pseudo-random is fine for a spreadsheet engine; use a simple LCG */
    static unsigned long seed = 42;
    seed = seed * 1103515245UL + 12345UL;
    int pct = (int)((seed >> 16) % 100);
    if (pct >= mutation_rate) return v;

    seed = seed * 1103515245UL + 12345UL;
    int dir = (int)((seed >> 16) % 2);  /* 0 = -1, 1 = +1 */
    return clamp_ternary(v + (dir ? 1 : -1));
}

void autofill_engine_fill(Grid *g,
                          int sr1, int sc1, int sr2, int sc2,
                          int dr1, int dc1, int dr2, int dc2,
                          int mutation_rate)
{
    int src_h = sr2 - sr1 + 1;
    int src_w = sc2 - sc1 + 1;

    for (int r = dr1; r <= dr2; r++) {
        for (int c = dc1; c <= dc2; c++) {
            int sr = sr1 + (r - dr1) % src_h;
            int sc = sc1 + (c - dc1) % src_w;
            ternary_t v = grid_get(g, sr, sc);
            grid_set(g, r, c, mutate(v, mutation_rate));
        }
    }
}
