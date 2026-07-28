#include <iostream>
#include <string>
#include <vector>
#include "MathEI/mathi.h"

int main() {
	MathI mathi;

	// ERROR: f(x) = (a = x)

	while (1) {
		std::string str;
		std::cout << ">> ";
		std::getline(std::cin, str);

		if (str == ".clear") {
			__debugbreak();
			system("cls");
			continue;
		}
		
		mathi.eval(str);
	}

	return 0;
}