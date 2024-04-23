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
	GenerateBasicPathes();

	//GeneratePathes();

	FindingBadNonTerminals();

	DeleteBadNonTerminals();

	FillShortestPathes();
}

void CF_Grammar::GenerateBasicPathes()
{
	Path current_path;
	std::vector<PathPermutations> pathes;
	std::map<std::string, std::vector<Path>> new_found_pathes;
	std::map<std::string, std::vector<Path>> current_new_found_pathes;
	std::map<std::string, std::vector<Path>> empty_pathes;
	std::set<std::string> analyzed_non_terminlas;
	std::string temp_str;
	int position = 0;

	for (Rule i_rule : rules) // Базис. Заполнить массив правил
	{
		pathes.push_back(PathPermutations(i_rule));

		for (std::string i_string : i_rule.right_part)
			pathes[IndexOfRule(i_rule)].right_part.push_back(std::map<std::string, std::vector<std::string>>::value_type(i_string, NULL));

		if (!GotNonTerminal(i_rule.right_part)) // Если правило порождает нетерминальное слово
		{
			if (!new_found_pathes.contains(i_rule.left_part))
				new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
			if (i_rule.right_part[0] == "[EPS]")
				current_path.length = 0;
			else
			{
				current_path.word = i_rule.right_part;
				current_path.length = 1;
			}
			current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
			current_path.path_rules.emplace(current_path.path_rules.begin(), i_rule);
			current_path.path_words.push_back(current_path.word);

			non_terminals[i_rule.left_part].push_back(current_path);
			pathes_amount++;

			new_found_pathes[i_rule.left_part].push_back(current_path);
			analyzed_non_terminlas.emplace(i_rule.left_part);

			current_path.path_rules.clear();
			current_path.path_words.clear();
			current_path.word.clear();
		}
	}

	while (new_found_pathes.size() > 0) // Рекуррентный шаг, создаем пути пока находим новые (только для новых нетерминалов)
	{
		for (Rule i_rule : rules)
		{
			// Если для нетерминала ещё не найден ни один вывод
			if (IsRuleViable(i_rule, new_found_pathes) && !analyzed_non_terminlas.contains(i_rule.left_part))
			{
				position = 0;
				for (std::string i_string : i_rule.right_part)
				{
					if (new_found_pathes.contains(i_string))
					{
						for (Path i_path : new_found_pathes[i_string])
						{
							temp_str.clear();
							for (std::string word_str : i_path.word)
								temp_str += word_str;

							if (!VecContStr(pathes[IndexOfRule(i_rule)].right_part[position].second, temp_str)) // Если такой путь еще не был применен к нетерминалу
							{
								current_path.length = 1;
								current_path.path_rules.push_back(i_rule);
								current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
								current_path.path_words.push_back(i_rule.right_part);
								current_path.word = i_rule.right_part;

								current_path = current_path.ApplyPath(i_path, position);

								if (IsUniquePath(current_path, i_path, non_terminals))
								{
									pathes[IndexOfRule(i_rule)].right_part[position].second.push_back(temp_str); // Добавляем новый вариант в таблицу
									if (GotNonTerminal(current_path)) // Генерируем новые пути
									{
										current_new_found_pathes = PathConvergence(current_new_found_pathes, GenerateSubPath(current_path));
										if (current_new_found_pathes.size() != 0)
											analyzed_non_terminlas.emplace(i_rule.left_part);
									}
									else
									{
										non_terminals[i_rule.left_part].push_back(current_path);
										pathes_amount++;
										if (!current_new_found_pathes.contains(i_rule.left_part))
											current_new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
										current_new_found_pathes[i_rule.left_part].push_back(current_path);
										analyzed_non_terminlas.emplace(i_rule.left_part);
									}
								}

								current_path.path_rules.clear();
								current_path.path_words.clear();
								current_path.word.clear();
							}
						}
					}
					position++;
				}
			}
		}

		new_found_pathes.clear();
		new_found_pathes = current_new_found_pathes;
		current_new_found_pathes.clear();
	}

	// Находим все уже известные пустые выводы
	analyzed_non_terminlas.clear();
	new_found_pathes.clear();
	for (auto& i_element : non_terminals)
	{
		for (Path i_path : non_terminals[i_element.first])
		{
			if (i_path.word.size() == 0 && !analyzed_non_terminlas.contains(i_element.first))
			{
				analyzed_non_terminlas.emplace(i_element.first);
				new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_element.first, NULL));
				new_found_pathes[i_element.first].push_back(i_path);
				break;
			}
		}
	}

	current_new_found_pathes = new_found_pathes;

	while (current_new_found_pathes.size() > 0) // Найти все пустые выводы
	{
		current_new_found_pathes.clear();
		for (Rule i_rule : rules)
		{
			if (!analyzed_non_terminlas.contains(i_rule.left_part))
			{
				current_path.length = 1;
				current_path.path_rules.push_back(i_rule);
				current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
				current_path.path_words.push_back(i_rule.right_part);
				current_path.word = i_rule.right_part;

				for (std::string i_string : i_rule.right_part)
				{
					if (analyzed_non_terminlas.contains(i_string))
					{
						current_path += new_found_pathes[i_string][0];

						if (current_path.word.size() == 0)
						{
							non_terminals[i_rule.left_part].push_back(current_path);
							pathes_amount++;
							analyzed_non_terminlas.emplace(i_rule.left_part);
							if (!current_new_found_pathes.contains(i_rule.left_part))
								current_new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
							current_new_found_pathes[i_rule.left_part].push_back(current_path);
							if (!new_found_pathes.contains(i_rule.left_part))
								new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
							new_found_pathes[i_rule.left_part].push_back(current_path);
							break;
						}
					}
				}
				current_path.path_rules.clear();
				current_path.path_words.clear();
				current_path.word.clear();
			}
		}
	}
}

