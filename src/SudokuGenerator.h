#pragma once

#include "SudokuCore.h"
#include "SudokuSolver.h"

#include <map>
#include <fstream>
#include <sstream>

/// Returns a random permutation of size n.
template<sudoku_size_t n, class rng>
std::array<sudoku_size_t, n> random_permutation(rng & g) {
	std::array<sudoku_size_t, n> cell_order;
	for (sudoku_size_t i = 0; i < n; ++i) {
		cell_order[i] = i;
	}
	std::shuffle(cell_order.begin(), cell_order.end(), g);
	return cell_order;
}

/// A pick is defined with the cell number and the number that was picked
typedef std::pair<sudoku_size_t, sudoku_size_t> random_pick_t;

/// Prints random picks to std::cout
///
/// Overloads the << operator for direct use with \ref random_pick_t.
/// @param p The random pick.
inline std::ostream& operator<<(std::ostream & os, const random_pick_t & p) {
	os << "Picked cell " << p.first << " and number " << p.second + 1 << "\n";
	return os;
}

/// The class for picking random cells.
///
/// Picks a random cell where the number is not set currently
/// and sets it with a random number that fits.
template<sudoku_size_t square_height, sudoku_size_t square_width>
class RandomNumberPicker {

public:
	RandomNumberPicker(): cell_order(random_permutation<tot_num_cells>(gen)), number_order(random_permutation<side_len>(gen)){};

