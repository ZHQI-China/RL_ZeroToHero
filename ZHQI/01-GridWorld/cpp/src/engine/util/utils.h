#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
static std::string to_string_with_precision(float a, int precision = 2) {
	std::ostringstream out;
	out << std::fixed << std::setprecision(precision) << a;
	return out.str();
}