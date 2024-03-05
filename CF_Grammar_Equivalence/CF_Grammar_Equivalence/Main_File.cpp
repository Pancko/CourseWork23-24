// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"
#include "Timer.h"

int main()
{
	srand(time(NULL));

	std::string Filename1 = "Grammar1.txt";
	std::string Filename2 = "Grammar2.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	std::set<std::string> words;

	bool IsDebug = 1;
	bool ShowPath = 1;

	int size1 = 46;
	int size2 = 127;

	grammar1.ReadFromFile(Filename1);
	grammar2.ReadFromFile(Filename2);

	grammar1.PrintGrammar(IsDebug, ShowPath);
	grammar2.PrintGrammar(IsDebug, ShowPath);
	Timer timer;


	std::cout << "Generating words in 1 grammar..." << std::endl;
	timer.reset();
	grammar1.GenerateMultipleWords(size1, 50);
	grammar1.PrintWords(IsDebug);
	std::cout << "Generated " << size1 << " words 1 grammar: " << timer.elapsed() << std::endl;


	std::cout << "Generating words in 2 grammar..." << std::endl;
	timer.reset();
	grammar2.GenerateMultipleWords(size2, 50);
	grammar2.PrintWords(IsDebug);
	std::cout << "Generated " << size2 << " words 2 grammar: " << timer.elapsed() << std::endl;

	words = grammar1.GetWords();

	// Тут пока что всё очень плохо
	// 
	//std::string test = "a";
	//std::cout << "Word " << test << ", size = " << test.size() << ", is " << grammar1.CYK_Alg(test) << std::endl;

	//for (std::string i_string : words)
	{
		//std::cout << "Word " << i_string << ", is " << grammar1.CYK_Alg(i_string) << std::endl;
	}

	return 0;
}