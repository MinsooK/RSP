
#ifndef VARIABLE_VARIABLE_MANAGER_H__
#define VARIABLE_VARIABLE_MANAGER_H__

#include <list>
#include <map>
#include <string>
#include <utility>

#include <bicomc/stdint.h>

#include <indurop/indurop.h>
#include <indurop/detail/shared_memory.h>

#include "memory_block.h"

namespace irp {

	struct VariableType
	{
		enum type
		{
			TYPE_INT8, TYPE_UINT8
			, TYPE_INT16, TYPE_UINT16
			, TYPE_INT32, TYPE_UINT32
			, TYPE_INT64, TYPE_UINT64
			, TYPE_FLOAT, TYPE_DOUBLE
		};
	};

	class VariableManager
	{
	private:
		typedef std::list<SharedMemoryBlock> BlockList;

		struct Info
		{
			BlockList::const_iterator blockItor;
			SharedPtr<char> holder;
			SharedVariable::Header* pHeader;
			std::string type;
		};

		typedef std::map<std::string, Info> VariableMap;

	private:
		static const bcc::uint32_t BLOCK_SIZE = 4 * 1024 * 1024;

	public:
		VariableManager(std::string const& path = std::string());
		~VariableManager();

	public:
		void swap(VariableManager& rhs);

	public:
		std::pair<std::string, bcc::uint64_t> create(std::string const& name, std::string const& type, bcc::uint32_t hintSize = 0);

	private:
		Info secure(std::string const& name, std::string const& type, bcc::uint32_t hintSize = 0);
		bcc::uint32_t estimateSize(std::string const& type, bcc::uint32_t hintSize = 0);

	private:
		void load();
		void save() const;

	private:
		std::string mContextPath;

		BlockList mBlocks;
		VariableMap mVariableMap;
	};

	extern VariableManager gVariableManager;

} // namespace irp

#endif // !def VARIABLE_VARIABLE_MANAGER_H__
