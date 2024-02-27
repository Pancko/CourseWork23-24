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

	void PrintPath();

	Path();
	~Path();
};

class CF_Grammar
{
private:
	std::string starting_non_terminal; // S
	std::map<std::string, std::vector<Path>> non_terminals; //N
	std::map<std::string, int> shortest_path;
	std::set<std::string> bad_non_terminals;
	std::set<std::string> terminals; //Sigma
	std::vector<Rule> rules; //P

public:
	CF_Grammar();
	~CF_Grammar();

	// ���������� ���������� �� �����
	void ReadFromFile(const std::string& File_Name);
	// ��������� ������� �� ������
	Rule GetRuleFromString(const std::string& String);
	// ���������� ������� � ����������
	void AddRule(const Rule& New_Rule);

	// ��������� �����. ��������� �����������, �� ������� ����� ��������� ��������� ������������ �����
	void GeneratePathes();
	// ��������� �����. ���������� ���� ������ �� ����������
	void GenerateSubPathes();
	// ��������� �����. ��������� ���� ���������� ����� �� ��������� ������������ ���� (���� ��������)
	void GenerateFinalPathes();

	// ���������� � ������� ������ ������������ (�� ������� ���������� ������� ��������� ������������ �����)
	void FindingBadNonTerminals();

	// �������� ������� ����������� � ����� ����
	bool GotNonTerminal(const Path& Current_Path);

	// ����� ���������� � �������
	void PrintGrammar();

	std::string GenerateWord(const int& Min_Length);


	//int min_element(const std::vector<Path, std::allocator<Path>>& Obj);

	bool KYCAlg(const std::vector<std::string>& Word);
	bool EarleyAlg(const std::vector<std::string>& Word);
};

	std::vector<std::string> ApplyRule(const std::vector<std::string>& String, const Rule& Rule, const int& Non_Terminal_Number);
