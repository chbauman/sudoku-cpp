#pragma once

#include <array>
#include <cassert>
#include <string>
#include <iostream>
#include <random>
#include <algorithm>
#include <map>
#include <fstream>

typedef int sudoku_size_t;
typedef int sudoku_value_t;

// Choose suitable
constexpr sudoku_size_t square_height = 3;
constexpr sudoku_size_t square_width = 3;

constexpr bool printDebugInfodefault = false;

constexpr sudoku_size_t side_len = square_height * square_width;
constexpr sudoku_size_t tot_num_cells = side_len * side_len;

static_assert(square_width > 0 && square_height > 0 && "Do you really want an empty fucking Sudoku?");

constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
constexpr sudoku_size_t tot_storage = n_stored_per_cell * tot_num_cells;
constexpr sudoku_size_t n_stored_per_side = n_stored_per_cell * side_len;

// Sudoku data type for solving, 2: possible, 1: not possible, 0: definite number set
typedef std::array<sudoku_size_t, tot_storage> sudoku_data_t;
// Sudoku data type as input, 9 x 9 usually
typedef std::array<sudoku_size_t, tot_num_cells> raw_sudoku_t;


// Random stuff
constexpr sudoku_size_t seed = 42;

// Returns a random permutation of size n
template<sudoku_size_t n, class rng>
std::array<sudoku_size_t, n> random_permutation(rng & g) {
	std::array<sudoku_size_t, n> cell_order;
	for (sudoku_size_t i = 0; i < n; ++i) {
		cell_order[i] = i;
	}
	std::shuffle(cell_order.begin(), cell_order.end(), g);
	return cell_order;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Random Number picking

// A pick is defined with the cell number and the number that was picked
typedef std::pair<sudoku_size_t, sudoku_size_t> random_pick_t;

// For printing to std::cout 
inline std::ostream& operator<<(std::ostream & os, const random_pick_t & p) {
	os << "Picked cell " << p.first << " and number " << p.second + 1 << "\n";
	return os;
}

// The class for picking 
template<sudoku_size_t square_height, sudoku_size_t square_width>
class RandomNumberPicker {
	
public:
	RandomNumberPicker(): cell_order(random_permutation<tot_num_cells>(gen)), number_order(random_permutation<side_len>(gen)){};

	// Picks a random cell and fills it with a random possible number
	random_pick_t pickAndSetRandom(sudoku_data_t & s_data){
		random_pick_t ret_val = std::make_pair(-1, -1);
		const sudoku_size_t cell_index_init = dis(gen);
		const sudoku_size_t num_ind_init = dis(gen);
		for(sudoku_size_t i = 0; i < tot_num_cells; ++i){
			const sudoku_size_t curr_pos = cell_order[(cell_index_init + i) % tot_num_cells];
			if (s_data[n_stored_per_cell * curr_pos] == 0) {
				ret_val.first = curr_pos;
				for (sudoku_size_t k = 0; k < side_len; ++k) {
					const sudoku_size_t curr_num = number_order[(num_ind_init + k) % side_len];
					if (s_data[n_stored_per_cell * curr_pos + 1 + curr_num] == 2) {
						ret_val.second = curr_num;
						s_data[n_stored_per_cell * curr_pos] = curr_num + 1;
						break;
					}
				}
				break;
			}
		}
		return ret_val;
	}

private:

	// Constants
	static constexpr sudoku_size_t side_len = square_height * square_width;
	static constexpr sudoku_size_t tot_num_cells = side_len * side_len;
	static constexpr sudoku_size_t n_stored_per_cell = side_len + 1;

	// Rng and int dist
	std::mt19937 gen = std::mt19937(seed);
	std::uniform_int_distribution<> dis = std::uniform_int_distribution<>(0, tot_num_cells - 1);

	// Permutation arrays
	const std::array<sudoku_size_t, tot_num_cells> cell_order;
	const std::array<sudoku_size_t, side_len> number_order;
};

// Eliminate the random pick as possibility
template<sudoku_size_t square_height, sudoku_size_t square_width>
void eliminate_random_pick(sudoku_data_t & s_data, const random_pick_t & rp) {
	constexpr sudoku_size_t side_len = square_height * square_width;
	constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
	s_data[rp.first * n_stored_per_cell + 1 + rp.second] = 1;
}


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

// Initialize an array with 0
template<sudoku_size_t n>
void setZero(std::array<sudoku_size_t, n> & arr){
	for (sudoku_size_t i = 0; i < n; ++i) {
		arr[i] = 0;
	}
};

// Sum all elements of an array
template<class value_t, sudoku_size_t n>
value_t sum(const std::array<value_t, n> & arr) {
	value_t sum_curr = (value_t)0;
	for (sudoku_size_t i = 0; i < n; ++i) {
		sum_curr += arr[i];
	}
	return sum_curr;
};

// Check if all elements of array are 1
template<class value_t, sudoku_size_t n>
bool check_all_1(const std::array<value_t, n> & arr) {
	for (sudoku_size_t i = 0; i < n; ++i) {
		if (arr[i] != (value_t)1) {
			return false;
		}
	}
	return true;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Sudoku and Convert
template<bool printDebugInfo = printDebugInfodefault>
sudoku_data_t init_sudoku() {
	sudoku_data_t s_data;
	std::fill(std::begin(s_data), std::end(s_data), (sudoku_size_t)0);
	if constexpr (printDebugInfo) std::cout << "Filled Sudoku with Zeros.\n";
	return s_data;
}

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

template<bool printDebugInfo = printDebugInfodefault>
raw_sudoku_t get_raw_sudoku(const sudoku_data_t & s_data) {
	raw_sudoku_t raw_s;
	for (sudoku_size_t ind = 0; ind < tot_num_cells; ++ind) {
		raw_s[ind] = s_data[ind * n_stored_per_cell];
	}
	if constexpr (printDebugInfo) std::cout << "Extracted Sudoku from Sudoku Data.\n";
	return raw_s;
}

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Solving Sudoku

// Status for intermediate solver
enum SolveStepRes {
	Invalid,
	ValidnNoChange,
	ValidNewFound
};

const std::string sol_step_msgs[] = { "Sudoku is invalid.", "Sudoku is valid, no new number found.", "Sudoku is valid, found new number." };

inline std::ostream& operator<<(std::ostream & os, const SolveStepRes & sol_step) {
	os << sol_step_msgs[sol_step] << "\n";
	return os;
}

// Status for final solver
enum SolveResultFinal {
	InvalidSolution,
	UniqueSolution,
	MultipleSolution,
	UnknownSolution
};

const std::string sol_res_fin_msgs[] = { "Sudoku is invalid, cannot be solved.", "Sudoku has a unique solution.", "Sudoku has multiple solutions.", "Solutions has not yet been found." };

inline std::ostream& operator<<(std::ostream & os, const SolveResultFinal & sol_step) {
	os << sol_res_fin_msgs[sol_step] << "\n";
	return os;
}

// Look for numbers that can only be placed in one cell in a given row/col
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes find_unique_in_rcs(sudoku_data_t & s_data) {
	
	bool found_number = false;

	// Iterate over all rows / cols / squares
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {
		
		const sudoku_size_t curr_row_start = row_num * side_len;
		const sudoku_size_t curr_col_start = row_num;

		// Iterate over numbers
		for (sudoku_size_t number = 0; number < side_len; ++number) {
		
			// Test if number not set already somewhere
			sudoku_size_t num_times_set_row = 0;
			sudoku_size_t num_times_set_col = 0;
			for (sudoku_size_t cell_ind = 0; cell_ind < side_len; ++cell_ind) {
				const sudoku_size_t curr_cell_ind_row = curr_row_start + cell_ind;
				const sudoku_size_t curr_cell_ind_col = curr_col_start + cell_ind * side_len;
				if (s_data[curr_cell_ind_row * n_stored_per_cell] == number + 1) {
					++num_times_set_row;
				}
				if (s_data[curr_cell_ind_col * n_stored_per_cell] == number + 1) {
					++num_times_set_col;
				}
			}
			if (num_times_set_row > 1 || num_times_set_col > 1) {
				if constexpr (printDebugInfo) {
					if (num_times_set_row > 1) {
						std::cout << "Number "
							<< number + 1 << " in row " << row_num + 1
							<< " set " << num_times_set_row << " times.\n";
					}
					if (num_times_set_col > 1) {
						std::cout << "Number "
							<< number + 1 << " in col " << row_num + 1
							<< " set " << num_times_set_col << " times.\n";
					}					
					
				}
				return Invalid;
			}
			else if (num_times_set_row == 1 && num_times_set_col == 1) {
				continue;
			}

			sudoku_size_t num_poss_places_row = 0;
			sudoku_size_t cell_poss_num_row = 0;
			sudoku_size_t num_poss_places_col = 0;
			sudoku_size_t cell_poss_num_col = 0;

			// Iterate over all cells in row / col / square
			for (sudoku_size_t cell_num = 0; cell_num < side_len; ++cell_num) {
				
				// Count possibilities of number

				// Row Stuff
				const sudoku_size_t curr_row_cell_ind = curr_row_start + cell_num;
				if (s_data[curr_row_cell_ind * n_stored_per_cell] == 0 && s_data[curr_row_cell_ind * n_stored_per_cell + number + 1] == 2) {
					++num_poss_places_row;
					cell_poss_num_row = cell_num;
				}
				// Col Stuff
				const sudoku_size_t curr_col_cell_ind = curr_col_start + cell_num * side_len;
				if (s_data[curr_col_cell_ind * n_stored_per_cell] == 0 && s_data[curr_col_cell_ind * n_stored_per_cell + number + 1] == 2) {
					++num_poss_places_col;
					cell_poss_num_col = cell_num;
				}
			}

			if (num_poss_places_row == 1 && num_times_set_row == 0) {
				s_data[(curr_row_start + cell_poss_num_row) * n_stored_per_cell] = number + 1;
				found_number = true;
				if constexpr (printDebugInfo) {
					std::cout << "Found a number "
						<< number + 1 << " in row " << row_num + 1 << " at index " 
						<< cell_poss_num_row  << ".\n";
				}
			}

			if (num_poss_places_col == 1 && num_times_set_col == 0) {
				s_data[(curr_col_start + cell_poss_num_col * side_len) * n_stored_per_cell] = number + 1;
				found_number = true;
				if constexpr (printDebugInfo) {
					std::cout << "Found a number "
						<< number + 1 << " in col " << row_num + 1 << " at index "
						<< cell_poss_num_col << ".\n";
				}
			}
			if ((num_poss_places_row == 0 && num_times_set_row == 0) || (num_poss_places_col == 0 && num_times_set_col == 0)) {
				if constexpr(printDebugInfo) {
					std::cout << "No possibility to put "
						<< number + 1 << " in " << (num_poss_places_row == 0?"row ":"col ") << row_num + 1 << ".\n";
				}
				return Invalid;
			}
		}
	}
	
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for numbers with unique place.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Look for numbers that can only be placed in one cell in a given square
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes find_unique_in_square(sudoku_data_t & s_data) {

	bool found_number = false;

	// Iterate over all rows 
	for (sudoku_size_t square_id = 0; square_id < side_len; ++square_id) {

		const sudoku_size_t square_col_ind = square_id % square_height;
		const sudoku_size_t square_row_ind = square_id / square_height;
		const sudoku_size_t square_beg_ind = square_col_ind * square_width + square_row_ind * side_len * square_height;
		// Iterate over numbers
		for (sudoku_size_t number = 0; number < side_len; ++number) {

			sudoku_size_t num_times_set = 0;

			// Iterate over all rows 
			for (sudoku_size_t cell_id = 0; cell_id < side_len; ++cell_id) {

				const sudoku_size_t square_col_ind_inner = cell_id % square_height;
				const sudoku_size_t square_row_ind_inner = cell_id / square_height;
				const sudoku_size_t cell_ind = square_beg_ind + square_col_ind_inner + square_row_ind_inner * side_len;
				if (s_data[cell_ind * n_stored_per_cell] == number + 1) {
					++num_times_set;
				}
			}

			// Check if it is set multiple times or once
			if (num_times_set > 1) {
				if constexpr (printDebugInfo) {
					std::cout << "Number "
							<< number + 1 << " in square " << square_id
							<< " set " << num_times_set << " times.\n";
				}
				return Invalid;
			}
			else if (num_times_set == 1) {
				continue;
			}

			// Count possibilities of number
			sudoku_size_t num_poss_places = 0;
			sudoku_size_t cell_poss_num = 0;
			for (sudoku_size_t cell_id = 0; cell_id < side_len; ++cell_id) {
				const sudoku_size_t square_col_ind_inner = cell_id % square_height;
				const sudoku_size_t square_row_ind_inner = cell_id / square_height;
				const sudoku_size_t cell_ind = square_beg_ind + square_col_ind_inner + square_row_ind_inner * side_len;
				if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + number + 1] == 2) {
					++num_poss_places;
					cell_poss_num = cell_id;
				}
			}

			// If it can only be in one place
			if (num_poss_places == 1) {
				const sudoku_size_t square_col_ind_inner = cell_poss_num % square_height;
				const sudoku_size_t square_row_ind_inner = cell_poss_num / square_height;
				const sudoku_size_t cell_ind = square_beg_ind + square_col_ind_inner + square_row_ind_inner * side_len;
				s_data[cell_ind * n_stored_per_cell] = number + 1;
				found_number = true;
				if constexpr (printDebugInfo) {
					std::cout << "Found a number "
						<< number + 1 << " in square " << square_id << " at cell "
						<< cell_poss_num << ".\n";
				}
			}

			// If it can't be set anywhere
			if (num_poss_places == 0) {
				if constexpr (printDebugInfo) {
					std::cout << "No possibility to put "
						<< number + 1 << " in square " << square_id << ".\n";
				}
				return Invalid;
			}
		}
	}
		
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for numbers with unique place in square.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Look for cells where only one number can be
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes find_single_number_cell(sudoku_data_t & s_data) {

	bool found_number = false;

	// Iterate over all rows 
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

		// Iterate over all cols 
		for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {

			const sudoku_size_t cell_ind = row_num + col_num * side_len;
			const sudoku_size_t data_ind = cell_ind * n_stored_per_cell;

			if (s_data[data_ind] > 0) continue;
			// Iterate over all numbers
			sudoku_size_t last_possible_num = -1;
			sudoku_size_t num_possible_num = 0;
			for (sudoku_size_t num = 0; num < side_len; ++num) {

				const sudoku_size_t curr_state = s_data[data_ind + 1 + num];
				if (curr_state == 0) {
					std::cout << "ERROR: Sudoku not filled yet.\n";
					return Invalid;
				}
				else if (curr_state == 2) {
					num_possible_num++;
					last_possible_num = num;
				}
			}

			if (num_possible_num == 1) {
				s_data[data_ind] = 1 + last_possible_num;
				if constexpr (printDebugInfo) std::cout << "Found new number!\n";
				found_number = true;
			}
			else if (num_possible_num == 1) {
				return Invalid;
			}

		}
	}
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for cells with unique numbers.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Look for possible numbers that can be eliminated in all rows
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes eliminate_possible_numbers_row(sudoku_data_t & s_data) {

	bool found_number = false;
	std::array<sudoku_size_t, square_height> occurrance_arr;

	// Iterate over all rows 
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

		// Check if a number in this row can only occur in a particular square

		// Iterate over all numbers 
		for (sudoku_size_t num = 0; num < side_len; ++num) {

			// Check if number already set somewhere
			bool already_set = false;
			for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {
				const sudoku_size_t cell_ind = row_num * side_len + col_num;
				if (s_data[cell_ind * n_stored_per_cell] == num + 1) {
					already_set = true;
				}
			}
			if (already_set == true) continue;

			setZero(occurrance_arr);
			// Iterate over parts of row
			for (sudoku_size_t square_col_num = 0; square_col_num < square_height; ++square_col_num) {

				const sudoku_size_t first_cell_index = row_num * side_len + square_col_num * square_width;

				// Iterate over cells in row in square
				for (sudoku_size_t square_num = 0; square_num < square_width; ++square_num) {

					const sudoku_size_t cell_ind = first_cell_index + square_num;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						occurrance_arr[square_col_num] = 1;
						break;
					}
				}
			}

			// Find how many times it occurred and where
			sudoku_size_t sum_occur = 0;
			sudoku_size_t pos_occur = 0;
			for (sudoku_size_t i = 0; i < square_height; ++i) {
				if (occurrance_arr[i] == 1) {
					sum_occur++;
					pos_occur = i;
				}
			}

			if (sum_occur > 1) {
				continue; 
			}
			else if (sum_occur == 0) {
				if constexpr (printDebugInfo) std::cout << "No possibility!\n";
				return Invalid;
			}

			// Iterate over square
			const sudoku_size_t square_row_ind = row_num / square_height;
			const sudoku_size_t square_col_ind = pos_occur;
			const sudoku_size_t first_cell_index = square_col_ind * square_width + square_row_ind * square_height * side_len;

			for (sudoku_size_t square_row_num = 0; square_row_num < square_height; ++square_row_num) {

				// Ignore overlap
				if (square_row_num == (row_num % square_height)) continue;

				const sudoku_size_t curr_row_cell_index = first_cell_index + square_row_num * side_len;

				// Iterate over cells in row in square
				for (sudoku_size_t square_num = 0; square_num < square_width; ++square_num) {

					const sudoku_size_t cell_ind = curr_row_cell_index + square_num;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						s_data[cell_ind * n_stored_per_cell + 1 + num] = 1;
						found_number = true;
						if constexpr (printDebugInfo) std::cout << "Eliminated possible number " << 1 + num << "!\n";
					}
				}
			}
		}
	}
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for possibility elimination.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Look for possible numbers that can be eliminated in all cols
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes eliminate_possible_numbers_col(sudoku_data_t & s_data) {

	bool found_number = false;
	std::array<sudoku_size_t, square_width> occurrance_arr;

	// Iterate over all rows 
	for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {

		// Check if a number in this row can only occur in a particular square

		// Iterate over all numbers 
		for (sudoku_size_t num = 0; num < side_len; ++num) {

			// Check if number already set somewhere
			bool already_set = false;
			for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {
				const sudoku_size_t cell_ind = row_num * side_len + col_num;
				if (s_data[cell_ind * n_stored_per_cell] == num + 1) {
					already_set = true;
				}
			}
			if (already_set == true) continue;

			setZero(occurrance_arr);
			// Iterate over parts of col
			for (sudoku_size_t square_row_num = 0; square_row_num < square_width; ++square_row_num) {

				const sudoku_size_t first_cell_index = square_row_num * side_len * square_height  + col_num;

				// Iterate over cells in col in square
				for (sudoku_size_t square_num = 0; square_num < square_height; ++square_num) {

					const sudoku_size_t cell_ind = first_cell_index + square_num * side_len;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						occurrance_arr[square_row_num] = 1;
						break;
					}
				}
			}

			// Find how many times it occurred and where
			sudoku_size_t sum_occur = 0;
			sudoku_size_t pos_occur = 0;
			for (sudoku_size_t i = 0; i < square_width; ++i) {
				if (occurrance_arr[i] == 1) {
					sum_occur++;
					pos_occur = i;
				}
			}

			if (sum_occur > 1) {
				continue;
			}
			else if (sum_occur == 0) {
				if constexpr (printDebugInfo) std::cout << "No possibility!\n";
				return Invalid;
			}

			// Iterate over square
			const sudoku_size_t square_row_ind = pos_occur;
			const sudoku_size_t square_col_ind = col_num / square_width;
			const sudoku_size_t first_cell_index = square_col_ind * square_width + square_row_ind * square_height * side_len;

			for (sudoku_size_t square_col_num = 0; square_col_num < square_height; ++square_col_num) {

				// Ignore overlap
				if (square_col_num == (col_num % square_width)) continue;

				const sudoku_size_t curr_row_cell_index = first_cell_index + square_col_num;

				// Iterate over cells in row in square
				for (sudoku_size_t square_num = 0; square_num < square_width; ++square_num) {

					const sudoku_size_t cell_ind = curr_row_cell_index + square_num * side_len;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						s_data[cell_ind * n_stored_per_cell + 1 + num] = 1;
						found_number = true;
						if constexpr (printDebugInfo) std::cout << "Eliminated possible number " << 1 + num << "!\n";
					}
				}
			}
		}
	}
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for possibility elimination.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Look for possible numbers that can be eliminated in all squares
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes eliminate_possible_numbers_square(sudoku_data_t & s_data) {

	bool found_number = false;
	std::array<sudoku_size_t, square_height> occurrance_arr_h;
	std::array<sudoku_size_t, square_width> occurrance_arr_w;

	// Iterate over all rows 
	for (sudoku_size_t square_id = 0; square_id < side_len; ++square_id) {

		const sudoku_size_t square_col_ind = square_id % square_height;
		const sudoku_size_t square_row_ind = square_id / square_height;
		const sudoku_size_t square_beg_ind = square_col_ind * square_width + square_row_ind * side_len * square_height;

		// Check if a number in this square can only occur in a particular row / col

		// Iterate over all numbers 
		for (sudoku_size_t num = 0; num < side_len; ++num) {

			// Check if number already set somewhere
			bool already_set = false;
			for (sudoku_size_t square_col = 0; square_col < square_width; ++square_col) {
				for (sudoku_size_t square_row = 0; square_row < square_width; ++square_row) {
					const sudoku_size_t cell_ind = square_beg_ind + square_col + square_row * side_len;
					if (s_data[cell_ind * n_stored_per_cell] == num + 1) {
						already_set = true;
					}
				}
			}
			if (already_set == true) continue;

			setZero(occurrance_arr_h);
			setZero(occurrance_arr_w);

			// Iterate over parts of square
			for (sudoku_size_t square_row = 0; square_row < square_height; ++square_row) {

				const sudoku_size_t first_cell_index = square_beg_ind + square_row * side_len;

				// Iterate over cells in row in square
				for (sudoku_size_t square_col = 0; square_col < square_width; ++square_col) {

					const sudoku_size_t cell_ind = first_cell_index + square_col;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						occurrance_arr_h[square_row] = 1;
						break;
					}
				}
			}

			// Iterate over parts of square
			for (sudoku_size_t square_row = 0; square_row < square_width; ++square_row) {

				const sudoku_size_t first_cell_index = square_beg_ind + square_row;

				// Iterate over cells in row in square
				for (sudoku_size_t square_col = 0; square_col < square_height; ++square_col) {

					const sudoku_size_t cell_ind = first_cell_index + square_col * side_len;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						occurrance_arr_w[square_row] = 1;
						break;
					}
				}
			}

			// Find how many times it occurred and where
			sudoku_size_t sum_occur_w = 0;
			sudoku_size_t pos_occur_w = 0;
			sudoku_size_t sum_occur_h = 0;
			sudoku_size_t pos_occur_h = 0;
			for (sudoku_size_t i = 0; i < square_width; ++i) {
				if (occurrance_arr_w[i] == 1) {
					sum_occur_w++;
					pos_occur_w = i;
				}
			}
			for (sudoku_size_t i = 0; i < square_height; ++i) {
				if (occurrance_arr_h[i] == 1) {
					sum_occur_h++;
					pos_occur_h = i;
				}
			}

			if (sum_occur_h > 1 && sum_occur_w > 1) {
				continue;
			}
			else if (sum_occur_h == 0 || sum_occur_w == 0) {
				if constexpr (printDebugInfo) std::cout << "No possibility!\n";
				return Invalid;
			}

			if (sum_occur_h == 1) {
				// Iterate over row
				const sudoku_size_t row_ind = square_row_ind * square_height + pos_occur_h;
				const sudoku_size_t first_cell_index = row_ind * side_len;

				for (sudoku_size_t col_ind = 0; col_ind < side_len; ++col_ind) {

					// Ignore overlap
					if (col_ind / square_width == square_col_ind) continue;

					const sudoku_size_t cell_ind = first_cell_index + col_ind;

					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						s_data[cell_ind * n_stored_per_cell + 1 + num] = 1;
						found_number = true;
						if constexpr (printDebugInfo) std::cout << "Eliminated possible number " << 1 + num << "!\n";
					}
				}
			}
			if (sum_occur_w == 1) {
				// Iterate over col
				const sudoku_size_t col_ind = square_col_ind * square_width + pos_occur_w;
				const sudoku_size_t first_cell_index = col_ind;

				for (sudoku_size_t row_ind = 0; row_ind < side_len; ++row_ind) {

					// Ignore overlap
					if (row_ind / square_height == square_row_ind) continue;

					const sudoku_size_t cell_ind = first_cell_index + row_ind * side_len;

					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + 1 + num] == 2) {
						s_data[cell_ind * n_stored_per_cell + 1 + num] = 1;
						found_number = true;
						if constexpr (printDebugInfo) std::cout << "Eliminated possible number " << 1 + num << "!\n";
					}
				}
			}			
		}
	}
	if constexpr (printDebugInfo) std::cout << "Sudoku checked for possibility elimination.\n";
	if (found_number) {
		return ValidNewFound;
	}
	else {
		return ValidnNoChange;
	}
}

