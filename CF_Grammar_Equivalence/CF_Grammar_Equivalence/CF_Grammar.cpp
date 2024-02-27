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
			// ƒобавить несколько правил, если они написаны через '|'
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
			rule_to_add.terminals_count++;
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

	// Ќайти правила, права€ часть которых состоит из терминалов
	for (Rule i_rule : rules)
	{
		got_wrong_non_terminal = false;
		for (std::string i_string : i_rule.right_part)
		{
			// ¬ правой части содержитс€ нетерминал, из которого пока нельз€ вывести слово
			if (non_terminals.contains(i_string))
			{
				got_wrong_non_terminal = true;
				break;
			}
		}
		// ƒобавл€ем правило в путь
		if (!got_wrong_non_terminal)
		{
			current_path.path_rules.emplace(current_path.path_rules.begin(), i_rule);
			current_path.length++;
			current_path.word = i_rule.right_part;

			non_terminals[i_rule.left_part].push_back(current_path);

			current_path.length = 0;
			current_path.path_rules.clear();
		}
		i_rule.~Rule();
	}

	GenerateSubPathes();

	FindingBadNonTerminals();
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
						// —оставление нового пути
						current_path.length = 1;
						current_path.path_rules.push_back(i_rule);
						current_path.word = i_rule.right_part;
						current_path += i_path;

						// ƒобавить путь, если такого еще нет
						if (std::find(new_pathes[i_rule.left_part].begin(), new_pathes[i_rule.left_part].end(), current_path) == new_pathes[i_rule.left_part].end())
						{
							new_pathes[i_rule.left_part].push_back(current_path);
							new_pathes_found = true;
						}
						current_path.~Path();
					}
				}
				break;
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
							// —оставление нового пути
							current_path = i_path;
							current_path += j_path;

							// ≈сли такого пути еще нет, то добавить
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

	// ≈сли были добавлены новые пути, то запустить снова
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

void CF_Grammar::PrintGrammar()
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
		std::cout << "           || terminals_count: " << i_rule.terminals_count << std::endl;
	}

	std::cout << "Pathes:" << std::endl;
	for (const auto& i_element : non_terminals)
	{
		for (Path i_path : i_element.second)
		{
			i_path.PrintPath();
		}
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
}

std::string CF_Grammar::GenerateWord(const int& Min_Length)
{
	std::string result;
	std::vector<std::string> word;
	std::vector<Rule> appliable_rules;
	std::vector<std::string> non_terminal_list;
	std::vector<std::string>::iterator iterator;
	size_t replaceable_non_terminals = 1;
	int non_terminal_to_replace = 0;
	int rule_to_use = 0;
	int expected_length = 0;
	bool doOnce = 0;

	// Ќачальный вид слова
	word.push_back(starting_non_terminal);
	non_terminal_list.push_back(starting_non_terminal);
	iterator = non_terminal_list.begin();

	// «аполнить список правил, которые можно применить
	for (Rule i_rule : rules)
	{
		if (i_rule.left_part != starting_non_terminal)
			break;
		appliable_rules.push_back(i_rule);
	}

	// ѕрименение случайных правил к случайным нетерминалам
	while (expected_length < Min_Length)
	{
		// –андомный выбор номера правила
		rule_to_use = rand() % (appliable_rules.size() + 1);

		// ѕрименение правила
		word = ApplyRule(word, appliable_rules[rule_to_use], non_terminal_to_replace);

		// –ассчет нового количества нетерминалов в слове
		replaceable_non_terminals += rules[rule_to_use].right_part.size() - rules[rule_to_use].terminals_count - 1;

		// –андомный выбор нетерминала дл€ замены
		non_terminal_to_replace = rand() % (replaceable_non_terminals + 1);

		// –едактирование списка доступных нетерминалов
		non_terminal_list.erase(iterator);
		for (std::string i_string : word)
		{
			if (non_terminals.contains(i_string))
			{
				non_terminal_list.emplace(iterator, i_string);
				iterator++;
			}
		}
		iterator = non_terminal_list.begin() + non_terminal_to_replace;

		// »зменение ожидаемой длины слова
		expected_length += appliable_rules[rule_to_use].terminals_count;
		expected_length -= shortest_path[appliable_rules[rule_to_use].left_part];
		for (std::string i_string : appliable_rules[rule_to_use].right_part)
		{
			if (non_terminals.contains(i_string))
			{
				expected_length += shortest_path[i_string];
			}
		}

		// —оставление нового списка возможных дл€ применени€ правил
		appliable_rules.clear();
		for (Rule i_rule : rules)
		{
			if (doOnce)
			{
				appliable_rules.push_back(i_rule);
			}
			else if (non_terminal_list[non_terminal_to_replace] == i_rule.left_part)
			{
				doOnce = 1;
				appliable_rules.push_back(i_rule);
			}
			else break;
		}

	}

	// ƒоведение слова до конца

	return std::string();
}

std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number)
{
	std::vector<std::string> result = String;
	std::vector<std::string> replace_string;
	int non_terminal_number = 0;
	std::vector<std::string>::iterator position = std::ranges::find(result.begin(), result.end(), Rule.left_part);

	// ѕоиск нужного места в строке
	while (non_terminal_number != Non_Terminal_Number)
	{
		position = std::ranges::find(position, result.end(), Rule.left_part);
		non_terminal_number++;
	}

	// —оставление строки-замены
	for (std::string i_string : Rule.right_part)
	{
		if (i_string != "[EPS]")
			replace_string.push_back(i_string);
	}

	// ¬ставка строки-замены на место нетерминала
	position = result.erase(position);
	for (std::string i_string : replace_string)
	{
		position = result.emplace(position, i_string);
		position++;
	}

	return result;
}

/*
int CF_Grammar::min_element(const std::vector<Path, std::allocator<Path>>& Obj)
{
	int result = Obj[0].length;
	for (Path i_path : Obj)
	{
		if (i_path.length < result)
			result = i_path.length;
	}
	return result;
}*/

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
	length += Object.length - 1;
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

void Path::PrintPath()
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
