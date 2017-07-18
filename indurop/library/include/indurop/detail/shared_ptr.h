
#ifndef INDUROP_DETAIL_SHARED_PTR_H__
#define INDUROP_DETAIL_SHARED_PTR_H__

#include <cstddef>
#include <exception>
#include <typeinfo>
#include <utility>

#include <bicomc/stdint.h>

namespace irp {
namespace detail {
	template<typename T>
	struct remove_extent
	{
		typedef T type;
	};

	template<typename T>
	struct remove_extent<T[]>
	{
		typedef T type;
	};

	template<typename T, std::size_t size>
	struct remove_extent<T[size]>
	{
		typedef T* type;
	};

	template<typename T>
	struct is_array
	{
		static bool const value = false;
	};

	template<typename T>
	struct is_array<T[]>
	{
		static bool const value = true;
	};

	template<typename T, std::size_t size>
	struct is_array<T[size]>
	{
		static bool const value = true;
	};

	template<typename T>
	T* addressof(T& arg)
	{
		return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(arg)));
	}

	class SharedCounter
	{
	public:
		SharedCounter() : mStrongCount(1), mWeakCount(1) {}

	protected:
		virtual ~SharedCounter() {}

	public:
		void increaseStrong();
		void decreaseStrong();
		bcc::intptr_t strong() const;

		void increaseWeak();
		void decreaseWeak();
		bcc::intptr_t weak() const;

		virtual void* deleter(std::type_info const& info) const { return nullptr; }

	protected:
		virtual void destroy() = 0;

	private:
		bcc::intptr_t mStrongCount;
		bcc::intptr_t mWeakCount;
	};

	template<typename T, typename Deleter = void>
	struct SharedPtrBase : public SharedCounter
	{
		SharedPtrBase(T* p, Deleter d) : pointer(p), deleter_(d) {}
		~SharedPtrBase() { if (pointer) destroy(); }

		void destroy()
		{
			static_assert(sizeof(T) != 0, "'T' is not defined.");
			deleter_(pointer);
			pointer = nullptr;
		}

		void* deleter(std::type_info const& info) const
		{
			if (typeid(Deleter) == info)
				return const_cast<Deleter*>(addressof(deleter_));
			return nullptr;
		}

		T* pointer;
		Deleter deleter_;
	};

	template<typename T>
	struct SharedPtrBase<T, void> : public SharedCounter
	{
		SharedPtrBase(T* p) : pointer(p) {}
		~SharedPtrBase() { if (pointer) destroy(); }

		void destroy()
		{
			static_assert(sizeof(T) != 0, "'T' is not defined.");
			is_array<T>::value ? delete[] pointer : delete pointer;
			pointer = nullptr;
		}

		T* pointer;
	};
} // namespace detail

	class bad_weak_ptr : public std::exception
	{
	public:
		char const* what() const throw() { return "bad_weak_ptr"; }
	};

	template<typename T>
	class SharedPtr;

	template<typename T>
	class WeakPtr;

	template<typename T>
	class EnableSharedFromThis;
	
	template<typename Deleter, typename T>
	Deleter* get_deleter(SharedPtr<T> const& p);

	template<typename T>
	class SharedPtr
	{
	public:
		template<typename U>
		friend class SharedPtr;
		template<typename U>
		friend class WeakPtr;
		template<typename Deleter, typename U>
		friend Deleter* get_deleter(SharedPtr<U> const& p);

		typedef typename detail::remove_extent<T>::type element_type;
		typedef WeakPtr<T> weak_type;
		typedef void(SharedPtr::*explicit_bool_type)() const;

	public:
		SharedPtr();
		SharedPtr(std::nullptr_t);
		template<typename U>
		explicit SharedPtr(U* ptr);
		template<typename U, typename Deleter>
		explicit SharedPtr(U* ptr, Deleter d);
		template<typename Deleter>
		SharedPtr(std::nullptr_t ptr, Deleter d);
		template<typename U>
		SharedPtr(SharedPtr<U> const& r, element_type* ptr);
		SharedPtr(SharedPtr const& r);
		template<typename U>
		SharedPtr(SharedPtr<U> const& r);
		template<typename U>
		SharedPtr(WeakPtr<U> const& r);

		~SharedPtr();

	public:
		SharedPtr& operator=(SharedPtr const& r);
		template<typename U>
		SharedPtr& operator=(SharedPtr<U> const& r);

		T& operator*() const;
		T* operator->() const;
		element_type& operator[](std::ptrdiff_t index);

		operator explicit_bool_type() const;
		bool operator!() const;

	public:
		void reset();
		template<typename U>
		void reset(U* ptr);
		template<typename U, typename Deleter>
		void reset(U* ptr, Deleter d);

		void swap(SharedPtr& r);

	public:
		element_type* get() const;
		bcc::intptr_t use_count() const;
		bool unique() const;

		template<typename U>
		bool owner_before(SharedPtr<U> const& other) const;
		template<typename U>
		bool owner_before(WeakPtr<U> const& other) const;

	private:
		void explicit_bool() const {}

		template<typename U>
		void enable(EnableSharedFromThis<U>* ptr);
		void enable(void const volatile*) {}

	private:
		element_type* mpElement;
		detail::SharedCounter* mpBase;
	};

	template<typename T>
	class WeakPtr
	{
	public:
		typedef typename detail::remove_extent<T>::type element_type;

		template<typename U>
		friend class WeakPtr;
		template<typename U>
		friend class SharedPtr;

	public:
		WeakPtr();
		WeakPtr(WeakPtr const& r);
		template<typename U>
		WeakPtr(WeakPtr<U> const& r);
		template<typename U>
		WeakPtr(SharedPtr<U> const& r);

		~WeakPtr();

	public:
		WeakPtr& operator=(WeakPtr const& r);
		template<typename U>
		WeakPtr& operator=(WeakPtr<U> const& r);
		template<typename U>
		WeakPtr& operator=(SharedPtr<U> const& r);

	public:
		void reset();
		void swap(WeakPtr& r);

	public:
		bcc::intptr_t use_count() const;
		bool expired() const;
		SharedPtr<T> lock() const;

		template<typename U>
		bool owner_before(WeakPtr<U> const& other) const;
		template<typename U>
		bool owner_before(SharedPtr<U> const& other) const;

	private:
		element_type* mpElement;
		detail::SharedCounter* mpBase;
	};

	template<typename T>
	class EnableSharedFromThis
	{
		template<typename U>
		friend class SharedPtr;

	protected:
		EnableSharedFromThis() {}
		EnableSharedFromThis(EnableSharedFromThis const&) {}

		~EnableSharedFromThis() {}

	protected:
		EnableSharedFromThis& operator=(EnableSharedFromThis const& r) { return *this; }

	public:
		SharedPtr<T> shared_from_this();
		SharedPtr<T const> shared_from_this() const;

		WeakPtr<T> weak_from_this();
		WeakPtr<T const> weak_from_this() const;

	private:
		WeakPtr<T> weak_this;
	};

	template<typename T>
	SharedPtr<T>::SharedPtr()
		: mpElement(), mpBase()
	{}

	template<typename T>
	SharedPtr<T>::SharedPtr(std::nullptr_t)
		: mpElement(), mpBase()
	{}

	template<typename T>
	template<typename U>
	SharedPtr<T>::SharedPtr(U* ptr)
		: mpElement(ptr), mpBase()
	{
		try
		{
			mpBase = new detail::SharedPtrBase<U>(ptr);
		}
		catch (...)
		{
			detail::is_array<U>::value ? delete[] ptr : delete ptr;
			throw;
		}

		enable(ptr);
	}

	template<typename T>
	template<typename U, typename Deleter>
	SharedPtr<T>::SharedPtr(U* ptr, Deleter d)
		: mpElement(ptr), mpBase()
	{
		try
		{
			mpBase = new detail::SharedPtrBase<U, Deleter>(ptr, d);
		}
		catch (...)
		{
			d(ptr);
			throw;
		}
		enable(ptr);
	}

	template<typename T>
	template<typename Deleter>
	SharedPtr<T>::SharedPtr(std::nullptr_t ptr, Deleter d)
		: mpElement()
		, mpBase(new detail::SharedPtrBase<T, Deleter>(ptr, d))
	{}

	template<typename T>
	template<typename U>
	SharedPtr<T>::SharedPtr(SharedPtr<U> const& r, element_type* ptr)
		: mpElement(ptr), mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseStrong();
	}

	template<typename T>
	SharedPtr<T>::SharedPtr(SharedPtr const& r)
		: mpElement(r.mpElement), mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseStrong();
	}

	template<typename T>
	template<typename U>
	SharedPtr<T>::SharedPtr(SharedPtr<U> const& r)
		: mpElement(r.mpElement)
		, mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseStrong();
	}

	template<typename T>
	template<typename U>
	SharedPtr<T>::SharedPtr(WeakPtr<U> const& r)
		: mpElement(r.mpElement)
		, mpBase(r.mpBase)
	{
		if (r.expired()) throw bad_weak_ptr();
		mpBase->increaseStrong();
	}

	template<typename T>
	SharedPtr<T>::~SharedPtr()
	{
		if (mpBase) mpBase->decreaseStrong();
	}

	template<typename T>
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr const& r)
	{
		if (this == &r) return *this;

		SharedPtr temp(r);
		temp.swap(*this);
		return *this;
	}

	template<typename T>
	template<typename U>
	SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr<U> const& r)
	{
		SharedPtr temp(r);
		temp.swap(*this);
		return *this;
	}

	template<typename T>
	T& SharedPtr<T>::operator*() const
	{
		return *mpElement;
	}

	template<typename T>
	T* SharedPtr<T>::operator->() const
	{
		return mpElement;
	}

	template<typename T>
	typename SharedPtr<T>::element_type& SharedPtr<T>::operator[](std::ptrdiff_t index)
	{
		return mpElement[index];
	}

	template<typename T>
	SharedPtr<T>::operator explicit_bool_type() const
	{
		return mpElement == nullptr ? nullptr : &SharedPtr::explicit_bool;
	}

	template<typename T>
	bool SharedPtr<T>::operator!() const
	{
		return mpElement == nullptr;
	}

	template<typename T>
	void SharedPtr<T>::reset()
	{
		mpElement = nullptr;
		if (mpBase)
		{
			mpBase->decreaseWeak();
			mpBase = nullptr;
		}
	}

	template<typename T>
	template<typename U>
	void SharedPtr<T>::reset(U* ptr)
	{
		SharedPtr temp(ptr);
		temp.swap(*this);
	}

	template<typename T>
	template<typename U, typename Deleter>
	void SharedPtr<T>::reset(U* ptr, Deleter d)
	{
		SharedPtr temp(ptr, d);
		temp.swap(*this);
	}

	template<typename T>
	void SharedPtr<T>::swap(SharedPtr& r)
	{
		if (this == &r) return;
		std::swap(mpElement, r.mpElement);
		std::swap(mpBase, r.mpBase);
	}

	template<typename T>
	typename SharedPtr<T>::element_type* SharedPtr<T>::get() const
	{
		return mpElement;
	}

	template<typename T>
	bcc::intptr_t SharedPtr<T>::use_count() const
	{
		return mpBase ? mpBase->strong() : 0;
	}

	template<typename T>
	bool SharedPtr<T>::unique() const
	{
		return use_count() == 1;
	}

	template<typename T>
	template<typename U>
	void SharedPtr<T>::enable(EnableSharedFromThis<U>* ptr)
	{
		if (ptr) ptr->weak_this = *this;
	}

	template<typename T>
	template<typename U>
	bool SharedPtr<T>::owner_before(SharedPtr<U> const& other) const
	{
		return mpBase < other.mpBase;
	}

	template<typename T>
	template<typename U>
	bool SharedPtr<T>::owner_before(WeakPtr<U> const& other) const
	{
		return mpBase < other.mpBase;
	}

	template<typename T>
	WeakPtr<T>::WeakPtr()
		: mpElement(), mpBase()
	{}

	template<typename T>
	WeakPtr<T>::WeakPtr(WeakPtr const& r)
		: mpElement(r.mpElement), mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseWeak();
	}

	template<typename T>
	template<typename U>
	WeakPtr<T>::WeakPtr(WeakPtr<U> const& r)
		: mpElement(r.mpElement)
		, mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseWeak();
	}

	template<typename T>
	template<typename U>
	WeakPtr<T>::WeakPtr(SharedPtr<U> const& r)
		: mpElement(r.mpElement)
		, mpBase(r.mpBase)
	{
		if (mpBase) mpBase->increaseWeak();
	}

	template<typename T>
	WeakPtr<T>::~WeakPtr()
	{
		if (mpBase) mpBase->decreaseWeak();
	}

	template<typename T>
	WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr const& r)
	{
		if (this == &r) return *this;

		WeakPtr temp(r);
		temp.swap(*this);
		return *this;
	}

	template<typename T>
	template<typename U>
	WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr<U> const& r)
	{
		WeakPtr temp(r);
		temp.swap(*this);
		return *this;
	}

	template<typename T>
	template<typename U>
	WeakPtr<T>& WeakPtr<T>::operator=(SharedPtr<U> const& r)
	{
		WeakPtr temp(r);
		temp.swap(*this);
		return *this;
	}

	template<typename T>
	void WeakPtr<T>::reset()
	{
		mpElement = nullptr;
		if (mpBase)
		{
			mpBase->decreaseWeak();
			mpBase = nullptr;
		}
	}

	template<typename T>
	void WeakPtr<T>::swap(WeakPtr& r)
	{
		if (this == &r) return;
		std::swap(mpElement, r.mpElement);
		std::swap(mpBase, r.mpBase);
	}

	template<typename T>
	bcc::intptr_t WeakPtr<T>::use_count() const
	{
		return mpBase ? mpBase->strong() : 0;
	}

	template<typename T>
	bool WeakPtr<T>::expired() const
	{
		return use_count() == 0;
	}

	template<typename T>
	SharedPtr<T> WeakPtr<T>::lock() const
	{
		if (expired())
			return SharedPtr<T>();
		return SharedPtr<T>(*this);
	}

	template<typename T>
	template<typename U>
	bool WeakPtr<T>::owner_before(WeakPtr<U> const& other) const
	{
		return mpBase < other.mpBase;
	}

	template<typename T>
	template<typename U>
	bool WeakPtr<T>::owner_before(SharedPtr<U> const& other) const
	{
		return mpBase < other.mpBase;
	}

	template<typename T>
	SharedPtr<T> EnableSharedFromThis<T>::shared_from_this()
	{
		return SharedPtr<T>(weak_this);
	}
	
	template<typename T>
	SharedPtr<T const> EnableSharedFromThis<T>::shared_from_this() const
	{
		return SharedPtr<T const>(weak_this);
	}
	
	template<typename T>
	WeakPtr<T> EnableSharedFromThis<T>::weak_from_this()
	{
		return weak_this;
	}
	
	template<typename T>
	WeakPtr<T const> EnableSharedFromThis<T>::weak_from_this() const
	{
		return weak_this;
	}
} // namespace irp