// Updating status uf solving process
SolveStepRes update(SolveStepRes old_step, SolveStepRes new_step) {
	if (old_step == Invalid || new_step == Invalid) {
		return Invalid;
	}
	else if(new_step == ValidNewFound){
		return ValidNewFound;
	}
	return old_step;
}

// Try to solve the sudoku using the previously defined functions
template<bool printDebugInfo = printDebugInfodefault>
SolveStepRes try_solving(sudoku_data_t & s_data) {

	SolveStepRes found_something = ValidNewFound;

	while (found_something == ValidNewFound) {
		found_something = ValidnNoChange;
		found_something = update(found_something, find_unique_in_rcs<printDebugInfo>(s_data));
		found_something = update(found_something, find_unique_in_square<printDebugInfo>(s_data));
		found_something = update(found_something, find_single_number_cell<printDebugInfo>(s_data));
		found_something = update(found_something, eliminate_possible_numbers_row<printDebugInfo>(s_data));
		found_something = update(found_something, eliminate_possible_numbers_col<printDebugInfo>(s_data));
		found_something = update(found_something, eliminate_possible_numbers_square<printDebugInfo>(s_data));
		
		auto_fill<printDebugInfo>(s_data, false);
	}
	return found_something;
}

// Check if sudoku is solved, raises an exception if it is invalid
template<sudoku_size_t square_height, sudoku_size_t square_width>
bool solved(sudoku_data_t & s_data) {
	constexpr sudoku_size_t side_len = square_height * square_width;
	constexpr sudoku_size_t tot_n_cells = side_len * side_len;
	constexpr sudoku_size_t n_stored_per_cell = side_len + 1;

	std::array<sudoku_size_t, side_len> col_arr;
	std::array<sudoku_size_t, side_len> row_arr;

	// Check if there is a number set in every cell
	for (sudoku_size_t cell_ind = 0; cell_ind < tot_n_cells; ++cell_ind) {
		if (s_data[cell_ind * n_stored_per_cell] == 0) {
			return false;
		}
	}
	return true;

	//for (sudoku_size_t row_ind = 0; row_ind < side_len; ++row_ind) {
	//	for (sudoku_size_t num = 0; num < side_len; ++num) {

	//		setZero(col_arr);
	//		setZero(row_arr);

	//		for (sudoku_size_t cell_ind = 0; cell_ind < side_len; ++cell_ind) {

	//			const sudoku_size_t cell_ind_row = row_ind * side_len + cell_ind;
	//			const sudoku_size_t cell_ind_col = row_ind + cell_ind * side_len;

	//			col_num = s_data[cell_ind_col * n_stored_per_cell];
	//			row_num = s_data[cell_ind_row * n_stored_per_cell];

	//			if (col_num == 0 || row_num == 0) {
	//				return false;
	//			}

	//		}
	//	}
	//}
}

