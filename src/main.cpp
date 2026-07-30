#include <iostream>
#include <string>
#include <vector>

#include "MathEI/mathi.h"
#include "MathEI/errors.h"

int main() {
	MathI mathi;

	// ERROR: f(x) = (a = x)

	// TODO: 
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
			mathi.eval(str); 
		}
		catch (const MathIError& err) {
			std::cout << err.what();
		}
	}

	return 0;
}