namespace std {
	template<typename CharT, typename Traits>
	class basic_ostream;

	template<typename T>
	void swap(irp::SharedPtr<T>& lhs, irp::SharedPtr<T>& rhs)
	{
		lhs.swap(rhs);
	}

	template<typename T>
	void swap(irp::WeakPtr<T>& lhs, irp::SharedPtr<T>& rhs)
	{
		lhs.swap(rhs);
	}
} // namespace std

namespace irp {
	template<typename Deleter, typename T>
	Deleter* get_deleter(SharedPtr<T> const& p)
	{
		if (p.mpBase)
			return static_cast<Deleter*>(p.mpBase->deleter(typeid(Deleter)));
		return nullptr;
	}

	template<typename T, typename U>
	bool operator==(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return lhs.get() == rhs.get();
	}

	template<typename T, typename U>
	bool operator!=(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return !(lhs.get() == rhs.get());
	}

	template<typename T, typename U>
	bool operator<(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return lhs.get() < rhs.get();
	}

	template<typename T, typename U>
	bool operator>(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return rhs < lhs;
	}

	template<typename T, typename U>
	bool operator<=(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return !(rhs < lhs);
	}

	template<typename T, typename U>
	bool operator>=(SharedPtr<T> const& lhs, SharedPtr<U> const& rhs)
	{
		return !(lhs < rhs);
	}

