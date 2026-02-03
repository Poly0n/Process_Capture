#include "Execution.h"
#include "Processes.h"

bool Execution::isNumber(const std::string& number) const {

	if (number.empty()) {
		return false;
	}

	if (number.length() > 10) {
		return false;
	}

	for (auto a : number) {
		if (!std::isdigit(static_cast<unsigned char>(a))) {
			return false;
		}
	}

	if (number.length() > 1 && number[0] == '0') {
		return false;
	}

	try {
		unsigned long value = std::stoul(number);
		if (value > 65535)
			return false;
	}
	catch (...) {
		return false;
	}

	return true;
}

std::wstring Execution::stringToWString(const std::string& str) {
	if (str.empty()) return L"";

	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
	if (size_needed <= 0) return L"";

	std::wstring wstr(size_needed - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size_needed);
	return wstr;
}

bool Execution::wordCommands(const std::string& input) const {
	return input == "find" || input == "filter";
}

bool Execution::oneLinerCommand(const std::string& input) const {
	return input == "help" || input == "refresh"
		|| input == "q" || input == "Q";
}

ULONGLONG Execution::fileTimeToULL(const FILETIME& ft) {
	return (((ULONGLONG)ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

double Execution::fileTimeToSeconds(const FILETIME& ft) {
	return fileTimeToULL(ft) / 10000000.0;
}

std::wstring Execution::toLower(std::wstring s) {
	std::transform(s.begin(), s.end(), s.begin(),
		[](wchar_t c) {
			return std::towlower(c);
		});
	return s;
}

void Execution::GetParentProcessID(DWORD pid, bool reset = true) {
	if (reset) {
		ppidInfo.ppid = 0;
		ppidInfo.ppName.clear();
	}

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32W pe;
	pe.dwSize = sizeof(PROCESSENTRY32W);

	if (Process32FirstW(hSnapshot, &pe)) {
		do {
			if (pe.th32ProcessID == pid) {
				if (ppidInfo.ppid == 0 && ppidInfo.ppName.empty()) {
					ppidInfo.ppid = pe.th32ParentProcessID;
					GetParentProcessID(ppidInfo.ppid, false);
				}
				else {
					ppidInfo.ppName = pe.szExeFile;
					CloseHandle(hSnapshot);
					return;
				}
				
			}
		} while (Process32NextW(hSnapshot, &pe));
	}

	CloseHandle(hSnapshot);
	return;
}

void Execution::printProcessTimes(int processID, std::wstring& processName) {
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processID);
	if (!hProcess) {
		if (GetLastError() == 5 || GetLastError() == 87) {
			std::cerr << "[-] OpenProcess Error: Can't Get Information On System Processes." << std::endl;
		}
		else {
			std::cerr << "[-] OpenProcess Error:" << GetLastError() << std::endl;
		}
		return;
	}

	FILETIME creationTime, exitTime, kernelTime, userTime;
	if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
		CloseHandle(hProcess);
		return;
	}

	SYSTEMTIME stUTC, stLocal;
	FileTimeToSystemTime(&creationTime, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

	CpuSample& cpuSample = cpuSamples[processID];
	GetProcessCpuUsage(hProcess, cpuSample);
	std::cout << "\t[i] Getting CPU Usage...\n";
	Sleep(2000);
	double cpu = GetProcessCpuUsage(hProcess, cpuSample);

	GetParentProcessID(processID);

	std::wcout << L"\t[+] Created: "
		<< stLocal.wMonth << L"/" << stLocal.wDay << L"/" << stLocal.wYear
		<< L" " << stLocal.wHour << L":" << stLocal.wMinute << L":" << stLocal.wSecond
		<< std::endl;
	std::wcout << L"\t[+] Kernel Time: " << fileTimeToSeconds(kernelTime) << L" sec\n";
	std::wcout << L"\t[+] User Time: " << fileTimeToSeconds(userTime) << L" sec\n";
	std::wcout << L"\t[+] Parent PID: " << ppidInfo.ppid << std::endl;
	if (ppidInfo.ppName.empty()) {
		std::wcout << L"\t[-] No Parent Name" << std::endl;
	}
	else {
		std::wcout << L"\t[+] Parent Name: " << ppidInfo.ppName << std::endl;
	}
	std::wcout << L"\t[+] CPU Usage: " << cpu << std::endl;

	CloseHandle(hProcess);
}

double Execution::GetProcessCpuUsage(HANDLE hProcess, CpuSample& sample) {
	FILETIME ftCreation, ftExit, ftKernel, ftUser;
	FILETIME ftNow;

	if (!GetProcessTimes(hProcess, &ftCreation, &ftExit, &ftKernel, &ftUser)) {
		std::cerr << "GetProcessTimes Error:" << GetLastError() << std::endl;
		Sleep(1000);
		return 0.0;
	}


	GetSystemTimePreciseAsFileTime(&ftNow);

	ULONGLONG procTime = fileTimeToULL(ftKernel) + fileTimeToULL(ftUser);
	ULONGLONG sysTime = fileTimeToULL(ftNow);

	if (sample.lastSystemTime == 0) {
		sample.lastProcessTime = procTime;
		sample.lastSystemTime = sysTime;
		return 0.0;
	}

	ULONGLONG procDelta = procTime - sample.lastProcessTime;
	ULONGLONG sysDelta = sysTime - sample.lastSystemTime;

	sample.lastProcessTime = procTime;
	sample.lastSystemTime = sysTime;


	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);

	if (sysDelta == 0 || sysInfo.dwNumberOfProcessors == 0)
		return 0.0;
	return (static_cast<double>(procDelta) * 100.0) / (static_cast<double>(sysDelta) * sysInfo.dwNumberOfProcessors);
}