constexpr bool printRecDebInfo = false;

// Find the cell with the least numbers possible
template<bool printDebugInfo = printDebugInfodefault>
sudoku_size_t find_least_uncertain_cell(sudoku_data_t & s_data) {

	sudoku_size_t min_poss_nums = side_len;
	sudoku_size_t min_data_ind = 0;

	// Iterate over all rows 
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

		// Iterate over all cols 
		for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {

			const sudoku_size_t cell_ind = row_num + col_num * side_len;
			const sudoku_size_t data_ind = cell_ind * n_stored_per_cell;

			if (s_data[data_ind] > 0) continue;
			// Iterate over all numbers
			sudoku_size_t num_possible_num = 0;
			for (sudoku_size_t num = 0; num < side_len; ++num) {

				const sudoku_size_t curr_state = s_data[data_ind + 1 + num];
				if (curr_state == 2) {
					num_possible_num++;
				}
			}
			// Found new cell with fewer possibilities
			if (min_poss_nums > num_possible_num) {
				min_poss_nums = num_possible_num;
				min_data_ind = data_ind;
			}
		}
	}
	return min_data_ind;
}


// Find a solution and check if it is unique
template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo>
SolveResultFinal solve_brute_force_multiple(sudoku_data_t & s_data) {

	// Try solving 
	SolveStepRes init_stat = try_solving(s_data);
	if (init_stat == Invalid) {
		return InvalidSolution;
	}
	else if (solved<square_height, square_width>(s_data)) {
		return UniqueSolution;
	}

	// Solve by guessing recursively
	sudoku_data_t s_data_copy = s_data;
	sudoku_data_t s_data_res = s_data;
	const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);
	SolveResultFinal res = UnknownSolution;
	sudoku_size_t num_sols = 0;

	// Loop over all possible guesses
	for (sudoku_size_t i = 0; i < side_len; ++i) {

		const sudoku_size_t curr_i = i;

		if (s_data[cell_picked + 1 + curr_i] == 2) {
			// Copy data and set guessed value
			s_data_copy = s_data;
			s_data_copy[cell_picked] = curr_i + 1;

			// Recursion
			res = solve_brute_force_multiple<square_height, square_width>(s_data_copy);
			if (res == UniqueSolution) {
				s_data_res = s_data_copy;
				num_sols += 1;
			}
			if (num_sols > 1 || res == MultipleSolution) {
				s_data = s_data_copy;
				return MultipleSolution;
			}
		}
	}
	s_data = s_data_res;
	if (num_sols == 1) {
		return UniqueSolution;
	}
	if (num_sols == 0) {
		return InvalidSolution;
	}
	if (num_sols > 1) {
		return MultipleSolution;
	}
	
	// Should not happen
	return UnknownSolution;
}


