
#include "monitor.h"

namespace test {

Variable_time time;

namespace { char const Variable_time_type[] = "int32_t"; }
Variable_time::Variable_time()
	: irp::SharedVariable("time", Variable_time_type, sizeof(value_type)), mCache()
{
}

void Variable_time::onPull(void const* p) const
{
	p = reinterpret_cast<char const*>(p) + 0;
	mCache = *reinterpret_cast<value_type const*>(p);
}

void Variable_time::onPush(void* p)
{
	p = reinterpret_cast<char*>(p) + 0;
	*reinterpret_cast<value_type*>(p) = mCache;
}
}
