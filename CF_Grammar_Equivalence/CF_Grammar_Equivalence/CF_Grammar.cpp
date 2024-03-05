// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#include "CF_Grammar.h"

CF_Grammar::CF_Grammar()
{
}

CF_Grammar::~CF_Grammar()
{
	non_terminals.clear();
	terminals.clear();
	rules.clear();
}

//=============== Считывание грамматики из файла и создание объекта =====================================

void CF_Grammar::ReadFromFile(const std::string& File_Name)
{
	std::ifstream file(File_Name);
	std::string current_string;
	std::string sub_string;
	Rule current_rule;
	size_t position;

	while (std::getline(file, current_string))
	{
		// Добавить несколько правил, если они написаны через '|'
		if (current_string.find('|') != std::string::npos)
		{
			position = current_string.find("->") + 2;
			current_rule.left_part = current_string.substr(0, position - 2);
			sub_string = current_string.substr(position);

			while (true)
			{
				position = sub_string.find('|');
				if (position != std::string::npos)
				{
					current_string = current_rule.left_part + "->" + sub_string.substr(0, position);
					sub_string = sub_string.substr(position + 1);
					current_rule = GetRuleFromString(current_string);
					AddRule(current_rule);
				}
				else
				{
					current_string = current_rule.left_part + "->" + sub_string;
					current_rule = GetRuleFromString(current_string);
					AddRule(current_rule);
					current_string.clear();
					break;
				}
			}
		}
		else
		{
			current_rule = GetRuleFromString(current_string);
			AddRule(current_rule);
			current_string.clear();
		}
	}
	file.close();
	AnalyzeNonTerminals();
}

Rule CF_Grammar::GetRuleFromString(const std::string& String)
{
	Rule result;
	std::string current_string;
	size_t position = String.find("->");
	result.left_part = String.substr(0, position);
	current_string = String.substr(position + 2);

	if (starting_non_terminal.length() == 0)
		starting_non_terminal = result.left_part;

	size_t i_char = 0;

	while (true)
	{
		if (current_string[i_char] == '[')
		{
			result.right_part.push_back(current_string.substr(i_char, current_string.substr(i_char).find(']') + 1));
			i_char += current_string.substr(i_char).find(']');
		}
		else
			result.right_part.push_back(current_string.substr(i_char, 1));
		i_char++;
		if (i_char == current_string.length())
			break;
	}

	return result;
}

void CF_Grammar::AddRule(const Rule& New_Rule)
{
	Rule rule_to_add = New_Rule;
	std::vector<Path> null_path;

	if (!non_terminals.contains(New_Rule.left_part))
	{
		non_terminals.insert(std::map<std::string, std::vector<Path>>::value_type(New_Rule.left_part, NULL));
	}
	for (std::string i_string : New_Rule.right_part)
	{
		if (i_string == "[EPS]")
		{
			if (!terminals.contains(i_string))
				terminals.insert(i_string);
		}
		else if (i_string[0] == '[' || isupper(i_string[0]))
		{
			if (!non_terminals.contains(i_string))
				non_terminals.insert(std::map<std::string, std::vector<Path>>::value_type(i_string, NULL));
		}
		else
		{
			rule_to_add.terminals_count++;
			if (!terminals.contains(i_string))
				terminals.insert(i_string);
		}
	}
	rules.push_back(rule_to_add);
}

//=============== Анализ полученной грамматики, составление путей для нетерминалов ======================

void CF_Grammar::AnalyzeNonTerminals()
{
	GeneratePathes();

	FindingBadNonTerminals();

	DeleteBadNonTerminals();

	FillShortestPathes();
}

