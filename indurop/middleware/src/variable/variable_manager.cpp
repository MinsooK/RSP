
#include "variable_manager.h"

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>

#include <bicomc/object.h>

#include "../util/environment.h"

using irp::VariableManager;

VariableManager irp::gVariableManager;

VariableManager::VariableManager(std::string const& path)
	: mContextPath(path)
{
	load();
}

VariableManager::~VariableManager()
{
	save();
}

void VariableManager::swap(VariableManager& rhs)
{
	if (this == &rhs) return;

	std::swap(mContextPath, rhs.mContextPath);
	std::swap(mBlocks, rhs.mBlocks);
	std::swap(mVariableMap, rhs.mVariableMap);
}

std::pair<std::string, bcc::uint64_t> VariableManager::create(std::string const& name, std::string const& type, bcc::uint32_t hintSize)
{
	VariableMap::iterator itor = mVariableMap.find(name);

	if (itor == mVariableMap.end())
	{
		Info info = secure(name, type, hintSize);

		itor = mVariableMap.insert(std::make_pair(name, info)).first;
	}

	if (itor->second.type != type)
		throw std::runtime_error(std::string("'").append(name).append("' variable's tpye is not matched"));

	// alias 확인
	if (itor->second.pHeader->payloadSize == 0)
	{
		std::string origin(type, type.find('(') + 1);
		{
			origin.resize(origin.size() - 1); // remove ')'
			std::size_t pos = origin.find_first_of(".[");
			if (pos != std::string::npos)
				origin.resize(pos);
		}

		itor = mVariableMap.find(origin);
		if (itor == mVariableMap.end())
			throw std::runtime_error(std::string("the origin of '").append(type).append("' dose not exist"));
	}

	SharedMemoryBlock const& block = *itor->second.blockItor;

	bcc::int64_t pos = block.relative(itor->second.pHeader);
	if (pos < 0)
		throw std::runtime_error("internal error");

	bcc::uint64_t size = bcc::uint64_t(block.size()) << (sizeof(bcc::uint32_t) * 8);
	size |= bcc::uint32_t(pos);
		
	return std::make_pair(block.key(), size);
}

VariableManager::Info VariableManager::secure(std::string const& name, std::string const& type, bcc::uint32_t hintSize)
{
	struct Pad
	{
		static bcc::uint32_t calculate(bcc::uint32_t size)
		{
			return bcc::uint32_t(float(size) / sizeof(bcc::uint64_t) + 1) * sizeof(bcc::uint64_t);
		}
	};

	bcc::uint32_t typeSize = estimateSize(type, hintSize);
	bcc::uint32_t payloadSize = typeSize ? Pad::calculate(sizeof(bcc::uint64_t) + typeSize) * 2 : 0;
	bcc::uint32_t size = sizeof(SharedVariable::Header) + payloadSize + Pad::calculate(type.size() + 1);

	BlockList::iterator itor = mBlocks.begin(), end = mBlocks.end();
	for (; itor != end; ++itor)
	{
		if (itor->remain() >= size)
			break;
	}

	if (itor == end)
	{
		std::stringstream stream;
		stream.exceptions(std::ios::failbit | std::ios::badbit);
		for (std::size_t i = mBlocks.size(); i < std::size_t(-1); ++i)
		{
			stream.str("");
			stream << __FILE__ ":" << i;

			std::string rawKey = stream.str();
			bcc::uint64_t hash = bcc::detail::Hasher<bcc::Object>::murmurHashNeutral64v2(rawKey.c_str(), rawKey.size(), 0);

			stream.str("");
			stream << std::hex << std::setw(2) << std::setfill('0') << hash;

			try
			{
				SharedMemoryBlock block(stream.str(), BLOCK_SIZE, true);
				itor = mBlocks.insert(mBlocks.end(), block);
				break;
			}
			catch (...)
			{}
		}

		if (itor == end)
			throw std::bad_alloc();

		try
		{
			save();
		}
		catch (...)
		{}
	}

	if (irp::SharedPtr<char> p = itor->acquire(size))
	{
		Info result;
		result.blockItor = itor;
		result.holder = p;
		result.pHeader = reinterpret_cast<SharedVariable::Header*>(p.get());
		result.type = type;

		std::memset(p.get(), 0, size);
		result.pHeader->next = 0;
		result.pHeader->usage = 1;
		result.pHeader->size = size;
		result.pHeader->payloadSize = payloadSize;

		std::strcpy(reinterpret_cast<char*>(result.pHeader + 1) + result.pHeader->payloadSize, name.c_str());

		if (itor->relative(p.get()) != sizeof(SharedMemoryBlock::Header))
		{
			bcc::uint32_t index = sizeof(SharedMemoryBlock::Header);
			for (;;)
			{
				SharedVariable::Header* pHeader = reinterpret_cast<SharedVariable::Header*>(itor->raw(index));
				index = pHeader->next;
				if (index == 0)
				{
					pHeader->next = itor->relative(p.get());
					break;
				}
			}

		}

		std::string path = name;
		std::replace(path.begin(), path.end(), '.', '/');
		path = irp::Environment::variableDirectory() + "/" + path + ".var";
		std::ofstream stream(path.c_str());
		stream << type;
		stream.close();

		return result;
	}

	throw std::bad_alloc();
}

