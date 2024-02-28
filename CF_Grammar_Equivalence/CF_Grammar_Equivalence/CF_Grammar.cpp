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

void CF_Grammar::ReadFromFile(const std::string& File_Name)
{
	std::ifstream file(File_Name);
	char current_char;
	std::string current_string;
	std::string sub_string;
	Rule current_rule;
	size_t position;

	while (true)
	{
		file.get(current_char);

		if (current_char == '\n' || file.eof())
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
		else
		{
			current_string += current_char;
		}

		if (file.eof())
			break;
	}

	file.close();
	GeneratePathes();
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

void CF_Grammar::GeneratePathes()
{
	Path current_path;
	bool got_wrong_non_terminal = false;
	bool non_terminal_found = false;
	bool good_non_terminal = false;

	// Найти правила, правая часть которых состоит из терминалов
	for (Rule i_rule : rules)
	{
		got_wrong_non_terminal = false;
		for (std::string i_string : i_rule.right_part)
		{
			// В правой части содержится нетерминал, из которого пока нельзя вывести слово
			if (non_terminals.contains(i_string))
			{
				got_wrong_non_terminal = true;
				break;
			}
		}
		if (i_rule.right_part[0] == "[EPS]")
			current_path.length = 0;
		else
			current_path.length = 1;

		// Добавляем правило в путь
		if (!got_wrong_non_terminal)
		{
			current_path.path_rules.emplace(current_path.path_rules.begin(), i_rule);
			//current_path.length = 1;
			current_path.word = i_rule.right_part;

			non_terminals[i_rule.left_part].push_back(current_path);

			//current_path.length = 0;
			current_path.path_rules.clear();
		}
		i_rule.~Rule();
	}

	GenerateSubPathes();

	FindingBadNonTerminals();

	DeleteBadNonTerminals();

	FillShortestPathes();
}

void CF_Grammar::GenerateSubPathes()
{
	Path current_path;
	std::map<std::string, std::vector<Path>> new_pathes = non_terminals;
	bool new_pathes_found = false;

	for (Rule i_rule : rules)
	{
		for (std::string i_string : i_rule.right_part)
		{
			if (non_terminals.contains(i_string) && i_string != i_rule.left_part && new_pathes[i_string].size() > 0)
			{
				for (Path i_path : new_pathes[i_string])
				{
					if (i_path.length > 0 && !GotNonTerminal(i_path))
					{
						// Составление нового пути
						current_path.length = 1;
						current_path.path_rules.push_back(i_rule);
						current_path.word = i_rule.right_part;
						current_path += i_path;

						// Добавить путь, если такого еще нет
						if (std::find(new_pathes[i_rule.left_part].begin(), new_pathes[i_rule.left_part].end(), current_path) == new_pathes[i_rule.left_part].end())
						{
							new_pathes[i_rule.left_part].push_back(current_path);
							new_pathes_found = true;
						}
						current_path.~Path();
					}
				}
			}
		}
	}

	non_terminals = new_pathes;

	if (new_pathes_found)
		GenerateFinalPathes();
}

void CF_Grammar::GenerateFinalPathes()
{
	Path current_path;
	std::map<std::string, std::vector<Path>> new_pathes = non_terminals;
	bool new_pathes_found = false;

	for (auto& i_element : non_terminals)
	{
		for (Path i_path : i_element.second)
		{
			for (std::string i_string : i_path.word)
			{
				if (non_terminals.contains(i_string) && new_pathes[i_string].size() > 0)
				{
					for (Path j_path : new_pathes[i_string])
					{
						if (j_path.length > 0 && !GotNonTerminal(j_path))
						{
							// Составление нового пути
							current_path = i_path;
							current_path += j_path;

							// Если такого пути еще нет, то добавить
							if (std::find(new_pathes[i_element.first].begin(), new_pathes[i_element.first].end(), current_path) == new_pathes[i_element.first].end())
							{
								new_pathes[i_element.first].erase(std::remove(new_pathes[i_element.first].begin(), new_pathes[i_element.first].end(), i_path), new_pathes[i_element.first].end());
								new_pathes[i_element.first].push_back(current_path);
								new_pathes_found = true;
							}
							current_path.~Path();
						}
					}
					break;
				}
			}
		}
	}

	non_terminals = new_pathes;

	// Если были добавлены новые пути, то запустить снова
	if (new_pathes_found)
		GenerateFinalPathes();
}

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

	/*	if (false)
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
		std::cout << counter << " pathes" << std::endl;
	}

	//DeleteBadNonTerminals();

	counter = 0;*/

	if (ShowPath)
	{
		std::cout << "Pathes:" << std::endl;
		for (const auto& i_element : non_terminals)
		{
			for (Path i_path : i_element.second)
			{
				i_path.PrintPath(IsDebug);
				counter++;
			}
		}
		if (IsDebug)
		{
			std::cout << counter << " pathes" << std::endl;
		}
	}

	if (IsDebug)
	{
		for (auto& i_element : shortest_path)
		{
			std::cout << "Shortest path for " << i_element.first << " is " << i_element.second.length << std::endl;
		}
	}
}

std::string CF_Grammar::GenerateWord(const int& Max_Length)
{
	std::string result;
	std::vector<std::string> word;
	std::vector<std::string> final_word;
	std::vector<Rule> appliable_rules;
	int rule_to_use = 0;
	int expected_length = 0;
	int actual_length = 0;
	bool non_terminal_found = 0;

	// Начальный вид слова
	word.push_back(starting_non_terminal);
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
	while (expected_length < Min_Length)
	{
		// Рандомный выбор номера правила
		rule_to_use = rand() % appliable_rules.size();

		// Применение правила
		word = ApplyRule(word, appliable_rules[rule_to_use]);

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
		if (expected_length == 0 || actual_length > 2 * Min_Length) break;

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
				}
			}
		}
	}
	word = final_word;

	for (std::string i_string : word)
	{
		result += i_string;
	}

	return result;
}

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

bool CF_Grammar::KYCAlg(const std::vector<std::string>& Word)
{
	return false;
}

bool CF_Grammar::EarleyAlg(const std::vector<std::string>& Word)
{
	return false;
}

bool Rule::operator==(const Rule& Object)
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

Rule::~Rule()
{
	terminals_count = 0;
	right_part.clear();
}

bool Path::operator==(const Path& Object)
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
		path_rules.clear();
		path_rules = new_path.path_rules;
		new_path.word = ApplyRule(new_path.word, i_rule, 0);
	}
	word = new_path.word;

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
	if (IsDebug)
		std::cout << ", length = " << length;
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
	word.clear();
}
