# ternary-spreadsheet-c

C implementation of a ternary spreadsheet engine — cells that are tiny ternary agents.

## Features

- **Cell**: single ternary cell with value (-1, 0, +1), optional formula, and computed flag
- **Grid**: N×M grid of cells with get/set/evaluate
- **Formulas**: SUM, PRODUCT, THRESHOLD over rectangular ranges
- **Evaluator**: topological (dependency-order) evaluation with chained formulas
- **SortEngine**: fitness-based natural-selection sorting of ranges
- **AutofillEngine**: mutation-based autofill with configurable mutation rate

## Build & Test

```bash
gcc -o test_spreadsheet tests/test_spreadsheet.c src/ternary_spreadsheet.c -lm -Wall -O2
./test_spreadsheet
```

## Usage

```c
#include "src/ternary_spreadsheet.h"

Grid g = grid_create(4, 4);
grid_set(&g, 0, 0, TERNARY_POS);

// Set a SUM formula at (3,3) over range (0,0)-(2,2)
Formula f = { FORMULA_SUM, 0, 0, 2, 2, 0 };
grid_set_formula(&g, 3, 3, f);
grid_evaluate(&g);

printf("%d\n", grid_get(&g, 3, 3)); // prints 1

grid_destroy(&g);
```

## License

MIT
