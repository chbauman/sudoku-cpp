#pragma once

#include "SudokuCore.h"

#include <numeric>
#include <vector>

/// Recursion depth returned by \ref SudokuSolver::solve_count_rec_depth.
///
/// -3: Error occurred, -2: Invalid, -1: Multiple, 0: Unique (no recursion
/// needed), n > 0: Unique, min. rec. depth n. (The difficulty level of
/// \ref SudokuSolver::rate_difficulty is a different number, never negative.)
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

/// Deterministic and brute-force solving algorithms for a sudoku of the
/// given board dimensions (square_height x square_width squares).
template<sudoku_size_t square_height, sudoku_size_t square_width>
class SudokuSolver {
private:
	using Dims = SudokuDims<square_height, square_width>;
	static constexpr sudoku_size_t side_len = Dims::side_len;
	static constexpr sudoku_size_t n_stored_per_cell = Dims::n_stored_per_cell;

public:
	using sudoku_data_t = ::sudoku_data_t<square_height, square_width>;
	using raw_sudoku_t = ::raw_sudoku_t<square_height, square_width>;

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

					const sudoku_size_t square_col_ind_inner = cell_id % square_width;
					const sudoku_size_t square_row_ind_inner = cell_id / square_width;
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
					const sudoku_size_t square_col_ind_inner = cell_id % square_width;
					const sudoku_size_t square_row_ind_inner = cell_id / square_width;
					const sudoku_size_t cell_ind = square_beg_ind + square_col_ind_inner + square_row_ind_inner * side_len;
					if (s_data[cell_ind * n_stored_per_cell] == 0 && s_data[cell_ind * n_stored_per_cell + number + 1] == 2) {
						++num_poss_places;
						cell_poss_num = cell_id;
					}
				}

				// If it can only be in one place
				if (num_poss_places == 1) {
					const sudoku_size_t square_col_ind_inner = cell_poss_num % square_width;
					const sudoku_size_t square_row_ind_inner = cell_poss_num / square_width;
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

	/// Fills in the last empty cell of a row or column with the number that is
	/// still missing there.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes find_last_in_row_col(sudoku_data_t & s_data) {

		bool found_number = false;

		// Lines 0 to side_len - 1 are the rows, the others the columns
		for (sudoku_size_t line = 0; line < 2 * side_len; ++line) {
			const bool is_row = line < side_len;
			const sudoku_size_t line_ind = line % side_len;

			sudoku_size_t num_empty = 0;
			sudoku_size_t empty_data_ind = 0;
			std::vector<bool> used(side_len + 1, false);
			for (sudoku_size_t i = 0; i < side_len; ++i) {
				const sudoku_size_t cell_ind = is_row ? line_ind * side_len + i : i * side_len + line_ind;
				const sudoku_size_t data_ind = cell_ind * n_stored_per_cell;
				const sudoku_size_t val = s_data[data_ind];
				if (val == 0) {
					++num_empty;
					empty_data_ind = data_ind;
				}
				else if (used[val]) {
					return Invalid;
				}
				else {
					used[val] = true;
				}
			}
			if (num_empty != 1) continue;

			for (sudoku_size_t num = 1; num <= side_len; ++num) {
				if (used[num]) continue;
				s_data[empty_data_ind] = num;
				found_number = true;
				if constexpr (printDebugInfo) {
					std::cout << "Found the last number " << num << " in " << (is_row ? "row " : "col ") << line_ind + 1 << ".\n";
				}
				break;
			}
		}
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
					for (sudoku_size_t square_row = 0; square_row < square_height; ++square_row) {
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

			auto_fill<square_height, square_width, printDebugInfo>(s_data, false);
		}
		return found_something;
	}

	/// Solves with the simplest deductions only: a cell with only one candidate,
	/// and a number that fits in only one cell of a row, column or square. The
	/// candidates are eliminated directly by the numbers that are set in the
	/// row, column and square of a cell, none of the other eliminations of
	/// \ref try_solving is used.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes try_solving_basic(sudoku_data_t & s_data) {

		SolveStepRes found_something = ValidNewFound;

		while (found_something == ValidNewFound) {
			found_something = ValidnNoChange;
			found_something = update(found_something, find_unique_in_rcs<printDebugInfo>(s_data));
			found_something = update(found_something, find_unique_in_square<printDebugInfo>(s_data));
			found_something = update(found_something, find_single_number_cell<printDebugInfo>(s_data));

			auto_fill<square_height, square_width, printDebugInfo>(s_data, false);
		}
		return found_something;
	}

	/// Solves with the very simplest deductions only: a number that fits in only
	/// one cell of a square, and the last empty cell of a row or column. The
	/// candidates are eliminated directly by the numbers that are set in the row,
	/// column and square of a cell.
	template<bool printDebugInfo = printDebugInfodefault>
	static SolveStepRes try_solving_easiest(sudoku_data_t & s_data) {

		SolveStepRes found_something = ValidNewFound;

		while (found_something == ValidNewFound) {
			found_something = ValidnNoChange;
			found_something = update(found_something, find_unique_in_square<printDebugInfo>(s_data));
			found_something = update(found_something, find_last_in_row_col<printDebugInfo>(s_data));

			auto_fill<square_height, square_width, printDebugInfo>(s_data, false);
		}
		return found_something;
	}

	// Check if sudoku is solved
	static bool solved(sudoku_data_t & s_data) {
		// Check if there is a number set in every cell
		for (sudoku_size_t cell_ind = 0; cell_ind < Dims::tot_num_cells; ++cell_ind) {
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
	template<bool printDebugInfo = printRecDebInfo>
	static SolveResultFinal solve_brute_force_multiple(sudoku_data_t & s_data) {

		// Try solving
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return InvalidSolution;
		}
		else if (solved(s_data)) {
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
				res = solve_brute_force_multiple(s_data_copy);
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
	template<bool printDebugInfo = printRecDebInfo, typename RNG>
	static SolveResultFinal solve_brute_force_multiple_random(sudoku_data_t & s_data, RNG & rng) {

		// Try solving
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return InvalidSolution;
		}
		else if (solved(s_data)) {
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
		std::iota(perm.begin(), perm.end(), 0);
		std::ranges::shuffle(perm, rng);

		// Loop over all possible guesses
		for (sudoku_size_t i = 0; i < side_len; ++i) {

			const sudoku_size_t curr_i = perm[i];

			if (s_data[cell_picked + 1 + curr_i] == 2) {
				// Copy data and set guessed value
				s_data_copy = s_data;
				s_data_copy[cell_picked] = curr_i + 1;

				// Recursion
				res = solve_brute_force_multiple_random(s_data_copy, rng);
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
	template<bool printDebugInfo = printRecDebInfo>
	static int solve_brute_force_all(sudoku_data_t & s_data) {

		// Try solving
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return 0;
		}
		else if (solved(s_data)) {
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
				const int res = solve_brute_force_all(s_data_copy);
				num_sols += res;
				if (res > 0) {
					s_data_res = s_data_copy;
				}
			}
		}
		s_data = s_data_res;
		return num_sols;
	}

	// Finds a single solution (not necessarily unique) and stops as soon as
	// one is found, without exploring remaining guesses. Faster than
	// solve_brute_force_all when only one solution is wanted, in particular
	// on puzzles with many solutions.
	template<bool printDebugInfo = printRecDebInfo>
	static bool solve_brute_force_first(sudoku_data_t & s_data) {

		// Try solving
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return false;
		}
		else if (solved(s_data)) {
			return true;
		}

		// Solve by guessing recursively
		sudoku_data_t s_data_copy = s_data;
		const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);

		// Loop over all possible guesses
		for (sudoku_size_t i = 0; i < side_len; ++i) {

			if (s_data[cell_picked + 1 + i] == 2) {
				// Copy data and set guessed value
				s_data_copy = s_data;
				s_data_copy[cell_picked] = i + 1;

				// Recursion
				if (solve_brute_force_first(s_data_copy)) {
					s_data = s_data_copy;
					return true;
				}
			}
		}
		return false;
	}

	/// Returns the data indices (cell * n_stored_per_cell) of the empty cells
	/// that have the fewest remaining candidates.
	static std::vector<sudoku_size_t> least_uncertain_cells(const sudoku_data_t & s_data) {
		std::vector<sudoku_size_t> cells;
		sudoku_size_t min_poss = side_len + 1;
		for (sudoku_size_t cell_ind = 0; cell_ind < side_len * side_len; ++cell_ind) {
			const sudoku_size_t data_ind = cell_ind * n_stored_per_cell;
			if (s_data[data_ind] > 0) continue;
			sudoku_size_t n_poss = 0;
			for (sudoku_size_t num = 0; num < side_len; ++num) {
				if (s_data[data_ind + 1 + num] == 2) ++n_poss;
			}
			if (n_poss < min_poss) {
				min_poss = n_poss;
				cells.clear();
			}
			if (n_poss == min_poss) cells.push_back(data_ind);
		}
		return cells;
	}

	/// Whether a sudoku can be solved completely by \ref try_solving_easiest.
	///
	/// @param s_data The sudoku with its candidates set (left unchanged).
	static bool solvable_by_easiest_rules(const sudoku_data_t & s_data) {
		sudoku_data_t work = s_data;
		return try_solving_easiest(work) != Invalid && solved(work);
	}

	/// Whether a sudoku can be solved completely by \ref try_solving_basic.
	///
	/// @param s_data The sudoku with its candidates set (left unchanged).
	static bool solvable_by_basic_rules(const sudoku_data_t & s_data) {
		sudoku_data_t work = s_data;
		return try_solving_basic(work) != Invalid && solved(work);
	}

	/// Difficulty level of a sudoku: 0 if \ref solvable_by_easiest_rules, 1 if
	/// \ref solvable_by_basic_rules, else the guessing depth
	/// \ref solve_min_rec_depth plus two (so 2 = solvable without guessing, but
	/// needs more than the basic rules).
	///
	/// @param s_data The sudoku with its candidates set (left unchanged).
	/// @param max_level The search for the guessing depth is cut off above this
	///   level, then \p max_level + 1 is returned (the exact level is not needed).
	static rec_depth_t rate_difficulty(const sudoku_data_t & s_data, const rec_depth_t max_level = 1000) {
		if (solvable_by_easiest_rules(s_data)) return 0;
		if (solvable_by_basic_rules(s_data)) return 1;
		if (max_level < 2) return max_level + 1;
		const rec_depth_t depth_cap = max_level - 1;
		const rec_depth_t depth = solve_min_rec_depth(s_data, 0, depth_cap);
		return depth < depth_cap ? depth + 2 : max_level + 1;
	}

	/// Guessing depth needed to settle a sudoku, minimised over the guessed cells.
	///
	/// \ref solve_count_rec_depth only counts the depth along the path to the
	/// solution and always guesses in the first least uncertain cell, so it
	/// ignores the effort of refuting the wrong guesses and depends on that
	/// arbitrary choice. Here the level is the nesting depth of the whole
	/// search tree: all candidates of a guessed cell have to be settled
	/// (maximum over the candidates) and the cell is the one with the
	/// shallowest tree among all cells tied for the fewest candidates
	/// (minimum over the cells). A contradiction found by plain deduction
	/// ends a branch at the current depth.
	///
	/// @param s_data The sudoku (left unchanged).
	/// @param rec_dep Current depth (number of guesses made so far).
	/// @param best Upper bound: branches that cannot get below it are cut off.
	/// @return The depth, or \p best if it cannot be lowered below it.
	static rec_depth_t solve_min_rec_depth(const sudoku_data_t & s_data, const rec_depth_t rec_dep = 0, const rec_depth_t best = 1000) {
		sudoku_data_t work = s_data;
		if (try_solving(work) == Invalid || solved(work)) return rec_dep;
		if (rec_dep + 1 >= best) return best;

		rec_depth_t curr_best = best;
		for (const sudoku_size_t cell_picked : least_uncertain_cells(work)) {
			rec_depth_t cell_depth = rec_dep + 1;
			for (sudoku_size_t i = 0; i < side_len && cell_depth < curr_best; ++i) {
				if (work[cell_picked + 1 + i] != 2) continue;
				sudoku_data_t guess = work;
				guess[cell_picked] = i + 1;
				cell_depth = std::max(cell_depth, solve_min_rec_depth(guess, rec_dep + 1, curr_best));
			}
			curr_best = std::min(curr_best, cell_depth);
		}
		return curr_best;
	}

	// Find a solution and check if it is unique
	// Additionally find recursion depth
	template<bool printDebugInfo = printRecDebInfo>
	static rec_depth_t solve_count_rec_depth(sudoku_data_t & s_data, const rec_depth_t rec_dep = 0) {

		// Try solving
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return -2;
		}
		else if (solved(s_data)) {
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
				res = solve_count_rec_depth(s_data_copy, rec_dep + 1);
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
