
#ifndef VARIABLE_MEMORY_BLOCK_H__
#define VARIABLE_MEMORY_BLOCK_H__

#include <string>

#include <bicomc/stdint.h>
#include <indurop/detail/shared_memory.h>

namespace irp {

	class SharedMemoryBlock
	{
	public:
		struct Header
		{
			bcc::uint32_t size;
			bcc::uint32_t last;
		};

	public:
		SharedMemoryBlock(std::string const& key, bcc::uint32_t blockSize = 4 * 1024 * 1024, bool exclusive = false);
		~SharedMemoryBlock();

	public:
		bcc::uint32_t size() const { return mpHeader->size; }
		bcc::uint32_t last() const { return mpHeader->last; }
		bcc::uint32_t remain() const { return size() - last(); }
		bool empty() const { return last() == sizeof(Header); }

	public:
		SharedPtr<char> allocate(std::size_t size);
		SharedPtr<char> acquire(bcc::uint32_t size);
		SharedPtr<char> get(bcc::uint32_t index) const;
		bcc::int64_t relative(void const* p) const;

	public:
		std::string const& key() const { return mBlock.key(); }

		void* raw() const { return mBlock.raw(); }
		void* raw(bcc::uint32_t index) const;
		void* body() const { return mBlock.get<char>() + sizeof(Header); }

	private:
		SharedMemory mBlock;
		Header* mpHeader;
	};

} // namespace irp

#endif // !def VARIABLE_MEMORY_BLOCK_H__