	template<typename T, typename U>
	SharedPtr<T> static_pointer_cast(SharedPtr<U> const& r)
	{
		typedef typename SharedPtr<T>::element_type element_type;
		return SharedPtr<T>(r, static_cast<element_type*>(r.get()));
	}

	template<typename T, typename U>
	SharedPtr<T> dynamic_pointer_cast(SharedPtr<U> const& r)
	{
		typedef typename SharedPtr<T>::element_type element_type;
		if (element_type* p = dynamic_cast<element_type*>(r.get()))
			return SharedPtr<T>(r, p);
		return SharedPtr<T>();
	}

	template<typename T, typename U>
	SharedPtr<T> const_pointer_cast(SharedPtr<U> const& r)
	{
		typedef typename SharedPtr<T>::element_type element_type;
		return SharedPtr<T>(r, const_cast<element_type*>(r.get()));
	}

	template<typename T, typename U>
	SharedPtr<T> reinterpret_pointer_cast(SharedPtr<U> const& r)
	{
		typedef typename SharedPtr<T>::element_type element_type;
		return SharedPtr<T>(r, reinterpret_cast<element_type*>(r.get()));
	}

	template<typename T, typename CharT, typename Traits>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, SharedPtr<T> const& ptr)
	{
		stream << ptr.get();
		return stream;
	}

	template<typename T>
	SharedPtr<T> make_shared()
	{
		return SharedPtr<T>(new T());
	}

	template<typename T, typename P1>
	SharedPtr<T> make_shared(P1 const& p1)
	{
		return SharedPtr<T>(new T(p1));
	}

	template<typename T, typename P1, typename P2>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2)
	{
		return SharedPtr<T>(new T(p1, p2));
	}

	template<typename T, typename P1, typename P2, typename P3>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3)
	{
		return SharedPtr<T>(new T(p1, p2, p3));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
		, typename P5>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
		, typename P5, typename P6>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5, P6 const& p6)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5, p6));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
		, typename P5, typename P6, typename P7>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5, P6 const& p6, P7 const& p7)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5, p6, p7));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
		, typename P5, typename P6, typename P7, typename P8>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5, p6, p7, p8));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
		, typename P5, typename P6, typename P7, typename P8, typename P9>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, P9 const& p9)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5, p6, p7, p8, p9));
	}

	template<typename T, typename P1, typename P2, typename P3, typename P4
			, typename P5, typename P6, typename P7, typename P8, typename P9, typename P10>
	SharedPtr<T> make_shared(P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4
		, P5 const& p5, P6 const& p6, P7 const& p7, P8 const& p8, P9 const& p9, P10 const& p10)
	{
		return SharedPtr<T>(new T(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10));
	}
} // namespace irp

#endif // !def INDUROP_DETAIL_SHARED_PTR_H__
