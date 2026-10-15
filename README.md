# sudoku-cpp
Sudoku Solving and generating with c++

## Building

There's no CMake/Makefile project here — just a few headers under `src/`
plus one `.cpp` file, so a plain `g++` invocation builds it:

```sh
g++ -std=c++17 -O2 src/Sudoku.cpp -o sudoku
```

This is exactly what [`.github/workflows/build.yml`](.github/workflows/build.yml)
runs on every push and pull request.

### API docs

[`Doxyfile`](Doxyfile) is set up to document `src/`. With
[Doxygen](https://www.doxygen.nl/) installed, run from the repo root:

```sh
doxygen Doxyfile
```

This generates browsable HTML docs under `docs/html/index.html`.

## Usage

```sh
./sudoku
```

`src/Sudoku.cpp`'s `main()` is a demo driver rather than a CLI — it takes no
arguments. Running it:

1. Loads one of the hardcoded sample puzzles (`easy_sudoku_3x3` by default;
   see the other `raw_sudoku_t` arrays defined in `main()` for harder ones).
2. Solves it with `SudokuSolver::solve_brute_force_all`, printing how many
   solutions it found.
3. Solves it again with `SudokuSolver::solve_count_rec_depth`, printing the
   minimum recursion depth needed.
4. Calls `SudokuGenerator::generate_hard_sudokus()`, which generates and
   rates new puzzles by difficulty, saving them under `./Data/`. This loops
   up to 50000 times and can run for a long time — interrupt it with
   `Ctrl+C` once you have enough.

### Input / output format

A puzzle is a `raw_sudoku_t`: a flat, row-major array of 81 cells (9x9),
where `0` means "empty" and `1`-`9` are given digits:

```cpp
const raw_sudoku_t puzzle = {
    0,0,4,0,8,9,5,7,0,
    0,0,0,7,1,0,6,3,4,
    5,0,0,4,6,3,0,0,0,
    9,3,0,0,0,0,2,0,0,
    6,0,0,9,0,1,0,0,3,
    0,0,2,0,0,0,0,9,5,
    0,0,0,1,5,2,0,0,9,
    8,5,3,0,9,4,0,0,0,
    0,2,9,8,3,0,4,0,0
};
```

Printing a `raw_sudoku_t` (e.g. with `std::cout << puzzle`) renders it as a
bordered grid grouped into 3x3 squares:

```
+-------+-------+-------+-
| 0 0 4 | 0 8 9 | 5 7 0 |
| 0 0 0 | 7 1 0 | 6 3 4 |
| 5 0 0 | 4 6 3 | 0 0 0 |
+-------+-------+-------+-
| 9 3 0 | 0 0 0 | 2 0 0 |
| 6 0 0 | 9 0 1 | 0 0 3 |
| 0 0 2 | 0 0 0 | 0 9 5 |
+-------+-------+-------+-
| 0 0 0 | 1 5 2 | 0 0 9 |
| 8 5 3 | 0 9 4 | 0 0 0 |
| 0 2 9 | 8 3 0 | 4 0 0 |
+-------+-------+-------+-
```

Internally, solving works on a `sudoku_data_t`: one entry per cell plus 9
more per cell tracking, for each of the 9 digits, whether it's still
possible (`2`), ruled out (`1`), or moot because the cell is already set
(the cell's own entry is non-zero). `init_sudoku_with_raw`/`get_raw_sudoku`
convert between the two, and `auto_fill` populates the possibility entries
from the given digits.

### Example: solving a puzzle from your own code

```cpp
#include "SudokuCore.h"
#include "SudokuSolver.h"

raw_sudoku_t puzzle = { /* ... 81 cells, 0 = empty ... */ };

sudoku_data_t sudoku = init_sudoku_with_raw(puzzle);
auto_fill(sudoku, true);

int n_solutions = SudokuSolver::solve_brute_force_all<3, 3>(sudoku);
std::cout << n_solutions << " solution(s)\n";
std::cout << get_raw_sudoku(sudoku); // the (first) solved grid, if any
```

### Using it as a library

The solving and generating logic lives in three headers you can include
directly in your own code:

- `SudokuCore.h` — sudoku types (`raw_sudoku_t`, `sudoku_data_t`),
  constants, printing, and init/conversion helpers.
- `SudokuSolver.h` — `class SudokuSolver`: deterministic solving techniques
  plus brute-force solvers (`solve_brute_force_all`,
  `solve_count_rec_depth`, ...).
- `SudokuGenerator.h` — `class SudokuGenerator` and `RandomNumberPicker`:
  puzzle generation and the on-disk puzzle collection format.
