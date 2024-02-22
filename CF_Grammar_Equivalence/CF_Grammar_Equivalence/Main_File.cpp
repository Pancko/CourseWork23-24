#include "CF_Grammar.h"

int main()
{
	srand(time(NULL));

	std::string Filename1 = "Grammar1.txt";
	std::string Filename2 = "Grammar2.txt";

	CF_Grammar grammar1;
	CF_Grammar grammar2;

	grammar1.ReadFromFile(Filename1);
	grammar1.PrintGrammar();

	grammar2.ReadFromFile(Filename2);
	grammar2.PrintGrammar();

	return 0;
}