
#include "api_server.h"

#include <cerrno>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <map>
#include <sstream>

#if defined(_WIN32)
#include <Windows.h>

#else
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#endif // defined(_WIN32)

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using irp::ApiServer;
using irp::Thread;
using irp::ApiHandler;

#if defined(_WIN32)
namespace {
	typedef int socklen_t;

	struct WinSocketInit
	{
		WinSocketInit()
		{
			if (WSAStartup(MAKEWORD(2, 2), &wsaData) < 0)
			{
				DWORD error = GetLastError();
				char message[256];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
					, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
					, message, sizeof(message), NULL);
				throw std::runtime_error(message);
			}
		}

		~WinSocketInit()
		{
			WSACleanup();
		}

		WSADATA wsaData;
	} gInitiator;

	int close(ApiServer::SOCKET socket)
	{
		return closesocket(socket);
	}

	int read(ApiServer::SOCKET socket, void* buf, std::size_t size)
	{
		return ::recv(socket, static_cast<char*>(buf), size, 0);
	}

	int write(ApiServer::SOCKET socket, void const* buf, std::size_t size)
	{
		return ::send(socket, static_cast<char const*>(buf), size, 0);
	}
}

#endif // defined(_WIN32)

ApiServer::ApiServer()
	: mState(SERVER_READY), mThread(), mSocket(-1)
{
	irp::listUpApiHandler(*this);
}

ApiServer::~ApiServer()
{
	stop();
}

void ApiServer::start(int port)
{
	if (bcc::detail::atomic_load(&mState) != SERVER_READY)
		throw std::runtime_error("API server is already running.");

	mSocket = ::socket(PF_INET, SOCK_STREAM, 0);
	if (mSocket == SOCKET(-1))
		throw std::runtime_error(std::strerror(errno));

	int isReuseMode = 1;
	::setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&isReuseMode), sizeof(isReuseMode));

	::sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (0 < port && port <= uint16_t(-1))
		address.sin_port = htons(static_cast<bcc::uint16_t>(port));

	if (::bind(mSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		std::runtime_error exception(std::strerror(errno));
		::close(mSocket);
		mSocket = -1;
		throw exception;
	}

	Thread thread(&ApiServer::routine, this);
	mThread.swap(thread);
}

void ApiServer::stop()
{
	if (bcc::detail::atomic_load(&mState) != SERVER_RUNNING) return;

	bcc::detail::atomic_store(&mState, SERVER_STOPPING);
	mThread.join();
	::close(mSocket);
	bcc::detail::atomic_store(&mState, SERVER_READY);
}

void ApiServer::registerHandler(std::string const& cmd, irp::SharedPtr<ApiHandler> const& handler)
{
	if (bcc::detail::atomic_load(&mState) != SERVER_READY)
		throw std::runtime_error("cannot register API handler in state of running.");

	mHandlerMap[cmd] = handler;
}

void ApiServer::unregisterHandler(std::string const& cmd)
{
	if (bcc::detail::atomic_load(&mState) != SERVER_READY)
		throw std::runtime_error("cannot unregister API handler in state of running.");

	HandlerMap::iterator itor = mHandlerMap.find(cmd);
	if (itor != mHandlerMap.end())
		mHandlerMap.erase(itor);
}

void ApiServer::run()
{
	if (bcc::detail::atomic_exchange(&mState, SERVER_RUNNING) != SERVER_READY)
	{
		stop();
		return;
	}

	if (::listen(mSocket, 10) < 0)
		throw std::runtime_error(std::strerror(errno));

	::timeval timeout;
	fd_set readMask = {};
	FD_SET(mSocket, &readMask);
	int maxSocket = mSocket;
	std::map<int, std::string> requests;

	while (bcc::detail::atomic_load(&mState) == SERVER_RUNNING)
	{
		fd_set readSet = readMask;

		timeout.tv_sec = 1;
		timeout.tv_usec = 0;

		int eventCount = ::select(maxSocket + 1, &readSet, nullptr, nullptr, &timeout);

		for (int socket = 0, end = maxSocket + 1; eventCount != 0 && socket < end; ++socket)
		{
			if (!FD_ISSET(socket, &readSet))
				continue;

			--eventCount;

			if (socket == mSocket)
			{
				::sockaddr_in address;
				socklen_t addrlen = static_cast<socklen_t>(sizeof(address));
				int client = ::accept(socket, reinterpret_cast<sockaddr*>(&address), &addrlen);

				if (client < 0)
				{
					std::cerr << std::strerror(errno) << std::endl;
					continue;
				}

				FD_SET(client, &readMask);
				maxSocket = (std::max)(maxSocket, client);
			}
			else
			{
				char buffer[1024] = {};
				bcc::intptr_t readSize = ::read(socket, buffer, sizeof(buffer));

				if (readSize <= 0)
				{
					FD_CLR(socket, &readMask);
					if (maxSocket == socket) --maxSocket;
					::close(socket);

					std::map<int, std::string>::iterator itor = requests.find(socket);
					if (itor != requests.end())
						requests.erase(itor);
					continue;
				}

				std::string& request = requests[socket];
				size_t prevSize = request.size();

				request.append(buffer);

				if (request.size() < prevSize + readSize)
				{
					std::string response;
					try
					{
						response = invoke(request);
					}
					catch (std::exception const& e)
					{
						response = makeError(e.what());
					}
					catch (...)
					{
						response = makeError("unknown error");
					}

					for (std::size_t size = 0, max = response.size() + 1; size < max; )
					{
						bcc::intptr_t written = ::write(socket, response.c_str() + size, max - size);
						if (written <= 0)
							break;
						size += std::size_t(written);
					}

					FD_CLR(socket, &readMask);
					if (maxSocket == socket) --maxSocket;
					::close(socket);

					std::map<int, std::string>::iterator itor = requests.find(socket);
					if (itor != requests.end())
						requests.erase(itor);
				}
			}
		}
	}
}

std::string ApiServer::invoke(std::string const& params)
{
	using rapidjson::Document;

	Document doc;
	doc.Parse(params.c_str());

	if (doc.GetParseError() != rapidjson::kParseErrorNone)
		return makeError("invalid request");

	Document::ConstMemberIterator cmdItor = doc.FindMember("cmd");
	if (cmdItor == doc.MemberEnd() || !cmdItor->value.IsString())
		return makeError("'cmd' cannot be found.");

	char const* cmd = cmdItor->value.GetString();
	HandlerMap::iterator itor = mHandlerMap.find(cmd);
	if (itor == mHandlerMap.end())
		return makeError("'cmd' is not supported.");

	try
	{
		Document response;
		itor->second->process(doc, response);

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		response.Accept(writer);
		return buffer.GetString();
	}
	catch (std::exception const& e)
	{
		return makeError(e.what());
	}
	catch (...)
	{
		return makeError("unknown error");
	}
}

std::string ApiServer::makeError(char const* message)
{
	return std::string("{\"error\":\"").append(message).append("\"}");
}

void* ApiServer::routine(void* arg)
{
	try
	{
		static_cast<ApiServer*>(arg)->run();
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
		static_cast<ApiServer*>(arg)->stop();
	}
	catch (...)
	{
		std::cerr << "unknown error" << std::endl;
		static_cast<ApiServer*>(arg)->stop();
	}

	return nullptr;
}
