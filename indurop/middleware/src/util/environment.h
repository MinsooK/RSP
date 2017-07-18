
#ifndef UTIL_ENVIRONMENT_H__
#define UTIL_ENVIRONMENT_H__

#include <string>

#include <bicomc/detail/config.h>

namespace irp {

	class Environment
	{
	public:
		static void init(int argc, char const* argv[]);

	public:
		static std::string const& programDirectory();
		static std::string const& programName();

		static std::string const& variableDirectory();

	public:
		static std::string absolute(std::string path);
		static bool exists(std::string const& path);
		static bool is_directory(std::string const& path);

		static bool create_directory(std::string const& path);
		static bool create_directory(std::string const& path, std::string const& parent);
		static bool create_directories(std::string const& path);

	private:
		static std::string mProgramDir;
		static std::string mProgramName;
		static std::string mVariableDir;
	};
}

#endif // !def UTIL_ENVIRONMENT_H__