void CF_Grammar::GeneratePathes()
{
	Path current_path;
	bool got_wrong_non_terminal;
	size_t searching_range = 3;//std::max(2, int(non_terminals.size() / 3));

	// Базис: заносим правила, которые уже создают терминальные слова
	for (Rule i_rule : rules)
	{
		// Добавляем правило в путь
		if (!GotNonTerminal(i_rule.right_part))
		{
			if (i_rule.right_part[0] == "[EPS]")
				current_path.length = 0;
			else
				current_path.length = 1;
			current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
			current_path.path_rules.emplace(current_path.path_rules.begin(), i_rule);
			current_path.word = i_rule.right_part;
			current_path.path_words.push_back(current_path.word);

			non_terminals[i_rule.left_part].push_back(current_path);

			current_path.path_rules.clear();
			current_path.path_words.clear();
		}
		i_rule.~Rule();
	}

	// Рекуррентный шаг: применяем к правилам пути, приводящие к терминальным словам
	for (int i = 0; i < searching_range; i++)
	{
		std::cout << "Generating pathes " << i * (100 / searching_range) << " ~ " << (i + 1) * (100 / searching_range) << "%" << std::endl;
		for (Rule i_rule : rules)
		{
			got_wrong_non_terminal = false;
			for (std::string i_string : i_rule.right_part)
			{
				if (i_string == i_rule.left_part)
				{
					got_wrong_non_terminal = true;
					break;
				}
				if (non_terminals.contains(i_string) && non_terminals[i_string].size() == 0)
				{
					got_wrong_non_terminal = true;
					break;
				}
			}

			if (!got_wrong_non_terminal && GotNonTerminal(i_rule.right_part))
			{
				current_path.length = 1;
				current_path.path_rules.push_back(i_rule);
				current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
				current_path.path_words.push_back(i_rule.right_part);
				current_path.word = i_rule.right_part;

				if (GotNonTerminal(current_path))
					GenerateSubPath(current_path);

				current_path.path_rules.clear();
				current_path.path_words.clear();
			}
		}
	}
}

bool CF_Grammar::GenerateSubPath(const Path& Current_Path)
{
	Path current_path = Current_Path;
	std::map<std::string, std::vector<Path>> old_pathes = non_terminals;
	bool new_path_found = false;

	for (std::string i_string : Current_Path.word)
	{
		if (old_pathes.contains(i_string))
		{
			for (Path i_path : old_pathes[i_string])
			{
				current_path = Current_Path;
				current_path += i_path;

				if (GotNonTerminal(current_path))
				{
					new_path_found += GenerateSubPath(current_path);
				}
				else if (IsUniquePath(current_path, non_terminals))
				{
					non_terminals[current_path.path_rules[0].left_part].push_back(current_path);
					new_path_found = true;
				}
			}
		}
	}

	return new_path_found;

}

bool CF_Grammar::IsUniquePath(const Path& Path_To_Check, std::map<std::string, std::vector<Path>> Current_Pathes)
{
	// Если путь с таким заключительным словом уже существует
	for (Path i_path : Current_Pathes[Path_To_Check.path_rules[0].left_part])
	{
		if (i_path.word == Path_To_Check.word) return false;
	}

	// Если одно и то же правило (кроме опустошающих) применяется больше одного раза
	for (int i = 0; i < Path_To_Check.path_rules.size(); i++)
	{
		for (int j = 0; j < Path_To_Check.path_rules.size(); j++)
		{
			if (i != j && Path_To_Check.path_rules[i] == Path_To_Check.path_rules[j]
				&& Path_To_Check.path_rules[i].right_part[0] != "[EPS]") return false;
		}
	}

	return true;
}

//=============== Поиск и удаление "плохих" нетерминалов, получение кратчайших путей =====================

void CF_Grammar::FindingBadNonTerminals()
{
	bool good_non_terminal = false;
	bool non_terminal_found = false;

	for (auto& i_element : non_terminals)
	{
		good_non_terminal = false;

		for (Path i_path : i_element.second)
		{
			non_terminal_found = false;
			for (std::string i_string : i_path.word)
			{
				if (non_terminals.contains(i_string))
				{
					non_terminal_found = true;
					break;
				}
			}
			if (!non_terminal_found)
			{
				good_non_terminal = true;
				break;
			}
		}
		if (!good_non_terminal)
		{
			bad_non_terminals.insert(i_element.first);
		}
	}
}

