#include <iostream>
#include <string>
#include <vector>
#include "MathEI/mathi.h"

int main() {
	MathI mathi;

	// ERROR: eq(x, y) = (x == y)

	while (1) {
		std::string str;
		std::cout << ">> ";
		std::getline(std::cin, str);

		std::cout << std::to_string(mathi.eval(str)) << std::endl << std::endl;
	}

	return 0;
}