// Find a solution and check if it is unique
template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo, typename RNG>
SolveResultFinal solve_brute_force_multiple_random(sudoku_data_t & s_data, RNG & rng) {

	// Try solving 
	SolveStepRes init_stat = try_solving(s_data);
	if (init_stat == Invalid) {
		return InvalidSolution;
	}
	else if (solved<square_height, square_width>(s_data)) {
		return UniqueSolution;
	}

	// Solve by guessing recursively
	sudoku_data_t s_data_copy = s_data;
	sudoku_data_t s_data_res = s_data;
	const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);
	SolveResultFinal res = UnknownSolution;
	sudoku_size_t num_sols = 0;

	// Random Order
	std::array<sudoku_value_t, side_len> perm;
	for (sudoku_size_t i = 0; i < side_len; ++i) {
		perm[i] = i;
	}
	std::shuffle(perm.begin(), perm.end(), rng);

	// Loop over all possible guesses
	for (sudoku_size_t i = 0; i < side_len; ++i) {

		const sudoku_size_t curr_i = perm[i];

		if (s_data[cell_picked + 1 + curr_i] == 2) {
			// Copy data and set guessed value
			s_data_copy = s_data;
			s_data_copy[cell_picked] = curr_i + 1;

			// Recursion
			res = solve_brute_force_multiple_random<square_height, square_width>(s_data_copy, rng);
			if (res == UniqueSolution) {
				s_data_res = s_data_copy;
				num_sols += 1;
			}
			if (num_sols > 1 || res == MultipleSolution) {
				s_data = s_data_copy;
				return MultipleSolution;
			}
		}
	}
	s_data = s_data_res;
	if (num_sols == 1) {
		return UniqueSolution;
	}
	if (num_sols == 0) {
		return InvalidSolution;
	}
	if (num_sols > 1) {
		return MultipleSolution;
	}

	// Should not happen
	return UnknownSolution;
}

