
#include "memory_block.h"

#include <algorithm>
#include <stdexcept>

using irp::SharedMemory;
using irp::SharedMemoryBlock;

SharedMemoryBlock::SharedMemoryBlock(std::string const& key, bcc::uint32_t blockSize, bool exclusive)
{
	blockSize = std::max<std::size_t>(blockSize, sizeof(Header));

	mBlock = SharedMemory::make(blockSize, key, exclusive);
	mpHeader = mBlock.get<Header>();
	mpHeader->size = blockSize;
	mpHeader->last = sizeof(Header);
}

SharedMemoryBlock::~SharedMemoryBlock()
{}

irp::SharedPtr<char> SharedMemoryBlock::allocate(std::size_t size)
{
	return acquire(size);
}

irp::SharedPtr<char> SharedMemoryBlock::acquire(bcc::uint32_t size)
{
	if (remain() < size)
		return nullptr;

	char* p = mBlock.get<char>() + mpHeader->last;
	mpHeader->last += size;

	return SharedPtr<char>(mBlock.holder(), p);
}

irp::SharedPtr<char> irp::SharedMemoryBlock::get(bcc::uint32_t index) const
{
	if (index >= last())
		throw std::out_of_range("'index' is out of range");

	return SharedPtr<char>(mBlock.holder(), mBlock.get<char>() + index);
}

void* SharedMemoryBlock::raw(bcc::uint32_t index) const
{
	if (index >= last())
		throw std::out_of_range("'index' is out of range");

	return mBlock.get<char>() + index;
}

bcc::int64_t SharedMemoryBlock::relative(void const* p) const
{
	std::ptrdiff_t diff = static_cast<char const*>(p) - mBlock.get<char>();

	if (diff < 0 || std::size_t(diff) >= mBlock.size())
		return -1;

	return bcc::int64_t(diff);
}
