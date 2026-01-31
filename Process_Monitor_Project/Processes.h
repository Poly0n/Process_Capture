#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <windows.h>
#include <TlHelp32.h>
#include "Pdh.h"

struct Process {
	int pid;
	std::wstring processName;
};

struct userCommand {
	std::string command;
	int value;
};

class Processes
{
public:
	~Processes();
	void execute();
private:
	bool breakLoop = false;
	bool cpuInitialized = false;
	std::vector<Process> processSnapshot;
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	double cpuUsage;
	DWORDLONG memoryUsage;

	void GetProcesses();
	void print();
	void cpuInit();
	void GetCurrentCPUUsage();
	void GetCurrentMemoryUsage();
	void ParseUserInput(const std::string& input);
	void executeCommand(userCommand com);
	void waitForEnter();
	void killProcess(int processID);
	bool findProcess(std::wstring& processName);

	bool findPID(int processID);
	bool isNumber(const std::string& number) const;
	bool checkCommand(const std::string& input) const;

	std::wstring stringToWString(const std::string& str);
};

