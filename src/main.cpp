#include <iostream>
#include <string>
#include <vector>

#include "MathEI/mathi.h"
#include "MathEI/errors.h"

int main() {
	MathI mathi;

	// ERROR: f(x) = (a = x)
	// ERROR: (1)-1
	
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
		
		mathi.eval(str); 

		if (!mathi.ok())
			std::cout << std::endl << mathi.get_error();
	}

	return 0;
}