	// Picks a random cell and fills it with a random possible number.
	// Returns std::nullopt if no empty cell is left.
	std::optional<random_pick_t> pickAndSetRandom(sudoku_data_t & s_data){
		std::optional<random_pick_t> ret_val = std::nullopt;
		const sudoku_size_t cell_index_init = dis(gen);
		const sudoku_size_t num_ind_init = dis(gen);
		for(sudoku_size_t i = 0; i < tot_num_cells; ++i){
			const sudoku_size_t curr_pos = cell_order[(cell_index_init + i) % tot_num_cells];
			if (s_data[n_stored_per_cell * curr_pos] == 0) {
				for (sudoku_size_t k = 0; k < side_len; ++k) {
					const sudoku_size_t curr_num = number_order[(num_ind_init + k) % side_len];
					if (s_data[n_stored_per_cell * curr_pos + 1 + curr_num] == 2) {
						s_data[n_stored_per_cell * curr_pos] = curr_num + 1;
						ret_val = random_pick_t{curr_pos, curr_num};
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

/// Eliminates the random pick as possibility.
template<sudoku_size_t square_height, sudoku_size_t square_width>
void eliminate_random_pick(sudoku_data_t & s_data, const random_pick_t & rp) {
	constexpr sudoku_size_t side_len = square_height * square_width;
	constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
	s_data[rp.first * n_stored_per_cell + 1 + rp.second] = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type Definition
typedef int num_sud_t;
typedef unsigned int num_filled_t;
typedef std::string sud_char_t;
typedef std::pair<raw_sudoku_t, raw_sudoku_t> sud_and_sol_t;
typedef std::map<sud_char_t, std::vector<sud_and_sol_t> > sud_coll_t;

typedef std::pair<SolveResultFinal, rec_depth_t> FullSol_t;

/// Generates and persists sudoku puzzles, rating each by solving difficulty.
class SudokuGenerator {
public:

	// Counts the number that is currently set in the given sudoku
	static sudoku_size_t count_num_known_numbers(const sudoku_data_t & s_data) {

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
	static void remove_nth(sudoku_data_t & s_data, const sudoku_size_t n) {

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
	static sud_char_t generate_sud_char(const raw_sudoku_t & s, const rec_depth_t lvl) {

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
	static bool add_to_coll(sud_coll_t & sud_map, const sud_char_t & desc, const raw_sudoku_t s, const raw_sudoku_t s_sol, const num_sud_t max_sud_per_key = 100) {

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

	/// Converts a raw sudoku to a string.
	///
	/// Does the opposite of \ref string_to_sud().
	static std::string sud_to_string(const raw_sudoku_t & s) {
		std::string res = "";
		for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
			res += std::to_string(s[i]) + " ";
		}
		return res;
	}

	/// Convert String to Sudoku.
	///
	/// Does the opposite of \ref sud_to_string().
	static raw_sudoku_t string_to_sud(const std::string & str) {
		raw_sudoku_t rs;
		for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
			const std::string num_str = str.substr(2 * i, 2 * i + 1);
			rs[i] = std::stoi(num_str);
		}
		return rs;
	}

	/// Saves the collection in text format.
	static void save_coll(const sud_coll_t & sud_map, std::string folder_path = file_path) {

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

	/// Checks if file 'name' exists.
	static bool f_exists(const std::string & f_name) {
		std::ifstream f(f_name.c_str());
		return f.good();
	}

	// Load the sudokus saved on disk into collection
	static sud_coll_t load_coll(std::string folder_path = file_path) {
		sud_coll_t sud_map;

		// Return empty map if file does not exist
		if (!f_exists(folder_path)) {
			std::cout << "Creating new file\n";
			return sud_map;
		}

		// Read file linewise and extract info
		std::ifstream file(folder_path);
		std::string str;
		while (std::getline(file, str)) {
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

	// Separate Sudokus into separate maps by difficulty
	static void separate_by_level_and_save(const sud_coll_t & sud_map) {

		const rec_depth_t max_lvl = 9;
		std::array<sud_coll_t, max_lvl> lvl_sep_map;

		for (auto& x : sud_map)
		{
			// Find Level
			const sud_char_t & desc = x.first;
			const std::vector<sud_and_sol_t> sas = x.second;
			const std::string lvl_string = desc.substr(0, 1);
			const rec_depth_t lvl = std::stoi(lvl_string);

			// Add to collection
			lvl_sep_map[lvl][desc] = sas;
		}

		// Save to separate files
		for (rec_depth_t i = 0; i < max_lvl; ++i) {
			std::string f_name = file_dir + "ext_lvl_" + std::to_string(i) + ".txt";
			save_coll(lvl_sep_map[i], f_name);
		}
	}

	/// Generate hard Sudokus and save them to the disk.
	static void generate_hard_sudokus(const num_sud_t max_suds_per_lvl = 1000) {

		// Initialize
		std::array<sudoku_value_t, side_len> lvl_count;
		std::fill(lvl_count.begin(), lvl_count.end(), 0);
		sud_coll_t sud_map = load_coll();
		std::mt19937 gen = std::mt19937(seed);

		for (int k = 0; k < 50000; ++k) {

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
			SudokuSolver::solve_brute_force_multiple_random<3, 3>(sudoku, gen);
			const raw_sudoku_t raw_s_sol = get_raw_sudoku(sudoku);
			sudoku_data_t sudoku_solution_copy = sudoku;

			for (int l = 0; l < 100; ++l) {
				sudoku = sudoku_solution_copy;

				// Remove digits randomly
				const sudoku_size_t n_init = 45;
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
					rec_depth_t rec_dep = SudokuSolver::solve_count_rec_depth<3, 3>(sudoku_copy);
					if (rec_dep > 3) {
						const sudoku_size_t n_sud_w_lvl = lvl_count[rec_dep];
						if (n_sud_w_lvl < max_suds_per_lvl) {
							const raw_sudoku_t raw_sud = get_raw_sudoku(sudoku);
							const sud_char_t desc = generate_sud_char(raw_sud, rec_dep);
							bool added = add_to_coll(sud_map, desc, raw_sud, raw_s_sol);
							if (added) {
								std::cout << "Added hard Sudoku :D, level: " << rec_dep;
								std::cout << ", With ID: " << desc << "\n";
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

private:
	static inline std::string file_dir = "./Data/";
	static inline std::string file_path = file_dir + "dat.txt";
};
