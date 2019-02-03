#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include "board.h"
#include "solver.h"

int main(int argc, const char* argv[]) {

	std::fstream FILEIN, FILEOUT;
	FILEIN.open(argv[1],std::ios::in);
	FILEOUT.open(argv[2], std::ios::out);

	std::cout << "Threes-Demo: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;
	std::string solve_args;
	int precision = 10;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0) {
			solve_args = para.substr(para.find("=") + 1);
		} else if (para.find("--precision=") == 0) {
			precision = std::stol(para.substr(para.find("=") + 1));
		}
	}
	solver solve(solve_args);

	board state;
	state_type type;
	state_hint hint(state);
	std::cout << std::setprecision(precision);

	while (FILEIN >> type >> state >> hint) {
		auto value = solve.solve(state, type);
		FILEOUT << type << " " << state << " " << hint;
		FILEOUT << " = " << value << std::endl;
	}
	FILEIN.close();
	FILEOUT.close();
	return 0;
}
