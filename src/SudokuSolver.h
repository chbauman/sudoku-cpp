#pragma once

#include "SudokuCore.h"

/// Recursion depth returned by \ref SudokuSolver::solve_count_rec_depth.
///
/// -3: Error occurred, -2: Invalid, -1: Multiple, 0: Unique (no recursion
/// needed), n > 0: Unique, min. rec. depth n.
typedef int rec_depth_t;

/// Status for intermediate solver.
enum SolveStepRes {
	Invalid, ///< Sudoku does not have a solution.
	ValidnNoChange, ///< Sudoku is valid and was not changed.
	ValidNewFound, ///< Sudoku is valid and a change was made towards solving it.
};

/// String array mapping each \ref SolveStepRes to an informative string.
const std::string sol_step_msgs[] = {
	"Sudoku is invalid.",
	"Sudoku is valid, no new number found.",
	"Sudoku is valid, found new number."
};

/// Printing \ref SolveStepRes to std::cout.
inline std::ostream& operator<<(std::ostream & os, const SolveStepRes & sol_step) {
	os << sol_step_msgs[sol_step] << "\n";
	return os;
}

/// Status for final solver.
enum SolveResultFinal {
	InvalidSolution, ///< Sudoku does not have a solution.
	UniqueSolution, ///< Sudoku has a unique solution.
	MultipleSolution, ///< Sudoku contains multiple solutions.
	UnknownSolution, ///< Unknown number of solutions.
};

/// String array mapping each \ref SolveResultFinal to an informative string.
const std::string sol_res_fin_msgs[] = { "Sudoku is invalid, cannot be solved.", "Sudoku has a unique solution.",
	"Sudoku has multiple solutions.", "Solutions has not yet been found." };

/// Printing \ref SolveResultFinal to std::cout.
inline std::ostream& operator<<(std::ostream & os, const SolveResultFinal & sol_step) {
	os << sol_res_fin_msgs[sol_step] << "\n";
	return os;
}

constexpr bool printRecDebInfo = false;

/// Deterministic and brute-force solving algorithms for a sudoku.
class SudokuSolver {
public:

	/// Looks for numbers that can only be placed in one cell in a given row/col.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes find_unique_in_rcs(sudoku_data_t & s_data) {

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

	/// Looks for numbers that can only be placed in one cell in a given square.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes find_unique_in_square(sudoku_data_t & s_data) {

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

	/// Looks for cells where only one number can be.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes find_single_number_cell(sudoku_data_t & s_data) {

		bool found_number = false;

		// Iterate over all rows
		for (sudoku_size_t row_num = 0; row_num < side_len; ++row_num) {

			// Iterate over all cols
			for (sudoku_size_t col_num = 0; col_num < side_len; ++col_num) {

				const sudoku_size_t cell_ind = row_num + col_num * side_len;
				const sudoku_size_t data_ind = cell_ind * n_stored_per_cell;

				if (s_data[data_ind] > 0) continue;
				// Iterate over all numbers
				std::optional<sudoku_size_t> last_possible_num = std::nullopt;
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
					s_data[data_ind] = 1 + *last_possible_num;
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

	/// Looks for possible numbers that can be eliminated in all rows.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes eliminate_possible_numbers_row(sudoku_data_t & s_data) {

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

	/// Looks for possible numbers that can be eliminated in all cols.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes eliminate_possible_numbers_col(sudoku_data_t & s_data) {

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

	/// Looks for possible numbers that can be eliminated in all squares.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes eliminate_possible_numbers_square(sudoku_data_t & s_data) {

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
	static SolveStepRes update(SolveStepRes old_step, SolveStepRes new_step) {
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
	static SolveStepRes try_solving(sudoku_data_t & s_data) {

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
	static bool solved(sudoku_data_t & s_data) {
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
	}

	// Find the cell with the least numbers possible
	template<bool printDebugInfo = printDebugInfodefault>
	static sudoku_size_t find_least_uncertain_cell(sudoku_data_t & s_data) {

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
	static SolveResultFinal solve_brute_force_multiple(sudoku_data_t & s_data) {

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
	static SolveResultFinal solve_brute_force_multiple_random(sudoku_data_t & s_data, RNG & rng) {

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
	static int solve_brute_force_all(sudoku_data_t & s_data) {

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

	// Find a solution and check if it is unique
	// Additionally find recursion depth
	template<sudoku_size_t square_height, sudoku_size_t square_width, bool printDebugInfo = printRecDebInfo>
	static rec_depth_t solve_count_rec_depth(sudoku_data_t & s_data, const rec_depth_t rec_dep = 0) {

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
				res = solve_count_rec_depth<square_height, square_width>(
					s_data_copy, rec_dep + 1);
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

};
