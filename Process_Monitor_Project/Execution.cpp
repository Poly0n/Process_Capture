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

	int size_needed = MultiByteToWideChar(CP_UTF8, 0,
		str.c_str(), (int)str.size(),
		NULL, 0);
	if (size_needed <= 0) return L"";

	std::wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0,
		str.c_str(), (int)str.size(),
		&wstr[0], size_needed);
	return wstr;
}

bool Execution::wordCommands(const std::string& input) const {
	return input == "find" || input == "filter";
}

bool Execution::oneLinerCommand(const std::string& input) const {
	return input == "help" || input == "refresh"
		|| input == "q" || input == "Q";
}