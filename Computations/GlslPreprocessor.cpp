#include "GlslPreprocessor.h"

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

namespace
{
	const char* INCLUDE_PATTERN = R"reg(^\s*#include\s"(.+)"\s*$)reg";
	const char* VERSION_STRING_PATTERN = R"reg(^\s*#version\s+(\d{3})\s+(core|compatability|es)?\s*$)reg";
	const char* WHITESPACE_OR_EMPTY_PATTERN = R"reg(\s*)reg";

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
	: m_includePattern(INCLUDE_PATTERN)
	, m_versionStringPattern(VERSION_STRING_PATTERN)
	, m_whitespaceOrEmptyPattern(WHITESPACE_OR_EMPTY_PATTERN)
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
	if (processVersionString() && processMacros() && processRest()) {
		result = m_processed.str();
		return true;
	}
	return false;
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
	return true;
	//return !!m_toProcess && !!m_processed;
}

bool GlslPreprocessor::processMacros()
{
	if (!m_macros.empty()) {
		m_processed << '\n';
	}
	for (auto& [macro, value] : m_macros) {
		m_processed << "#define " << macro << " " << value << '\n';
	}
	return true;
	//return !!m_toProcess && !!m_processed;
}

bool GlslPreprocessor::processRest()
{
	include(m_shaderPath);

	std::string line;
	while (std::getline(m_toProcess, line)) {
		std::smatch match;
		if (std::regex_match(line, match, m_includePattern)) {
			std::string includePathStr = relate(m_shaderPath, match[1].str());
			if (!include(includePathStr)) {
				if (!processInclude(includePathStr)) {
					return false;
				}
			}
		}
		else {
			m_processed << line << '\n';
		}
	}
	return true;
	//return !!m_toProcess && !!m_processed;
}

bool GlslPreprocessor::processInclude(const std::string& includePath)
{
	std::string contents;
	if (!read_whole_file(includePath, contents)) {
		return false;
	}

	std::istringstream input(contents);

	std::string line;
	while (std::getline(input, line)) {
		std::smatch match;
		if (std::regex_match(line, match, m_includePattern)) {
			std::string includePathStr = relate(includePath, match[1].str());
			if (!include(includePathStr)) {
				if (!processInclude(includePathStr)) {
					return false;
				}
			}
		}
		else {
			m_processed << line << '\n';
		}
	}
	return true;
	//return !!input && !!m_processed;
}

bool GlslPreprocessor::include(const std::string& includePath)
{
	auto absolute = fs::absolute(includePath).string();
	auto [_, wasInserted] = m_included.insert(absolute);
	return !wasInserted;
}

std::string GlslPreprocessor::relate(const std::string& basePath, const std::string& relative)
{
	fs::path path = relative;
	if (!path.is_absolute()) {
		fs::path base = basePath;
		path = base.parent_path() / path;
	}
	return path.string();
}
