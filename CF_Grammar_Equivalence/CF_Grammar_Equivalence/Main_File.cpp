// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"

int main()
{
	srand((unsigned int)time(NULL));

	std::string Filename1 = "Grammar4.txt";
	std::string Filename2 = "Grammar3.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	std::set<std::string> words;

	bool IsDebug = 1;
	bool ShowPath = 0;

	bool temp_bool = false;
	float temp_float = 0;
	float grammar1_2 = 0;
	float grammar2_1 = 0;
	std::vector<std::string> incorrect_words;

	int size1 = 100;
	int size2 = 100;

	grammar1.ReadFromFile(Filename1);
	grammar2.ReadFromFile(Filename2);

	grammar1.PrintGrammar(IsDebug, ShowPath);
	grammar2.PrintGrammar(IsDebug, ShowPath);
	Timer timer;

	std::cout << std::endl << "Generating words in 1 grammar..." << std::endl;
	timer.reset();
	grammar1.GenerateMultipleWords(size1, 50);
	grammar1.PrintWords(IsDebug);
	std::cout << "Generation took: " << timer.elapsed() << std::endl;


	std::cout << std::endl << "Generating words in 2 grammar..." << std::endl;
	timer.reset();
	grammar2.GenerateMultipleWords(size2, 50);
	grammar2.PrintWords(IsDebug);
	std::cout << "Generation took: " << timer.elapsed() << std::endl;

	words = grammar1.GetWords();
	std::cout << std::endl << "First grammar words in second grammar test" << std::endl;
	for (std::string i_string : words)
	{
		temp_bool = grammar2.CYK_Alg_Modified(i_string);
		if (!temp_bool)
			incorrect_words.push_back(i_string);
		else
			temp_float++;
	}
	if (incorrect_words.size() > 0)
	{
		std::cout << "Second grammar can't produce these words:" << std::endl;
		for (std::string i_string : incorrect_words)
			std::cout << i_string << std::endl;
	}
	else
		std::cout << "Second grammar can produce all words!" << std::endl;
	grammar1_2 = 100 * temp_float / words.size();
	temp_float = 0;
	incorrect_words.clear();

	words = grammar2.GetWords();
	std::cout << std::endl << "Second grammar words in first grammar test" << std::endl;
	for (std::string i_string : words)
	{
		temp_bool = grammar1.CYK_Alg_Modified(i_string);
		if (!temp_bool)
			incorrect_words.push_back(i_string);
		else
			temp_float++;
	}
	if (incorrect_words.size() > 0)
	{
		std::cout << "First grammar can't produce these words:" << std::endl;
		for (std::string i_string : incorrect_words)
			std::cout << i_string << std::endl;
	}
	else
		std::cout << "First grammar can produce all words!" << std::endl;
	grammar2_1 = 100 * temp_float / words.size();

	std::cout << std::endl;
	std::cout << "Grammar 2 can replicate " << grammar1_2 << "% of grammar 1 words" << std::endl;
	std::cout << "Grammar 1 can replicate " << grammar2_1 << "% of grammar 2 words" << std::endl;
	return 0;
}