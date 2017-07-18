
#ifndef INDUROP_DETAIL_STRING_VIEW_H__
#define INDUROP_DETAIL_STRING_VIEW_H__

#include <iterator>
#include <string>
#include <utility>

#include <bicomc/object.h>

namespace irp {
	template<typename CharT, typename Traits = std::char_traits<CharT> >
	class BasicStringView
	{
	public:
		typedef Traits traits_type;
		typedef CharT value_type;
		typedef CharT* pointer;
		typedef CharT const* const_pointer;
		typedef CharT& reference;
		typedef CharT const& const_reference;
		typedef const_pointer const_iterator;
		typedef const_iterator iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
		typedef const_reverse_iterator reverse_iterator;
		typedef bcc::uintptr_t size_type;
		typedef bcc::intptr_t difference_type;

	public:
		static BICOMC_CONSTEXPR size_type const npos = size_type(-1);

	public:
		BICOMC_CONSTEXPR BasicStringView() BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR BasicStringView(CharT const* s, size_type size) BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR BasicStringView(CharT const* s);
		BICOMC_CONSTEXPR BasicStringView(std::basic_string<CharT, Traits> const& s) BICOMC_NOEXCEPT;

	public:
		BICOMC_CONSTEXPR const_reference operator[](size_type pos) const;

		operator std::basic_string<CharT, Traits>() const;

	public:
		BICOMC_CONSTEXPR const_reference at(size_type pos) const;
		BICOMC_CONSTEXPR const_reference front() const;
		BICOMC_CONSTEXPR const_reference back() const;
		BICOMC_CONSTEXPR const_pointer data() const BICOMC_NOEXCEPT;

	public:
		BICOMC_CONSTEXPR size_type size() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type length() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type max_size() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR bool empty() const BICOMC_NOEXCEPT;

	public:
		BICOMC_CONSTEXPR void remove_prefix(size_type n);
		BICOMC_CONSTEXPR void remove_suffix(size_type n);
		BICOMC_CONSTEXPR void swap(BasicStringView& view) BICOMC_NOEXCEPT;

	public:
		size_type copy(CharT* dest, size_type count, size_type pos = 0) const;
		BICOMC_CONSTEXPR BasicStringView substr(size_type pos = 0, size_type count = npos) const;

		BICOMC_CONSTEXPR int compare(BasicStringView v) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR int compare(size_type pos1, size_type count1, BasicStringView v) const;
		BICOMC_CONSTEXPR int compare(size_type pos1, size_type count1, BasicStringView v, size_type pos2, size_type count2) const;
		BICOMC_CONSTEXPR int compare(CharT const* s) const;
		BICOMC_CONSTEXPR int compare(size_type pos1, size_type count1, CharT const* s) const;
		BICOMC_CONSTEXPR int compare(size_type pos1, size_type count1, CharT const* s, size_type count2) const;

