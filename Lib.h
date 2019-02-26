#pragma once

#include <array>
#include <cassert>
#include <string>
#include <iostream>
#include <random>

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

typedef std::array<sudoku_size_t, tot_storage> sudoku_data_t;
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

// Find a solution
template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo>
SolveResultFinal solve_brute_force(sudoku_data_t & s_data, RandomNumberPicker<square_height, square_width> rnp) {
	
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
	random_pick_t pick;

	SolveResultFinal res = UnknownSolution;
	while (res == UnknownSolution) {

		pick = rnp.pickAndSetRandom(s_data_copy);
		res = solve_brute_force<square_height, square_width>(s_data_copy, rnp);
		if constexpr (printDebugInfo) {
			std::cout << res;
		}

		// Got further in solving
		if (res == InvalidSolution) {
			
			// Eliminate and try solving again
			eliminate_random_pick<square_height, square_width>(s_data, pick);
			SolveStepRes stat = try_solving(s_data);
			s_data_copy = s_data;			
			if (stat == Invalid) {
				return InvalidSolution;
			}
			else if (solved<square_height, square_width>(s_data)) {
				return UniqueSolution;
			}
			res = UnknownSolution;
		}
		// Found solution
		else if (res == UniqueSolution || res == MultipleSolution) {
			s_data = s_data_copy;
			std::cout << "Solution may or may not be unique.\n";
			return MultipleSolution;
		}
	}	
	return UnknownSolution;
}

