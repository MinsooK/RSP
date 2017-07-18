
#ifndef INDUROP_DETAIL_FILE_SYSTEM_H__
#define INDUROP_DETAIL_FILE_SYSTEM_H__

#include <string>
#include <utility>

#include <bicomc/detail/config.h>

namespace irp {
namespace filesystem {

	class Path
	{
	public:
#if defined(_WIN32)
		typedef wchar_t value_type;
#else
		typedef char value_type;
#endif

		typedef std::basic_string<value_type> string_type;
		enum format { native_format, generic_format, auto_format };

	public:
		static value_type const generic_separator = value_type('/');
		static value_type const windows_separator = value_type('\\');

#if defined(_WIN32)
		static value_type const preferred_separator = windows_separator;
#else
		static value_type const preferred_separator = generic_separator;
#endif

	public:
		Path();
		Path(string_type const& path, format fmt = auto_format);

		template<typename Source>
		Path(Source const& source, format fmt = auto_format);

	public:
		Path& operator=(Path const& p);
		Path& operator/=(Path const& p) { return append(p); }
		Path& operator+=(Path const& p) { return concat(p); }

		operator string_type() const { return mPath; }

	public:
		Path& append(Path const& p);
		Path& concat(Path const& p);

	public:
		void clear() { Path temp; temp.swap(*this); }
		Path& make_preferred() { return *this; }
		Path& remove_filename();
		Path& replace_filename(Path const& replacement);
		Path& replace_extension(Path const& replacement = Path());

		void swap(Path& p);

	public:
		value_type const* c_str() const { return mPath.c_str(); }
		string_type const& native() const { return mPath; }

		std::string string() const;
		std::wstring wstring() const;

		std::string generic_string() const;
		std::wstring generic_wstring() const;

	public:
		int compare(Path const& p) const { return mPath.compare(p); }

	public:
		Path root_name() const;
		Path root_directory() const;
		Path root_path() const;
		Path relative_path() const;
		Path parent_path() const;
		Path filename() const;
		Path stem() const;
		Path extension() const;

	public:
		bool empty() const { return mPath.empty(); }

		bool has_root_path() const;
		bool has_root_name() const;
		bool has_root_directory() const;
		bool has_relative_path() const;
		bool has_parent_path() const;
		bool has_filename() const;
		bool has_stem() const;
		bool has_extension() const;

		bool is_absolute() const;
		bool is_relative() const;

	private:
		void init(string_type const& path, format fmt);
		void init(char const* path, format fmt);
		void init(wchar_t const* path, format fmt);

		template<typename Source>
		void init(Source const& source, format fmt);

	private:
		string_type mPath;
	};

	template<typename Source>
	Path::Path(Source const& source, format fmt)
	{
		init(source, fmt);
	}

	template<typename Source>
	void Path::init(Source const& source, format fmt)
	{
		init(source.c_str(), fmt);
	}

	inline void Path::swap(Path& p)
	{
		if (this == &p) return;
		std::swap(mPath, p.mPath);
	}

	inline Path operator/(Path const& lhs, Path const& rhs)
	{
		Path result = lhs;
		return result /= rhs;
	}

	inline bool operator<(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return lhs.compare(rhs) < 0;
	}

	inline bool operator<=(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return !(rhs < lhs);
	}

	inline bool operator>(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return rhs < lhs;
	}

	inline bool operator>=(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return !(lhs < rhs);
	}

	inline bool operator==(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return !(lhs < rhs) && !(rhs < lhs);
	}

	inline bool operator!=(Path const& lhs, Path const& rhs) BICOMC_NOEXCEPT
	{
		return !(lhs == rhs);
	}


	Path current_path();
	Path absolute(Path const& path, Path const& parent = current_path());
	Path canonical(Path const& path, Path const& parent = current_path());

	bool exists(Path const& path);
	bool is_directory(Path const& path);

	bool create_directory(Path const& path);
	bool create_directory(Path const& path, Path const& parent);
	bool create_directories(Path const& path);

} // namespace filesystem
} // namespace irp

namespace std {
	inline void swap(irp::filesystem::Path& lhs, irp::filesystem::Path& rhs)
	{
		lhs.swap(rhs);
	}
} // namespace std

#endif // !def INDUROP_DETAIL_FILE_SYSTEM_H__
