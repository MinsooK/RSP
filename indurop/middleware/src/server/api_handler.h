
#ifndef SERVER_API_HANDLER_H__
#define SERVER_API_HANDLER_H__

#include <functional>

#include <rapidjson/document.h>

namespace irp {
	class ApiHandler
	{
	public:
		~ApiHandler() {}

	public:
		virtual void process(rapidjson::Document const& resuest, rapidjson::Document& response) {}
	};

	class ApiServer;

	void listUpApiHandler(ApiServer& server);
} // namespace irp

#endif // !def SERVER_API_HANDLER_H__
