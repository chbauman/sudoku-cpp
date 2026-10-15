// Sudoku.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include "SudokuCore.h"
#include "SudokuSolver.h"
#include "SudokuGenerator.h"

#include <string>
#include <iostream>
#include <random>

/// The main function.
///
/// It executes everything that is needed.
int main(int argc, char* argv[]){


	const raw_sudoku_t<3, 3> input_sudoku_3x3 = {
		6, 0, 0, 0, 0, 8, 9, 4, 0,
		9, 0, 0, 0, 0, 6, 1, 0, 0,
		0, 7, 0, 0, 4, 0, 0, 0, 0,
		2, 0, 0, 6, 1, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0, 2, 0, 0, 
		0, 8, 9, 0, 0, 2, 0, 0, 0, 
		0, 0, 0, 0, 6, 0, 0, 0, 5,
		0, 0, 0, 0, 0, 0, 0, 3, 0, 
		8, 0, 0, 0, 0, 1, 6, 0, 0
	};
	const raw_sudoku_t<3, 3> easy_sudoku_3x3 = {
		0,0,4,0,8,9,5,7,0,
		0,0,0,7,1,0,6,3,4,
		5,0,0,4,6,3,0,0,0,
		9,3,0,0,0,0,2,0,0,
		6,0,0,9,0,1,0,0,3,
		0,0,2,0,0,0,0,9,5,
		0,0,0,1,5,2,0,0,9,
		8,5,3,0,9,4,0,0,0,
		0,2,9,8,3,0,4,0,0
	};
	const raw_sudoku_t<3, 3> hard_sudoku_3x3 = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,4,6,2,0,0,1,
		0,0,0,1,0,0,3,4,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,2,0,6,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,1,0,0,4,0,0,0,
		2,0,0,5,8,7,0,0,0,
		9,0,0,0,0,0,0,0,8
	};	
	const raw_sudoku_t<3, 3> hardest_sudoku_3x3 = {
	8, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 3, 6, 0, 0, 0, 0, 0,
	0, 7, 0, 0, 9, 0, 2, 0, 0,

	0, 5, 0, 0, 0, 7, 0, 0, 0,
	0, 0, 0, 0, 4, 5, 7, 0, 0,
	0, 0, 0, 1, 0, 0, 0, 3, 0,

	0, 0, 1, 0, 0, 0, 0, 6, 8,
	0, 0, 8, 5, 0, 0, 0, 1, 0,
	0, 9, 0, 0, 0, 0, 4, 0, 0
	};	

	const raw_sudoku_t<3, 3> only_53_solutions_sudoku_3x3 = {
		5,0,0,0,0,0,0,0,7,
		0,0,0,4,6,2,0,0,1,
		0,0,0,1,0,0,3,4,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,2,0,6,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,1,0,0,4,0,0,0,
		2,0,0,0,8,7,0,0,0,
		9,0,0,0,0,0,0,0,8
	};
	const raw_sudoku_t<3, 3> many_11199_solutions_sudoku_3x3 = {
		5,0,0,0,0,0,0,0,7,
		0,0,0,4,0,2,0,0,1,
		0,0,0,1,0,0,0,4,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,0,0,6,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,1,0,0,4,0,0,0,
		2,0,0,5,8,7,0,0,0,
		9,0,0,0,0,0,0,0,8
	};
	const raw_sudoku_t<3, 3> many_177859_solutions_sudoku_3x3 = {
		5,0,0,0,0,0,0,0,7,
		0,0,0,4,0,2,0,0,1,
		0,0,0,1,0,0,0,4,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,0,0,6,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,0,0,0,4,0,0,0,
		2,0,0,0,8,7,0,0,0,
		9,0,0,0,0,0,0,0,8
	};
	const raw_sudoku_t<3, 3> too_maaaaany_sol_sudoku_3x3 = {
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,2,0,0,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,0,0,0,4,0,0,0,
		2,0,0,5,8,0,0,0,0,
		9,0,0,0,0,0,0,0,8
	};// 8 Solutions
	const raw_sudoku_t<3, 3> zero_sudoku_3x3 = {
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

	const raw_sudoku_t<2, 3> input_sudoku_2x3 = {
		0, 1, 4, 0, 5, 0,
		2, 0, 5, 1, 3, 0, 
		0, 0, 3, 0, 6, 0, 
		0, 4, 0, 3, 0, 0,
		0, 5, 1, 6, 0, 3,
		0, 3, 0, 5, 4, 0
	};
	const auto input_sudoku = easy_sudoku_3x3;
	print_raw_sudoku<3, 3>(std::cout, input_sudoku);
	std::cout << "\n";
	sudoku_data_t<3, 3> sudoku = init_sudoku_with_raw<3, 3>(input_sudoku);
	auto_fill<3, 3>(sudoku, true);
	raw_sudoku_t<3, 3> raw_sud = get_raw_sudoku<3, 3>(sudoku);

	std::cout << SudokuSolver<3, 3>::solve_brute_force_all(sudoku) << " Solutions\n";
	sudoku = init_sudoku_with_raw<3, 3>(input_sudoku);
	auto_fill<3, 3>(sudoku, true);
	std::cout << SudokuSolver<3, 3>::solve_count_rec_depth(sudoku) << " Min. Recursion Depth\n";

	raw_sud = get_raw_sudoku<3, 3>(sudoku);
	print_raw_sudoku<3, 3>(std::cout, raw_sud);
	std::cout << "\n";

	SudokuGenerator<3, 3>::generate_hard_sudokus();

	//auto s_map = SudokuGenerator<3, 3>::load_coll("./Data/dat_copy.txt");
	//SudokuGenerator<3, 3>::separate_by_level_and_save(s_map);

}