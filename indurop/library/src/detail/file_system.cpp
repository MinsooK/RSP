
#include <indurop/detail/file_system.h>

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>

#include <algorithm>
#include <stdexcept>

#if defined(_WIN32)
#	include <Windows.h>

#else
#	include <sys/stat.h>
#	include <unistd.h>
#	include <limits.h>

#endif // defined(_WIN32)

#include <bicomc/type_traits.h>

using namespace irp::filesystem;

namespace {
	std::string convert(wchar_t const* str)
	{
		if (!str || !*str) return std::string();

		std::string temp;
		temp.resize(std::wcslen(str) * MB_LEN_MAX);

		std::size_t size = std::wcstombs(&temp[0], str, temp.size());
		if (size == std::size_t(-1))
			throw std::invalid_argument("encoding of 'path' is invalid");
		temp.resize(size);

		return temp;
	}

	std::wstring convert(char const* str)
	{
		if (!str || !*str) return std::wstring();

		std::wstring temp;
		temp.resize(std::strlen(str));

		std::size_t size = std::mbstowcs(&temp[0], str, temp.size());
		if (size == std::size_t(-1))
			throw std::invalid_argument("encoding of 'path' is invalid");
		temp.resize(size);

		return temp;
	}

	std::string toString(std::wstring const& str)
	{
		return convert(str.c_str());
	}

	std::string toString(std::string const& str)
	{
		return str;
	}

	std::wstring toWstring(std::wstring const& str)
	{
		return str;
	}

	std::wstring toWstring(std::string const& str)
	{
		return convert(str.c_str());
	}

	std::string trim(std::string const& str)
	{
		if (str.size() == 0)
			return str;

		std::size_t beg = str.find_first_not_of(" \a\b\f\n\r\t\v");
		std::size_t end = str.find_last_not_of(" \a\b\f\n\r\t\v");
		if (beg == std::string::npos) // No non-spaces
			return std::string();

		return std::string(str, beg, end - beg + 1);
	}

	std::wstring trim(std::wstring const& str)
	{
		if (str.size() == 0)
			return str;

		std::size_t beg = str.find_first_not_of(L" \a\b\f\n\r\t\v");
		std::size_t end = str.find_last_not_of(L" \a\b\f\n\r\t\v");
		if (beg == std::wstring::npos) // No non-spaces
			return std::wstring();

		return std::wstring(str, beg, end - beg + 1);
	}

	Path::string_type::size_type findRootDirectoryPredicted(Path::string_type const& path)
	{
		if (path.empty())
			return Path::string_type::npos;
		else if (path.size() < 2)
			return path[0] == Path::preferred_separator ? 0 : Path::string_type::npos;

		if (std::isalpha(path[0]) && path[1] == Path::value_type(':'))
			return 2;

		if (path[0] != Path::preferred_separator)
			return Path::string_type::npos;

		if (path[0] == path[1])
			return (std::min)(path.find(Path::preferred_separator, 2), path.size());
		return 0;
	}
}

irp::filesystem::Path::Path()
	: mPath()
{}

irp::filesystem::Path::Path(string_type const& path, format fmt)
{
	init(path, fmt);
}

Path& irp::filesystem::Path::operator=(Path const& p)
{
	if (this == &p) return *this;

	Path temp(p);
	temp.swap(*this);
	return *this;
}

Path& irp::filesystem::Path::append(Path const& p)
{
	if (p.empty()) return *this;

	string_type rhs = p.mPath;

	if (*mPath.rend() != preferred_separator
#if defined(_WIN32)
		&& *mPath.rend() != value_type(':')
#endif // defined(_WIN32)		
		)
	{
		mPath.append(1, preferred_separator);
	}

	if (rhs[0] == preferred_separator)
		mPath.append(rhs.c_str() + 1);
	else
		mPath.append(rhs);

	return *this;
}

Path& irp::filesystem::Path::concat(Path const& p)
{
	mPath.append(p.mPath);
	return *this;
}

Path& irp::filesystem::Path::remove_filename()
{
	string_type path = filename().mPath;
	mPath.erase(mPath.size() - path.size());
	return *this;
}

Path& irp::filesystem::Path::replace_filename(Path const& replacement)
{
	remove_filename();
	operator/=(replacement);
	return *this;
}

