#include <iostream>
#include <string>
#include <vector>
#include "MathEI/mathi.h"

int main() {
	MathI mathi;

	while (1) {
		std::string str;
		std::cout << ">> ";
		std::getline(std::cin, str);

		std::cout << mathi.eval(str) << std::endl;
	}

	return 0;
}