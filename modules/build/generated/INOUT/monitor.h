
#pragma once

#include <indurop/indurop.h>

namespace test {

class Variable_time : public irp::SharedVariable
{
public:
typedef int32_t value_type;

public:
	Variable_time();

public:

public:
	operator value_type const&() const { irp::SharedVariable::pull(); return mCache; }
	Variable_time& operator=(value_type const& rhs) { mCache = rhs; irp::SharedVariable::push(); return *this; }

private:
	void onPull(void const* p) const;
	void onPush(void* p);
	void* cache() const { return &mCache; }

private:
	mutable value_type mCache;
};
extern Variable_time time;

}
