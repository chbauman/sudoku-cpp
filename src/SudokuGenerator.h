#pragma once

#include "SudokuCore.h"
#include "SudokuSolver.h"

#include <map>
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <numeric>
#include <sstream>
#include <vector>

namespace sudoku_detail {

/// Splits a string on a single-character delimiter. Used by the on-disk
/// puzzle collection format (see SudokuGenerator::generate_sud_char and
/// SudokuGenerator::save_coll), which is delimiter-separated rather than
/// fixed-width so it works regardless of how many digits a cell value or
/// frequency count takes (boards with more than 9 values per cell need
/// two-digit values).
inline std::vector<std::string> split(const std::string & s, char delim) {
	std::vector<std::string> tokens;
	std::size_t start = 0;
	while (true) {
		const std::size_t pos = s.find(delim, start);
		if (pos == std::string::npos) {
			tokens.push_back(s.substr(start));
			break;
		}
		tokens.push_back(s.substr(start, pos - start));
		start = pos + 1;
	}
	return tokens;
}

} // namespace sudoku_detail

/// Returns a random permutation of size n.
template<sudoku_size_t n, class rng>
std::array<sudoku_size_t, n> random_permutation(rng & g) {
	std::array<sudoku_size_t, n> cell_order;
	std::iota(cell_order.begin(), cell_order.end(), 0);
	std::ranges::shuffle(cell_order, g);
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
	std::optional<random_pick_t> pickAndSetRandom(sudoku_data_t<square_height, square_width> & s_data){
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
void eliminate_random_pick(sudoku_data_t<square_height, square_width> & s_data, const random_pick_t & rp) {
	constexpr sudoku_size_t side_len = square_height * square_width;
	constexpr sudoku_size_t n_stored_per_cell = side_len + 1;
	s_data[rp.first * n_stored_per_cell + 1 + rp.second] = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Type Definition
typedef int num_sud_t;
typedef unsigned int num_filled_t;
typedef std::string sud_char_t;

typedef std::pair<SolveResultFinal, rec_depth_t> FullSol_t;

/// Generates and persists sudoku puzzles of the given board dimensions,
/// rating each by solving difficulty.
template<sudoku_size_t square_height, sudoku_size_t square_width>
class SudokuGenerator {
private:
	using Dims = SudokuDims<square_height, square_width>;
	static constexpr sudoku_size_t side_len = Dims::side_len;
	static constexpr sudoku_size_t tot_num_cells = Dims::tot_num_cells;
	static constexpr sudoku_size_t n_stored_per_cell = Dims::n_stored_per_cell;

	using Solver = SudokuSolver<square_height, square_width>;

public:
	using sudoku_data_t = ::sudoku_data_t<square_height, square_width>;
	using raw_sudoku_t = ::raw_sudoku_t<square_height, square_width>;
	using sud_and_sol_t = std::pair<raw_sudoku_t, raw_sudoku_t>;
	using sud_coll_t = std::map<sud_char_t, std::vector<sud_and_sol_t> >;

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

	// Generates a string that describes the sudoku's difficulty and digit
	// distribution. Used both as a human-readable summary and as the
	// bucketing key add_to_coll() groups puzzles by (so at most
	// max_sud_per_key puzzles with the same profile are kept).
	//
	// Comma-separated fields: difficulty level, count of filled-in
	// digits, then one frequency count per digit 1..side_len (how many
	// times each digit occurs among the filled-in digits), sorted in
	// decreasing order.
	// E.g. "1,33,6,6,4,4,3,3,3,2,2" for a 9x9 sudoku containing 33
	// filled-in digits and difficulty level 1.
	static sud_char_t generate_sud_char(const raw_sudoku_t & s, const rec_depth_t lvl) {

		// Initialize
		std::array<sudoku_value_t, side_len> freq{};
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
		std::ranges::sort(freq, std::greater{});

		// Construct String Output
		sud_char_t res = std::to_string(lvl) + "," + std::to_string(tot_n_digits);
		for (auto& e : freq) {
			res += "," + std::to_string(e);
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

	/// Converts a raw sudoku to a space-separated string of its
	/// side_len*side_len cell values - the same row format \c solve's
	/// command line accepts, so a puzzle/solution from the on-disk
	/// collection can be pasted directly into \c solve.
	///
	/// Does the opposite of \ref string_to_sud().
	static std::string sud_to_string(const raw_sudoku_t & s) {
		std::string res;
		for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
			if (i > 0) res += " ";
			res += std::to_string(s[i]);
		}
		return res;
	}

	/// Converts a space-separated string of side_len*side_len cell
	/// values back to a sudoku. Returns std::nullopt if it doesn't
	/// contain exactly that many values.
	///
	/// Does the opposite of \ref sud_to_string().
	static std::optional<raw_sudoku_t> string_to_sud(const std::string & str) {
		std::istringstream iss(str);
		raw_sudoku_t rs;
		for (sudoku_size_t i = 0; i < side_len * side_len; ++i) {
			if (!(iss >> rs[i])) return std::nullopt;
		}
		sudoku_size_t leftover;
		if (iss >> leftover) return std::nullopt;
		return rs;
	}

	/// Default directory for this board size's on-disk puzzle collection
	/// (kept separate per board size, so different dimensions never collide).
	static std::string default_file_dir() {
		return "./Data/" + std::to_string(square_height) + "x" + std::to_string(square_width) + "/";
	}

	static std::string default_file_path() {
		return default_file_dir() + "dat.txt";
	}

	/// Saves the collection in text format.
	static void save_coll(const sud_coll_t & sud_map, std::string folder_path = "") {
		if (folder_path.empty()) folder_path = default_file_path();

		const std::filesystem::path p(folder_path);
		if (p.has_parent_path()) {
			std::filesystem::create_directories(p.parent_path());
		}

		std::ofstream myfile;
		myfile.open(folder_path);
		for (auto& x : sud_map)
		{
			const sud_char_t & desc = x.first;
			const std::vector<sud_and_sol_t> sas = x.second;
			for (auto& e : sas) {
				const auto&[s, sol] = e;
				myfile << desc << ";" << sud_to_string(s) << ";" << sud_to_string(sol) << "\n";
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
	static sud_coll_t load_coll(std::string folder_path = "") {
		if (folder_path.empty()) folder_path = default_file_path();
		sud_coll_t sud_map;

		// Return empty map if file does not exist
		if (!f_exists(folder_path)) {
			std::cout << "Creating new file\n";
			return sud_map;
		}

		// Read file linewise and extract info. Each line has 3
		// semicolon-separated sections: the descriptor generate_sud_char()
		// produces, the puzzle, and its solution - the latter two in the
		// same space-separated row format solve's command line accepts
		// (see sud_to_string()), so either can be copied straight out of
		// this file and pasted into solve.
		std::ifstream file(folder_path);
		std::string str;
		while (std::getline(file, str)) {
			const std::vector<std::string> sections = sudoku_detail::split(str, ';');
			if (sections.size() != 3) {
				std::cout << "Skipping malformed line (expected 3 ';'-separated sections, got "
					<< sections.size() << ")\n";
				continue;
			}

			const std::string & desc = sections[0];
			const std::optional<raw_sudoku_t> s = string_to_sud(sections[1]);
			const std::optional<raw_sudoku_t> s_sol = string_to_sud(sections[2]);
			if (!s || !s_sol) {
				std::cout << "Skipping malformed line (puzzle/solution doesn't have exactly "
					<< side_len * side_len << " cells)\n";
				continue;
			}

			const bool tr = add_to_coll(sud_map, desc, *s, *s_sol);
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
			// Find Level (the first comma-separated field of desc)
			const sud_char_t & desc = x.first;
			const std::vector<sud_and_sol_t> sas = x.second;
			const rec_depth_t lvl = std::stoi(desc.substr(0, desc.find(',')));

			// Add to collection, skipping levels this fixed-size bucket array can't hold
			if (lvl < 0 || lvl >= max_lvl) continue;
			lvl_sep_map[lvl][desc] = sas;
		}

		// Save to separate files
		for (rec_depth_t i = 0; i < max_lvl; ++i) {
			std::string f_name = default_file_dir() + "ext_lvl_" + std::to_string(i) + ".txt";
			save_coll(lvl_sep_map[i], f_name);
		}
	}

	/// Removes digits in random order as long as the solution stays unique.
	///
	/// A digit that cannot be removed now never can be later (fewer digits only
	/// allow more solutions), so a single pass over all digits ends in a
	/// minimal puzzle: none of its digits can be removed any more.
	///
	/// @param s_data A sudoku with a unique solution, minimized in place.
	template<class rng>
	static void minimize(sudoku_data_t & s_data, rng & gen) {
		std::vector<sudoku_size_t> cells;
		for (sudoku_size_t i = 0; i < tot_num_cells; ++i) {
			if (s_data[i * n_stored_per_cell] > 0) cells.push_back(i);
		}
		std::ranges::shuffle(cells, gen);
		for (const sudoku_size_t cell : cells) {
			sudoku_data_t candidate = s_data;
			candidate[cell * n_stored_per_cell] = 0;
			auto_fill<square_height, square_width>(candidate, true);
			sudoku_data_t candidate_copy = candidate;
			if (Solver::solve_count_rec_depth(candidate_copy) >= 0) s_data = candidate;
		}
	}

	/// Local search for harder puzzles: moves one clue to another cell and
	/// keeps the change if the solution stays unique and the level does not
	/// drop. Every puzzle of at least \p min_level that is reached is added to the
	/// collection.
	///
	/// Random clue removal almost never produces puzzles of the highest
	/// levels, but they are close to each other in this neighbourhood.
	///
	/// @param s The puzzle to start from.
	/// @param s_sol Its solution.
	/// @param lvl Its level.
	/// @param steps Number of moves to try.
	/// @return The highest level reached.
	template<class rng>
	static rec_depth_t climb(sud_coll_t & sud_map, std::array<num_sud_t, side_len + 1> & lvl_count, const num_sud_t max_suds_per_lvl, const rec_depth_t min_level,
			raw_sudoku_t s, const raw_sudoku_t & s_sol, rec_depth_t lvl, const int steps, rng & gen) {
		std::uniform_int_distribution<sudoku_size_t> cell_dis(0, tot_num_cells - 1);
		for (int step = 0; step < steps; ++step) {
			raw_sudoku_t cand = s;
			sudoku_size_t rem = cell_dis(gen);
			sudoku_size_t add = cell_dis(gen);
			if (cand[rem] == 0 || cand[add] != 0) continue;
			cand[rem] = 0;
			cand[add] = s_sol[add];

			sudoku_data_t data = init_sudoku_with_raw<square_height, square_width>(cand);
			auto_fill<square_height, square_width>(data, true);
			sudoku_data_t data_copy = data;
			if (Solver::solve_count_rec_depth(data_copy) < 0) continue; // Not unique
			rec_depth_t new_lvl = Solver::solve_min_rec_depth(data);
			if (new_lvl < lvl) continue;

			// Only minimal puzzles are collected. Removing digits can change
			// the level, the minimal puzzle is only used if it did not drop.
			sudoku_data_t minimal = data;
			minimize(minimal, gen);
			const rec_depth_t minimal_lvl = Solver::solve_min_rec_depth(minimal);
			const bool use_minimal = minimal_lvl >= new_lvl;
			if (use_minimal) {
				data = minimal;
				new_lvl = minimal_lvl;
			}

			s = get_raw_sudoku<square_height, square_width>(data);
			lvl = new_lvl;
			if (use_minimal && lvl >= min_level && lvl <= static_cast<rec_depth_t>(side_len) && lvl_count[lvl] < max_suds_per_lvl) {
				const sud_char_t desc = generate_sud_char(s, lvl);
				if (add_to_coll(sud_map, desc, s, s_sol)) {
					std::cout << "Climbed to hard Sudoku, level: " << lvl << ", With ID: " << desc << "\n";
					lvl_count[lvl]++;
				}
			}
		}
		return lvl;
	}

	/// Generate hard Sudokus and save them to the disk.
	///
	/// @param max_suds_per_lvl Stop collecting more puzzles for a difficulty
	///   level once there are this many for it in the collection, counting
	///   those loaded from \p output_path (a loaded collection may already
	///   have more, then nothing is added for that level).
	/// @param min_level Minimum difficulty level a puzzle must have to be kept.
	/// @param max_iterations Number of distinct fully-solved grids to try
	///   generating puzzles from before stopping.
	/// @param output_path File to load existing puzzles from and save to.
	///   Defaults to default_file_path() (./Data/<H>x<W>/dat.txt) when empty.
	/// @param climb_steps Moves of the local search started from each puzzle
	///   of at least \p climb_from_level, see climb(). 0 disables it.
	/// @param climb_from_level Minimum level of puzzles to climb from.
	static void generate_hard_sudokus(const num_sud_t max_suds_per_lvl = 1000, const rec_depth_t min_level = 3, const int max_iterations = 50000, std::string output_path = "", const int climb_steps = 2000, const rec_depth_t climb_from_level = 1) {
		if (output_path.empty()) output_path = default_file_path();

		// Initialize
		std::array<num_sud_t, side_len + 1> lvl_count{};
		sud_coll_t sud_map = load_coll(output_path);

		// Puzzles that are already in the collection count towards the maximum
		for (const auto & [desc, sas] : sud_map) {
			const rec_depth_t lvl = std::stoi(desc.substr(0, desc.find(',')));
			if (lvl >= 0 && lvl <= static_cast<rec_depth_t>(side_len)) {
				lvl_count[lvl] += static_cast<num_sud_t>(sas.size());
			}
		}
		const std::array<num_sud_t, side_len + 1> lvl_count_loaded = lvl_count;
		std::mt19937 gen = std::mt19937(seed);

		for (int k = 0; k < max_iterations; ++k) {

			// Generate full sudoku
			raw_sudoku_t zero_sudoku{};
			sudoku_data_t sudoku = init_sudoku_with_raw<square_height, square_width>(zero_sudoku);
			auto_fill<square_height, square_width>(sudoku, true);
			Solver::solve_brute_force_multiple_random(sudoku, gen);
			const raw_sudoku_t raw_s_sol = get_raw_sudoku<square_height, square_width>(sudoku);
			sudoku_data_t sudoku_solution_copy = sudoku;

			for (int l = 0; l < 100; ++l) {
				sudoku = sudoku_solution_copy;

				// Remove digits randomly. Scaled to match the original
				// hardcoded 45-of-81 ratio for the default 9x9 board.
				const sudoku_size_t n_init = tot_num_cells * 5 / 9;
				for (sudoku_size_t i = 0; i < n_init; ++i) {
					sudoku_size_t remove_ind = std::rand() % (tot_num_cells - i);
					remove_nth(sudoku, remove_ind);
				}
				auto_fill<square_height, square_width>(sudoku, true);

				// The random removal above may already have lost uniqueness
				sudoku_data_t sudoku_copy = sudoku;
				if (Solver::solve_count_rec_depth(sudoku_copy) < 0) continue;

				// Remove the remaining digits until the puzzle is minimal, and
				// rate it. The depth depends on which cell is guessed, so use the
				// shallowest search tree.
				minimize(sudoku, gen);
				const rec_depth_t rec_dep = Solver::solve_min_rec_depth(sudoku);
				if (rec_dep >= min_level && rec_dep <= static_cast<rec_depth_t>(side_len)) {
					const num_sud_t n_sud_w_lvl = lvl_count[rec_dep];
					if (n_sud_w_lvl < max_suds_per_lvl) {
						const raw_sudoku_t raw_sud = get_raw_sudoku<square_height, square_width>(sudoku);
						const sud_char_t desc = generate_sud_char(raw_sud, rec_dep);
						bool added = add_to_coll(sud_map, desc, raw_sud, raw_s_sol);
						if (added) {
							std::cout << "Added hard Sudoku :D, level: " << rec_dep;
							std::cout << ", With ID: " << desc << "\n";
							lvl_count[rec_dep]++;
						}
					}
				}
				if (climb_steps > 0 && rec_dep >= climb_from_level) {
					climb(sud_map, lvl_count, max_suds_per_lvl, min_level,
						get_raw_sudoku<square_height, square_width>(sudoku), raw_s_sol, rec_dep, climb_steps, gen);
				}
			}
			if ((k + 1) % 200 == 0) {
				std::cout << "Iteration: " << k + 1 << ", Saving...\n";
				save_coll(sud_map, output_path);
			}
		}

		// The periodic saving above misses the last iterations
		save_coll(sud_map, output_path);
		std::cout << "Finished! Saved to " << output_path << "\n";

		// Frequency count: puzzles per level in the collection
		std::cout << "Level | In collection | Added in this run\n";
		num_sud_t tot_in_coll = 0;
		num_sud_t tot_added = 0;
		for (sudoku_size_t lvl = 0; lvl <= side_len; ++lvl) {
			if (lvl_count[lvl] == 0) continue;
			const num_sud_t added = lvl_count[lvl] - lvl_count_loaded[lvl];
			std::cout << std::setw(5) << lvl << " | " << std::setw(13) << lvl_count[lvl] << " | " << std::setw(17) << added << "\n";
			tot_in_coll += lvl_count[lvl];
			tot_added += added;
		}
		std::cout << "Total | " << std::setw(13) << tot_in_coll << " | " << std::setw(17) << tot_added << "\n";
	}
};
