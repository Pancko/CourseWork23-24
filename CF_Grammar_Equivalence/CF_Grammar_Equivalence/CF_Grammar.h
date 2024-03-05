// ТВГУ ПМиК ФИиИТ 2024
// Андреев Семен Витальевич
#pragma once
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>

// Правило из КС-грамматики
struct Rule
{
	std::string left_part;					// Левая часть правила
	std::vector<std::string> right_part;	// Правая часть правила
	size_t terminals_count = 0;				// Счетчик терминалов

	bool operator==(const Rule& Object) const;

	Rule();
	Rule(std::string Left_Part, std::vector<std::string> Right_Part);
	~Rule();
};

// Путь из нетерминала до определенного слова
struct Path
{
	int length;											// Длина пути
	std::vector<Rule> path_rules;						// Правила
	std::vector<std::vector<std::string>> path_words;	// Последовательность слов
	std::vector<std::string> word;						// Конечное слово

	bool operator==(const Path& Object) const;
	bool operator+=(const Path& Object);

	void PrintPath(bool IsDebug = false);

	Path();
	~Path();
};

// Основной класс - КС-грамматика
class CF_Grammar
{
private:
	std::string starting_non_terminal;						// S
	std::map<std::string, std::vector<Path>> non_terminals; // N
	std::map<std::string, Path> shortest_path;				// Кратчайшие пути для нетерминалов
	std::set<std::string> bad_non_terminals;				// "Плохие" нетерминалы
	std::set<std::string> terminals;						// Sigma
	std::vector<Rule> rules;								// P

	std::set<std::string> words;							// Сгенерированныые слова

public:
	CF_Grammar();
	~CF_Grammar();

	// Считывание грамматики из файла
	void ReadFromFile(const std::string& File_Name);
	// Получение правила из строки
	Rule GetRuleFromString(const std::string& String);
	// Добавление правила в грамматику
	void AddRule(const Rule& New_Rule);


	// Генерация путей. Анализ и удаление циклов, бесполезных нетерминалов
	void AnalyzeNonTerminals();
	// Полная генерация путей.
	void GeneratePathes();
	// Генерация путей, доведение всех правил до терминальных слов
	bool GenerateSubPath(const Path& Current_Path);
	// Проверка пути на уникальность
	bool IsUniquePath(const Path& Path_To_Check, std::map<std::string, std::vector<Path>> Current_Pathes);


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
	// Изменение слова в угоду уникальности
	bool ReGenerateWord(const Path& Word);
	// Генерация нескольких случайных терминальных слов с заданной максимальной длиной
	std::vector<std::string> GenerateMultipleWords(const int& Amount, const int& Max_Length);


	// Печать сгенерированных слов
	void PrintWords(bool IsDebug = false);
	
	std::set<std::string> GetWords();

	// Алгоритм Кока-Янгера-Касами, модификация для произвольной грамматики
	bool CYK_Alg(const std::string& Word);
	// Вспомогательные функции алгоритма Кока-Янгера-Касами
	bool a(const std::string& Non_Terminal, const int& I, const int& J);
	bool h(const std::string& Word, const Rule& Rule, const int& I, const int& J, const int& K);

	// Алгоритм Эрли
	bool EarleyAlg(const std::vector<std::string>& Word);
};

// Применение правила к слову
std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number = 0);