// Count all solutions and check if it is unique
template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo>
int solve_brute_force_all(sudoku_data_t & s_data) {

	// Try solving 
	SolveStepRes init_stat = try_solving(s_data);
	if (init_stat == Invalid) {
		return 0;
	}
	else if (solved<square_height, square_width>(s_data)) {
		return 1;
	}

	// Solve by guessing recursively
	sudoku_data_t s_data_copy = s_data;
	sudoku_data_t s_data_res = s_data;
	const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);
	sudoku_size_t num_sols = 0;

	// Loop over all possible guesses
	for (sudoku_size_t i = 0; i < side_len; ++i) {

		if (s_data[cell_picked + 1 + i] == 2) {
			// Copy data and set guessed value
			s_data_copy = s_data;
			s_data_copy[cell_picked] = i + 1;

			// Recursion
			const int res = solve_brute_force_all<square_height, square_width>(s_data_copy);
			num_sols += res;
			if (res > 0) {
				s_data_res = s_data_copy;
			}
		}
	}
	s_data = s_data_res;
	return num_sols;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sudoku Generation

// Type Definition
typedef int num_sud_t;
typedef int rec_depth_t;
typedef unsigned int num_filled_t;
typedef std::string sud_char_t;
typedef std::pair<raw_sudoku_t, raw_sudoku_t> sud_and_sol_t;
typedef std::map<sud_char_t, std::vector<sud_and_sol_t> > sud_coll_t;