void CF_Grammar::DeleteBadNonTerminals()
{
	auto new_non_terminals = non_terminals;
	std::vector<Rule> new_rules = rules;

	for (std::string i_string : bad_non_terminals)
	{
		new_non_terminals.erase(i_string);
		
		for (Rule i_rule : rules)
		{
			if (i_rule.left_part == i_string)
			{
				new_rules.erase(std::remove(new_rules.begin(), new_rules.end(), i_rule), new_rules.end());
			}
			if (std::ranges::find(i_rule.right_part.begin(), i_rule.right_part.end(), i_string) != i_rule.right_part.end())
			{
				new_rules.erase(std::remove(new_rules.begin(), new_rules.end(), i_rule), new_rules.end());
			}
		}
		
		shortest_path.erase(i_string);
		
		for (auto& i_element : non_terminals)
		{
			for (Path i_path : non_terminals[i_element.first])
			{
				if (std::ranges::find(i_path.word.begin(), i_path.word.end(), i_string) != i_path.word.end())
				{
					new_non_terminals[i_element.first].erase(std::remove(new_non_terminals[i_element.first].begin(), 
																			new_non_terminals[i_element.first].end(), i_path), new_non_terminals[i_element.first].end());

					// Пересмотреть длину кратчайшего пути
					if (i_path.length == shortest_path[i_element.first].length)
					{
						shortest_path[i_element.first] = new_non_terminals[i_element.first][0];
						for (auto& j_element : new_non_terminals)
						{
							for (Path j_path : j_element.second)
							{
								if (shortest_path[i_element.first].length > j_path.length)
									shortest_path[i_element.first] = j_path;
							}
						}
					}
				}
			}
		}
	}
	non_terminals.clear();
	non_terminals = new_non_terminals;
	rules.clear();
	rules = new_rules;
}

void CF_Grammar::FillShortestPathes()
{
	for (auto& i_element : non_terminals)
	{
		if (i_element.second.size() > 0)
			shortest_path[i_element.first] = i_element.second[0];
	}
	for (auto& i_element : non_terminals)
	{
		for (Path i_path : i_element.second)
		{
			if (shortest_path[i_element.first].length > i_path.length)
				shortest_path[i_element.first] = i_path;
		}
	}
}

//=============== Анализ путей и слов на наличие в них нетерминалов ======================================

bool CF_Grammar::GotNonTerminal(const Path& Current_Path)
{
	for (std::string i_string : Current_Path.word)
	{
		if (non_terminals.contains(i_string))
		{
			return true;
		}
	}
	return false;
}

bool CF_Grammar::GotNonTerminal(const std::vector<std::string>& Word)
{
	for (std::string i_string : Word)
	{
		if (non_terminals.contains(i_string))
		{
			return true;
		}
	}
	return false;
}

//=============== Печать грамматики ======================================================================

void CF_Grammar::PrintGrammar(bool IsDebug, bool ShowPath)
{
	int counter = 0;

	std::cout << "Grammar:" << std::endl;
	std::cout << "Non-Terminals:" << std::endl;

	for(const auto& i_element: non_terminals)
	{
		std::cout << i_element.first;
		if (counter != non_terminals.size() - 1)
			std::cout << ", ";
		counter++;
	}

	counter = 0;
	std::cout << std::endl << "Terminals:" << std::endl;
	for (std::string i_string : terminals)
	{
		std::cout << i_string;
		if (counter != terminals.size() - 1)
			std::cout << ", ";
		counter++;
	}

	std::cout << std::endl << "Rules:" << std::endl;
	for (Rule i_rule : rules)
	{
		std::cout << std::setw(10) << i_rule.left_part << " -> ";
		for (std::string i_string : i_rule.right_part)
		{
			std::cout << i_string;
		}
		if (IsDebug)
		{
			std::cout << "           || terminals_count: " << i_rule.terminals_count;
		}
		std::cout << std::endl;
	}

	counter = 0;
	std::cout << "Bad non-terminals";
	if (bad_non_terminals.size() > 0)
	{
		std::cout << ": ";
		for (std::string i_string : bad_non_terminals)
		{
			std::cout << i_string;
			if (counter != bad_non_terminals.size() - 1)
				std::cout << ", ";
			counter++;
		}
		std::cout << std::endl;
	}
	else std::cout << " not found" << std::endl;

	counter = 0;

	if (ShowPath)
	{
		std::cout << "Pathes:" << std::endl;
		for (const auto& i_element : non_terminals)
		{
			for (Path i_path : i_element.second)
			{
				i_path.PrintPath();
				counter++;
			}
		}
	}

	std::cout << counter << " pathes" << std::endl;

	counter = 0;

	if (IsDebug)
	{
		for (auto& i_element : shortest_path)
		{
			std::cout << "Shortest path for " << i_element.first << " is " << i_element.second.length << std::endl;
		}
	}
}

//=============== Генерация рандомных слов и взаимодействие со словами ===================================

