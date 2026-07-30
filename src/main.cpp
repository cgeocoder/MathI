#include <iostream>
#include <string>
#include <vector>

#include "MathEI/mathi.h"

int main() {
	MathI mathi;

	// ERROR: f(x) = (a = x)

	// TODO: 
	// * Error handling
	// * unique_ptr
	// * user_decl functions
	//

	while (1) {
		std::string str;
		std::cout << ">> ";
		std::getline(std::cin, str);

		if (str == ".clear") {
			system("cls");
			continue;
		}
		
		try {
			mathi.eval(str); __debugbreak();
		}
		catch (const std::exception& ex) {
			__debugbreak();
			std::cout << ex.what() << '\n';
		}
	}

	return 0;
}