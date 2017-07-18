
#ifndef INDUROP_API_CLIENT_H__
#define INDUROP_API_CLIENT_H__

#include <string>
#include <utility>

#include <bicomc/stdint.h>

namespace irp {

	class ApiClient
	{
	public:
		static std::pair<std::string, bcc::uint64_t> bind(std::string const& name, std::string const& type, bcc::uint32_t hint = 0);
		static bool unbind(std::string const& name);

		static std::string request(std::string const& params);
	};

} // namespace irp

#endif // !def INDUROP_API_CLIENT_H__