std::string CF_Grammar::GenerateWord(const int& Max_Length)
{
	std::string result;
	std::vector<std::string> word;
	std::vector<std::string> final_word;
	std::vector<Rule> appliable_rules;
	Path current_word_path;
	int rule_to_use = 0;
	size_t expected_length = 0;
	size_t actual_length = 0;
	bool non_terminal_found = 0;

	// Начальный вид слова
	word.push_back(starting_non_terminal);
	current_word_path.path_words.push_back(std::vector<std::string>({ starting_non_terminal }));
	expected_length = shortest_path[starting_non_terminal].length;
	actual_length = 1;

	// Заполнить список правил, которые можно применить
	for (Rule i_rule : rules)
	{
		if (i_rule.left_part != starting_non_terminal)
			break;
		appliable_rules.push_back(i_rule);
	}

	// Применение случайных правил к случайным нетерминалам
	while (expected_length < Max_Length)
	{
		// Рандомный выбор номера правила
		rule_to_use = rand() % appliable_rules.size();

		// Применение правила
		word = ApplyRule(word, appliable_rules[rule_to_use]);

		current_word_path.path_rules.push_back(appliable_rules[rule_to_use]);
		current_word_path.path_words.push_back(word);
		current_word_path.word = word;
		current_word_path.length++;

		// Проверка наличия нетерминалов в слове
		if (!GotNonTerminal(word)) break;

		// Изменение ожидаемой длины слова
		actual_length += appliable_rules[rule_to_use].right_part.size() - 1;
		expected_length += appliable_rules[rule_to_use].terminals_count;
		expected_length -= shortest_path[appliable_rules[rule_to_use].left_part].length;
		for (std::string i_string : appliable_rules[rule_to_use].right_part)
		{
			if (non_terminals.contains(i_string))
				expected_length += shortest_path[i_string].length;
		}
		if (expected_length == 0 || actual_length > Max_Length) break;

		// Составление нового списка возможных для применения правил
		appliable_rules.clear();

		for (std::string i_string : word)
		{
			if (non_terminals.contains(i_string))
			{
				non_terminal_found = 0;
				for (Rule i_rule : rules)
				{
					if (i_rule.left_part == i_string && (!non_terminal_found || GotNonTerminal(i_rule.right_part)))
					{
						non_terminal_found = 1;
						appliable_rules.push_back(i_rule);
					}
					else if (non_terminal_found && i_rule.left_part != i_string) break;
				}
			}
		}
	}

	// Доведение слова до конца
	final_word = word;
	while (GotNonTerminal(final_word))
	{
		for (std::string i_string : word)
		{
			if (non_terminals.contains(i_string))
			{
				for (Rule i_rule : shortest_path[i_string].path_rules)
				{
					final_word = ApplyRule(final_word, i_rule);

					current_word_path.path_rules.push_back(i_rule);
					current_word_path.path_words.push_back(final_word);
					current_word_path.word = final_word;
					current_word_path.length++;
				}
			}
		}
	}
	word = final_word;

	for (std::string i_string : word)
	{
		result += i_string;
	}

	if (words.contains(result))
	{
		ReGenerateWord(current_word_path);
	}
	else 
		words.insert(result);

	word.~vector();
	final_word.~vector();
	appliable_rules.~vector();

	return result;
}

bool CF_Grammar::ReGenerateWord(const Path& Word)
{
	std::vector<std::string> current_word;
	std::vector<std::string> final_word;
	std::vector<Rule> appliable_rules;
	bool non_terminal_found;
	std::string i_string;
	std::string result_word;

	// Идем в обратном порядке применения правил
	for (int i = Word.path_rules.size() - 2; i >= 0; --i)
	{
		current_word = Word.path_words[i + 1];
		appliable_rules.clear();
		i_string = Word.path_rules[i + 1].left_part;
		non_terminal_found = 0;
		result_word.clear();

		// Создание списка возможных для применения правил, исключая уже примененное
		for (Rule i_rule : rules)
		{
			if (i_rule.left_part == i_string && (!non_terminal_found || GotNonTerminal(i_rule.right_part)))
			{
				non_terminal_found = 1;
				if (i_rule != Word.path_rules[i + 1])
					appliable_rules.push_back(i_rule);
			}
			else if (non_terminal_found && i_rule.left_part != i_string) break;
		}

		// Попытки применить другие правила и получить новые слова
		while (appliable_rules.size() > 0)
		{
			current_word = Word.path_words[i + 1];
			current_word = ApplyRule(current_word, appliable_rules[0]);

			// Доведение слова до конца
			final_word = current_word;
			while (GotNonTerminal(final_word))
			{
				for (std::string k_string : current_word)
				{
					if (non_terminals.contains(k_string))
					{
						for (Path i_path : non_terminals[k_string])
						{
							final_word = current_word;
							for (Rule i_rule : i_path.path_rules)
							{
								final_word = ApplyRule(final_word, i_rule);
							}
						}
					}
				}
				current_word = final_word;
			}

			for (std::string j_string : final_word)
			{
				result_word += j_string;
			}
			if (!words.contains(result_word))
			{
				words.insert(result_word);
				return true;
			}

			appliable_rules.erase(appliable_rules.begin());
		}
	}

	return false;
}

