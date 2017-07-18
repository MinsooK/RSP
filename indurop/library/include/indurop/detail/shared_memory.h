
#ifndef INDUROP_DETAIL_SHARED_MEMORY_H__
#define INDUROP_DETAIL_SHARED_MEMORY_H__

#include <cstddef>
#include <string>

#include "shared_ptr.h"

namespace irp {

	class SharedMemory
	{
	public:
		SharedMemory() : mpRaw(), mSize(), mKey() {}

	public:
		void* raw() const { return mpRaw.get(); }
		std::size_t size() const { return mSize; }
		std::string const& key() const { return mKey; }

	public:
		template<typename T>
		T* get() const { return static_cast<T*>(raw()); }
		template<typename T>
		T* get(std::size_t index) const { return &get<T>()[index]; }

		SharedPtr<char const> holder() const { return mpRaw; }

	public:
		static SharedMemory make(std::size_t size, std::string const& key, bool exclusive);

	private:
		SharedPtr<char> mpRaw;
		std::size_t mSize;
		std::string mKey;
	};

} // namespace irp

#endif // !def INDUROP_DETAIL_SHARED_MEMORY_H__
