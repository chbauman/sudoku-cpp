#pragma once

#include <array>
#include <cassert>
#include <cstddef>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>
#include <optional>

/// Size type of sudoku.
typedef std::size_t sudoku_size_t;

/// Type of sudoku entries.
typedef int sudoku_value_t;

constexpr bool printDebugInfodefault = false;

/// Random seed.
constexpr sudoku_size_t seed = 50;

/// Bundles the dimensions of a sudoku board (square_height x square_width
/// squares, arranged in a (square_height*square_width) x (square_height*square_width)
/// grid) and the sizes derived from them.
///
/// Every templated piece of the solver/generator takes `square_height` and
/// `square_width` as explicit template parameters and derives everything
/// else (side_len, tot_num_cells, ...) from this, so a single binary can
/// be instantiated for more than one board size.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
struct SudokuDims {
	static_assert(sq_h > 0 && sq_w > 0, "Do you really want an empty fucking Sudoku?");

	static constexpr sudoku_size_t square_height = sq_h;
	static constexpr sudoku_size_t square_width = sq_w;
	static constexpr sudoku_size_t side_len = sq_h * sq_w;
	static constexpr sudoku_size_t tot_num_cells = side_len * side_len;
	static constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
	static constexpr sudoku_size_t tot_storage = n_stored_per_cell * tot_num_cells;
	static constexpr sudoku_size_t n_stored_per_side = n_stored_per_cell * side_len;
};

/// Sudoku data type for solving.
///
/// Meaning of entries: 2: possible, 1: not possible, 0: definite number set.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
using sudoku_data_t = std::array<sudoku_size_t, SudokuDims<sq_h, sq_w>::tot_storage>;

/// Sudoku input data type.
///
/// 0 denotes empty cell, other numbers denote the set numbers.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
using raw_sudoku_t = std::array<sudoku_size_t, SudokuDims<sq_h, sq_w>::tot_num_cells>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print The Sudoku to std::cout

template<sudoku_size_t sq_h, sudoku_size_t sq_w>
inline void printLine(std::ostream & os) {
	os << "+-";
	for (sudoku_size_t square_col = 0; square_col < sq_h; ++square_col) {
		for (sudoku_size_t cell_col = 0; cell_col < sq_w; ++cell_col) {
			os << "--";
		}
		os << "+-";
	}
	os << "\n";
}

/// Prints a raw sudoku to the given stream.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
inline void print_raw_sudoku(std::ostream & os, const raw_sudoku_t<sq_h, sq_w> & sud){
	using Dims = SudokuDims<sq_h, sq_w>;
	printLine<sq_h, sq_w>(os);
	for (sudoku_size_t square_row = 0; square_row < sq_w; ++square_row) {
		for (sudoku_size_t cell_row = 0; cell_row < sq_h; ++cell_row) {
			os << "| ";
			for (sudoku_size_t square_col = 0; square_col < sq_h; ++square_col) {
				for (sudoku_size_t cell_col = 0; cell_col < sq_w; ++cell_col) {
					sudoku_size_t col_ind = cell_col + sq_w * square_col;
					sudoku_size_t row_ind = cell_row + sq_h * square_row;
					sudoku_size_t arr_ind = col_ind + Dims::side_len * row_ind;
					os << sud[arr_ind] << " ";
				}
				os << "| ";
			}
			os << "\n";
		}
		printLine<sq_h, sq_w>(os);
	}
}

