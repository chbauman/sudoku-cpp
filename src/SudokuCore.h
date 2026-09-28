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

/// The heigth of each square in the sudoku.
constexpr sudoku_size_t square_height = 3;

/// The width of each square in the sudoku.
constexpr sudoku_size_t square_width = 3;

constexpr bool printDebugInfodefault = false;

// Derived constants
constexpr sudoku_size_t side_len = square_height * square_width;
constexpr sudoku_size_t tot_num_cells = side_len * side_len;
constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
constexpr sudoku_size_t tot_storage = n_stored_per_cell * tot_num_cells;
constexpr sudoku_size_t n_stored_per_side = n_stored_per_cell * side_len;

static_assert(square_width > 0 && square_height > 0 && "Do you really want an empty fucking Sudoku?");

/// Sudoku data type for solving.
///
/// Meaning of entries: 2: possible, 1: not possible, 0: definite number set.
typedef std::array<sudoku_size_t, tot_storage> sudoku_data_t;

/// Sudoku input data type.
///
/// Usually of size 9 x 9, 0 denotes empty cell, other numbers denote
/// the set numbers.
typedef std::array<sudoku_size_t, tot_num_cells> raw_sudoku_t;

/// Random seed.
constexpr sudoku_size_t seed = 50;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Print The Sudoku to std::cout
inline void printLine(std::ostream & os, const sudoku_size_t square_height, const sudoku_size_t square_width) {
	os << "+-";
	for (sudoku_size_t square_col = 0; square_col < square_height; ++square_col) {
		for (sudoku_size_t cell_col = 0; cell_col < square_width; ++cell_col) {
			os << "--";
		}
		os << "+-";
	}
	os << "\n";
}

/// Prints a raw sudoku to \ref std::cout.
inline std::ostream& operator<<(std::ostream & os, const raw_sudoku_t & sud){
	printLine(os, square_height, square_width);
	for (sudoku_size_t square_row = 0; square_row < square_width; ++square_row) {
		for (sudoku_size_t cell_row = 0; cell_row < square_height; ++cell_row) {
			os << "| ";
			for (sudoku_size_t square_col = 0; square_col < square_height; ++square_col) {
				for (sudoku_size_t cell_col = 0; cell_col < square_width; ++cell_col) {
					sudoku_size_t col_ind = cell_col + square_width * square_col;
					sudoku_size_t row_ind = cell_row + square_height * square_row;
					sudoku_size_t arr_ind = col_ind + side_len * row_ind;
					os << sud[arr_ind] << " ";
				}
				os << "| ";
			}
			os << "\n";
		}
		printLine(os, square_height, square_width);
	}
	return os;
}

/// Prints a sudoku to \ref std::cout.
inline std::ostream& operator<<(std::ostream & os, const sudoku_data_t & sud){
	for (sudoku_size_t i = 0; i < tot_num_cells; ++i) {
		if (i % side_len == 0) os << "\n";
		for (sudoku_size_t k = 0; k < n_stored_per_cell; ++k) {
			os << sud[k + n_stored_per_cell * i] << " ";
		}
		os << "\n";
	}
	return os;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper
template<class Func>
void iterateDouble() {
	for (sudoku_size_t square_row = 0; square_row < square_width; ++square_row) {
		for (sudoku_size_t cell_row = 0; cell_row < square_height; ++cell_row) {
			for (sudoku_size_t square_col = 0; square_col < square_height; ++square_col) {
				for (sudoku_size_t cell_col = 0; cell_col < square_width; ++cell_col) {
					sudoku_size_t col_ind = cell_col + square_width * square_col;
					sudoku_size_t row_ind = cell_row + square_height * square_row;
					sudoku_size_t arr_ind = col_ind + side_len * row_ind;
				}
			}
		}
	}
}

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
template<bool printDebugInfo = printDebugInfodefault>
sudoku_data_t init_sudoku() {
	sudoku_data_t s_data;
	std::fill(std::begin(s_data), std::end(s_data), (sudoku_size_t)0);
	if constexpr (printDebugInfo) std::cout << "Filled Sudoku with Zeros.\n";
	return s_data;
}

/// Initializes sudoku with a raw sudoku.
template<bool printDebugInfo = printDebugInfodefault>
sudoku_data_t init_sudoku_with_raw(const raw_sudoku_t & raw_s) {
	sudoku_data_t s_data = init_sudoku();
	for (sudoku_size_t ind = 0; ind < tot_num_cells; ++ind) {
		const sudoku_size_t temp = raw_s[ind];
		if (temp) {
			s_data[ind * n_stored_per_cell] = temp;
		}
	}
	if constexpr (printDebugInfo) std::cout << "Constructed Sudoku Data with raw Sudoku.\n";
	return s_data;
}

/// Convert sudoku to raw.
template<bool printDebugInfo = printDebugInfodefault>
raw_sudoku_t get_raw_sudoku(const sudoku_data_t & s_data) {
	raw_sudoku_t raw_s;
	for (sudoku_size_t ind = 0; ind < tot_num_cells; ++ind) {
		raw_s[ind] = s_data[ind * n_stored_per_cell];
	}
	if constexpr (printDebugInfo) std::cout << "Extracted Sudoku from Sudoku Data.\n";
	return raw_s;
}

/// Auto-fill sudoku.
template<bool printDebugInfo = printDebugInfodefault>
void auto_fill(sudoku_data_t & s_data, const bool init = false) {
	for (sudoku_size_t ind = 0; ind < tot_num_cells; ++ind) {
		const sudoku_size_t curr_ind = ind * n_stored_per_cell;
		const sudoku_size_t cell_col_ind = ind % side_len;
		const sudoku_size_t cell_row_ind = ind / side_len;

		const sudoku_size_t col_ind = curr_ind % n_stored_per_side;
		const sudoku_size_t row_ind = curr_ind / n_stored_per_side;

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
			const sudoku_size_t square_begin = side_len * (cell_row_ind - (cell_row_ind % square_height)) + (cell_col_ind - (cell_col_ind % square_width));
			const sudoku_size_t square_row_ind = cell_row_ind / square_height;
			const sudoku_size_t square_col_ind = cell_col_ind / square_width;
			for (sudoku_size_t k = 0; k < square_height; ++k) {
				for (sudoku_size_t i = 0; i < square_width; ++i) {
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
