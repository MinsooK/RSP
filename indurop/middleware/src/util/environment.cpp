
#include "environment.h"

#include <stdlib.h>

#include <algorithm>
#include <string>
#include <stdexcept>

#if defined(_WIN32)
#	include <Windows.h>

#else
#	include <sys/stat.h>
#	include <limits.h>

#endif

#include "../variable/variable_manager.h"

using irp::Environment;

std::string Environment::mProgramDir;
std::string Environment::mProgramName;
std::string Environment::mVariableDir;

void Environment::init(int argc, char const* argv[])
{
	if (argc <= 0 || argv == nullptr)
		throw std::invalid_argument("size of 'argv' must be lager than 0");

	// program path
	std::string abs = absolute(argv[0]);
	mProgramDir = abs.substr(0, abs.find_last_of('/'));
	mProgramName = abs.substr(mProgramDir.empty() ? 0 : mProgramDir.size());

	// variable path
	std::string candidates[] = {
		absolute(programDirectory() + "/var")
		, absolute(programDirectory() + "/../var")
	};
	for (std::size_t i = 0, size = sizeof(candidates) / sizeof(*candidates); i < size; ++i)
	{
		if (exists(candidates[i]))
		{
			mVariableDir = candidates[i];
			break;
		}
	}

	if (mVariableDir.empty())
	{
		if (!create_directory(candidates[0]))
			throw std::runtime_error("cannot create directory of variable");
		mVariableDir = candidates[0];
	}

	{
		irp::VariableManager newVarManager(mVariableDir + "/InduRoP-Shared-Variable.txt");
		gVariableManager.swap(newVarManager);
	}
}

std::string const& Environment::programDirectory()
{
	return mProgramDir;
}

std::string const& Environment::programName()
{
	return mProgramName;
}

std::string const& irp::Environment::variableDirectory()
{
	return mVariableDir;
}

std::string Environment::absolute(std::string path)
{
	if (path.empty()) return path;

#if defined(_WIN32)
	std::replace(path.begin(), path.end(), '/', '\\');
	char converted[_MAX_PATH] = {};
	if (::_fullpath(converted, path.c_str(), _MAX_PATH))
	{
		path = converted;
	}
	else
	{
		std::string converted;
		for (;;)
		{
			converted.resize(converted.size() + _MAX_PATH * 2);

			if (::_fullpath(&converted[0], path.c_str(), converted.size()))
			{
				path = converted.c_str();
				break;
			}

			if (errno != EINVAL)
				throw std::runtime_error("cannot get a absolute path.");
		}
	}

#else
	std::replace(path.begin(), path.end(), '\\', '/');
	char converted[PATH_MAX] = {};
	realpath(path.c_str(), converted);
	path = converted;

#endif

	std::replace(path.begin(), path.end(), '\\', '/');
	return path;
}

bool Environment::exists(std::string const& path)
{
#if defined(_WIN32)
	DWORD attr = GetFileAttributesA(path.c_str());
	if (attr == INVALID_FILE_ATTRIBUTES)
		return false;
	return true;

#else
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;
	return true;

#endif // defined(_WIN32)
}

bool Environment::is_directory(std::string const& path)
{
#if defined(_WIN32)
	DWORD attr = GetFileAttributesA(path.c_str());
	if (attr == INVALID_FILE_ATTRIBUTES || !(attr & FILE_ATTRIBUTE_DIRECTORY))
		return false;
	return true;

#else
	struct stat st;
	if (stat(path.c_str(), &st) != 0)
		return false;
	return S_ISDIR(st.st_mode);

#endif // defined(_WIN32)
}

bool Environment::create_directory(std::string const& path)
{
	std::string parent = path.substr(0, path.rfind('/'));
	return create_directory(path, parent);
}

bool Environment::create_directory(std::string const& path, std::string const& parent)
{
#if defined(_WIN32)
	if (!CreateDirectoryExA(parent.c_str(), path.c_str(), 0))
		return false;
	return true;

#else
	struct stat st;
	if (stat(parent.c_str(), &st) != 0)
		return false;
	if (mkdir(path.c_str(), st.st_mode) != 0)
		return false;
	return true;

#endif
}

bool Environment::create_directories(std::string const& path)
{
	if (path.empty()) return false;

	std::string temp;
	temp.reserve(path.size());
	
	std::string::size_type pos = std::string::npos;
	do
	{
		pos = path.find('/', pos + 1);
		temp.clear();
		temp.assign(path.c_str(), pos);

		if (!create_directory(temp))
			return false;
	} while (pos != std::string::npos);

	return true;
}
