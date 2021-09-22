#pragma once

#include <string>
#include <sstream>

template<class T>
bool parse(const std::string& str, T& value)
{
	std::istringstream in(str);
	return (bool)(in >> value);
}
