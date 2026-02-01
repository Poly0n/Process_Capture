#include "Processes.h"
#include <windows.h>
#include <iostream>
#include <TlHelp32.h>
#include <stdexcept>
#include <cstdlib>
#include <cwchar>
#include <Pdh.h>
#include "Execution.h"

/*

	PROBLEMS:
		when printing out filtered content you can still access other processes not listed (may not fix because who cares tbh)

*/

#pragma comment(lib, "pdh.lib")

// Prevent possible overflow
const size_t MAX_PROCESSES = 10000;

Processes::~Processes() {
	if (cpuInitialized) {
		PdhCloseQuery(cpuQuery);
	}
}

void Processes::waitForEnter() {
	std::cout << "[#] Press <Enter> to continue...";

	std::string dummy;
	std::getline(std::cin, dummy);

	if (std::cin.fail()) {
		std::cerr << "\n[!] Input error occurred" << std::endl;
		std::cin.clear();
	}
}

bool Processes::findPID(int processID) {
	for (const auto& proc : processSnapshot) {
		if (proc.pid == processID) {
			std::wcout << L"\n\t[+] PID Found: " << processID
				<< L" " << "\n\tProcess Name: [" << proc.processName <<
				"]" << std::endl;
			waitForEnter();
			return true;
		}
	}

	std::cout << "[-] Couldn't find PID :(" << std::endl;
	waitForEnter();
	return false;
}

std::wstring Processes::getName(int processID)	{
	for (const auto& proc : processSnapshot) {
		if (proc.pid == processID) {
			return proc.processName;
		}
	}
	return {};
}

void Processes::execute() {

	std::string userInput;
	cpuInit();
	std::cout << "Process Monitor Running... Press 'q' to quit" << std::endl;
	Sleep(2000);

	while (!breakLoop) {

		GetProcesses();
		if (filteredSnapsot.empty())
			print();
		else
			filteredPrint();

		std::cout << ">";
		std::getline(std::cin, userInput);

		ParseUserInput(userInput);
	}

	return;
}

bool Processes::findProcess(const std::wstring& processName) {
	for (const auto& proc : processSnapshot) {
		if (_wcsicmp(proc.processName.c_str(), processName.c_str()) == 0) {
			std::wcout << "\n\t[+] Found Process: \n\tPID: ["
				<< proc.pid << "] \n\tProcess Name:["
				<< proc.processName << "]" << std::endl;
			waitForEnter();
			return true;
		}
	}

	std::cout << "[-] Couldn't Find Process" << std::endl;
	waitForEnter();
	return false;
}

void Processes::ParseUserInput(const std::string& input) {

	if (input.empty()) return;

	size_t tokenFind = input.find(' ');
	std::string com;
	std::string value;

	if (tokenFind == std::string::npos) {
		com = input;
	}
	else {
		com = input.substr(0, tokenFind);
		value = input.substr(tokenFind + 1);
	}

	if (checkExecution.wordCommands(com)) {
		executeWordCommands(com, value);
		return;
	}


	if (checkExecution.oneLinerCommand(com)) {
		userCommand usrCom;
		usrCom.command = com;
		executeCommand(usrCom);
		return;
	}

	if (value.empty()) {
		std::cerr << "[!] You Didn't Enter A Value" << std::endl;
		waitForEnter();
		return;
	}

	if (!checkExecution.isNumber(value)) {
		std::cerr << "[!] The value you entered isn't a number" << std::endl;
		waitForEnter();
		return;
	}

	userCommand usrCom;
	usrCom.command = com;
	usrCom.value = std::stoi(value);

	executeCommand(usrCom);

	return;
}

void Processes::executeWordCommands(std::string& command, std::string& value) {
	//Execution checker;

	/*

			FIND FEATURE

	*/

	if (command == "find") {
		if (value.empty()) {
			std::cerr << "[!] find Requires A Process Name\n";
			waitForEnter();
			return;
		}

		std::wstring wstringValue = checkExecution.stringToWString(value);
		findProcess(wstringValue);
		return;
	}

	/*

			FILTER FEATURE

	*/

	else if (command == "filter") {
		if (value.empty()) {
			std::cerr << "[!] filter Requires A Process Name\n";
			waitForEnter();
			return;
		}

		filteredSnapsot.clear();
		filteredSnapsot.reserve(1000);

		std::wstring wstringValue = checkExecution.stringToWString(value);
		for (const auto& proc : processSnapshot) {
			if (_wcsicmp(proc.processName.c_str(), wstringValue.c_str()) == 0) {
				filteredSnapsot.push_back(proc);
			}
		}
		if (filteredSnapsot.empty()) {
			std::cout << "[-] Couldn't Find Process" << std::endl;
			filteredSnapsot.clear();
			waitForEnter();
			return;
		}

		filteredPrint();
		return;
	}
}

