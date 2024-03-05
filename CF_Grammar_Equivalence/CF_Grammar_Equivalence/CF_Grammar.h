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

// ������� �� ��-����������
struct Rule
{
	std::string left_part;					// ����� ����� �������
	std::vector<std::string> right_part;	// ������ ����� �������
	size_t terminals_count = 0;				// ������� ����������

	bool operator==(const Rule& Object) const;

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

	void PrintPath(bool IsDebug = false);

	Path();
	~Path();
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
	bool GenerateSubPath(const Path& Current_Path);
	// �������� ���� �� ������������
	bool IsUniquePath(const Path& Path_To_Check, std::map<std::string, std::vector<Path>> Current_Pathes);


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
	// ��������������� ������� ��������� ����-������-������
	bool a(const std::string& Non_Terminal, const int& I, const int& J);
	bool h(const std::string& Word, const Rule& Rule, const int& I, const int& J, const int& K);

	// �������� ����
	bool EarleyAlg(const std::vector<std::string>& Word);
};

// ���������� ������� � �����
std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number = 0);
