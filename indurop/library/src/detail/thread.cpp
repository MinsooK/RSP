
#include <indurop/detail/thread.h>

#include <cstring>
#include <stdexcept>

#if defined(_WIN32)
#include <Windows.h>
#else
#include <sys/sysinfo.h>
#include <pthread.h>
#include <time.h>
#endif // defined(_WIN32)

using irp::Thread;
using irp::detail::ThreadCallable;

Thread::id::id()
	: handle()
{}

Thread::~Thread()
{
	if (joinable())
		std::terminate();
}

bool Thread::joinable() const
{
	return mHandle != native_handle_type();
}

Thread::id Thread::get_id() const
{
	return id(mHandle);
}

void Thread::detach()
{
	if (!joinable())
		throw std::runtime_error("thread is not joinable.");
	mHandle = native_handle_type();
	mpCallable.reset();
}

#if defined(_WIN32)
namespace {
	struct Win32ThreadDeleter
	{
		Win32ThreadDeleter(HANDLE handle_ = nullptr) : handle(handle_) {}

		void operator()(ThreadCallable* p)
		{
			if (handle != nullptr)
			{
				WaitForSingleObject(handle, INFINITE);
				CloseHandle(handle);
			}
			delete p;
		}

		HANDLE handle;
	};

	DWORD WINAPI win32ThreadRoutine(LPVOID param)
	{
		try
		{
			ThreadCallable* p = static_cast<ThreadCallable*>(param);
			p->call();
		}
		catch (...)
		{
			std::terminate();
		}

		return 0;
	}
}

Thread::Thread()
	: mHandle()
{}

void Thread::join()
{
	if (!joinable())
		throw std::runtime_error("thread is not joinable.");

	if (get_id() == this_thread::get_id())
		throw std::runtime_error("deadlock detected");

	if (WaitForSingleObject(mHandle, INFINITE) == WAIT_FAILED)
	{
		DWORD error = GetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, message, sizeof(message), NULL);
		throw std::runtime_error(message);
	}
	mHandle = nullptr;
	mpCallable.reset();
}

unsigned int Thread::hardware_concurrency()
{
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	return static_cast<unsigned int>(info.dwNumberOfProcessors);
}

void Thread::start(ThreadCallable* pCallable)
{
	if (!pCallable)
		throw std::invalid_argument("'pCallable' must not be null.");

	irp::SharedPtr<ThreadCallable> p(pCallable, Win32ThreadDeleter());
	Win32ThreadDeleter* pDeleter = irp::get_deleter<Win32ThreadDeleter>(p);

	pDeleter->handle = CreateThread(nullptr, 0, &win32ThreadRoutine, pCallable, 0, nullptr);
	if (pDeleter->handle == nullptr)
	{
		DWORD error = GetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, message, sizeof(message), NULL);
		throw std::runtime_error(message);
	}

	mHandle = pDeleter->handle;
	mpCallable.swap(p);
}

void irp::this_thread::yield()
{
	Sleep(0);
}

void irp::this_thread::sleep_for(bcc::uint32_t ms)
{
	Sleep(DWORD(ms));
}

Thread::id irp::this_thread::get_id()
{
	return Thread::id(GetCurrentThread());
}

#else
namespace {
	struct PosixThreadDeleter
	{
		PosixThreadDeleter(pthread_t handle_ = pthread_t()) : handle(handle_) {}

		void operator()(ThreadCallable* p)
		{
			if (handle != pthread_t())
			{
				pthread_join(handle, nullptr);
			}
			delete p;
		}

		pthread_t handle;
	};

	void* posixThreadRoutine(void* param)
	{
		try
		{
			ThreadCallable* p = static_cast<ThreadCallable*>(param);
			p->call();
		}
		catch (...)
		{
			std::terminate();
		}

		return nullptr;
	}
}

Thread::Thread()
	: mHandle()
{}

void Thread::join()
{
	if (!joinable())
		throw std::runtime_error("thread is not joinable.");

	if (get_id() == this_thread::get_id())
		throw std::runtime_error("deadlock detected");

	int error = pthread_join(mHandle, nullptr);
	if (error != 0)
		throw std::runtime_error(std::strerror(error));

	mHandle = pthread_t();
	mpCallable.reset();
}

unsigned int Thread::hardware_concurrency()
{
	return static_cast<unsigned int>(get_nprocs());
}

void Thread::start(ThreadCallable* pCallable)
{
	if (!pCallable)
		throw std::invalid_argument("'pCallable' must not be null.");

	irp::SharedPtr<ThreadCallable> p(pCallable, PosixThreadDeleter());
	PosixThreadDeleter* pDeleter = irp::get_deleter<PosixThreadDeleter>(p);

	int error = pthread_create(&pDeleter->handle, nullptr, &posixThreadRoutine, pCallable);
	if (error != 0)
		throw std::runtime_error(std::strerror(error));

	mHandle = pDeleter->handle;
	mpCallable.swap(p);
}

void irp::this_thread::yield()
{
	pthread_yield();
}

void irp::this_thread::sleep_for(bcc::uint32_t ms)
{
	struct timespec time = {};
	time.tv_sec = ms / 1000;
	time.tv_nsec = long(ms * 1000000) % 1000000000;
	nanosleep(&time, nullptr);
}

Thread::id irp::this_thread::get_id()
{
	return Thread::id(pthread_self());
}

#endif // defined(_WIN32)