// -3: Error occurred
// -2: Invalid
// -1: Multiple
// 0: Unique, no recursion needed
// n > 0: Unique, min. rec. depth n

// Find a solution and check if it is unique
// Additionally find recursion depth
template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo>
rec_depth_t solve_count_rec_depth(sudoku_data_t & s_data, const rec_depth_t rec_dep = 0) {

	// Try solving 
	SolveStepRes init_stat = try_solving(s_data);
	if (init_stat == Invalid) {
		return -2;
	}
	else if (solved<square_height, square_width>(s_data)) {
		return rec_dep;
	}

	// Solve by guessing recursively
	sudoku_data_t s_data_copy = s_data;
	sudoku_data_t s_data_res = s_data;
	const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);
	rec_depth_t res = -3;
	sudoku_size_t num_sols = 0;

	rec_depth_t curr_min_rd = -3;

	// Loop over all possible guesses
	for (sudoku_size_t i = 0; i < side_len; ++i) {

		if (s_data[cell_picked + 1 + i] == 2) {
			// Copy data and set guessed value
			s_data_copy = s_data;
			s_data_copy[cell_picked] = i + 1;

			// Recursion
			res = solve_count_rec_depth<square_height, square_width>(s_data_copy, rec_dep + 1);
			if (res >= 0) {
				s_data_res = s_data_copy;
				num_sols += 1;
				if (curr_min_rd == -3 || res < curr_min_rd) {
					curr_min_rd = res;
				}
			}
			if (num_sols > 1 || res == -1) {
				s_data = s_data_copy;
				return -1;
			}
		}
	}
	s_data = s_data_res;
	if (num_sols == 1) {
		return curr_min_rd;
	}
	if (num_sols == 0) {
		return -2;
	}
	if (num_sols > 1) {
		return -1;
	}

	// Should not happen
	return -3;
}