std::vector<std::string> CF_Grammar::GenerateMultipleWords(const int& Amount, const int& Max_Length)
{
	std::vector<std::string> result;

	while (words.size() < Amount)
	{
		result.push_back(GenerateWord(Max_Length));
	}

	return result;
}

void CF_Grammar::PrintWords(bool IsDebug)
{
	if (IsDebug)
		for (std::string i_string : words)
		{
			std::cout << "word = " << i_string << ", length = " << i_string.length() << std::endl;
		}
	std::cout << "Words count = " << words.size() << std::endl;
}

std::set<std::string> CF_Grammar::GetWords()
{
	return words;
}

//=============== Алгоритмы анализа принадлежности слова грамматике =======================================

bool CF_Grammar::CYK_Alg(const std::string& Word)
{
	std::map<std::pair<std::string, std::pair<int, int>>, bool> a;
	std::map<std::pair<std::pair<std::string, std::vector<std::string>>, std::pair<int, std::pair<int, int>>>, bool> h;
	int j = 0;
	std::string i_string;

	std::string test;

	//std::cout << "Word = " << Word << std::endl;

	for (int i = 0; i <= Word.size(); i++)
	{
		if (Word[0] != '\0' && i == Word.size()) break;
		i_string = Word[i];
		//std::cout << "i_string =" << std::distance(i_string.begin(), i_string.end()) << "sss" << std::endl;
		if (Word[i] == '\0') i_string = "[EPS]";
		std::cout << "i_string = " << i_string << std::endl;
		for (Rule i_rule : rules)
		{
			// Если в грамматике присутствует правило A -> Word[i]
			if ((i_rule.right_part.size() == 1) && (i_rule.right_part[0] == i_string))
				a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i + 1))] = true;
			//else
				//if (!a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i + 1))])
					//a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i + 1))] = false;

			// Если в грамматике присутствует правило A -> [EPS]
			if (!GotNonTerminal(i_rule.right_part) && (i_rule.terminals_count == 0))
				a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i))] = true;
			//else
				//if (!a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i))])
					//a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i))] = false;

			std::cout << "a[" << i_rule.left_part << ", " << i << ", " << i << "] = " << a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i))] << std::endl;
			//std::cout << "right_part[0] = " << i_rule.right_part[0] << std::endl;
			std::cout << "a[" << i_rule.left_part << ", " << i << ", " << i + 1 << "] = " << a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, i + 1))] << std::endl;

			h[std::pair<std::pair<std::string, std::vector<std::string>>, std::pair<int, std::pair<int, int>>>
				(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part), std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(i, 0)))] = true;
			///*
			test.clear();
			for (std::string r_string : i_rule.right_part)
			{
				test += r_string;
			}

			std::cout << "h[" << i_rule.left_part << " -> " << test << ", " << i << ", " << i << ", 0] = " <<
				h[std::pair<std::pair<std::string, std::vector<std::string>>, std::pair<int, std::pair<int, int>>>
				(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part), std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(i, 0)))] << std::endl;//*/
		}
	}
	//std::cout << std::endl << "first" << std::endl;
	for (int m = 0; m <= Word.size(); m++)
	{
		if (Word[0] != '\0' && m == Word.size()) break;
		for (int i = 0; i <= Word.size(); i++)
		{
			if (Word[0] != '\0' && i == Word.size()) break;
			j = i + m;
			//for (int k = 0; k <= m; k++)
			{
				for (Rule i_rule : rules)
				{
					for (int k = 0; k < i_rule.right_part.size(); k++)
					{
						//if (i_rule.right_part[0] != "[EPS]" && k == i_rule.right_part.size()) break;
						for (int r = 0; r <= j + 1; r++)
						{
							h[std::pair<std::pair<std::string, std::vector<std::string>>,
								std::pair<int, std::pair<int, int>>>
								(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part),
									std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(j + 1, k)))] +=

								h[std::pair<std::pair<std::string, std::vector<std::string>>,
								std::pair<int, std::pair<int, int>>>
								(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part),
									std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(r, k - 1)))] *

								a[std::pair<std::string, std::pair<int, int>>(i_rule.right_part[k], std::pair<int, int>(r, j + 1))];

							///*
							test.clear();
							for (std::string r_string : i_rule.right_part)
							{
								test += r_string;
							}

							std::cout << "h[" << i_rule.left_part << " -> " << test << ", " << i << ", " << j + 1 << ", " << k << " ] = " <<
								h[std::pair<std::pair<std::string, std::vector<std::string>>,
								std::pair<int, std::pair<int, int>>>
								(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part),
									std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(j + 1, k)))] << std::endl;//*/
							///*
							test.clear();
							for (std::string r_string : i_rule.right_part)
							{
								test += r_string;
							}

							std::cout << "*h[" << i_rule.left_part << " -> " << test << ", " << i << ", " << r << ", " << k - 1 << " ] = " <<
								h[std::pair<std::pair<std::string, std::vector<std::string>>,
								std::pair<int, std::pair<int, int>>>
								(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part),
									std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(r, k - 1)))] << std::endl;//*/
							///*
							std::cout << "a[" << i_rule.right_part[k] << ", " << r << ", " << j + 1 << "] = " <<
								a[std::pair<std::string, std::pair<int, int>>(i_rule.right_part[k], std::pair<int, int>(r, j + 1))] << std::endl << std::endl;//*/
						}
					}
				}
			}
		}
		//std::cout << "second" << std::endl;
		for (int i = 0; i <= Word.size(); i++)
		{
			//if (Word[0] != '\0' && i == Word.size()) break;
			for (int j = 0; j <= Word.size(); j++)
			{
				//if (Word[0] != '\0' && j == Word.size()) break;
				for (Rule i_rule : rules)
				{
					a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, j))] +=

						h[std::pair<std::pair<std::string, std::vector<std::string>>,
						std::pair<int, std::pair<int, int>>>
						(std::pair<std::string, std::vector<std::string>>(i_rule.left_part, i_rule.right_part),
							std::pair<int, std::pair<int, int>>(i, std::pair<int, int>(j, i_rule.right_part.size() - 1)))];
					std::cout << "*a[" << i_rule.left_part << ", " << i << ", " << j << "] = " << a[std::pair<std::string, std::pair<int, int>>(i_rule.left_part, std::pair<int, int>(i, j))] << std::endl;
				}
			}
		}
	}
	//std::cout << "third" << std::endl;

	/*
	 CYK_Modified(S, Г): // S — строка длины n, Г — КС-грамматика 
     for i = 1..n
      for Rj -> alpha // перебор состояний 
       if( A -> w[i] in Г) a[A, i, i+1] = true // если в грамматике Г присутствует правило A -> w[i] 
       else a[A, i, i+1] = false
       if( A -> eps in Г) a[A, i, i] = true // если в грамматике Г присутствует правило A -> eps 
       else a[A, i, i] = false
       h[A->alpha, i, i, 0] = true
     for m = 1..n
      for i = 1..n
          j = i+m
       for k = 1..M
        for Rj -> alpha // перебор состояний 
         h[A->alpha, i, j+1, k] = OR( for r = i..j+1) (h[A->alpha, i, r, k-1] & a[alpha[k],r,j+1])
     for i = 1..n
       for j = 1..n
         for Rj -> alpha
          a[A, i, j] = OR( for A->alpha) h[A->alpha, i, j, |alpha|] // где |alpha| — размер правой части правила
     return a[S, 1, n]
	*/

	return a[std::pair<std::string, std::pair<int, int>>(starting_non_terminal, std::pair<int, int>(0, Word.size()))];
}

