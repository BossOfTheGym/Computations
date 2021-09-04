#include "GlslPreprocessor.h"

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace
{
	const char* INCLUDE_PATTERN = "^[\\\\w]*#include \"(.)+\"[\\\\w]*$";
	const char* VERSION_STRING_PATTERN = "^[\\\\w]*#version ([\\\\d]{3}) (core|compatability|es)?[\\\\w]*$";
	const char* WHITESPACE_OR_EMPTY_PATTERN = "[\\\\w]*";

	bool read_whole_file(const std::string& path, std::string& result)
	{
		std::ifstream stream(path);
		if (!stream) {
			return false;
		}

		auto start  = std::istreambuf_iterator<char>(stream);
		auto finish = std::istreambuf_iterator<char>();
		result = std::string(start, finish);

		return true;
	}
}

GlslPreprocessor::GlslPreprocessor()
	: m_includePattern(INCLUDE_PATTERN, std::regex_constants::ECMAScript)
	, m_versionStringPattern(VERSION_STRING_PATTERN, std::regex_constants::ECMAScript)
	, m_whitespaceOrEmptyPattern(WHITESPACE_OR_EMPTY_PATTERN, std::regex_constants::ECMAScript)
{}

bool GlslPreprocessor::addShaderSource(const std::string& shaderPath)
{
	if (read_whole_file(shaderPath, m_source)) {
		m_shaderPath = shaderPath;
		return true;
	}
	return false;
}

bool GlslPreprocessor::specifyVersionString(const std::string& versionString)
{
	if (!versionString.empty() && !std::regex_match(versionString, m_versionStringPattern)) {
		return false;
	}
	m_versionString = versionString;
	return true;
}

void GlslPreprocessor::addMacro(const std::string& macro, const std::string& value)
{
	if (!macro.empty()) {
		m_macros[macro] = value;
	}
}

void GlslPreprocessor::removeMacro(const std::string& macro)
{
	m_macros.erase(macro);
}

void GlslPreprocessor::defineMacro(const std::string& macro)
{
	addMacro(macro, "");
}

void GlslPreprocessor::reset()
{
	m_source = std::string();
	m_versionString = std::string();
	m_macros.clear();
	m_included.clear();
	m_processed = std::ostringstream();
	m_toProcess = std::istringstream();
}

bool GlslPreprocessor::process(std::string& result)
{
	m_toProcess = std::istringstream(m_source);
	return processVersionString() 
		&& processMacros()
		&& processRest();
}

bool GlslPreprocessor::processVersionString()
{
	std::string line;
	while (std::getline(m_toProcess, line)) {
		if (std::regex_match(line, m_whitespaceOrEmptyPattern)) {
			continue;
		}

		if (std::regex_match(line, m_versionStringPattern)) {
			if (!m_versionString.empty()) {
				m_processed << m_versionString << '\n';
			}
			else {
				m_processed << line << '\n';
			}
			break;
		}
	}
	return (bool)m_toProcess && (bool)m_processed;
}

bool GlslPreprocessor::processMacros()
{
	for (auto& [macro, value] : m_macros) {
		m_processed << "#define " << macro << " " << value << '\n';
	}
	return (bool)m_toProcess && (bool)m_processed;
}

bool GlslPreprocessor::processRest()
{
	std::string line;
	while (std::getline(m_toProcess, line)) {
		std::smatch match;
		if (std::regex_match(line, match, m_includePattern)) {
			std::string includePathStr = match[1].str();
			if (!wasIncluded(includePathStr)) {
				processInclude(m_shaderPath, includePathStr);
			}
		}
		else {
			m_processed << line << '\n';
		}
	}
	return (bool)m_toProcess && (bool)m_processed;
}

bool GlslPreprocessor::processInclude(const std::string& includingFile, const std::string& includePath)
{
	fs::path path = includePath;
	if (!path.is_absolute()) {
		path = fs::path(includingFile).parent_path() / includePath;
	}

	std::string contents;
	if (!read_whole_file(path.string(), contents)) {
		return false;
	}

	std::istringstream input(contents);

	std::string line;
	while (std::getline(input, line)) {
		std::smatch match;
		if (std::regex_match(line, match, m_includePattern)) {
			std::string includePathStr = match[1].str();
			if (!wasIncluded(includePathStr)) {
				processInclude(path.string(), includePathStr);
			}
		}
		else {
			m_processed << line << '\n';
		}
	}
	return (bool)input && (bool)m_processed;
}

bool GlslPreprocessor::wasIncluded(const std::string& includePath)
{
	auto absolute = fs::absolute(includePath).string();
	auto [_, wasInserted] = m_included.insert(absolute);
	return wasInserted;
}