Path& irp::filesystem::Path::replace_extension(Path const& replacement)
{
	string_type oldExt = extension().mPath;
	mPath.erase(mPath.size() - oldExt.size());

	if (replacement.empty()) return *this;

	if (replacement.mPath[0] != value_type('.'))
		mPath.push_back(value_type('.'));

	mPath += replacement.mPath;
	return *this;
}

std::string irp::filesystem::Path::string() const
{
	return toString(mPath);
}

std::wstring irp::filesystem::Path::wstring() const
{
	return toWstring(mPath);
}

std::string irp::filesystem::Path::generic_string() const
{
	if (preferred_separator == generic_separator)
		return string();

	std::string result = string();
	std::replace(result.begin(), result.end(), char(preferred_separator), char(generic_separator));
	return result;
}

std::wstring irp::filesystem::Path::generic_wstring() const
{
	if (preferred_separator == generic_separator)
		return wstring();

	std::wstring result = wstring();
	std::replace(result.begin(), result.end(), wchar_t(preferred_separator), wchar_t(generic_separator));
	return result;
}

Path irp::filesystem::Path::root_name() const
{
	string_type::size_type pos = findRootDirectoryPredicted(mPath);
	if (pos == string_type::npos)
		return Path();
	return mPath.substr(0, pos);
}

Path irp::filesystem::Path::root_directory() const
{
	string_type::size_type pos = findRootDirectoryPredicted(mPath);
	if (pos == string_type::npos || pos == mPath.size())
		return Path();
	return mPath.substr(pos, 1);
}

Path irp::filesystem::Path::root_path() const
{
	return root_name() / root_directory();
}

Path irp::filesystem::Path::relative_path() const
{
	string_type root = root_path().mPath;
	return mPath.substr(root.size());
}

Path irp::filesystem::Path::parent_path() const
{
	string_type::size_type pos = mPath.rfind(preferred_separator);
	if (pos == string_type::npos)
		return Path();
	return mPath.substr(0, pos);
}

Path irp::filesystem::Path::filename() const
{
	string_type path = relative_path();
	string_type::size_type pos = mPath.rfind(preferred_separator);
	if (pos == string_type::npos)
		return Path();
	return path.substr(pos + 1);
}

Path irp::filesystem::Path::stem() const
{
	string_type name = filename().mPath;
	string_type ext = extension().mPath;
	return name.substr(0, name.size() - ext.size());
}

Path irp::filesystem::Path::extension() const
{
	string_type name = filename().mPath;

	switch (name.size())
	{
	case 2:
		if (name[1] != value_type('.'))
			break;
	case 1:
		if (name[0] != value_type('.'))
			break;
	case 0:
		return Path();
	}

	string_type::size_type pos = name.rfind(value_type('.'));
	if (pos == string_type::npos)
		return Path();

	string_type ext = name.substr(pos);
	if (ext.size() == name.size())
		return Path();
	return ext;
}

bool irp::filesystem::Path::has_root_path() const
{
	return !root_path().empty();
}

bool irp::filesystem::Path::has_root_name() const
{
	return !root_name().empty();
}

bool irp::filesystem::Path::has_root_directory() const
{
	return !root_directory().empty();
}

bool irp::filesystem::Path::has_relative_path() const
{
	return !relative_path().empty();
}

bool irp::filesystem::Path::has_parent_path() const
{
	return !parent_path().empty();
}

bool irp::filesystem::Path::has_filename() const
{
	return !filename().empty();
}

bool irp::filesystem::Path::has_stem() const
{
	return !stem().empty();
}

bool irp::filesystem::Path::has_extension() const
{
	return !extension().empty();
}

bool irp::filesystem::Path::is_absolute() const
{
#if defined(_WIN32)
	return has_root_name() && has_root_directory();
#else
	return has_root_directory();
#endif // defined(_WIN32)
}

bool irp::filesystem::Path::is_relative() const
{
	return !is_absolute();
}

