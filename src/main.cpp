#include <iostream>
#include <string>
#include <vector>

#include "MathEI/mathi.h"
#include "MathEI/errors.h"

int main() {
	MathI mathi;
	
	// TODO: 
	// * unique_ptr
	// * user_decl functions
	//

	for (std::string str; std::cout << ">> ", std::getline(std::cin, str);) {
		if (str == ".clear") {
			system("cls");
			continue;
		}
		
		mathi.eval(str);

		while (!mathi.ok())
			std::cout << std::endl << mathi.get_error();
	}

	return 0;
}