// Counts the number that is currently set in the given sudoku
sudoku_size_t count_num_known_numbers(const sudoku_data_t & s_data) {

	sudoku_size_t num_ct = 0;

	// Iterate over all rows 
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

		// Check if number already set somewhere
		for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {
			const sudoku_size_t cell_ind = row_num * side_len + col_num;
			if (s_data[cell_ind * n_stored_per_cell] > 0) {
				++num_ct;
			}
		}
	}

	return num_ct;
}

// Removes the nth number that is currently set in the given sudoku
void remove_nth(sudoku_data_t & s_data, const sudoku_size_t n) {

	sudoku_size_t num_ct = 0;

	// Iterate over all rows 
	for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

		// Check if number already set somewhere
		for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {
			const sudoku_size_t cell_ind = row_num * side_len + col_num;
			if (s_data[cell_ind * n_stored_per_cell] > 0) {
				if (num_ct == n) {
					s_data[cell_ind * n_stored_per_cell] = 0;
					return;
				}
				++num_ct;
			}
		}
	}
	std::cout << "Fucking index too high :(\n";
}

// Generates a string that describes the sudoku.
// First number:			Difficulty level {0, ... , 9}
// First 2 numbers:			# Filled-in digits 
// Next 9 numbers:			# Decreasing frequency count of numbers from 1 to 9
// E.g. "133664433322" for a sudoku containing 33 filled-in digits
// and difficulty level 1.
sud_char_t generate_sud_char(const raw_sudoku_t & s, const rec_depth_t lvl) {

	// Initialize
	std::array<sudoku_value_t, side_len> freq;
	std::fill(freq.begin(), freq.end(), 0);
	sudoku_size_t tot_n_digits = 0;

	// Loop over sudoku
	for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
		const sudoku_value_t el = s[i];
		if (el > 0) {
			tot_n_digits++;
			freq[el - 1]++;
		}
	}

	// Sort Frequencies
	std::sort(freq.begin(), freq.end(), std::greater<sudoku_value_t>());

	// Construct String Output
	sud_char_t res = std::to_string(tot_n_digits);
	if (tot_n_digits < 10) {
		res = "0" + res;
	}
	res = std::to_string(lvl) + res;
	for (auto& e : freq) {
		res = res + std::to_string(e);
	}
	return res;
}

// Adds the sudoku 's' and its solution 's_sol' in raw form to the collection
// 'sud_map' if there are less than 'max_sud_per_key' sudokus already there.
bool add_to_coll(sud_coll_t & sud_map, const sud_char_t & desc, const raw_sudoku_t s, const raw_sudoku_t s_sol, const num_sud_t max_sud_per_key = 100) {

	const sud_and_sol_t s_and_sol = std::make_pair(s, s_sol);
	const auto pos = sud_map.find(desc);
	if (pos == sud_map.end()) {
		std::vector<sud_and_sol_t> val;
		val.push_back(s_and_sol);
		sud_map[desc] = val;
		return true;
	}
	else {
		std::vector<sud_and_sol_t> & val = sud_map[desc];
		if (val.size() > max_sud_per_key) {
			return false;
		}
		else {
			sud_map[desc].push_back(s_and_sol);
			return true;
		}
	}
	return false;
}

