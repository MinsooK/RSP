
#include <indurop/detail/shared_memory.h>

#include <cerrno>
#include <stdexcept>

#include <bicomc/object.h>

using irp::SharedMemory;

#if defined(_WIN32)
// Windows
#include <Windows.h>

namespace {
	struct Deleter
	{
		Deleter(HANDLE handle_) : handle(handle_) {}

		void operator()(char* p)
		{
			UnmapViewOfFile(p);
			CloseHandle(handle);
		}

		HANDLE handle;
	};
}

SharedMemory SharedMemory::make(std::size_t size, std::string const& key, bool exclusive)
{
	HANDLE handle = NULL;
	if (!exclusive)
		handle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, key.c_str());

	if (handle == NULL)
	{
		handle = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE
			, DWORD(bcc::uint64_t(size) >> (sizeof(DWORD) * 8)), DWORD(size & DWORD(-1))
			, key.c_str());
	}

	void* p = nullptr;
	if (handle != NULL)
		p = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, size);

	if (!p)
	{
		DWORD error = GetLastError();
		CloseHandle(handle);

		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, message, sizeof(message), NULL);

		throw std::runtime_error(message);
	}

	SharedMemory result;
	result.mpRaw.reset(static_cast<char*>(p), Deleter(handle));
	result.mSize = size;
	result.mKey = key;

	return result;
}

#else
// POSIX
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

namespace {
	bcc::uint64_t calculate(void const* key, bcc::uint64_t size)
	{
		return bcc::detail::Hasher<bcc::Object>::murmurHashNeutral64v2(key, size, 0);
	}

	struct Deleter
	{
		Deleter(int handle_) : handle(handle_) {}

		void operator()(char* p)
		{
			shmdt(p);
			shmctl(handle, IPC_RMID, nullptr);
			close(handle);
		}

		int handle;
	};
}

SharedMemory SharedMemory::make(std::size_t size, std::string const& key, bool exclusive)
{
	bcc::uint64_t hash = calculate(key.c_str(), key.size());
	key_t rawKey = static_cast<key_t>(hash);
	int flag = 0666 | IPC_CREAT | (exclusive ? IPC_EXCL : 0);

	int handle = shmget(rawKey, size, flag);
	if (handle < 0)
		throw std::runtime_error(std::strerror(errno));

	void* p = shmat(handle, nullptr, 0);
	if (p == reinterpret_cast<void*>(-1))
	{
		int error = errno;
		close(handle);
		throw std::runtime_error(std::strerror(error));
	}

	SharedMemory result;
	result.mpRaw.reset(static_cast<char*>(p), Deleter(handle));
	result.mSize = size;
	result.mKey = key;

	return result;
}

#endif // defined(_WIN32)