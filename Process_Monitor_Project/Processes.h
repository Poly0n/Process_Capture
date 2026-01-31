#pragma once
#include "Execution.h"
#include <iostream>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <Pdh.h>

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
	Execution checkExecution;
private:
	bool breakLoop = false;
	bool cpuInitialized = false;
	std::vector<Process> processSnapshot;
	std::vector<Process> filteredSnapsot;
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	double cpuUsage;
	DWORDLONG memoryUsage;

	void GetProcesses();
	void print();
	void filteredPrint();
	void cpuInit();
	void GetCurrentCPUUsage();
	void GetCurrentMemoryUsage();
	void ParseUserInput(const std::string& input);
	void executeCommand(userCommand com);
	void executeWordCommands(std::string& command, std::string& value);
	void waitForEnter();
	void killProcess(int processID);
	std::string findPath(int processID);
	bool findProcess(const std::wstring& processName);

	bool findPID(int processID);
};
