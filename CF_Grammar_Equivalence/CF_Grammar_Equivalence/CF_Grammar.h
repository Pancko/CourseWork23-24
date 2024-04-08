// ���� ���� ����� 2024
// ������� ����� ����������
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
#include "Timer.h"

// ������� �� ��-����������
struct Rule
{
	std::string left_part;					// ����� ����� �������
	std::vector<std::string> right_part;	// ������ ����� �������
	size_t terminals_count = 0;				// ������� ����������

	bool operator==(const Rule& Object) const;

	bool operator<(const Rule& Object) const;

	Rule();
	Rule(std::string Left_Part, std::vector<std::string> Right_Part);
	~Rule();
};

// ���� �� ����������� �� ������������� �����
struct Path
{
	int length;											// ����� ����
	std::vector<Rule> path_rules;						// �������
	std::vector<std::vector<std::string>> path_words;	// ������������������ ����
	std::vector<std::string> word;						// �������� �����

	bool operator==(const Path& Object) const;
	bool operator+=(const Path& Object);
	Path ApplyPath(const Path& Object, const int& position = 0);

	void PrintPath(bool IsDebug = false);

	Path();
	~Path();
};

// ����������� ���� ��������� ������������ ���� � ������ ���������� ������� (��� ������� ������������� ����������������)
struct PathPermutations
{
	Rule rule;
	std::vector<std::pair<std::string, std::vector<std::string>>> right_part;
};

// �������� ����� - ��-����������
class CF_Grammar
{
private:
	std::string starting_non_terminal;						// S
	std::map<std::string, std::vector<Path>> non_terminals; // N
	std::map<std::string, Path> shortest_path;				// ���������� ���� ��� ������������
	std::set<std::string> bad_non_terminals;				// "������" �����������
	std::set<std::string> terminals;						// Sigma
	std::vector<Rule> rules;								// P
	int pathes_amount = 0;
	std::set<std::string> words;							// ���������������� �����

public:
	CF_Grammar();
	~CF_Grammar();

	// ���������� ���������� �� �����
	void ReadFromFile(const std::string& File_Name);
	// ��������� ������� �� ������
	Rule GetRuleFromString(const std::string& String);
	// ���������� ������� � ����������
	void AddRule(const Rule& New_Rule);


	// ��������� �����. ������ � �������� ������, ����������� ������������
	void AnalyzeNonTerminals();
	// ������ ��������� �����.
	void GeneratePathes();
	// ��������� �����, ��������� ���� ������ �� ������������ ����
	std::map<std::string, std::vector<Path>> GenerateSubPath(const Path& Current_Path);
	// �������� ���� �� ������������
	bool IsUniquePath(const Path& Path_To_Check, const std::map<std::string, std::vector<Path>>& Current_Pathes);
	// �������� ������ ����� ������� �� �������������� �������� �������� ������ ����
	bool IsRuleViable(const Rule& Current_Rule, const std::map<std::string, std::vector<Path>>& Non_Terminals);


	// ���������� � ������� "������" ������������ (�� ������� ���������� ������� ��������� ������������ �����)
	void FindingBadNonTerminals();
	// �������� "������" ������������
	void DeleteBadNonTerminals();
	// ���������� ������� ���������� ����� ��� ������������� � ���������
	void FillShortestPathes();


	// �������� ������� ����������� � ����� ����
	bool GotNonTerminal(const Path& Current_Path);
	// �������� ������� ����������� � �����
	bool GotNonTerminal(const std::vector<std::string>& Word);


	// ����� ���������� � �������
	void PrintGrammar(bool IsDebug = false, bool ShowPath = false);


	// ��������� ���������� ������������� �����
	std::string GenerateWord(const int& Max_Length);
	// ��������� ����� � ����� ������������
	bool ReGenerateWord(const Path& Word);
	// ��������� ���������� ��������� ������������ ���� � �������� ������������ ������
	std::vector<std::string> GenerateMultipleWords(const int& Amount, const int& Max_Length);


	// ������ ��������������� ����
	void PrintWords(bool IsDebug = false);
	
	std::set<std::string> GetWords();

	// �������� ����-������-������, ����������� ��� ������������ ����������
	bool CYK_Alg(const std::string& Word);
	// ��������������� ������� ��� ����������
	int IndexOfNonTerminal(const std::string& Non_Terminal);
	int IndexOfRule(const Rule& Current_Rule);
	std::string VectorToString(const std::vector<std::string>& Object);


	std::vector<Rule> NonTerminalRules(const std::string& Non_Terminal);

	// �������� ����
	bool EarleyAlg(const std::vector<std::string>& Word);
};

// ���������� ������� � �����
std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number = 0);
// ���������� ���� ������� ������
std::map<std::string, std::vector<Path>> PathConvergence(const std::map<std::string, std::vector<Path>>& First_Object, const std::map<std::string, std::vector<Path>>& Second_Object);
// ���������� �� ������ � �������
bool VecContStr(const std::vector<std::string>& Vector, const std::string& String);