// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"

int main()
{
	srand(time(NULL));

	std::string Filename1 = "Grammar1.txt";
	std::string Filename2 = "Grammar2.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	std::string word;
	bool IsDebug = 0;

	grammar1.ReadFromFile(Filename1);
	grammar1.PrintGrammar(IsDebug);

	for (int i = 0; i < 30; i++)
	{
		word = grammar1.GenerateWord(100);
		std::cout << "word = " << word << ", length = " << word.length() << std::endl;
	}

	grammar2.ReadFromFile(Filename2);
	grammar2.PrintGrammar(IsDebug);

	for (int i = 0; i < 30; i++)
	{
		word = grammar2.GenerateWord(100);
		std::cout << "word = " << word << ", length = " << word.length() << std::endl;
	}
	return 0;
}