void CF_Grammar::GeneratePathes()
{
	Path current_path;
	std::vector<PathPermutations> pathes;
	std::map<std::string, std::vector<Path>> new_found_pathes;
	std::map<std::string, std::vector<Path>> current_new_found_pathes;
	std::string temp_str;
	int position = 0;

	for (Rule i_rule : rules) // Базис. Заполнить массив правил
	{
		pathes.push_back(PathPermutations(i_rule));

		for (std::string i_string : i_rule.right_part)
			pathes[IndexOfRule(i_rule)].right_part.push_back(std::map<std::string, std::vector<std::string>>::value_type(i_string, NULL));

		if (!GotNonTerminal(i_rule.right_part)) // Если правило порождает нетерминальное слово
		{
			if (!new_found_pathes.contains(i_rule.left_part))
				new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
			if (i_rule.right_part[0] == "[EPS]")
				current_path.length = 0;
			else
			{
				current_path.word = i_rule.right_part;
				current_path.length = 1;
			}
			current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
			current_path.path_rules.emplace(current_path.path_rules.begin(), i_rule);
			current_path.path_words.push_back(current_path.word);

			non_terminals[i_rule.left_part].push_back(current_path);
			pathes_amount++;

			new_found_pathes[i_rule.left_part].push_back(current_path);

			current_path.path_rules.clear();
			current_path.path_words.clear();
			current_path.word.clear();
		}
	}
	int i = 1;
	Timer t;
	while (new_found_pathes.size() > 0) // Рекуррентный шаг, создаем пути пока находим новые
	{
		std::cout << "Generating pathes step " << i << ", found new pathes for non-terminals: ";

		for (auto& i_element : new_found_pathes)
		{
			std::cout << i_element.first << ", ";
		}
		t.reset();

		for (Rule i_rule : rules)
		{
			if (IsRuleViable(i_rule, new_found_pathes))
			{
				position = 0;
				for (std::string i_string : i_rule.right_part)
				{
					if (new_found_pathes.contains(i_string))
					{
						for (Path i_path : new_found_pathes[i_string])
						{
							temp_str.clear();
							for (std::string word_str : i_path.word)
								temp_str += word_str;

							if (!VecContStr(pathes[IndexOfRule(i_rule)].right_part[position].second, temp_str)) // Если такой путь еще не был применен к нетерминалу
							{
								current_path.length = 1;
								current_path.path_rules.push_back(i_rule);
								current_path.path_words.push_back(std::vector<std::string>({ i_rule.left_part }));
								current_path.path_words.push_back(i_rule.right_part);
								current_path.word = i_rule.right_part;

								current_path = current_path.ApplyPath(i_path, position);

								if (IsUniquePath(current_path, i_path, non_terminals))
								{
									pathes[IndexOfRule(i_rule)].right_part[position].second.push_back(temp_str); // Добавляем новый вариант в таблицу
									if (GotNonTerminal(current_path)) // Генерируем новые пути
										current_new_found_pathes = PathConvergence(current_new_found_pathes, GenerateSubPath(current_path));
									else
									{
										non_terminals[i_rule.left_part].push_back(current_path);
										pathes_amount++;
										if (!current_new_found_pathes.contains(i_rule.left_part))
											current_new_found_pathes.emplace(std::map<std::string, std::vector<Path>>::value_type(i_rule.left_part, NULL));
										current_new_found_pathes[i_rule.left_part].push_back(current_path);
									}
								}

								current_path.path_rules.clear();
								current_path.path_words.clear();
								current_path.word.clear();
							}
						}
					}
					position++;
				}
			}
		}

		new_found_pathes.clear();
		new_found_pathes = current_new_found_pathes;
		int count = 0;
		for (auto& i_element : new_found_pathes)
		{
			count += (int)i_element.second.size();
		}
		current_new_found_pathes.clear();
		i++;
		std::cout << "time elapsed = " << t.elapsed() << ", new pathes = " << count << std::endl;
	}
}

