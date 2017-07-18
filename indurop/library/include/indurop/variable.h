
#ifndef INDUROP_VARIABLE_H__
#define INDUROP_VARIABLE_H__

#include <bicomc/array.h>
#include <bicomc/tuple.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#include <memory>
#include <sstream>
#include <vector>

#include <indurop/detail/shared_memory.h>

#include "api_client.h"

namespace irp {

	class SharedVariable;

namespace detail {
	template<typename T, std::size_t size>
	std::size_t length(T (&array)[size])
	{
		return size;
	}

	template<typename T, std::size_t size>
	std::size_t length(bcc::array<T, size> const& array)
	{
		return array.size();
	}

	struct ViewInitializer
	{
		ViewInitializer(SharedVariable& base_, bcc::uint32_t offset_)
			: base(base_), offset(offset_)
		{}

		SharedVariable& base;
		bcc::uint32_t offset;
	};
} // namespace detail

	template<typename T>
	class VariableView;

	class SharedVariable
	{
	public:
		struct Header
		{
			bcc::uint32_t next, usage;
			bcc::uint32_t size, payloadSize;
			bcc::uint32_t latest, state;
		};

		template<typename T>
		friend class VariableView;

	private:
		static bcc::uint32_t const REPEAT_COUNT = 10;

	public:
		SharedVariable(char const* name, char const* type, bcc::uint32_t size);

	protected:
		void pull() const;
		void push();

		Header const& header() const { return *mpHeader; }
		Header& header() { return *mpHeader; }

		bcc::uint64_t time() const;
		bcc::uint64_t stamp() const;
		bcc::uint64_t stamp(bcc::uint64_t time) const;

	protected:
		virtual void onPull(void const* p) const = 0;
		virtual void onPush(void* p) = 0;
		virtual void* cache() const = 0;

	private:
		SharedMemory mMemory;
		Header* mpHeader;
		void* mValues[2];
		char const* mpFooter;
		mutable bcc::uint64_t mTimeStamp;
	};

	template<typename T>
	class VariableView
	{
	public:
		typedef T value_type;
		typedef value_type& reference;
		typedef value_type const& const_reference;

	public:
		VariableView();
		VariableView(SharedVariable& base, bcc::uint32_t offset = 0);

	public:
		VariableView& operator=(detail::ViewInitializer const& rhs);
		VariableView& operator=(value_type const& rhs);

		operator const_reference() const;

	private:
		VariableView& operator=(VariableView const&) { throw std::runtime_error("invalid access"); }

	private:
		SharedVariable* mpBase;
		value_type* mpCache;
	};

	template<typename T>
	VariableView<T>::VariableView()
		: mpBase(), mpCache()
	{}

	template<typename T>
	VariableView<T>::VariableView(SharedVariable& base, bcc::uint32_t offset)
		: mpBase(&base)
	{
		mpCache = reinterpret_cast<value_type*>(static_cast<char*>(base.cache()) + offset);
		if (!mpCache)
			throw std::runtime_error("'mpCache' must not be null");
	}

	template<typename T>
	VariableView<T>& VariableView<T>::operator=(detail::ViewInitializer const& rhs)
	{
		mpBase = &rhs.base;
		mpCache = reinterpret_cast<value_type*>(static_cast<char*>(mpBase->cache()) + rhs.offset);
		if (!mpCache)
			throw std::runtime_error("'mpCache' must not be null");
		return *this;
	}

	template<typename T>
	VariableView<T>& VariableView<T>::operator=(value_type const& rhs)
	{
		*mpCache = rhs;
		mpBase->push();
		return *this;
	}

	template<typename T>
	VariableView<T>::operator const_reference() const
	{
		mpBase->pull();
		return *mpCache;
	}

	template<typename T, bool isReadOnly = false>
	class Variable : public SharedVariable
	{
	public:
		typedef T value_type;

		typedef typename bcc::conditional<
			isReadOnly
			, typename bcc::add_const<value_type>::type
			, value_type
		>::type reference;
		typedef typename bcc::add_const<reference>::type const_reference;

		typedef typename bcc::add_const<value_type>::type* const_pointer;

	private:
		static bcc::uint32_t const REPEAT_COUNT = 10;

	public:
		Variable(char const* name, char const* type)
			: SharedVariable(name, type, sizeof(mHolder)), mHolder()
		{}

	private:
		Variable(Variable const&) BICOMC_DELETE;
		Variable& operator=(Variable const&) BICOMC_DELETE;

	public:
		const_reference operator*() const
		{
			SharedVariable::pull();
			return mHolder;
		}

		operator const_reference() const
		{
			return operator*();
		}

		const_pointer operator->() const
		{
			SharedVariable::pull();
			return mHolder;
		}

		template<typename U>
		const_reference operator=(U const& rhs)
		{
			static_assert(!isReadOnly, "Global variable is read-only.");
			push(mHolder = rhs);
			return mHolder;
		}

	private:
		Variable* operator&() BICOMC_DELETE;

	private:
		void pull(value_type& value) const
		{
			SharedVariable::pull();
		}

		void onPull(void const* p) const
		{
			std::memcpy(&mHolder, p, sizeof(mHolder));
		}

		void push(value_type const& value, bcc::uint32_t repeat = bcc::uint32_t(10))
		{
			SharedVariable::push();
		}

		void onPush(void* p)
		{
			std::memcpy(p, &mHolder, sizeof(mHolder));
		}

		void* cache() const
		{
			return detail::addressof(mHolder);
		}

	private:
		mutable value_type mHolder;
	};

	template<typename T, size_t size, bool isReadOnly>
	class Variable<T[size], isReadOnly> : public Variable<bcc::array<T, size>, isReadOnly>
	{
	public:
		typedef Variable<bcc::array<T, size>, isReadOnly> Base;

		Variable(char const* name, char const* type)
			: Base(name, type)
		{}

		template<typename U>
		typename Base::const_reference operator=(U const& rhs)
		{
			return Base::operator=(rhs);
		}
	};


} // namespace irp

#define INDUROP_DECL_VARIABLE(NAME, TYPE) \
	extern irp::Variable<TYPE> NAME;

#define INDUROP_VARIABLE(NAME, TYPE) \
	irp::Variable<TYPE> NAME(#NAME, #TYPE);


namespace irp {
	bool synchronize(SharedVariable const& var);
	bool isUpdated(SharedVariable const& var);
}

#endif // !def INDUROP_VARIABLE_H__
