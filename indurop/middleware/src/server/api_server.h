
#ifndef SERVER_API_SERVER_H__
#define SERVER_API_SERVER_H__

#include <bicomc/detail/config.h>
#include <bicomc/detail/atomic.h>

#include <string>
#include <map>

#include <indurop/detail/thread.h>
#include <indurop/detail/shared_ptr.h>

#include "api_handler.h"

namespace irp {
	class ApiServer
	{
	public:
		enum { SERVER_READY, SERVER_RUNNING, SERVER_STOPPING };
		typedef std::map<std::string, irp::SharedPtr<ApiHandler> > HandlerMap;

#if defined(_WIN32)
		typedef bcc::uintptr_t SOCKET;
#else
		typedef int SOCKET;
#endif // defined(_WIN32)

	public:
		ApiServer();
		~ApiServer();

	private:
		ApiServer(ApiServer const&) BICOMC_DELETE;
		ApiServer& operator=(ApiServer const&) BICOMC_DELETE;

	public:
		void start(int port = -1);
		void stop();

	public:
		void registerHandler(std::string const& cmd, irp::SharedPtr<ApiHandler> const& handler);
		void unregisterHandler(std::string const& cmd);

	private:
		void run();
		std::string invoke(std::string const& params);
		std::string makeError(char const* message);

	private:
		static void* routine(void* arg);

	private:
		bcc::detail::atomic_intptr_t mState;
		Thread mThread;
		SOCKET mSocket;

		HandlerMap mHandlerMap;
	};
} // namespace irp

#endif // !def SERVER_API_SERVER_H__