void irp::filesystem::Path::init(string_type const& path, format fmt)
{
	string_type temp = trim(path);
	if (generic_separator == preferred_separator)
		std::replace(temp.begin(), temp.end(), value_type(windows_separator), value_type(preferred_separator));
	else
		std::replace(temp.begin(), temp.end(), value_type(generic_separator), value_type(preferred_separator));

	mPath.clear();
	if (temp.empty()) return;

	mPath.reserve(temp.size());

	std::size_t prev = 0;
	for (;;)
	{
		std::size_t pos = temp.find_first_of(preferred_separator, prev);
		if (pos == string_type::npos)
			break;

		mPath.append(trim(temp.substr(prev, pos - prev)));
		mPath.append(1, preferred_separator);

		prev = pos + 1;
	}

	if (prev < temp.size())
		mPath.append(trim(temp.c_str() + prev));
}

void irp::filesystem::Path::init(char const* path, format fmt)
{
	if (bcc::is_same<char, value_type>::value)
	{
		std::string p(path);
		init(p, fmt);
	}
	else
	{
		std::wstring p = convert(path);
		init(p, fmt);
	}
}

void irp::filesystem::Path::init(wchar_t const* path, format fmt)
{
	if (bcc::is_same<wchar_t, value_type>::value)
	{
		std::wstring p(path);
		init(p, fmt);
	}
	else
	{
		std::string p = convert(path);
		init(p, fmt);
	}
}

Path irp::filesystem::current_path()
{
#if defined(_WIN32)
	DWORD size = GetCurrentDirectoryW(0, nullptr);
	size = size == 0 ? _MAX_PATH : size;

	std::wstring path;
	path.resize(size);

	if (GetCurrentDirectoryW(size, &path[0]) == 0)
	{
		DWORD error = GetLastError();

		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, message, sizeof(message), NULL);

		throw std::runtime_error(message);
	}

	return path.c_str();

#else
	std::string path;
	path.resize(PATH_MAX);

	for (;; path.resize(path.size() * 2))
	{
		if (getcwd(&path[0], path.size()))
			return path.c_str();

		if (errno != ERANGE)
			throw std::runtime_error(std::strerror(errno));
	}

#endif // defined(_WIN32)
}

Path irp::filesystem::absolute(Path const& path, Path const& parent)
{
	if (path.is_absolute())
		return path;

	Path absParent = parent.is_absolute() ? parent : absolute(parent);

	if (path.empty())
		return absParent;

	Path rootName = path.root_name();
	Path rootDir = path.root_directory();

	if (!rootName.empty())
	{
		return rootName / absParent.root_directory()
			/ absParent.relative_path() / path.relative_path();
	}
	else if (!rootDir.empty())
	{
		return absParent.root_name() / path;
	}
	else
	{
		return absParent / path;
	}
}

Path irp::filesystem::canonical(Path const& path, Path const& parent)
{
	Path absPath = path.is_absolute() ? path : absolute(path, parent);

#if defined(_WIN32)
	std::wstring converted;
	converted.resize(PATH_MAX);

	for (;; converted.resize(converted.size() * 2))
	{
		if (::_wfullpath(&converted[0], absPath.c_str(), converted.size()))
			return converted.c_str();

		if (errno != EINVAL)
			throw std::runtime_error(std::strerror(errno));
	}

#else
	std::string converted;
	converted.resize(PATH_MAX);
	if (!realpath(absPath.c_str(), &converted[0]))
		throw std::runtime_error(std::strerror(errno));
	return converted.c_str();

#endif // defined(_WIN32)
}

bool irp::filesystem::exists(Path const& path)
{
#if defined(_WIN32)
	DWORD attr = GetFileAttributesW(path.c_str());
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

bool irp::filesystem::is_directory(Path const& path)
{
#if defined(_WIN32)
	DWORD attr = GetFileAttributesW(path.c_str());
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

bool irp::filesystem::create_directory(Path const& path)
{
#if defined(_WIN32)
	if (!CreateDirectoryW(path.c_str(), 0))
		return false;
	return true;

#else
	if (mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IRWXO) != 0)
		return false;
	return true;

#endif // defined(_WIN32)
}

bool irp::filesystem::create_directory(Path const& path, Path const& parent)
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

#endif // defined(_WIN32)
}

bool irp::filesystem::create_directories(Path const& path)
{
	if (path.empty()) return false;

	Path parent = path.parent_path();
	if (!exists(parent))
	{
		if (!create_directories(parent))
			return false;
	}

	if (is_directory(path))
		return false;

	return create_directory(path);
}
