
#include "api_handler.h"

#include "api_server.h"
#include "../variable/variable_manager.h"

using irp::SharedPtr;
using irp::ApiHandler;

namespace {
	template<typename Function>
	struct ApiFunction : public ApiHandler
	{
		ApiFunction(Function const& f) : func(f) {}

		void process(rapidjson::Document const& resuest, rapidjson::Document& response)
		{
			func(resuest, response);
		}

		Function func;
	};

	template<typename Function>
	SharedPtr<ApiHandler> makeHandler(Function const& f)
	{
		return SharedPtr<ApiHandler>(new ApiFunction<Function>(f));
	}
}

namespace {
	void bindVariable(rapidjson::Document const& req, rapidjson::Document& res)
	{
		char const* name = req.HasMember("name") && req["name"].IsString() ? req["name"].GetString() : "";
		char const* type = req.HasMember("type") && req["type"].IsString() ? req["type"].GetString() : "";
		bcc::uint32_t size = req.HasMember("size") && req["size"].IsUint() ? req["size"].GetUint() : 0;

		if (*name == '\0' || *type == '\0')
			throw std::invalid_argument("'name' and 'type' must not be empty.");

		std::pair<std::string, bcc::uint64_t> result = irp::gVariableManager.create(name, type, size);
		res.SetObject();

		res.AddMember("key", result.first, res.GetAllocator());
		res.AddMember("size", result.second, res.GetAllocator());
		res.AddMember("os"
#if defined(_WIN32)
			, "windows"
#else
			, "posix"
#endif // defined(_WIN32)
			, res.GetAllocator()
		);
	}

	void unbindVariable(rapidjson::Document const& resuest, rapidjson::Document& response)
	{

	}
}



void irp::listUpApiHandler(ApiServer& server)
{
	using namespace rapidjson;

	server.registerHandler("bind", makeHandler(&bindVariable));
	server.registerHandler("unbind", makeHandler(&unbindVariable));
}
