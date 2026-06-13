# Ternary Spreadsheet (C)

A **ternary spreadsheet** is a grid where each cell holds a balanced ternary value (−1, 0, +1) and optional formulas (SUM, PRODUCT, THRESHOLD) over rectangular ranges. This C library provides grid creation, formula evaluation with topological dependency ordering, fitness-based sorting, and mutation-based autofill.

## Why It Matters

Spreadsheets are one of the most successful computation models ever devised — they make dependency graphs visual and computation accessible to non-programmers. A *ternary* spreadsheet extends this to three-valued logic, where each cell captures not just presence (1) or absence (0) but also a neutral/unknown state (−1 in balanced ternary). This maps naturally to decision matrices (accept/reject/abstain), game boards (win/loss/draw), and agent strategy spaces (choose/avoid/neutral). The formula evaluation engine handles dependency cycles via topological sort, and the sort engine provides natural-selection-style ordering where "fitter" cells (more positive values) rise to the top — a visual demonstration of evolutionary dynamics on a grid.

## How It Works

### Grid Model

```c
typedef struct {
    ternary_t value;       // -1, 0, or +1
    Formula   formula;     // SUM, PRODUCT, THRESHOLD
    bool      has_formula;
    bool      computed;    // evaluated this cycle
} Cell;
```

Grids are row-major `Cell` arrays of size rows × cols.

### Formula Evaluation

Formulas reference rectangular ranges and are evaluated in **topological order** — a cell's formula is computed only after all cells it depends on have been computed:

```
topological_sort(cells with formulas) → evaluation order
for each cell in order:
    cell.value = evaluate_formula(cell.formula, grid)
```

**SUM(range)**: Sum all values in the rectangular range, clamped to ternary:
```
result = clamp(Σ cells, -1, +1)
```

**PRODUCT(range)**: Multiply (logical AND-like):
```
result = clamp(Π cells, -1, +1)
```

**THRESHOLD(range, t)**: If sum exceeds threshold, +1; if below −t, −1; else 0.

Cycle detection: if the topological sort encounters a cycle, those cells remain unevaluated (marked `computed = false`).

Complexity: O(N·M + E) where E = number of formula dependencies.

### Sort Engine

The sort engine orders cells by a fitness metric based on ternary values:

```
fitness(row) = Σ positive_values - Σ negative_values
```

Rows with more +1 values sort higher; rows with more −1 values sort lower. This mimics natural selection: "fitter" rows rise to the top. O(N log N) via quicksort.

### Autofill Engine

The autofill engine fills a destination range with **mutated copies** of a source range:

```c
void autofill_engine_fill(grid, src_range, dst_range, mutation_rate);
```

For each destination cell, the corresponding source cell is copied with probability `(100 - mutation_rate)%` of exact copy and `mutation_rate%` of one-step ternary mutation:

```
mutate(v) = clamp(v + random_choice({-1, 0, +1}), -1, +1)
```

This is a simple genetic operator — the spreadsheet becomes a visual GA where patterns evolve across the grid.

## Quick Start

```c
#include "ternary_spreadsheet.h"
#include <stdio.h>

int main(void) {
    Grid g = grid_create(5, 5);

    // Set some values
    grid_set(&g, 0, 0, TERNARY_POS);
    grid_set(&g, 0, 1, TERNARY_NEG);
    grid_set(&g, 1, 0, TERNARY_ZERO);

    // Set a formula: cell(2,2) = SUM of range (0,0)-(1,1)
    Formula sum = {.type = FORMULA_SUM, .r1=0, .c1=0, .r2=1, .c2=1};
    grid_set_formula(&g, 2, 2, sum);
    grid_evaluate(&g);

    printf("Cell(2,2) = %d\n", grid_get(&g, 2, 2));  // evaluated SUM

    // Sort rows by fitness
    sort_engine_sort(&g, 0, 0, 4, 4);

    // Autofill with 10% mutation
    autofill_engine_fill(&g, 0,0, 1,4, 3,0, 4,4, 10);

    grid_destroy(&g);
    return 0;
}
```

Compile: `gcc -o demo src/ternary_spreadsheet.c && ./demo`

## API

| Function | Description |
|----------|-------------|
| `grid_create(rows, cols)` | Allocate N×M grid |
| `grid_set(r, c, val)` | Set cell value |
| `grid_get(r, c)` | Get cell value |
| `grid_set_formula(r, c, Formula)` | Attach formula |
| `grid_evaluate()` | Evaluate all formulas (topological) |
| `sort_engine_sort(range)` | Sort range by ternary fitness |
| `autofill_engine_fill(src, dst, rate)` | Mutated copy from src to dst |
| `clamp_ternary(v)` | Clamp integer to {-1, 0, +1} |

## Architecture Notes

The Ternary Spreadsheet is a visual γ + η = C computer. Each cell's value is a ternary decision: +1 (γ — constructive), −1 (η — avoidant), 0 (neutral). Formulas propagate γ and η across dependency edges, computing the emergent competence C of the grid. The sort engine applies selection pressure (preferring γ over η), and the autofill engine applies mutation (the source of variation). Together, these three operations — evaluate, sort, fill — constitute a complete evolutionary loop running on a spreadsheet grid. See [ARCHITECTURE.md](https://github.com/SuperInstance/SuperInstance/blob/main/ARCHITECTURE.md).

## References

1. Knuth, D. E. (1981). *TAOCP Vol. 2*. — Balanced ternary number system.
2. Codd, E. F. (1970). "A Relational Model of Data for Large Shared Data Banks." *Communications of the ACM*. — On the relational model underlying spreadsheet data.
3. Holland, J. H. (1975). *Adaptation in Natural and Artificial Systems*. — Selection, mutation, and fitness in evolutionary systems.

## License

MIT
