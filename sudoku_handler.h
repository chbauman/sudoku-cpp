#pragma once

#include "Lib.h"

template<sudoku_size_t square_height = 3, sudoku_size_t square_width = 3>
class SudokuHandler {

	// Constants
	static constexpr sudoku_size_t side_len = square_height * square_width;
	static constexpr sudoku_size_t tot_num_cells = side_len * side_len;
	static constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
	static constexpr sudoku_size_t tot_storage = n_stored_per_cell * tot_num_cells;
	static constexpr sudoku_size_t n_stored_per_side = n_stored_per_cell * side_len;
	
	// The sudoku data
	sudoku_data_t sud_data;
	raw_sudoku_t raw_sud;

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

	/// Looks for numbers that can only be placed in one cell in a given row/col.
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
							<< cell_poss_num_row << ".\n";
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
					if constexpr (printDebugInfo) {
						std::cout << "No possibility to put "
							<< number + 1 << " in " << (num_poss_places_row == 0 ? "row " : "col ") << row_num + 1 << ".\n";
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

	/// Looks for cells where only one number can be.
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

	/// Looks for possible numbers that can be eliminated in all rows.
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

	/// Looks for possible numbers that can be eliminated in all cols.
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

					const sudoku_size_t first_cell_index = square_row_num * side_len * square_height + col_num;

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

	/// Updating status uf solving process
	SolveStepRes update(SolveStepRes old_step, SolveStepRes new_step) {
		if (old_step == Invalid || new_step == Invalid) {
			return Invalid;
		}
		else if (new_step == ValidNewFound) {
			return ValidNewFound;
		}
		return old_step;
	}

	/// Try to solve the sudoku using the previously defined functions
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

	/// Check if sudoku is solved.
	bool solved(sudoku_data_t & s_data) {
		// Check if there is a number set in every cell
		for (sudoku_size_t cell_ind = 0; cell_ind < tot_num_cells; ++cell_ind) {
			if (s_data[cell_ind * n_stored_per_cell] == 0) {
				return false;
			}
		}
		return true;
	}

	/// Find the cell with the least numbers possible
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

	/// Find a solution and check if it is unique
	template<bool printDebugInfo = printRecDebInfo, typename RNG>
	FullSol_t solve_brute_force_multiple_random(
		sudoku_data_t & s_data,
		RNG & rng, 
		const rec_depth_t rec_dep = 0, 
		const bool random_order = true
	) {

		// Try solving 
		SolveStepRes init_stat = try_solving(s_data);
		if (init_stat == Invalid) {
			return std::make_pair(InvalidSolution, rec_dep);
		}
		else if (solved(s_data)) {
			return std::make_pair(UniqueSolution, rec_dep);
		}

		// Solve by guessing recursively
		sudoku_data_t s_data_copy = s_data;
		sudoku_data_t s_data_res = s_data;
		const sudoku_size_t cell_picked = find_least_uncertain_cell(s_data);
		SolveResultFinal res = UnknownSolution;
		sudoku_size_t num_sols = 0;

		rec_depth_t curr_min_rd = -3;
		rec_depth_t res_rd = -3;

		// Random Order
		std::array<sudoku_value_t, side_len> perm;
		for (sudoku_size_t i = 0; i < side_len; ++i) {
			perm[i] = i;
		}
		if (random_order) {
			//std::shuffle(perm.begin(), perm.end(), rng);
		}

		// Loop over all possible guesses
		for (sudoku_size_t i = 0; i < side_len; ++i) {

			const sudoku_size_t curr_i = perm[i];

			if (s_data[cell_picked + 1 + curr_i] == 2) {
				// Copy data and set guessed value
				s_data_copy = s_data;
				s_data_copy[cell_picked] = curr_i + 1;

				// Recursion
				auto[res, res_rd] = solve_brute_force_multiple_random<printDebugInfo>(
					s_data_copy, rng, rec_dep + 1, random_order);
				if (res == UniqueSolution) {
					s_data_res = s_data_copy;
					num_sols += 1;
					if (curr_min_rd == -3 || res_rd < curr_min_rd) {
						curr_min_rd = res;
					}
				}
				if (num_sols > 1 || res == MultipleSolution) {
					s_data = s_data_copy;
					return std::make_pair(MultipleSolution, -1);
				}
			}
		}
		s_data = s_data_res;
		if (num_sols == 1) {
			return std::make_pair(UniqueSolution, curr_min_rd);
		}
		if (num_sols == 0) {
			return std::make_pair(InvalidSolution, -2);
		}
		if (num_sols > 1) {
			return std::make_pair(MultipleSolution, curr_min_rd);
		}

		// Should not happen
		return std::make_pair(UnknownSolution, -3);
	}

public:
	/// Default Constructor
	SudokuHandler() {};

	/// Construct from raw sudoku.
	SudokuHandler(raw_sudoku_t raw_sud) {
		set_sudoku(raw_sud);
	};

	/// Set the sudoku.
	void set_sudoku(raw_sudoku_t raw_sud) {
		this->raw_sud = raw_sud;
		sud_data = init_sudoku_with_raw(raw_sud);
		auto_fill(this->sud_data, true);
	}

	/// Solves the loaded sudoku.
	FullSol_t solve() {
		int rng = 5;
		return solve_brute_force_multiple_random(sud_data, rng);
	}


	void test_init() const {
		assert(this->tot_num_cells == this->side_len * this->side_len);
	}

};
