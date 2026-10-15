// Solve.cpp : command-line tool for solving a sudoku.

#include "SudokuCore.h"
#include "SudokuSolver.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Args {
	sudoku_size_t square_height = 3;
	sudoku_size_t square_width = 3;
	std::string puzzle;
};

void print_usage(const char * prog) {
	std::cerr <<
		"Usage: " << prog << " [--square-height H] [--square-width W] \"<puzzle>\"\n"
		"\n"
		"  <puzzle>          Row-major, space-separated cell values, 0 = empty.\n"
		"                    Length must be (H*W)^2, e.g. 81 for the default 9x9 board.\n"
		"  --square-height   Block height (default 3).\n"
		"  --square-width    Block width (default 3).\n"
		"\n"
		"Supported (square-height, square-width) pairs: 2x2, 2x3, 3x2, 3x3, 3x4, 4x3, 4x4.\n"
		"\n"
		"Example: " << prog << " \"0 0 4 0 8 9 5 7 0  0 0 0 7 1 0 6 3 4  5 0 0 4 6 3 0 0 0"
		"  9 3 0 0 0 0 2 0 0  6 0 0 9 0 1 0 0 3  0 0 2 0 0 0 0 9 5"
		"  0 0 0 1 5 2 0 0 9  8 5 3 0 9 4 0 0 0  0 2 9 8 3 0 4 0 0\"\n";
}

bool parse_args(int argc, char** argv, Args & args) {
	std::vector<std::string> positional;
	for (int i = 1; i < argc; ++i) {
		const std::string a = argv[i];
		if (a == "--square-height" && i + 1 < argc) {
			args.square_height = std::stoul(argv[++i]);
		}
		else if (a == "--square-width" && i + 1 < argc) {
			args.square_width = std::stoul(argv[++i]);
		}
		else if (a == "-h" || a == "--help") {
			return false;
		}
		else {
			positional.push_back(a);
		}
	}
	if (positional.size() != 1) return false;
	args.puzzle = positional[0];
	return true;
}

/// Parses `args.puzzle` into a raw_sudoku_t<sq_h,sq_w> and solves it.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
int run(const Args & args) {
	using Dims = SudokuDims<sq_h, sq_w>;

	raw_sudoku_t<sq_h, sq_w> raw{};
	std::istringstream iss(args.puzzle);
	sudoku_size_t count = 0;
	sudoku_size_t value = 0;
	while (iss >> value) {
		if (count >= Dims::tot_num_cells) {
			std::cerr << "error: too many cells; expected " << Dims::tot_num_cells
				<< " for a " << Dims::side_len << "x" << Dims::side_len << " board.\n";
			return 1;
		}
		if (value > Dims::side_len) {
			std::cerr << "error: cell value " << value << " out of range (max " << Dims::side_len << ").\n";
			return 1;
		}
		raw[count++] = value;
	}
	if (count != Dims::tot_num_cells) {
		std::cerr << "error: expected " << Dims::tot_num_cells << " cells for a "
			<< Dims::side_len << "x" << Dims::side_len << " board, got " << count << ".\n";
		return 1;
	}

	sudoku_data_t<sq_h, sq_w> sudoku = init_sudoku_with_raw<sq_h, sq_w>(raw);
	auto_fill<sq_h, sq_w>(sudoku, true);

	const int n_solutions = SudokuSolver<sq_h, sq_w>::solve_brute_force_all(sudoku);
	std::cout << n_solutions << " solution(s) found.\n";
	if (n_solutions > 0) {
		print_raw_sudoku<sq_h, sq_w>(std::cout, get_raw_sudoku<sq_h, sq_w>(sudoku));
	}
	return n_solutions > 0 ? 0 : 1;
}

} // namespace

int main(int argc, char** argv) {
	Args args;
	if (!parse_args(argc, argv, args)) {
		print_usage(argv[0]);
		return 2;
	}

	const sudoku_size_t h = args.square_height;
	const sudoku_size_t w = args.square_width;

	if (h == 2 && w == 2) return run<2, 2>(args);
	if (h == 2 && w == 3) return run<2, 3>(args);
	if (h == 3 && w == 2) return run<3, 2>(args);
	if (h == 3 && w == 3) return run<3, 3>(args);
	if (h == 3 && w == 4) return run<3, 4>(args);
	if (h == 4 && w == 3) return run<4, 3>(args);
	if (h == 4 && w == 4) return run<4, 4>(args);

	std::cerr << "error: unsupported board size " << h << "x" << w << " squares.\n";
	print_usage(argv[0]);
	return 2;
}
