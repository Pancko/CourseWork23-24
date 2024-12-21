// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"

int main()
{
	srand((unsigned int)time(NULL));

	std::string Filename1 = "Grammar14.txt";
	std::string Filename2 = "Grammar15.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	bool IsDebug = 0;
	bool ShowPath = 0;

	grammar1.ReadFromFile(Filename1);
	grammar2.ReadFromFile(Filename2);

	std::cout << std::endl << "Grammar 1:" << std::endl;
	grammar1.PrintGrammar(IsDebug, ShowPath);
	std::cout << std::endl << "Grammar 2:" << std::endl;
	grammar2.PrintGrammar(IsDebug, ShowPath);

	EquivalenceTest(grammar1, grammar2, 21);
	
	return 0;
}