void Processes::killProcess(int processID) {
	HANDLE hProcess = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);

	if (hProcess == NULL) {
		std::cerr << "[-] OpenProcess Failed With Error: " << GetLastError() << std::endl;
		return;
	}
	if (TerminateProcess(hProcess, 1)) {
		std::cout << "\n\t[+] Process Terminated!" << std::endl;

		WaitForSingleObject(hProcess, 5000);
	}
	else {
		std::cerr << "[-] TerminateProcess Failed With Error: " << GetLastError() << std::endl;
		CloseHandle(hProcess);
		return;
	}
	CloseHandle(hProcess);
	return;
}

std::string Processes::findPath(int processID) {
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);

	if (hProcess == NULL) {
		int errorCode = GetLastError();
		if (errorCode = ERROR_ACCESS_DENIED) {
			std::cerr << "[-] Unable To Access This Process Due To Privaledges, Windows Doesn't Like It." << std::endl;
			waitForEnter();
			return {};
		}
		std::cerr << "[-] OpenProcess Failed With Error: " << GetLastError() << std::endl;
		waitForEnter();
		return {};
	}

	char path[MAX_PATH];
	DWORD size = MAX_PATH;

	if (!QueryFullProcessImageNameA(hProcess, 0, path, &size)) {
		std::cerr << "[-] QueryFullProcessImageNameA Failed: "
			<< GetLastError() << "\n";
		CloseHandle(hProcess);
		waitForEnter();
		return {};
	}

	CloseHandle(hProcess);
	return std::string(path, size);
}

void Processes::executeCommand(userCommand com) {

	if (com.command == "pid") {
		findPID(com.value);
		return;
	}
	else if (com.command == "kill") {
		if (findPID(com.value)) {
			killProcess(com.value);
			waitForEnter();
			return;
		}
		else {
			return;
		}
	}
	else if (com.command == "path") {
		// find full path of executable
		if (findPID(com.value)) {
			std::string fullPath = findPath(com.value);
			if (fullPath.empty()) {
				return;
			}
			else {
				std::cout << "\t[+]Full Path: " << fullPath << std::endl;
				waitForEnter();
				return;
			}
		}
		else {
			return;
		}
	}
	else if (com.command == "help") {
		std::cout << "\n\tCommands:\n" << std::endl;
		std::cout << "\tpid (number)\tRetrieves Program Name Associated With The Process ID." << std::endl;
		std::cout << "\tkill (pid)\tTerminates Specified Process." << std::endl;
		std::cout << "\tfind (name.exe)\tFinds Processes PID." << std::endl;
		std::cout << "\trefresh\t\tRefreshes The Screen To Update Results. This Also Can Be Used To Undo Filtered Results." << std::endl;
		std::cout << "\tfilter (name.exe) Filters Processes To Only Display Processes Specified By The User." << std::endl;
		std::cout << "\tpath (pid)\tShows The Full File Path Of A Running Process." << std::endl;
		std::cout << "\tinfo (pid)\tGets Process's Uptime, CPU Usage, And Parent Process" << std::endl;
		std::cout << "\tq\t\tQuits Application." << std::endl;
		waitForEnter();
		return;
	}
	else if (com.command == "refresh") {
		filteredSnapsot.clear();
		return;
	}
	else if (com.command == ("q") || com.command == ("Q")) {
		std::cout << "\nExiting..." << std::endl;
		breakLoop = true;
		return;
	}
	else if (com.command == ("info")) {
		if (findPID(com.value)) {
			std::wstring procName = getName(com.value);
			if (procName.empty()) {
				std::cerr << "[-] Couldn't Find Process" << std::endl;
				return;
			}
			checkExecution.printProcessTimes(com.value, procName);
			waitForEnter();
		}

		return;
	}
	else {
		std::cerr << "[!] The command you entered '" << com.command << "' doesn't exist. Please try again." << std::endl;
		waitForEnter();
	}

	return;
}

