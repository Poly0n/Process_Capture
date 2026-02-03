#pragma once
#include "Execution.h"
#include <iostream>
#include <string>
#include <vector>
#include <TlHelp32.h>
#include <Pdh.h>
#include <unordered_map>

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
	Processes() = default;
	virtual ~Processes();
	void execute();
	Execution checkExecution;
private:
	bool breakLoop = false;
	bool cpuInitialized = false;
	PDH_HQUERY cpuQuery;
	PDH_HCOUNTER cpuTotal;
	double cpuUsage;
	DWORDLONG memoryUsage;
	std::unordered_map<DWORD, Process> processSnapshot;
	std::unordered_map<DWORD, Process> filteredSnapsot;

	void GetProcesses();
	void print();
	void filteredPrint();
	void cpuInit();
	void GetCurrentCPUUsage();
	void GetCurrentMemoryUsage();
	void ParseUserInput(const std::string& input);
	void executeCommand(userCommand com);
	void executeWordCommands(std::string& command, std::string& value);
	void killProcess(int processID);
	void waitForEnter();
	std::string findPath(int processID);
	std::wstring getName(int processID);
	bool findProcess(const std::wstring& processName);

	bool findPID(int processID);
};