bool CF_Grammar::a(const std::string& Non_Terminal, const int& I, const int& J)
{
	if (I == J)
	{
		if (shortest_path[Non_Terminal].length == 0) return true;
		else return false;
	}

	return false;
}

bool CF_Grammar::h(const std::string& Word, const Rule& Rule, const int& I, const int& J, const int& K)
{
	std::vector<std::string> prefix;
	std::string result;
	std::string temp;
	std::string searching_for;

	for (int i = I; i < J; i++)
		searching_for += Word[i];

	for (int i = 0; i < K; i++)
		prefix.push_back(Rule.right_part[i]);

	for (std::string i_string : prefix)
	{
		if (!non_terminals.contains(i_string))
			result += i_string;
		else
		{
			for (Path i_path : non_terminals[i_string])
			{

			}
		}
		if (result == searching_for) return true;
	}

	return false;
}

bool CF_Grammar::EarleyAlg(const std::vector<std::string>& Word)
{
	return false;
}

//=============== Методы структуры "Правило" ==============================================================

bool Rule::operator==(const Rule& Object) const
{
	if (this->left_part != Object.left_part) return false;
	if (this->right_part.size() != Object.right_part.size()) return false;
	if (this->right_part != Object.right_part) return false;

	return true;
}

Rule::Rule()
{
	terminals_count = 0;
}