// Converts a sudoku to a string
std::string sud_to_string(const raw_sudoku_t & s) {
	std::string res = "";
	for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
		res += std::to_string(s[i]) + " ";
	}
	return res;
}

// Convert String to Sudoku
raw_sudoku_t string_to_sud(const std::string & str) {	
	raw_sudoku_t rs;
	for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
		const std::string num_str = str.substr(2 * i, 2 * i + 1);
		rs[i] = std::stoi(num_str);
	}
	return rs;
}

std::string file_path = "./Data/dat33.txt";

// Save the collection in text format
void save_coll(const sud_coll_t & sud_map, std::string folder_path = file_path) {

	std::ofstream myfile;
	myfile.open(folder_path);
	for (auto& x : sud_map)
	{
		const sud_char_t & desc = x.first;
		const std::vector<sud_and_sol_t> sas = x.second;
		for (auto& e : sas) {
			const auto&[s, sol] = e;
			myfile << desc << " " << sud_to_string(s) << sud_to_string(sol) << "\n";
		}
	}
	myfile.close();
}

// Checks if file 'name' exists
inline bool f_exists(const std::string & f_name) {
	std::ifstream f(f_name.c_str());
	return f.good();
}

#include<sstream>

// Load the sudokus saved on disk into collection
sud_coll_t load_coll(std::string folder_path = file_path) {
	sud_coll_t sud_map;

	// Return empty map if file does not exist
	if (!f_exists(folder_path)) {
		std::cout << "Creating new file\n";
		return sud_map;
	}

	// Read file linewise and extract info
	std::ifstream file(folder_path);

	//if (file) {
	//	std::string str;
	//	std::ostringstream ss;
	//	ss << file.rdbuf(); // reading data
	//	str = ss.str();
	//	std::cout << str << "\n";
	//	std::cout << "hoiiii\n";
	//}

	std::string str;
	while (std::getline(file, str)) {
		std::cout << "hoiiii\n";
		std::cout << str << "\n";
		const std::string desc = str.substr(0, 12);
		const sudoku_size_t s_end = 13 + 2 * side_len * side_len;
		const std::string s_str = str.substr(13, s_end);
		const std::string s_sol_str = str.substr(s_end, s_end + 2 * side_len * side_len);
		const raw_sudoku_t s = string_to_sud(s_str);
		const raw_sudoku_t s_sol = string_to_sud(s_sol_str);
		const bool tr = add_to_coll(sud_map, desc, s, s_sol);
		if (tr == false) {
			std::cout << "Fucking Error Ocurred!!!!!!!!!\n\n\n\n";
		}
	}
	return sud_map;
}

// Generate Sudokus and save them to the disk
void generate_hard_sudokus(const num_sud_t max_suds_per_lvl = 1000) {

	// Initialize
	std::array<sudoku_value_t, side_len> lvl_count;
	std::fill(lvl_count.begin(), lvl_count.end(), 0);
	sud_coll_t sud_map = load_coll();
	std::mt19937 gen = std::mt19937(seed);

	for (int k = 0; k < 200; ++k) {

		// Generate full sudoku
		const raw_sudoku_t zero_sudoku_3x3 = {
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0
		};
		sudoku_data_t sudoku = init_sudoku_with_raw(zero_sudoku_3x3);
		auto_fill(sudoku, true);
		raw_sudoku_t raw_sud = get_raw_sudoku(sudoku);
		solve_brute_force_multiple_random<3, 3>(sudoku, gen);
		const raw_sudoku_t raw_s_sol = get_raw_sudoku(sudoku);
		sudoku_data_t sudoku_solution_copy = sudoku;

		for (int l = 0; l < 10; ++l) {
			sudoku = sudoku_solution_copy;

			// Remove digits randomly
			const sudoku_size_t n_init = 35;
			for (sudoku_size_t i = 0; i < n_init; ++i) {
				sudoku_size_t remove_ind = std::rand() % (tot_num_cells - i);
				remove_nth(sudoku, remove_ind);
			}
			auto_fill(sudoku, true);
			sudoku_data_t sudoku_copy = sudoku;

			// Remove more, untill multiple solutions possible
			sudoku_size_t n_curr = n_init;
			bool unique_sol_exists = true;
			while (unique_sol_exists) {

				// Remove one digit
				sudoku_size_t remove_ind = std::rand() % (tot_num_cells - n_curr);
				remove_nth(sudoku, remove_ind);
				auto_fill(sudoku, true);
				sudoku_copy = sudoku;
				++n_curr;

				// Try solving
				rec_depth_t rec_dep = solve_count_rec_depth<3, 3>(sudoku_copy);
				if (rec_dep == 0) {
					//std::cout << "Found easy Sudoku :)\n";
				}
				else if (rec_dep > 0) {
					const sudoku_size_t n_sud_w_lvl = lvl_count[rec_dep];
					if (n_sud_w_lvl < max_suds_per_lvl) {
						const raw_sudoku_t raw_sud = get_raw_sudoku(sudoku);
						const sud_char_t desc = generate_sud_char(raw_sud, rec_dep);
						bool added = add_to_coll(sud_map, desc, raw_sud, raw_s_sol);
						if (added) {
							std::cout << "Added hard Sudoku :D, level: " << rec_dep << "\n";
							std::cout << "With ID: " << desc << "\n";
							lvl_count[rec_dep]++;
						}
					}
				}
				else if (rec_dep < 0) {
					//std::cout << "No more unique sudokus :( " << rec_dep << "\n";
					unique_sol_exists = false;
				}
			}			
		}
		if ((k + 1) % 200 == 0) {
			std::cout << "Iteration: " << k + 1 << ", Saving...\n";
			save_coll(sud_map);
		}
	}
	std::cout << "Finished!\n";
}