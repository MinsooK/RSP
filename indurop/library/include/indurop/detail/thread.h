
#ifndef INDUROP_DETAIL_THREAD_H__
#define INDUROP_DETAIL_THREAD_H__

#include <cstddef>
#include <exception>
#include <utility>

#include <bicomc/stdint.h>
#include <bicomc/tuple.h>

#include "shared_ptr.h"

namespace irp {
namespace detail {
	class ThreadCallable : public EnableSharedFromThis<ThreadCallable>
	{
	public:
		virtual ~ThreadCallable() {}
	
	public:
		void call()
		{
			SharedPtr<ThreadCallable> pHold = shared_from_this();
			onCall();
		}

	protected:
		virtual void onCall() = 0;
	};

	template<typename Function, typename Args>
	struct ThreadFunction : public ThreadCallable
	{
		ThreadFunction()
			: func(), args()
		{}

		ThreadFunction(Function const& func_)
			: func(func_), args()
		{}

		ThreadFunction(Function const& func_, Args const& args_)
			: func(func_), args(args_)
		{}

		void onCall()
		{
			callImpl<Args>();
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 0>::type* = 0)
		{
			func();
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 1>::type* = 0)
		{
			func(bcc::get<0>(args));
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 2>::type* = 0)
		{
			func(bcc::get<0>(args), bcc::get<1>(args));
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 3>::type* = 0)
		{
			func(bcc::get<0>(args), bcc::get<1>(args), bcc::get<2>(args));
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 4>::type* = 0)
		{
			func(bcc::get<0>(args), bcc::get<1>(args), bcc::get<2>(args)
				, bcc::get<3>(args));
		}

		template<typename U>
		void callImpl(typename bcc::enable_if<bcc::tuple_size<U>::value == 5>::type* = 0)
		{
			func(bcc::get<0>(args), bcc::get<1>(args), bcc::get<2>(args)
				, bcc::get<3>(args), bcc::get<4>(args));
		}

		Function func;
		Args args;
	};
} // namespace detail

	class Thread
	{
	public:
#if defined(_WIN32)
		typedef void* native_handle_type;
#else
		typedef unsigned long native_handle_type;
#endif // defined(_WIN32)

		class id
		{
		public:
			id();
			id(Thread::native_handle_type const& h) : handle(h) {}

		public:
			Thread::native_handle_type handle;
		};

	public:
		Thread();
		~Thread();

	public:
		template<typename Function>
		explicit Thread(Function const& f);

		template<typename Function, typename P1>
		explicit Thread(Function const& f, P1 const& p1);

		template<typename Function, typename P1, typename P2>
		explicit Thread(Function const& f, P1 const& p1, P2 const& p2);

		template<typename Function, typename P1, typename P2, typename P3>
		explicit Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3);

		template<typename Function, typename P1, typename P2, typename P3, typename P4>
		explicit Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4);

		template<typename Function, typename P1, typename P2, typename P3, typename P4, typename P5>
		explicit Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5);

	private:
		Thread(Thread const&);

	public:
		bool joinable() const;
		id get_id() const;
		native_handle_type native_handle();

	public:
		void join();
		void detach();
		void swap(Thread& other);

	public:
		static unsigned int hardware_concurrency();

	private:
		void start(detail::ThreadCallable* pCallable);

	private:
		native_handle_type mHandle;
		SharedPtr<detail::ThreadCallable> mpCallable;
	};

	template<typename Function>
	Thread::Thread(Function const& f)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		start(p);
	}

	template<typename Function, typename P1>
	Thread::Thread(Function const& f, P1 const& p1)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<P1> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		try
		{
			bcc::get<0>(p->args) = p1;
		}
		catch (...)
		{
			delete p;
			throw;
		}

		start(p);
	}

	template<typename Function, typename P1, typename P2>
	Thread::Thread(Function const& f, P1 const& p1, P2 const& p2)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<P1, P2> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		try
		{
			bcc::get<0>(p->args) = p1;
			bcc::get<1>(p->args) = p2;
		}
		catch (...)
		{
			delete p;
			throw;
		}

		start(p);
	}

	template<typename Function, typename P1, typename P2, typename P3>
	Thread::Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<P1, P2, P3> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		try
		{
			bcc::get<0>(p->args) = p1;
			bcc::get<1>(p->args) = p2;
			bcc::get<2>(p->args) = p3;
		}
		catch (...)
		{
			delete p;
			throw;
		}

		start(p);
	}

	template<typename Function, typename P1, typename P2, typename P3, typename P4>
	Thread::Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<P1, P2, P3, P4> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		try
		{
			bcc::get<0>(p->args) = p1;
			bcc::get<1>(p->args) = p2;
			bcc::get<2>(p->args) = p3;
			bcc::get<3>(p->args) = p4;
		}
		catch (...)
		{
			delete p;
			throw;
		}

		start(p);
	}

	template<typename Function, typename P1, typename P2, typename P3, typename P4, typename P5>
	Thread::Thread(Function const& f, P1 const& p1, P2 const& p2, P3 const& p3, P4 const& p4, P5 const& p5)
		: mHandle()
	{
		using detail::ThreadFunction;
		typedef bcc::tuple<P1, P2, P3, P4, P5> Args;
		ThreadFunction<Function, Args>* p = new ThreadFunction<Function, Args>(f);
		try
		{
			bcc::get<0>(p->args) = p1;
			bcc::get<1>(p->args) = p2;
			bcc::get<2>(p->args) = p3;
			bcc::get<3>(p->args) = p4;
			bcc::get<4>(p->args) = p5;
		}
		catch (...)
		{
			delete p;
			throw;
		}

		start(p);
	}

	inline Thread::native_handle_type Thread::native_handle()
	{
		return mHandle;
	}

	inline void Thread::swap(Thread& other)
	{
		if (this == &other) return;
		std::swap(mHandle, other.mHandle);
		std::swap(mpCallable, other.mpCallable);
	}

namespace this_thread {
	void yield();
	void sleep_for(bcc::uint32_t ms);
	Thread::id get_id();
} // namespace this_thread
} // namespace irp

namespace std {
	template<typename CharT, typename Traits>
	class basic_ostream;

	inline void swap(irp::Thread& lhs, irp::Thread& rhs)
	{
		lhs.swap(rhs);
	}
} // namespace std

namespace irp {
	inline bool operator==(Thread::id lhs, Thread::id rhs)
	{
		return lhs.handle == rhs.handle;
	}

	inline bool operator!=(Thread::id lhs, Thread::id rhs)
	{
		return !(lhs.handle == rhs.handle);
	}

	inline bool operator<(Thread::id lhs, Thread::id rhs)
	{
		return lhs.handle < rhs.handle;
	}

	inline bool operator>(Thread::id lhs, Thread::id rhs)
	{
		return rhs.handle < lhs.handle;
	}

	inline bool operator<=(Thread::id lhs, Thread::id rhs)
	{
		return !(rhs.handle < lhs.handle);
	}

	inline bool operator>=(Thread::id lhs, Thread::id rhs)
	{
		return !(lhs.handle < rhs.handle);
	}

	template<typename CharT, typename Traits>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& stream, Thread::id id)
	{
		stream << id.handle;
		return stream;
	}
} // namespace irp

#endif // !def INDUROP_DETAIL_THREAD_H__
