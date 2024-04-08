// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"

int main()
{
	srand((unsigned int)time(NULL));

	std::string Filename1 = "Grammar1.txt";
	std::string Filename2 = "Grammar2.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	std::set<std::string> words;

	bool IsDebug = 1;
	bool ShowPath = 1;

	int size1 = 100;
	int size2 = 100;

	grammar1.ReadFromFile(Filename1);
	grammar2.ReadFromFile(Filename2);

	grammar1.PrintGrammar(IsDebug, ShowPath);
	grammar2.PrintGrammar(IsDebug, ShowPath);
	Timer timer;


	/*
	std::cout << std::endl << "Generating words in 1 grammar..." << std::endl;
	timer.reset();
	grammar1.GenerateMultipleWords(size1, 50);
	grammar1.PrintWords(IsDebug);
	std::cout << "Generation took: " << timer.elapsed() << std::endl;


	std::cout << std::endl << "Generating words in 2 grammar..." << std::endl;
	timer.reset();
	grammar2.GenerateMultipleWords(size2, 50);
	grammar2.PrintWords(IsDebug);
	std::cout << "Generation took: " << timer.elapsed() << std::endl;//*/

	//words = grammar1.GetWords();
	//std::cout << std::endl << "First grammar words in second grammar test" << std::endl;
	//for (std::string i_string : words)
	//{
	//	/*IsDebug = grammar2.CYK_Alg(i_string);
	//	if (!IsDebug)
	//		std::cout << "Word " << i_string << ", is " << IsDebug << std::endl;*/
	//	std::cout << "Word " << i_string << ", is " << grammar2.CYK_Alg(i_string) << std::endl;
	//}

	//words = grammar2.GetWords();
	//std::cout << std::endl << "Second grammar words in first grammar test" << std::endl;
	//for (std::string i_string : words)
	//{
	//	/*IsDebug = grammar1.CYK_Alg(i_string);
	//	if (!IsDebug)
	//		std::cout << "Word " << i_string << ", is " << IsDebug << std::endl;*/
	//	std::cout << "Word " << i_string << ", is " << grammar1.CYK_Alg(i_string) << std::endl;
	//}

	std::string test = "bbbaaa";
	//std::cout << "Word " << test << ", is " << grammar1.CYK_Alg(test) << std::endl;
	return 0;
}