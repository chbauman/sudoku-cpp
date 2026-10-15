// Generate.cpp : command-line tool for generating sudoku puzzles.

#include "SudokuCore.h"
#include "SudokuSolver.h"
#include "SudokuGenerator.h"

#include <iostream>
#include <string>

namespace {

struct Args {
	sudoku_size_t square_height = 3;
	sudoku_size_t square_width = 3;
	num_sud_t max_per_level = 1000;
	rec_depth_t min_level = 3;
	int iterations = 50000;
};

void print_usage(const char * prog) {
	std::cerr <<
		"Usage: " << prog << " [--square-height H] [--square-width W]"
		" [--max-per-level N] [--min-level L] [--iterations N]\n"
		"\n"
		"  --square-height   Block height (default 3).\n"
		"  --square-width    Block width (default 3).\n"
		"  --max-per-level   Stop collecting puzzles for a difficulty level once\n"
		"                    this many have been found for it (default 1000).\n"
		"  --min-level       Minimum recursion-depth difficulty a puzzle must\n"
		"                    reach to be kept (default 3).\n"
		"  --iterations      Number of distinct fully-solved grids to try\n"
		"                    generating puzzles from before stopping (default 50000).\n"
		"\n"
		"Puzzles are saved under ./Data/<square_height>x<square_width>/.\n"
		"Supported (square-height, square-width) pairs: 2x2, 2x3, 3x2, 3x3, 3x4, 4x3, 4x4.\n";
}

bool parse_args(int argc, char** argv, Args & args) {
	for (int i = 1; i < argc; ++i) {
		const std::string a = argv[i];
		if (a == "--square-height" && i + 1 < argc) {
			args.square_height = std::stoul(argv[++i]);
		}
		else if (a == "--square-width" && i + 1 < argc) {
			args.square_width = std::stoul(argv[++i]);
		}
		else if (a == "--max-per-level" && i + 1 < argc) {
			args.max_per_level = std::stoi(argv[++i]);
		}
		else if (a == "--min-level" && i + 1 < argc) {
			args.min_level = std::stoi(argv[++i]);
		}
		else if (a == "--iterations" && i + 1 < argc) {
			args.iterations = std::stoi(argv[++i]);
		}
		else if (a == "-h" || a == "--help") {
			return false;
		}
		else {
			std::cerr << "error: unrecognized argument '" << a << "'\n";
			return false;
		}
	}
	return true;
}

template<sudoku_size_t sq_h, sudoku_size_t sq_w>
int run(const Args & args) {
	SudokuGenerator<sq_h, sq_w>::generate_hard_sudokus(args.max_per_level, args.min_level, args.iterations);
	return 0;
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
