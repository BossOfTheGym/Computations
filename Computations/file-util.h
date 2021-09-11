#pragma once

#include <string>
#include <iosfwd>

bool read_whole_file(const std::string& path, std::string& contents);

bool read_whole_stream(std::istream& is, std::string& contents);
