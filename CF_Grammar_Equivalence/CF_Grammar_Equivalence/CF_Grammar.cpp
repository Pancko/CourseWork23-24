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
	int position;

	while (true)
	{
		file.get(current_char);

		if (current_char == '\n' || file.eof())
		{
			//if string contaits '|' add multiple rules
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
}

Rule CF_Grammar::GetRuleFromString(const std::string& String)
{
	Rule result;
	std::string current_string;
	int position = String.find("->");
	result.left_part = String.substr(0, position);
	current_string = String.substr(position + 2);

	if (starting_non_terminal.length() == 0)
		starting_non_terminal = result.left_part;

	int i_char = 0;

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

void CF_Grammar::PrintGrammar()
{
	std::cout << "Grammar:" << std::endl;
	std::cout << "Non-Terminals:" << std::endl;

	for(const auto& i_element: non_terminals)
	{
		std::cout << i_element.first << ",";
	}

	std::cout << std::endl << "Terminals:" << std::endl;
	for (std::string i_string : terminals)
	{
		std::cout << i_string << ",";
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
}

std::string CF_Grammar::GenerateWord(const int& Min_Length)
{
	std::string result;
	std::vector<std::string> word;
	std::vector<Rule> appliable_rules;
	std::vector<std::string> non_terminal_list;
	std::vector<std::string>::iterator iterator;
	int replaceable_non_terminals = 1;
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

std::vector<std::string> CF_Grammar::ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number)
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
	result.erase(position);
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

Rule::Rule()
{
	terminals_count = 0;
}

Rule::~Rule()
{
	terminals_count = 0;
	right_part.clear();
}

Path::Path()
{
	length = 0;
}

Path::~Path()
{
	length = 0;
	path_rules.clear();
}