std::map<std::string, std::vector<Path>> CF_Grammar::GenerateSubPath(const Path& Current_Path)
{
	std::map<std::string, std::vector<Path>> result;
	Path current_path;
	std::map<std::string, std::vector<Path>> old_pathes = non_terminals;
	for (std::string i_string : Current_Path.word)      // Рассматриваем символы в слове
	{
		if (non_terminals.contains(i_string))           // Если символ является нетерминалом
		{
			for (Path i_path : old_pathes[i_string])    // Пути этого нетерминала
			{
				current_path = Current_Path;
				current_path += i_path;
				if (IsUniquePath(current_path, i_path, non_terminals))
				{
					if (GotNonTerminal(current_path))
						result = PathConvergence(result, GenerateSubPath(current_path));
					else
					{
						result.emplace(std::map<std::string, std::vector<Path>>::value_type(current_path.path_rules[0].left_part, NULL));
						result[current_path.path_rules[0].left_part].push_back(current_path);
						non_terminals[current_path.path_rules[0].left_part].push_back(current_path);
						pathes_amount++;
					}
				}
			}
		}
	}
	return result;
}

bool CF_Grammar::IsRuleViable(const Rule& Current_Rule, const std::map<std::string, std::vector<Path>>& Non_Terminals)
{
	bool found_viable_non_terminal = false;
	std::map<std::string, std::vector<Path>> current_non_terminals = Non_Terminals;

	for (std::string i_string : Current_Rule.right_part)
	{
		if (i_string == Current_Rule.left_part) return false; // В правой части правила содержится тот же нетерминал (цикл)

		if (current_non_terminals.contains(i_string) && current_non_terminals[i_string].size() == 0) return false; // Нетерминал, у которого нет путей

		if (current_non_terminals.contains(i_string)) found_viable_non_terminal = true; // Подходящий нетерминал
	}

	return found_viable_non_terminal;
}

bool CF_Grammar::IsUniquePath(const Path& Path_To_Check, const Path& Added_Path, const std::map<std::string, std::vector<Path>>& Current_Pathes)
{
	std::map<std::string, std::vector<Path>> current_pathes = Current_Pathes;

	// Если путь с таким заключительным словом уже существует
	for (Path i_path : current_pathes[Path_To_Check.path_rules[0].left_part])
	{
		if (i_path.word == Path_To_Check.word) return false;
	}

	// Если только что добавленный путь является опустошающим (то есть просто убирает нетерминал), то дальше можно не расследовать
	if (Added_Path.path_rules.size() > 0 && Added_Path.word.size() == 0) return true;

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
	std::map<std::string, std::vector<Path>> new_non_terminals = non_terminals;
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

	if (ShowPath)
	{
		std::cout << "Pathes:" << std::endl;
		for (const auto& i_element : non_terminals)
		{
			for (Path i_path : i_element.second)
				i_path.PrintPath();
		}
	}

	std::cout << pathes_amount << " pathes" << std::endl;

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
	int temp_int = 0;
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
		if (actual_length > Max_Length) break;

		// Составление нового списка возможных для применения правил
		appliable_rules.clear();
		for (std::string i_string : word)
		{
			if (non_terminals.contains(i_string))
			{
				non_terminal_found = 0;
				for (Rule i_rule : rules)
				{
					if (i_rule.left_part == i_string)
					{
						non_terminal_found = 1;
						appliable_rules.push_back(i_rule);
						temp_int = std::max(temp_int, (int)i_rule.terminals_count);
					}
					else if (non_terminal_found && i_rule.left_part != i_string) break;
				}
			}
		}
		if (temp_int == 0) break;
		temp_int = 0;
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

	if (!words.contains(result))
	{
		words.insert(result);
	}

	word.~vector();
	final_word.~vector();
	appliable_rules.~vector();

	return result;
}

