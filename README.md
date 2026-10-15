# sudoku-cpp

![Sample sudoku](Sudoku.png)

Sudoku solving and generating with c++

## Building

Since the code is only a handful of header files, there are no project files.
Directly build it with g++:

```sh
g++ -std=c++17 -O2 src/Solve.cpp    -o solve
g++ -std=c++17 -O2 src/Generate.cpp -o generate
```

This is what [`.github/workflows/build.yml`](.github/workflows/build.yml)
runs on every push and pull request. It also cross-compiles `solve.exe`/
`generate.exe` for Windows with MinGW-w64, uploaded as a `windows-binaries`
artifact on each run (see the Actions tab on GitHub) — no CMake/MSVC
project needed, since the code is plain, portable C++17.

### Releases

[`.github/workflows/release.yml`](.github/workflows/release.yml) runs when
a GitHub release is published: it builds `solve`/`generate` for both Linux
and Windows (cross-compiled with MinGW-w64) and attaches
`sudoku-cpp-<tag>-linux-x86_64.tar.gz`/`sudoku-cpp-<tag>-windows-x86_64.zip`
to the release as downloadable assets.

### API docs

[`Doxyfile`](Doxyfile) is set up to document `src/`. With
[Doxygen](https://www.doxygen.nl/) installed, run from the repo root:

```sh
doxygen Doxyfile
```

This generates browsable HTML docs under `docs/html/index.html`.

## Usage

Two command-line tools are built from `src/`: `solve` and `generate`. Both
default to a standard 9x9 board (3x3 squares) and accept
`--square-height`/`--square-width` to work with other board sizes — see
[Board sizes](#board-sizes) below.

### `solve`

```sh
./solve "6 0 0 0 0 8 9 4 0  9 0 0 0 0 6 1 0 0  0 7 0 0 4 0 0 0 0  2 0 0 6 1 0 0 0 0  0 0 0 0 0 0 2 0 0  0 8 9 0 0 2 0 0 0  0 0 0 0 6 0 0 0 5  0 0 0 0 0 0 0 3 0  8 0 0 0 0 1 6 0 0"
```

Takes a puzzle in [row format](#input--output-format) as its one positional
argument. By default, solves it and reports whether the solution is unique
(stopping as soon as that's known, without enumerating every solution if
there are many), printing the solution if at least one exists. Exit code
is `0` if solved, `1` if invalid/unsolvable, `2` on a usage error.

```
$ ./solve --help
Usage: ./solve [--square-height H] [--square-width W] [--first-only | --count-all] "<puzzle>"

  <puzzle>          Row-major, space-separated cell values, 0 = empty.
                    Length must be (H*W)^2, e.g. 81 for the default 9x9 board.
  --square-height   Block height (default 3).
  --square-width    Block width (default 3).
  --first-only      Stop at the first solution found, without checking
                    whether it's the only one. Rarely needed: the default
                    already checks uniqueness at the same speed, but this
                    can be faster on puzzles where a second solution
                    exists but is hard to find.
  --count-all       Count every solution exactly. Can be far slower than
                    the default on underconstrained puzzles (many
                    solutions); use when you need the exact count rather
                    than just "unique" vs "multiple".
```

### `generate`

```sh
./generate
```

Generates sudoku puzzles, rates each by solving difficulty (recursion depth
needed), and saves them to `./Data/<square_height>x<square_width>/dat.txt`
by default (override with `--output-path`). This can run for a long time —
interrupt it with `Ctrl+C` once you have enough.

```
$ ./generate --help
Usage: ./generate [--square-height H] [--square-width W] [--max-per-level N] [--min-level L] [--iterations N] [--output-path PATH]

  --square-height   Block height (default 3).
  --square-width    Block width (default 3).
  --max-per-level   Stop collecting puzzles for a difficulty level once
                    this many have been found for it (default 1000).
  --min-level       Minimum recursion-depth difficulty a puzzle must
                    reach to be kept (default 3).
  --iterations      Number of distinct fully-solved grids to try
                    generating puzzles from before stopping (default 50000).
  --output-path     File to load existing puzzles from and save to
                    (default ./Data/<square_height>x<square_width>/dat.txt).
                    Parent directories are created if missing.
```

### Generated puzzle file format

`generate` saves puzzles under `./Data/<square_height>x<square_width>/dat.txt`
— one line per puzzle, all fields comma-separated:

```
<level>,<filled-in count>,<freq_1>,...,<freq_N>,<puzzle cell_1>,...,<puzzle cell_M>,<solution cell_1>,...,<solution cell_M>
```

where `N = square_height*square_width` (one frequency count per digit
value) and `M = N*N` (one value per cell, row-major, same layout as
[row format](#input--output-format)). For example, this line was written
by actually saving the 9x9 puzzle from the [`solve` example](#solve) above
together with a solution:

```
2,38,7,6,5,5,4,3,3,3,2,0,0,4,0,8,9,5,7,0,0,0,0,7,1,0,6,3,4,5,0,0,4,6,3,0,0,0,9,3,0,0,0,0,2,0,0,6,0,0,9,0,1,0,0,3,0,0,2,0,0,0,0,9,5,0,0,0,1,5,2,0,0,9,8,5,3,0,9,4,0,0,0,0,2,9,8,3,0,4,0,0,3,6,4,2,8,9,5,7,1,2,9,8,7,1,5,6,3,4,5,1,7,4,6,3,9,2,8,9,3,1,5,4,8,2,6,7,6,7,5,9,2,1,8,4,3,4,8,2,3,7,6,1,9,5,7,4,6,1,5,2,3,8,9,8,5,3,6,9,4,7,1,2,1,2,9,8,3,7,4,5,6
```

Breaking that down:
- `2` — difficulty level (the minimum recursion depth `solve_count_rec_depth`
  needed to solve it).
- `38` — 38 cells were given in the puzzle.
- `7,6,5,5,4,3,3,3,2` — how many times each digit occurs among those 38
  given cells, one count per digit 1-9, sorted in decreasing order.
- Next 81 values — the puzzle itself (`0` = empty).
- Final 81 values — its solution.

The first `2 + square_height*square_width` fields (level, count,
frequencies) together form the string `generate_sud_char()` produces, the
same string `add_to_coll()` uses as a bucketing key — so `--max-per-level`
really limits "puzzles with this exact level and digit-frequency profile",
not just "puzzles at this level".

`separate_by_level_and_save()` (a library function on `SudokuGenerator`,
not currently exposed by `generate`'s CLI) splits a loaded collection by
level into `./Data/<H>x<W>/ext_lvl_<N>.txt` files in the same format.

The format is comma-delimited (rather than fixed-width) specifically so it
still works for board sizes with more than 9 values per cell, like 4x4
squares (16x16 grid, two-digit cell values).

### `generated_dat.txt`

[`generated_dat.txt`](generated_dat.txt) is a pre-generated sample
collection of 2908 hard 9x9 sudokus (difficulty levels 1 through 8 —
mostly 1-3; level 8 appears once), in the file format above. It isn't
referenced anywhere in the code — it's just sample data — but
`SudokuGenerator<3, 3>::load_coll("generated_dat.txt")` will load it
correctly, and every entry has been verified: puzzle/solution lengths,
the descriptor's digit-count and frequency fields matching the puzzle's
actual contents, the solution agreeing with the puzzle's given cells, and
every row/column/square of every solution being a valid 1-9 arrangement.

(This file used to be called `example_data.txt` and predate the
comma-delimited format above — space-delimited and fixed-width instead,
one single-digit-assuming 12-character descriptor followed by each cell as
a 2-character `"N "` token. It's been converted and renamed to match the
current format and what it actually is.)

### Board sizes

Board dimensions aren't limited to the standard 9x9: `--square-height`/
`--square-width` select the block size (e.g. `2 2` for a 4x4 board, `2 3` for
a 6x6 board). Both tools are built for a small fixed set of sizes, listed in
their own `--help`: `2x2, 2x3, 3x2, 3x3, 3x4, 4x3, 4x4`. An unsupported size
is rejected with an error.

### Input / output format

A puzzle is a `raw_sudoku_t<square_height, square_width>`: a flat, row-major
array of `(square_height*square_width)^2` cells, where `0` means "empty" and
other digits (up to `square_height*square_width`) are given numbers. For the
default 9x9 board:

```cpp
const raw_sudoku_t<3, 3> puzzle = {
    6,0,0,0,0,8,9,4,0,
    9,0,0,0,0,6,1,0,0,
    0,7,0,0,4,0,0,0,0,
    2,0,0,6,1,0,0,0,0,
    0,0,0,0,0,0,2,0,0,
    0,8,9,0,0,2,0,0,0,
    0,0,0,0,6,0,0,0,5,
    0,0,0,0,0,0,0,3,0,
    8,0,0,0,0,1,6,0,0
};
```

`print_raw_sudoku<3, 3>(std::cout, puzzle)` renders it as a bordered grid
grouped into 3x3 squares:

```
+-------+-------+-------+-
| 6 0 0 | 0 0 8 | 9 4 0 |
| 9 0 0 | 0 0 6 | 1 0 0 |
| 0 7 0 | 0 4 0 | 0 0 0 |
+-------+-------+-------+-
| 2 0 0 | 6 1 0 | 0 0 0 |
| 0 0 0 | 0 0 0 | 2 0 0 |
| 0 8 9 | 0 0 2 | 0 0 0 |
+-------+-------+-------+-
| 0 0 0 | 0 6 0 | 0 0 5 |
| 0 0 0 | 0 0 0 | 0 3 0 |
| 8 0 0 | 0 0 1 | 6 0 0 |
+-------+-------+-------+-
```

The `solve` CLI tool's row format is the same grid flattened into one line,
cells separated by whitespace.

Internally, solving works on a `sudoku_data_t<square_height, square_width>`:
one entry per cell plus one more per cell per digit, tracking whether that
digit is still possible (`2`), ruled out (`1`), or moot because the cell is
already set (the cell's own entry is non-zero).
`init_sudoku_with_raw`/`get_raw_sudoku` convert between the two
representations, and `auto_fill` populates the possibility entries from the
given digits.

### Using it as a library

The solving and generating logic lives in three headers you can include
directly in your own code. Every type and function is templated on
`<square_height, square_width>` so one binary can work with more than one
board size:

- `SudokuCore.h` — sudoku types (`raw_sudoku_t<H,W>`, `sudoku_data_t<H,W>`),
  constants (`SudokuDims<H,W>`), printing, and init/conversion helpers.
- `SudokuSolver.h` — `class SudokuSolver<H,W>`: deterministic solving
  techniques plus brute-force solvers (`solve_brute_force_all`,
  `solve_count_rec_depth`, ...).
- `SudokuGenerator.h` — `class SudokuGenerator<H,W>` and
  `RandomNumberPicker<H,W>`: puzzle generation and the on-disk puzzle
  collection format.