Rule::Rule(std::string Left_Part, std::vector<std::string> Right_Part)
{
	left_part = Left_Part;
	terminals_count = Right_Part.size();
	for (std::string i_string : Right_Part)
	{
		right_part.push_back(i_string);
	}
}

Rule::~Rule()
{
	terminals_count = 0;
	right_part.clear();
}

//=============== Методы структуры "Путь" =================================================================

bool Path::operator==(const Path& Object) const
{
	if (this->length != Object.length) return false;
	if (this->path_rules.size() != Object.path_rules.size()) return false;
	for (int i = 0; i < path_rules.size(); i++)
	{
		if (this->path_rules[i] != Object.path_rules[i]) return false;
	}

	return true;
}

bool Path::operator+=(const Path& Object)
{
	Path new_path;
	new_path = *this;
	length += Object.length;
	for (Rule i_rule : Object.path_rules)
	{
		new_path.path_rules.push_back(i_rule);
		new_path.word = ApplyRule(new_path.word, i_rule, 0);
		new_path.path_words.push_back(new_path.word);
	}
	word = new_path.word;
	path_words = new_path.path_words;
	path_rules = new_path.path_rules;

	return true;
}

void Path::PrintPath(bool IsDebug)
{
	std::vector<std::string> word;
	std::cout << path_rules[0].left_part << " -> ";
	word.push_back(path_rules[0].left_part);
	for (int i = 0; i < path_rules.size(); i++)
	{
		word = ApplyRule(word, path_rules[i], 0);
		for (std::string i_string : word)
		{
			std::cout << i_string;
		}
		if (i != path_rules.size() - 1)
			std::cout << " -> ";
	}
	/*
	if (IsDebug)
	{
		std::cout << ", length = " << length;
		std::cout << std::endl;
		for (int i = 0; i < path_words.size(); i++)
		{
			for (std::string i_string : path_words[i])
			{
				std::cout << i_string;
			}
			if (i != path_words.size() - 1)
				std::cout << " -> ";
		}
		std::cout << std::endl;
	}
	//*/
	std::cout << std::endl;
}

Path::Path()
{
	length = 0;
}

Path::~Path()
{
	length = 0;
	path_rules.clear();
	path_words.clear();
	word.clear();
}

//=============== Применение правила к слову ===============================================================

std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number)
{
	std::vector<std::string> result = String;
	std::vector<std::string> replace_string;
	int non_terminal_number = 0;
	std::vector<std::string>::iterator position = std::ranges::find(result.begin(), result.end(), Rule.left_part);

	// Поиск нужного места в строке
	while (non_terminal_number != Non_Terminal_Number)
	{
		position = std::ranges::find(position, result.end(), Rule.left_part);
		non_terminal_number++;
	}

	// Составление строки-замены
	for (std::string i_string : Rule.right_part)
	{
		if (i_string != "[EPS]")
			replace_string.push_back(i_string);
	}

	// Вставка строки-замены на место нетерминала
	position = result.erase(position);
	for (std::string i_string : replace_string)
	{
		position = result.emplace(position, i_string);
		position++;
	}

	return result;
}
