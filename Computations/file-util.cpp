#include "file-util.h"


#include <fs.h>
#include <fstream>
#include <sstream>

bool read_whole_file(const std::string& path, std::string& contents)
{
	std::ifstream stream(path);
	if (!stream) {
		return false;
	}
	return read_whole_stream(stream, contents);
}

bool read_whole_stream(std::istream& is, std::string& contents)
{
	std::string line;
	std::ostringstream output;
	// TODO : IO error checks
	while (std::getline(is, line)) {
		output << line << '\n';
	}
	contents = output.str();

	return true;
}