#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <windows.h>
#include <TlHelp32.h>
#include <unordered_map>
#include <algorithm>
#include <cwctype>
#include "Pdh.h"

struct CpuSample {
	ULONGLONG lastProcessTime = 0;
	ULONGLONG lastSystemTime = 0;
};

struct ppIDInformation {
	DWORD ppid = 0;
	std::wstring ppName = {};
};

class Execution
{
private:
	std::unordered_map<DWORD, CpuSample> cpuSamples;
	ppIDInformation ppidInfo = {};
	double fileTimeToSeconds(const FILETIME& ft);
	ULONGLONG fileTimeToULL(const FILETIME& ft);
public:
	std::wstring toLower(std::wstring s);
	bool oneLinerCommand(const std::string& input) const;
	bool isNumber(const std::string& number) const;
	bool wordCommands(const std::string& input) const;
	std::wstring stringToWString(const std::string& str);
	void printProcessTimes(int processID, std::wstring& processName);
	double GetProcessCpuUsage(HANDLE hProcess, CpuSample& sample);
	void GetParentProcessID(DWORD pid, bool reset);
};
