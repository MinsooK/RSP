
#include <indurop/detail/shared_ptr.h>

#include <stdexcept>

#include <bicomc/detail/atomic.h>

using namespace bcc::detail;
using irp::detail::SharedCounter;

namespace {
	bcc::intptr_t increase(bcc::intptr_t& count)
	{
		for (;;)
		{
			bcc::intptr_t old = atomic_load(&count);
			if (old == 0) throw std::runtime_error("invalid reference count");
			if (atomic_compare_exchange_strong(&count, &old, old + 1))
				return old;
		}
	}

	bcc::intptr_t decrease(bcc::intptr_t& count)
	{
		for (;;)
		{
			bcc::intptr_t old = atomic_load(&count);
			if (old == 0) throw std::runtime_error("invalid reference count");
			if (atomic_compare_exchange_strong(&count, &old, old - 1))
				return old;
		}
	}
}

void SharedCounter::increaseStrong()
{
	increase(mStrongCount);
}

void SharedCounter::decreaseStrong()
{
	if (decrease(mStrongCount) != 1)
		return;

	try
	{
		destroy();
	}
	catch (...)
	{
		decreaseWeak();
		throw;
	}

	decreaseWeak();
}

bcc::intptr_t SharedCounter::strong() const
{
	return atomic_load(&mStrongCount);
}

void SharedCounter::increaseWeak()
{
	increase(mWeakCount);
}

void SharedCounter::decreaseWeak()
{
	if (decrease(mWeakCount) == 1)
		delete this;
}

bcc::intptr_t SharedCounter::weak() const
{
	return atomic_load(&mWeakCount);
}
