
#include <indurop/variable.h>

#include <indurop/detail/thread.h>

#if defined(_MSC_VER)
#	include <intrin.h>

#endif // defined(_MSC_VER)

#if defined(_WIN32)
#	include <Windows.h>

#else
#	include <sys/time.h>

#endif // defined(_MSC_VER)


using irp::SharedVariable;

namespace {
	enum memory_order
	{
		memory_order_acquire, memory_order_release
	};


#if defined(_MSC_VER)
	void atomic_store(bcc::uint32_t& object, bcc::uint32_t desired)
	{
		long* p = reinterpret_cast<long*>(&object);
		::_InterlockedExchange(p, desired);
	}

	bcc::uint32_t atomic_fetch_or(bcc::uint32_t& object, bcc::uint32_t value)
	{
		long* p = reinterpret_cast<long*>(&object);
		return ::_InterlockedOr(p, value);
	}

	bcc::uint32_t atomic_fetch_sub(bcc::uint32_t& object, bcc::uint32_t value)
	{
		long* p = reinterpret_cast<long*>(&object);
		return ::_InterlockedExchangeAdd(p, bcc::uint32_t(-bcc::int32_t(value)));
	}

	bool atomic_compare_exchange_strong(bcc::uint32_t& object, bcc::uint32_t& expected, bcc::uint32_t desired)
	{
		long* p = reinterpret_cast<long*>(&object);
		bcc::intptr_t e = expected;
		return e == (expected = ::_InterlockedCompareExchange(p, desired, e));
	}

	void atomic_thread_fence(memory_order order)
	{
		_ReadWriteBarrier();
	}

#elif defined(__GNUC__)
#	if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 7) || (__GNUC__ > 4)
	void atomic_store(bcc::uint32_t& object, bcc::uint32_t desired)
	{
		__atomic_store_n(&object, desired, __ATOMIC_SEQ_CST);
	}

	bcc::uint32_t atomic_fetch_or(bcc::uint32_t& object, bcc::uint32_t value)
	{
		return __atomic_fetch_or(&object, value, __ATOMIC_SEQ_CST);
	}

	bcc::uint32_t atomic_fetch_sub(bcc::uint32_t& object, bcc::uint32_t value)
	{
		return __atomic_fetch_add(&object, value, __ATOMIC_SEQ_CST);
	}

	bool atomic_compare_exchange_strong(bcc::uint32_t& object, bcc::uint32_t& expected, bcc::uint32_t desired)
	{
		return __atomic_compare_exchange_n(&object, &expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
	}

	void atomic_thread_fence(memory_order order)
	{
		switch (order)
		{
		case memory_order_acquire:
			__atomic_thread_fence(__ATOMIC_ACQUIRE);
			break;

		case memory_order_release:
			__atomic_thread_fence(__ATOMIC_RELEASE);
			break;

		default:
			__atomic_thread_fence(__ATOMIC_SEQ_CST);
			break;
		}
	}

#	else
	void atomic_store(bcc::uint32_t& object, bcc::uint32_t desired)
	{
		__sync_lock_test_and_set(&object, desired);
	}

	bcc::uint32_t atomic_fetch_or(bcc::uint32_t& object, bcc::uint32_t value)
	{
		return __sync_fetch_and_or(&object, value);
	}

	bcc::uint32_t atomic_fetch_sub(bcc::uint32_t& object, bcc::uint32_t value)
	{
		return __sync_fetch_and_sub(&object, value);
	}

	bool atomic_compare_exchange_strong(bcc::uint32_t& object, bcc::uint32_t& expected, bcc::uint32_t desired)
	{
		bcc::intptr_t e = expected;
		return e == (expected = __sync_val_compare_and_swap(&object, e, desired));
	}

	void atomic_thread_fence(memory_order order)
	{
		__sync_synchronize();
	}

#	endif
#endif // defined(_MSC_VER)
}

SharedVariable::SharedVariable(char const* name, char const* type, bcc::uint32_t size)
	: mTimeStamp(bcc::uint32_t(-1))
{
	std::pair<std::string, bcc::uint64_t> key;
	{
		key = ApiClient::bind(name, type, size);
	}

	mMemory = SharedMemory::make(bcc::uint32_t(key.second >> (sizeof(bcc::uint32_t) * 8)), key.first, false);
	mpHeader = reinterpret_cast<Header*>(mMemory.get<char>(bcc::uint32_t(key.second)));

	mValues[0] = reinterpret_cast<char*>(mpHeader + 1);
	mValues[1] = reinterpret_cast<char*>(mValues[0]) + (mpHeader->payloadSize / 2);

	mpFooter = reinterpret_cast<char*>(mValues[0]) + mpHeader->payloadSize;
}

void SharedVariable::pull() const
{
	bcc::uint32_t index = atomic_fetch_or(mpHeader->latest, 0);
	if (time() == *static_cast<bcc::uint64_t*>(mValues[index]))
		return;

	for (bcc::uint32_t state, count = 0;;)
	{
		if (!(++count % REPEAT_COUNT))
			irp::this_thread::yield();

		state = atomic_fetch_or(mpHeader->state, 0);

		if (state & 1) continue;

		bcc::uint32_t old = state;
		if (atomic_compare_exchange_strong(mpHeader->state, old, state + 2))
			break;
	}

	index = atomic_fetch_or(mpHeader->latest, 0);
	try
	{
		atomic_thread_fence(memory_order_acquire);
		stamp(*static_cast<bcc::uint64_t*>(mValues[index]));
		onPull(static_cast<bcc::uint64_t*>(mValues[index]) + 1);
	}
	catch (...)
	{}

	atomic_fetch_sub(mpHeader->state, 2);
}

void SharedVariable::push()
{
	bcc::uint32_t old;
	bool isOwner;
	do
	{
		old = atomic_fetch_or(mpHeader->state, 1);
		if (old == 0) break;
		isOwner = !(old & 1);
		if (!isOwner) continue;

		for (bcc::uint32_t count = 1; count < REPEAT_COUNT; ++count)
		{
			old = atomic_fetch_or(mpHeader->state, 0);
			if (old <= 1) break;

			if (!(count % REPEAT_COUNT))
				irp::this_thread::yield();
		}
	} while (!isOwner);

	bcc::uint32_t index = atomic_fetch_or(mpHeader->latest, 0);
	bcc::uint32_t nextIndex = (index + 1) % bcc::uint32_t(detail::length(mValues));
	try
	{
		std::memcpy(mValues[nextIndex], mValues[index], mpHeader->payloadSize / 2);

		*static_cast<bcc::uint64_t*>(mValues[nextIndex]) = stamp();
		onPush(static_cast<bcc::uint64_t*>(mValues[nextIndex]) + 1);
		atomic_thread_fence(memory_order_release);
	}
	catch (...)
	{}

	atomic_store(mpHeader->latest, nextIndex);
	atomic_fetch_sub(mpHeader->state, 1);
}

bcc::uint64_t SharedVariable::time() const
{
	return mTimeStamp;
}

bcc::uint64_t SharedVariable::stamp() const
{
	bcc::uint64_t time;
#if defined(_WIN32)
	LARGE_INTEGER frequency, count;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&count);
	time = count.QuadPart * 1000000 / frequency.QuadPart;

#else
	struct timeval val;
	gettimeofday(&val, nullptr);
	time = val.tv_sec * 1000000 + val.tv_usec;

#endif

	return stamp(time);
}

bcc::uint64_t irp::SharedVariable::stamp(bcc::uint64_t time) const
{
	return mTimeStamp = time;
}

