#pragma once

#include <regex>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>

class GlslPreprocessor
{
public:
	GlslPreprocessor();

public:
	bool addShaderSource(const std::string& shaderPath);
	bool specifyVersionString(const std::string& versionString);

	void addMacro(const std::string& macro, const std::string& value);
	void removeMacro(const std::string& macro);
	void defineMacro(const std::string& macro);

	void reset();

	bool process(std::string& result);

private:
	bool processVersionString();
	bool processMacros();
	bool processRest();

	bool processInclude(const std::string& includePath);

	bool include(const std::string& includePath);
	std::string relate(const std::string& basePath, const std::string& relative);

private:
	std::regex m_includePattern;
	std::regex m_versionStringPattern;
	std::regex m_whitespaceOrEmptyPattern;

	std::string m_source;
	std::string m_shaderPath;
	std::string m_versionString;
	std::unordered_map<std::string, std::string> m_macros;
	std::unordered_set<std::string> m_included;

	std::ostringstream m_processed;
	std::istringstream m_toProcess;
};
