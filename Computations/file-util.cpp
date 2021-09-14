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
	constexpr int CHUNK_SIZE = 256;
	char chunk[CHUNK_SIZE + 1];

	std::ostringstream output;
	while (is.good()) {
		if (!is.read(chunk, CHUNK_SIZE)) {
			return false;
		}
		chunk[is.gcount()] = '\0';

		output << chunk;
	}
	if (is.fail()) {
		return false;
	}

	contents = output.str();
	return true;
}