void Processes::cpuInit() {
	if (!cpuInitialized) {
		PDH_STATUS status = PdhOpenQuery(NULL, NULL, &cpuQuery);
		if (status != ERROR_SUCCESS) {
			std::cerr << "[-] Failed to open PDH query" << std::endl;
			return;
		}
		status = PdhAddCounterW(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
		if (status != ERROR_SUCCESS) {
			std::cerr << "[-] Failed to add PDH counter" << std::endl;
			PdhCloseQuery(cpuQuery);
			return;
		}

		PdhCollectQueryData(cpuQuery);
		Sleep(100);
		PdhCollectQueryData(cpuQuery);
		cpuInitialized = true;
	}
}

void Processes::GetCurrentCPUUsage() {
	PDH_FMT_COUNTERVALUE counterVal;
	PDH_STATUS status;

	status = PdhCollectQueryData(cpuQuery);

	if (status != ERROR_SUCCESS) {
		std::cerr << "[-] PdhCollectQueryData failed: " << status << std::endl;
		return;
	}

	status = PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
	if (status != ERROR_SUCCESS) {
		printf("[-] PdhGetFormattedCounterValue failed: %.08x", status);
		return;
	}

	cpuUsage = counterVal.doubleValue;
	return;
}

void Processes::GetCurrentMemoryUsage() {
	DWORDLONG convert;

	MEMORYSTATUSEX memInfo;
	memInfo.dwLength = sizeof(MEMORYSTATUSEX);
	GlobalMemoryStatusEx(&memInfo);
	convert = memInfo.ullTotalPhys - memInfo.ullAvailPhys;

	memoryUsage = static_cast<double>(convert) / (1024.0 * 1024 * 1024);

	return;
}

void Processes::GetProcesses() {
	processSnapshot.clear();
	processSnapshot.reserve(1000);
	HANDLE hSnapshot;
	PROCESSENTRY32W pe;
	int counter = 0;
	BOOL hResult;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		std::cerr << "[-] CreateToolhelp32Snapshot Failed With Error: " << GetLastError() << std::endl;
		waitForEnter();
		return;
	}

	pe.dwSize = sizeof(PROCESSENTRY32W);

	hResult = Process32FirstW(hSnapshot, &pe);

	// Looping through all running processes
	while (hResult && processSnapshot.size() < MAX_PROCESSES) {
		Process proc;
		proc.pid = pe.th32ProcessID;
		proc.processName = pe.szExeFile;
		processSnapshot.push_back(proc);

		hResult = Process32NextW(hSnapshot, &pe);
	}

	if (processSnapshot.size() >= MAX_PROCESSES) {
		std::cerr << "[!] Process limit reaced" << std::endl;
	}

	CloseHandle(hSnapshot);

	return;
}

void Processes::print() {
	system("cls");
	GetCurrentMemoryUsage();
	GetCurrentCPUUsage();

	for (size_t i = 0; i < processSnapshot.size(); ++i) {
		std::cout << "Process ID: [" << processSnapshot[i].pid << "]";
		std::wcout << "\tProcess Name: [" << processSnapshot[i].processName << "]\n" << std::endl;
	}

	std::cout << "-----------------------------------" << std::endl;
	std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
	std::cout << "Memory Usage: " << memoryUsage << " GB" << std::endl;
	std::cout << "Total Processes: " << processSnapshot.size() << "\n" << std::endl;
}

void Processes::filteredPrint() {

	system("cls");
	GetCurrentMemoryUsage();
	GetCurrentCPUUsage();

	for (size_t i = 0; i < filteredSnapsot.size(); ++i) {
		std::cout << "Process ID: [" << filteredSnapsot[i].pid << "]";
		std::wcout << "\tProcess Name: [" << filteredSnapsot[i].processName << "]\n" << std::endl;
	}

	std::cout << "-----------------------------------" << std::endl;
	std::cout << "CPU Usage: " << cpuUsage << "%" << std::endl;
	std::cout << "Memory Usage: " << memoryUsage << " GB" << std::endl;
	std::cout << "Total Processes: " << processSnapshot.size() << "\n" << std::endl;
}
