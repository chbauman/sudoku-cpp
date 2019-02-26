// Sudoku.cpp : Diese Datei enthält die Funktion "main". Hier beginnt und endet die Ausführung des Programms.
//

#include "pch.h"
#include "Lib.h"

#include <string>
#include <iostream>
#include <random>

int main()
{
	const raw_sudoku_t input_sudoku_3x3 = {
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
	const raw_sudoku_t easy_sudoku_3x3 = {
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
	const raw_sudoku_t hard_sudoku_3x3 = {
		5,0,0,0,0,0,0,0,7,
		0,0,0,4,6,2,0,0,1,
		0,0,0,1,0,0,3,4,0,
		0,0,0,0,4,0,1,0,0,
		0,0,0,2,0,6,0,0,0,
		0,0,8,0,3,0,0,0,0,
		0,5,1,0,0,4,0,0,0,
		2,0,0,5,8,7,0,0,0,
		9,0,0,0,0,0,0,0,8
	};

	const raw_sudoku_t input_sudoku_2x3 = {
		0, 1, 4, 0, 5, 0,
		2, 0, 5, 1, 3, 0, 
		0, 0, 3, 0, 6, 0, 
		0, 4, 0, 3, 0, 0,
		0, 5, 1, 6, 0, 3,
		0, 3, 0, 5, 4, 0
	};
	const auto input_sudoku = input_sudoku_3x3;
	std::cout << input_sudoku << "\n";
	sudoku_data_t sudoku = init_sudoku_with_raw(input_sudoku);
	auto_fill(sudoku, true);
	std::cout << sudoku << "\n";
	std::cout << try_solving(sudoku) << "\n";

	raw_sudoku_t raw_sud = get_raw_sudoku(sudoku);
	std::cout << raw_sud << "\n";

	RandomNumberPicker rnp = RandomNumberPicker<square_height, square_width>();
	std::cout << solve_brute_force<3, 3>(sudoku, rnp);

	raw_sud = get_raw_sudoku(sudoku);
	std::cout << raw_sud << "\n";

}