std::vector<std::string> CF_Grammar::GenerateMultipleWords(const int& Amount, const int& Max_Length)
{
	std::vector<std::string> result;
	int iterations = 0;
	int words_size;
	std::string temp_str;

	while (words.size() < Amount)
	{
		words_size = (int)words.size();
		temp_str = GenerateWord(Max_Length);
		result.push_back(temp_str);

		if (words_size == words.size()) iterations++;
		else iterations = 0;
		
		if (iterations > Amount)
		{
			for (Path i_path : non_terminals[starting_non_terminal])
			{
				temp_str.clear();
				for (std::string i_string : i_path.word)
					temp_str += i_string;
				if (temp_str == "[EPS]")
					temp_str = "";
				if (!words.contains(temp_str))
					words.insert(temp_str);
			}
			break;
		}
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

bool CF_Grammar::CYK_Alg_Modified(const std::string& Word)
{
	bool result = false;
	bool temp_bool = false;
	int temp_int = 0;
	int max_right_part_length = 0;
	int index_of_rule = 0;
	int index_of_non_terminal = 0;
	int j = 0;
	std::string temp_str;
	std::string word = Word;
	std::vector<Rule> temp_rules;

	if (word.size() == 0)
		word = "[EPS]";

	// a[A][i][j] = true, если из нетерминала А можно вывести подстроку word[i...j - 1]
	std::vector<std::vector<std::vector<bool>>> a;
	a.resize(non_terminals.size() + terminals.size());
	for (int i = 0; i < non_terminals.size() + terminals.size(); i++)
	{
		a[i].resize(2 * word.size());
		for (int j = 0; j < 2 * word.size(); j++)
			a[i][j].resize(2 * word.size());
	}

	for (Rule i_rule : rules)
	{
		max_right_part_length = std::max(max_right_part_length, (int)i_rule.right_part.size());
	}

	// h[A -> alpha][i][j][k] = true, если из префикса длины k правила A -> alpha можно вывести подстроку word[i...j - 1]
	std::vector<std::vector<std::vector<std::vector<bool>>>> h;
	h.resize(rules.size());
	for (int i = 0; i < rules.size(); i++)
	{
		h[i].resize(2 * word.size());
		for (int j = 0; j < 2 * word.size(); j++)
		{
			h[i][j].resize(2 * word.size());
			for (int k = 0; k < 2 * word.size(); k++)
				h[i][j][k].resize(max_right_part_length + 1);
		}
	}

	for (int i = 0; i <= word.size(); i++)
	{
		// Выводимость из нетерминалов
		for (j = i + 1; j <= word.size() + 1; j++)
		{
			temp_str.clear();
			if (word == "[EPS]")
				temp_str = "";
			else
				for (int r = i; r < std::min(j, (int)word.size()); r++)
				{
					temp_str += word[r];
				}
			for (auto& i_element : non_terminals)
			{
				index_of_non_terminal = IndexOfNonTerminal(i_element.first);
				for (Path i_path : non_terminals[i_element.first])
				{
					if (i_path.word.size() == 0)
					{
						a[index_of_non_terminal][i][i] = true;
					}
					if (VectorToString(i_path.word) == temp_str)
					{
						a[index_of_non_terminal][i][j] = true;
						break;
					}
				}
			}
		}
		if (a[IndexOfNonTerminal(starting_non_terminal)][0][Word.size()]) return true;
		// Выводимость терминалов
		for (std::string i_string : terminals)
		{
			index_of_non_terminal = IndexOfNonTerminal(i_string);
			if (word[i] == i_string[0] && i_string != "[EPS]")
				a[index_of_non_terminal][i][i + 1] = true;
		}
		for (Rule i_rule : rules)
		{
			h[IndexOfRule(i_rule)][i][i][0] = true;
		}
	}
	for (int m = 0; m < word.size(); m++)
	{
		for (int i = 0; i < word.size(); i++)
		{
			j = i + m;
			for (int times = 0; times <= non_terminals.size(); times++)
			{
				for (Rule i_rule : rules)
				{
					index_of_rule = IndexOfRule(i_rule);
					for (int k = 1; k <= i_rule.right_part.size(); k++)
					{
						index_of_non_terminal = IndexOfNonTerminal(i_rule.right_part[k - 1]);
						for (int r = i; r <= j + 1; r++)
						{
							if (h[index_of_rule][i][j + 1][k] == true) break;
							h[index_of_rule][i][j + 1][k] = (h[index_of_rule][i][r][k - 1] * a[index_of_non_terminal][r][j + 1]);
							if (!h[index_of_rule][i][j + 1][k])
							{
								temp_bool = true;
								for (int v = 0; v < k - 1; v++)
								{
									temp_bool &= a[IndexOfNonTerminal(i_rule.right_part[v])][0][0];
									if (!temp_bool) break;
								}
								if (temp_bool)
								{
									h[index_of_rule][i][j + 1][k] = h[index_of_rule][i][j + 1][k] + a[index_of_non_terminal][i][j + 1];
								}
							}
						}
					}
					if (h[index_of_rule][i][j + 1][i_rule.right_part.size()] == true)
					{
						a[IndexOfNonTerminal(i_rule.left_part)][i][j + 1] = true;
					}
				}
			}
		}
	}

	result = a[IndexOfNonTerminal(starting_non_terminal)][0][Word.size()];

	return result;
}

int CF_Grammar::IndexOfNonTerminal(const std::string& Non_Terminal)
{
	int index = (int)std::distance(non_terminals.begin(), non_terminals.find(Non_Terminal));
	if (index != non_terminals.size()) return index;
	return (int)std::distance(terminals.begin(), terminals.find(Non_Terminal)) + (int)non_terminals.size();
}

int CF_Grammar::IndexOfRule(const Rule& Current_Rule)
{
	return std::max(-1, (int)std::distance(rules.begin(), std::find(rules.begin(), rules.end(), Current_Rule)));
}

std::string CF_Grammar::VectorToString(const std::vector<std::string>& Object)
{
	std::string result;

	for (int i = 0; i < Object.size(); i++)
		result += Object[i];

	return result;
}

std::vector<Rule> CF_Grammar::NonTerminalRules(const std::string& Non_Terminal)
{
	std::vector<Rule> result;
	bool found_one = false;
	for (Rule i_rule : rules)
	{
		if (i_rule.left_part == Non_Terminal)
		{
			result.push_back(i_rule);
			found_one = true;
		}
		else if (!found_one) break;
	}
	return result;
}

//=============== Методы структуры "Правило" ==============================================================

bool Rule::operator==(const Rule& Object) const
{
	if (this->left_part != Object.left_part) return false;
	if (this->right_part.size() != Object.right_part.size()) return false;
	if (this->right_part != Object.right_part) return false;

	return true;
}

bool Rule::operator<(const Rule& Object) const
{
	if (this->left_part < Object.left_part)
		return true;
	return false;
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
	//pathes_used.push_back(Object);

	return true;
}

Path Path::ApplyPath(const Path& Object, const int& position)
{
	Path new_path;
	int pos = 0;
	int place = 0;
	new_path = *this;
	Path obj = Object;
	new_path.length += Object.length;
	//new_path.pathes_used.push_back(Object);
	for (Rule i_rule : Object.path_rules)
	{
		for (pos = 0; pos < std::min(obj.path_words[place].size(), obj.path_words[place + 1].size()); pos++)
		{
			if (obj.path_words[place][pos] != obj.path_words[place + 1][pos])
				break;
		}
		new_path.path_rules.push_back(i_rule);
		new_path.word = ApplyRule(new_path.word, i_rule, position + pos);
		new_path.path_words.push_back(new_path.word);
		place++;
	}

	return new_path;
}

void Path::PrintPath(bool IsDebug)
{
	std::cout << path_rules[0].left_part << " -> ";
	for (int i = 1; i < path_words.size(); i++)
	{
		if (i != path_words.size() - 1)
		{
			for (std::string i_string : path_words[i])
			{
				std::cout << i_string;
			}
			std::cout << " -> ";
		}
		else
			for (std::string i_string : this->word)
				std::cout << i_string;
	}
	std::cout << " size = " << this->word.size();
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

	if (Non_Terminal_Number > 0)
		position = result.begin() + Non_Terminal_Number;

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

//=============== Совмещение двух наборов правил ===========================================================
std::map<std::string, std::vector<Path>> PathConvergence(const std::map<std::string, std::vector<Path>>& First_Object, const std::map<std::string, std::vector<Path>>& Second_Object)
{
	std::map<std::string, std::vector<Path>> result = First_Object;
	for (auto& i_element : Second_Object)
	{
		if (!result.contains(i_element.first))
			result.emplace(std::map<std::string, std::vector<Path>>::value_type(i_element.first, NULL));
		for (Path i_path : i_element.second)
		{
			if (CF_Grammar().IsUniquePath(i_path, Path(), result))
				result[i_element.first].push_back(i_path);
		}
	}
	return result;
}

//=============== Содержится ли строка в векторе ===========================================================
bool VecContStr(const std::vector<std::string>& Vector, const std::string& String)
{
	if (std::ranges::find(Vector.begin(), Vector.end(), String) != Vector.end())
		return true;
	return false;
}

//=============== Генерация и проверка слов двух грамматик =================================================
void EquivalenceTest(const CF_Grammar& Grammar1, const CF_Grammar& Grammar2, const int& Words_Lenght)
{
	CF_Grammar grammar1 = Grammar1;
	CF_Grammar grammar2 = Grammar2;

	std::set<std::string> words;
	std::vector<std::string> incorrect_words;

	bool temp_bool = false;
	float temp_float = 0;
	float grammar1_in_grammar2 = 0;
	float grammar2_in_grammar1 = 0;
	int number_of_words = 10;

	Timer timer;
	timer.reset();

	// Генерируем слова в первой грамматике и проверяем их выводимость во второй
	while (number_of_words < 1001)
	{
		std::cout << std::endl << "Generating " << number_of_words << " words in 1 grammar..." << std::endl;
		temp_float = 0;
		grammar1.GenerateMultipleWords(number_of_words, Words_Lenght);
		words.clear();
		words = grammar1.GetWords();

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
			{
				if (i_string == "")
					std::cout << "' '" << std::endl;
				else
					std::cout << i_string << std::endl;
			}
			break;
		}
		else
			std::cout << "Second grammar can produce all " << words.size() << " words!" << std::endl;
		number_of_words *= 10;
	}

	grammar1_in_grammar2 = 100 * temp_float / words.size();
	number_of_words = 10;
	incorrect_words.clear();
	words.clear();

	std::cout << "Generation and checking took: " << timer.elapsed() << std::endl;
	timer.reset();

	// Генерируем слова во второй грамматике и проверяем их выводимость в первой
	while (number_of_words < 1001)
	{
		std::cout << std::endl << "Generating " << number_of_words << " words in 2 grammar..." << std::endl;
		temp_float = 0;
		grammar2.GenerateMultipleWords(number_of_words, Words_Lenght);

		words = grammar2.GetWords();

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
			break;
		}
		else
			std::cout << "First grammar can produce all " << words.size() << " words!" << std::endl;
		number_of_words *= 10;
	}

	grammar2_in_grammar1 = 100 * temp_float / words.size();
	incorrect_words.clear();
	words.clear();

	std::cout << "Generation and checking took: " << timer.elapsed() << std::endl;

	std::cout << std::endl;
	std::cout << "Grammar 2 can replicate " << grammar1_in_grammar2 << "% of grammar 1 words" << std::endl;
	std::cout << "Grammar 1 can replicate " << grammar2_in_grammar1 << "% of grammar 2 words" << std::endl;
}
