// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#pragma once
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>

struct Rule
{
	std::string left_part;
	std::vector<std::string> right_part;

	int terminals_count = 0;

	bool operator==(const Rule& Object);

	Rule();
	~Rule();
};

struct Path
{
	int length;
	std::vector<Rule> path_rules;
	std::vector<std::string> word;

	bool operator==(const Path& Object);
	bool operator+=(const Path& Object);

	void PrintPath(bool IsDebug = false);

	Path();
	~Path();
};

class CF_Grammar
{
private:
	std::string starting_non_terminal; // S
	std::map<std::string, std::vector<Path>> non_terminals; //N
	std::map<std::string, Path> shortest_path;
	std::set<std::string> bad_non_terminals;
	std::set<std::string> terminals; //Sigma
	std::vector<Rule> rules; //P

public:
	CF_Grammar();
	~CF_Grammar();

	// Считывание грамматики из файла
	void ReadFromFile(const std::string& File_Name);
	// Получение правила из строки
	Rule GetRuleFromString(const std::string& String);
	// Добавление правила в грамматику
	void AddRule(const Rule& New_Rule);

	// Генерация путей. Начальные нетерминалы, из которых сразу выводятся полностью терминальные слова
	void GeneratePathes();
	// Генерация путей. Добавление всех правил из грамматики
	void GenerateSubPathes();
	// Генерация путей. Доведение всех полученных путей до полностью терминальных слов (если возможно)
	void GenerateFinalPathes();


	// Вычисление и пометка "плохих" нетерминалов (из которых невозможно вывести полностью терминальное слово)
	void FindingBadNonTerminals();
	// Удаление "плохих" нетерминалов
	void DeleteBadNonTerminals();
	// Заполнение вектора кратчайших путей для использования в генерации
	void FillShortestPathes();

	// Проверка наличия нетерминала в конце пути
	bool GotNonTerminal(const Path& Current_Path);
	// Проверка наличия нетерминала в слове
	bool GotNonTerminal(const std::vector<std::string>& Word);

	// Вывод грамматики в консоль
	void PrintGrammar(bool IsDebug = false, bool ShowPath = false);

	// Генерация случайного терминального слова
	std::string GenerateWord(const int& Max_Length);

	bool KYCAlg(const std::vector<std::string>& Word);
	bool EarleyAlg(const std::vector<std::string>& Word);
};

// Применение правила к слову
std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number = 0);