/// Prints the internal (possibility-tracking) sudoku representation to the given stream.
template<sudoku_size_t sq_h, sudoku_size_t sq_w>
inline void print_sudoku_data(std::ostream & os, const sudoku_data_t<sq_h, sq_w> & sud){
	using Dims = SudokuDims<sq_h, sq_w>;
	for (sudoku_size_t i = 0; i < Dims::tot_num_cells; ++i) {
		if (i % Dims::side_len == 0) os << "\n";
		for (sudoku_size_t k = 0; k < Dims::n_stored_per_cell; ++k) {
			os << sud[k + Dims::n_stored_per_cell * i] << " ";
		}
		os << "\n";
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper

/// Initialize an array with 0.
template<std::size_t n>
void setZero(std::array<sudoku_size_t, n> & arr){
	for (std::size_t i = 0; i < n; ++i) {
		arr[i] = 0;
	}
};

/// Sum all elements of an array.
template<class value_t, std::size_t n>
value_t sum(const std::array<value_t, n> & arr) {
	value_t sum_curr = (value_t)0;
	for (std::size_t i = 0; i < n; ++i) {
		sum_curr += arr[i];
	}
	return sum_curr;
};

/// Check if all elements of array are 1.
template<class value_t, std::size_t n>
bool check_all_1(const std::array<value_t, n> & arr) {
	for (std::size_t i = 0; i < n; ++i) {
		if (arr[i] != (value_t)1) {
			return false;
		}
	}
	return true;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Sudoku and Convert

/// Initialize a sudoku.
///
/// Fills it with zeros.
template<sudoku_size_t sq_h, sudoku_size_t sq_w, bool printDebugInfo = printDebugInfodefault>
sudoku_data_t<sq_h, sq_w> init_sudoku() {
	sudoku_data_t<sq_h, sq_w> s_data{};
	if constexpr (printDebugInfo) std::cout << "Filled Sudoku with Zeros.\n";
	return s_data;
}

/// Initializes sudoku with a raw sudoku.
template<sudoku_size_t sq_h, sudoku_size_t sq_w, bool printDebugInfo = printDebugInfodefault>
sudoku_data_t<sq_h, sq_w> init_sudoku_with_raw(const raw_sudoku_t<sq_h, sq_w> & raw_s) {
	using Dims = SudokuDims<sq_h, sq_w>;
	sudoku_data_t<sq_h, sq_w> s_data = init_sudoku<sq_h, sq_w>();
	for (sudoku_size_t ind = 0; ind < Dims::tot_num_cells; ++ind) {
		const sudoku_size_t temp = raw_s[ind];
		if (temp) {
			s_data[ind * Dims::n_stored_per_cell] = temp;
		}
	}
	if constexpr (printDebugInfo) std::cout << "Constructed Sudoku Data with raw Sudoku.\n";
	return s_data;
}

/// Convert sudoku to raw.
template<sudoku_size_t sq_h, sudoku_size_t sq_w, bool printDebugInfo = printDebugInfodefault>
raw_sudoku_t<sq_h, sq_w> get_raw_sudoku(const sudoku_data_t<sq_h, sq_w> & s_data) {
	using Dims = SudokuDims<sq_h, sq_w>;
	raw_sudoku_t<sq_h, sq_w> raw_s;
	for (sudoku_size_t ind = 0; ind < Dims::tot_num_cells; ++ind) {
		raw_s[ind] = s_data[ind * Dims::n_stored_per_cell];
	}
	if constexpr (printDebugInfo) std::cout << "Extracted Sudoku from Sudoku Data.\n";
	return raw_s;
}

/// Auto-fill sudoku.
template<sudoku_size_t sq_h, sudoku_size_t sq_w, bool printDebugInfo = printDebugInfodefault>
void auto_fill(sudoku_data_t<sq_h, sq_w> & s_data, const bool init = false) {
	using Dims = SudokuDims<sq_h, sq_w>;
	constexpr sudoku_size_t side_len = Dims::side_len;
	constexpr sudoku_size_t n_stored_per_cell = Dims::n_stored_per_cell;
	constexpr sudoku_size_t n_stored_per_side = Dims::n_stored_per_side;

	for (sudoku_size_t ind = 0; ind < Dims::tot_num_cells; ++ind) {
		const sudoku_size_t curr_ind = ind * n_stored_per_cell;
		const sudoku_size_t cell_col_ind = ind % side_len;
		const sudoku_size_t cell_row_ind = ind / side_len;

		const sudoku_size_t temp = s_data[curr_ind];
		if (temp == 0) {// Number not set
			if (init) {
				// Initialize as all possible
				for (sudoku_size_t i = 0; i < side_len; ++i) {
					s_data[curr_ind + i + 1] = 2;
				}
			}
			// Iterate over row and column
			for (sudoku_size_t i = 0; i < side_len; ++i) {
				const sudoku_size_t temp2 = s_data[cell_row_ind * n_stored_per_side + i * n_stored_per_cell];
				if (temp2) {//Number in same row set
					s_data[curr_ind + temp2] = 1;
				}
				const sudoku_size_t temp3 = s_data[i * n_stored_per_side + cell_col_ind * n_stored_per_cell];
				if (temp3) {//Number in same col set
					s_data[curr_ind + temp3] = 1;
				}
			}
			// Iterate over squares
			const sudoku_size_t square_begin = side_len * (cell_row_ind - (cell_row_ind % sq_h)) + (cell_col_ind - (cell_col_ind % sq_w));
			for (sudoku_size_t k = 0; k < sq_h; ++k) {
				for (sudoku_size_t i = 0; i < sq_w; ++i) {
					const sudoku_size_t temp4 = s_data[(square_begin + i + k * side_len) * n_stored_per_cell];
					if (temp4) {//Number in same col set
						s_data[curr_ind + temp4] = 1;
					}
				}
			}
		}
	}
	if constexpr (printDebugInfo) std::cout << "Sudoku initialized with autofill.\n";
}
