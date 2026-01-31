#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <windows.h>
#include <TlHelp32.h>
#include "Pdh.h"

class Execution
{
private:
public:
	bool oneLinerCommand(const std::string& input) const;
	bool isNumber(const std::string& number) const;
	bool wordCommands(const std::string& input) const;
	std::wstring stringToWString(const std::string& str);
};