bcc::uint32_t VariableManager::estimateSize(std::string const& type, bcc::uint32_t hintSize)
{
	if (std::strncmp(type.c_str(), "alias(", sizeof("alias(") - 1) == 0)
		return 0;

	if (hintSize != 0)
		return hintSize;

	if (type == "int")
		return 4;

	std::stringstream stream(type);
	stream.exceptions(std::ios::failbit | std::ios::badbit);

	bcc::uint32_t result;
	stream >> result;
	return result;
}

void VariableManager::load()
{
	BlockList prevBlocks;
	{
		std::ifstream stream(mContextPath.c_str(), std::ios::in);

		std::string key;
		bcc::uint32_t size;

		while (stream >> key >> size)
		{
			SharedMemoryBlock block(key, size);
			if (block.empty())
				continue;

			prevBlocks.push_back(block);
		}
	}

	VariableMap prevMap;
	// prevBlocks 를 읽어서 변수 맵을 작성
	for (BlockList::const_iterator itor = prevBlocks.begin(), end = prevBlocks.end()
		; itor != end; ++itor)
	{
		SharedMemoryBlock const& block = *itor;
		if (block.last() == sizeof(SharedMemoryBlock::Header))
			continue;

		std::size_t index = sizeof(SharedMemoryBlock::Header);
		std::size_t last = block.last();
		while (index < last || index != 0)
		{
			Info info;
			info.blockItor = itor;
			info.holder = block.get(index);
			info.pHeader = reinterpret_cast<SharedVariable::Header*>(info.holder.get());
			char const* name = reinterpret_cast<char*>(info.pHeader + 1) + info.pHeader->payloadSize;

			if (mVariableMap.count(name) == 0)
			{
				std::string path = name;
				std::replace(path.begin(), path.end(), '.', '/');
				path = irp::Environment::variableDirectory() + "/" + path + ".var";

				std::ifstream stream(path.c_str());
				if (stream.is_open())
				{
					stream.seekg(0, std::ios::end);
					info.type.reserve(stream.tellg());
					stream.seekg(0, std::ios::beg);

					info.type.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());

					mVariableMap.insert(std::make_pair(name, info));
				}
			}

			index = info.pHeader->next;
		}
	}

	mVariableMap.swap(prevMap);
	mBlocks.swap(prevBlocks);
}

void VariableManager::save() const
{
	if (mContextPath.empty())
		return;

	std::ofstream stream(mContextPath.c_str(), std::ios::trunc);
	stream.exceptions(std::ios::failbit | std::ios::badbit);

	for (BlockList::const_iterator itor = mBlocks.begin(), end = mBlocks.end()
		; itor != end; ++itor)
	{
		stream << itor->key() << "\t" << itor->size() << "\n";
	}

	stream.flush();
}