		BICOMC_CONSTEXPR size_type find(BasicStringView v, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find(CharT c, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find(CharT const* s, size_type pos = 0) const;
		BICOMC_CONSTEXPR size_type find(CharT const* s, size_type pos, size_type count) const;

		BICOMC_CONSTEXPR size_type rfind(BasicStringView v, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type rfind(CharT c, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type rfind(CharT const* s, size_type pos = npos) const;
		BICOMC_CONSTEXPR size_type rfind(CharT const* s, size_type pos, size_type count) const;

		BICOMC_CONSTEXPR size_type find_first_of(BasicStringView v, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_first_of(CharT c, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_first_of(CharT const* s, size_type pos = 0) const;
		BICOMC_CONSTEXPR size_type find_first_of(CharT const* s, size_type pos, size_type count) const;

		BICOMC_CONSTEXPR size_type find_last_of(BasicStringView v, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_last_of(CharT c, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_last_of(CharT const* s, size_type pos = npos) const;
		BICOMC_CONSTEXPR size_type find_last_of(CharT const* s, size_type pos, size_type count) const;

		BICOMC_CONSTEXPR size_type find_first_not_of(BasicStringView v, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_first_not_of(CharT c, size_type pos = 0) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_first_not_of(CharT const* s, size_type pos = 0) const;
		BICOMC_CONSTEXPR size_type find_first_not_of(CharT const* s, size_type pos, size_type count) const;

		BICOMC_CONSTEXPR size_type find_last_not_of(BasicStringView v, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_last_not_of(CharT c, size_type pos = npos) const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR size_type find_last_not_of(CharT const* s, size_type pos = npos) const;
		BICOMC_CONSTEXPR size_type find_last_not_of(CharT const* s, size_type pos, size_type count) const;

	public:
		BICOMC_CONSTEXPR const_iterator begin() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR const_iterator end() const BICOMC_NOEXCEPT;

		BICOMC_CONSTEXPR const_iterator cbegin() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR const_iterator cend() const BICOMC_NOEXCEPT;

		BICOMC_CONSTEXPR const_reverse_iterator rbegin() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR const_reverse_iterator rend() const BICOMC_NOEXCEPT;

		BICOMC_CONSTEXPR const_reverse_iterator crbegin() const BICOMC_NOEXCEPT;
		BICOMC_CONSTEXPR const_reverse_iterator crend() const BICOMC_NOEXCEPT;

	private:
		const_pointer mString;
		size_type mSize;
	};

	typedef BasicStringView<char> StringView;
	typedef BasicStringView<wchar_t> WStringView;

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits>::BasicStringView() BICOMC_NOEXCEPT
		: mString(), mSize()
	{}

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits>::BasicStringView(CharT const* s, size_type size) BICOMC_NOEXCEPT
		: mString(s), mSize(size)
	{}

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits>::BasicStringView(CharT const* s)
		: mString(s), mSize(Traits::length(s))
	{}

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits>::BasicStringView(std::basic_string<CharT, Traits> const& s) BICOMC_NOEXCEPT
		: mString(s.c_str()), mSize(s.size())
	{}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reference BasicStringView<CharT, Traits>::operator[](size_type pos) const
	{
		return mString[pos];
	}

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits>::operator std::basic_string<CharT, Traits>() const
	{
		return std::basic_string<CharT, Traits>(mString, mSize);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reference BasicStringView<CharT, Traits>::at(size_type pos) const
	{
		if (pos >= mSize)
			throw std::out_of_range("'pos' is out of range.");
		return mString[pos];
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reference BasicStringView<CharT, Traits>::front() const
	{
		return *mString;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reference BasicStringView<CharT, Traits>::back() const
	{
		return *(mString + mSize - 1);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_pointer BasicStringView<CharT, Traits>::data() const BICOMC_NOEXCEPT
	{
		return mString;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::size() const BICOMC_NOEXCEPT
	{
		return mSize;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::length() const BICOMC_NOEXCEPT
	{
		return mSize;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::max_size() const BICOMC_NOEXCEPT
	{
		return size_type(-1) / sizeof(value_type);
	}

	template<typename CharT, typename Traits>
	bool BasicStringView<CharT, Traits>::empty() const BICOMC_NOEXCEPT
	{
		return mSize == 0;
	}

	template<typename CharT, typename Traits>
	void BasicStringView<CharT, Traits>::remove_prefix(size_type n)
	{
		if (n > mSize)
			throw std::out_of_range("'n' is out of range.");

		mString += n;
		mSize -= n;
	}

	template<typename CharT, typename Traits>
	void BasicStringView<CharT, Traits>::remove_suffix(size_type n)
	{
		if (n > mSize)
			throw std::out_of_range("'n' is out of range.");

		mSize -= n;
	}

	template<typename CharT, typename Traits>
	void BasicStringView<CharT, Traits>::swap(BasicStringView& view) BICOMC_NOEXCEPT
	{
		if (this == &view) return;

		std::swap(mString, view.mString);
		std::swap(mSize, view.mSize);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::copy(CharT* dest, size_type count, size_type pos) const
	{
		if (pos > mSize)
			throw std::out_of_range("'pos' is out of range.");

		size_type rcount = (std::min)(count, mSize - pos);
		traits_type::copy(dest, mString + pos, rcount);
		return rcount;
	}

	template<typename CharT, typename Traits>
	BasicStringView<CharT, Traits> BasicStringView<CharT, Traits>::substr(size_type pos, size_type count) const
	{
		if (pos > mSize)
			throw std::out_of_range("'pos' is out of range.");

		size_type rcount = (std::min)(count, mSize - pos);
		return BasicStringView(mString + pos, rcount);
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(BasicStringView v) const BICOMC_NOEXCEPT
	{
		size_type rcount = (std::min)(mSize, v.size());
		int result = traits_type::compare(mString, v.data(), rcount);
		if (result == 0)
			return mSize - v.size();
		return result;
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(size_type pos1, size_type count1, BasicStringView v) const
	{
		return substr(pos1, count1).compare(v);
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(size_type pos1, size_type count1, BasicStringView v, size_type pos2, size_type count2) const
	{
		return substr(pos1, count1).compare(v.substr(pos2, count2));
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(CharT const* s) const
	{
		return compare(BasicStringView(s));
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(size_type pos1, size_type count1, CharT const* s) const
	{
		return substr(pos1, count1).compare(BasicStringView(s));
	}

	template<typename CharT, typename Traits>
	int BasicStringView<CharT, Traits>::compare(size_type pos1, size_type count1, CharT const* s, size_type count2) const
	{
		return substr(pos1, count1).compare(BasicStringView(s, count2));
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		if (v.size() > mSize - pos) return npos;
		if (v.empty()) return (std::min)(mSize, pos);

		for (; pos < mSize; ++pos)
		{
			if (traits_type::eq(mString[pos], v.front()))
			{
				if (mSize - pos < v.size())
					return npos;
				if (traits_type::compare(mString + pos, v.data(), v.size()) == 0)
					return pos;
			}
			else
			{
				return npos;
			}
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return find(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find(CharT const* s, size_type pos) const
	{
		return find(BasicStringView(s), pos);
	}
	
	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find(CharT const* s, size_type pos, size_type count) const
	{
		return find(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::rfind(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		pos = (std::min)(mSize, pos);
		if (v.empty()) return pos;

		for (--pos; pos != npos; --pos)
		{
			if (traits_type::eq(mString[pos], v.front()))
			{
				if (mSize - pos < v.size())
					continue;
				if (traits_type::compare(mString + pos, v.data(), v.size()) == 0)
					return pos;
			}
			else
			{
				return npos;
			}
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::rfind(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return rfind(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::rfind(CharT const* s, size_type pos) const
	{
		return rfind(BasicStringView(s), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::rfind(CharT const* s, size_type pos, size_type count) const
	{
		return rfind(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_of(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		for (; pos < mSize; ++pos)
		{
			if (traits_type::find(v.data(), v.size(), mString[pos]))
				return pos;
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_of(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return find_first_of(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_of(CharT const* s, size_type pos) const
	{
		return find_first_of(BasicStringView(s), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_of(CharT const* s, size_type pos, size_type count) const
	{
		return find_first_of(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_of(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		pos = (std::min)(mSize, pos);
		--pos;
		for (--pos; pos != npos; --pos)
		{
			if (traits_type::find(v.data(), v.size(), mString[pos]))
				return pos;
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_of(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return find_last_of(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_of(CharT const* s, size_type pos) const
	{
		return find_last_of(BasicStringView(s), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_of(CharT const* s, size_type pos, size_type count) const
	{
		return find_last_of(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_not_of(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		for (; pos < mSize; ++pos)
		{
			if (!traits_type::find(v.data(), v.size(), mString[pos]))
				return pos;
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_not_of(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return find_first_not_of(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_not_of(CharT const* s, size_type pos) const
	{
		return find_first_not_of(BasicStringView(s), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_first_not_of(CharT const* s, size_type pos, size_type count) const
	{
		return find_first_not_of(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_not_of(BasicStringView v, size_type pos) const BICOMC_NOEXCEPT
	{
		pos = (std::min)(mSize, pos);
		--pos;
		for (--pos; pos != npos; --pos)
		{
			if (!traits_type::find(v.data(), v.size(), mString[pos]))
				return pos;
		}

		return npos;
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_not_of(CharT c, size_type pos) const BICOMC_NOEXCEPT
	{
		return find_last_not_of(BasicStringView(&c, 1), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_not_of(CharT const* s, size_type pos) const
	{
		return find_last_not_of(BasicStringView(s), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::size_type BasicStringView<CharT, Traits>::find_last_not_of(CharT const* s, size_type pos, size_type count) const
	{
		return find_last_not_of(BasicStringView(s, count), pos);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_iterator BasicStringView<CharT, Traits>::begin() const BICOMC_NOEXCEPT
	{
		return cbegin();
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_iterator BasicStringView<CharT, Traits>::end() const BICOMC_NOEXCEPT
	{
		return cend();
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_iterator BasicStringView<CharT, Traits>::cbegin() const BICOMC_NOEXCEPT
	{
		return const_iterator(mString);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_iterator BasicStringView<CharT, Traits>::cend() const BICOMC_NOEXCEPT
	{
		return const_iterator(mString + mSize);
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reverse_iterator BasicStringView<CharT, Traits>::rbegin() const BICOMC_NOEXCEPT
	{
		return crbegin();
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reverse_iterator BasicStringView<CharT, Traits>::rend() const BICOMC_NOEXCEPT
	{
		return crend();
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reverse_iterator BasicStringView<CharT, Traits>::crbegin() const BICOMC_NOEXCEPT
	{
		return const_reverse_iterator(end());
	}

	template<typename CharT, typename Traits>
	typename BasicStringView<CharT, Traits>::const_reverse_iterator BasicStringView<CharT, Traits>::crend() const BICOMC_NOEXCEPT
	{
		return const_reverse_iterator(begin());
	}


	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator==(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return lhs.compare(rhs) == 0;
	}

	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator!=(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return !(lhs == rhs);
	}

	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator<(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return lhs.compare(rhs) < 0;
	}

	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator<=(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return !(rhs < lhs);
	}

	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator>(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return rhs < lhs;
	}

	template<typename CharT, typename Traits>
	BICOMC_CONSTEXPR bool operator>=(BasicStringView<CharT, Traits> lhs, BasicStringView<CharT, Traits> rhs) BICOMC_NOEXCEPT
	{
		return !(lhs < rhs);
	}
} // namespace irp

namespace std {
	template<typename CharT, typename Traits>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os, irp::BasicStringView<CharT, Traits> v)
	{
		std::streamsize padding = os.width() - v.size();
		padding = padding < 0 ? 0 : padding;

		if ((os.flags() & std::ios::adjustfield) == std::ios::left)
		{
			for (; padding > 0; --padding)
				os.put(os.fill());
		}

		os.write(v.data(), v.size());

		for (; padding > 0; --padding)
			os.put(os.fill());

		os.width(0);
		return os;
	}
} // namespace std


namespace bcc {
namespace detail {
	template<typename CharT>
	struct Compatibility<irp::BasicStringView<CharT> >
	{
		typedef irp::BasicStringView<CharT> type;

		static_assert(sizeof(type) == sizeof(CharT*) + sizeof(bcc::uintptr_t), "'BasicStringView<CharT>' is incompatible.");
	};
} // namespace detail
} // namespace bcc

template<typename CharT>
struct BICOMC_SIGNATURE_CUSTOM_NAME<irp::BasicStringView<CharT> >
{
	static std::wstring to_wstring()
	{
		std::wstring signature(L"irp::BasicStringView<");
		signature.append(bcc::detail::Signature<CharT>::to_wstring());
		signature.append(L">");
		return signature;
	}

	static std::string to_utf8()
	{
		std::string signature("irp::BasicStringView<");
		signature.append(bcc::detail::Signature<CharT>::to_utf8());
		signature.append(">");
		return signature;
	}
};

#endif // INDUROP_DETAIL_STRING_VIEW_H__
