
#include <indurop/api_client.h>

#include <cerrno>
#include <cstring>
#include <stdexcept>

#if defined(_WIN32)
// Windows
#include <Windows.h>

#else
// POSIX
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#endif // defined(_WIN32)


#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <bicomc/object.h>

#if defined(_WIN32)
// Windows
namespace {
	typedef bcc::detail::FixedSizeInt<sizeof(std::size_t)>::signed_type ssize_t;

	std::string getErrorMessage()
	{
		DWORD error = ::WSAGetLastError();
		char message[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error
			, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)
			, message, sizeof(message), NULL);
		return message;
	}

	struct SocketInit
	{
		SocketInit()
		{
			if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
				throw std::runtime_error(getErrorMessage());
		}

		~SocketInit()
		{
			::WSACleanup();
		}

		WSADATA wsaData;
	};

	int close(SOCKET socket)
	{
		return ::closesocket(socket);
	}

	int write(SOCKET socket, void const* buf, size_t count)
	{
		return ::send(socket, static_cast<char const*>(buf), count, 0);
	}

	int read(SOCKET socket, void* buf, size_t count)
	{
		return ::recv(socket, static_cast<char*>(buf), count, 0);
	}
}
#else
// POSIX
namespace {
	typedef int SOCKET;

	struct SocketInit
	{};

	std::string getErrorMessage()
	{
		return std::strerror(errno);
	}
}

#endif // defined(_WIN32)

using irp::ApiClient;

std::pair<std::string, bcc::uint64_t> ApiClient::bind(std::string const& name, std::string const& type, bcc::uint32_t hint)
{
	std::string command;
	{
		rapidjson::Document doc;
		doc.SetObject();
		doc.AddMember("cmd", "bind", doc.GetAllocator());
		doc.AddMember("name", name, doc.GetAllocator());
		doc.AddMember("type", type, doc.GetAllocator());
		doc.AddMember("size", hint, doc.GetAllocator());

		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);
		command =  buffer.GetString();
	}

	std::string response = request(command);
	rapidjson::Document doc;
	doc.Parse(response.c_str());

	if (doc.GetParseError() != rapidjson::kParseErrorNone)
		throw std::runtime_error(std::string("Cannot bind a variable '").append(name).append("'"));

	if (doc.HasMember("error"))
		throw std::runtime_error(doc["error"].GetString());

	if (!doc.HasMember("key"))
		throw std::runtime_error("'key' cannot be found.");

	if (!doc.HasMember("size") || !doc["size"].IsUint64())
		throw std::runtime_error("'size' cannot be found.");

	rapidjson::GenericValue<rapidjson::UTF8<> >& key = doc["key"];
	rapidjson::GenericValue<rapidjson::UTF8<> >& size = doc["size"];

	return std::make_pair(doc["key"].GetString(), doc["size"].GetUint64());
}

bool ApiClient::unbind(std::string const& name)
{
	std::string command("{");
	command.append("\"cmd\":").append(1, '"').append("unbind").append(1, '"');
	command.append(", \"name\":").append(1, '"').append(name).append(1, '"');
	command.append("}");

	std::string response = request(command);
	rapidjson::Document doc;
	doc.Parse(response.c_str());

	if (doc.GetParseError() != rapidjson::kParseErrorNone)
		throw std::runtime_error(std::string("Cannot bind a variable '").append(name).append("'"));

	if (doc.HasMember("error"))
		throw std::runtime_error(doc["error"].GetString());

	return true;
}

std::string ApiClient::request(std::string const& params)
{
	static SocketInit volatile socketInit______;

	SOCKET socket = ::socket(PF_INET, SOCK_STREAM, 0);
	if (socket == SOCKET(-1))
		throw std::runtime_error(getErrorMessage());

	::sockaddr_in address = {};
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = htons(7789);

	if (::connect(socket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0)
	{
		std::runtime_error exception(getErrorMessage());
		close(socket);
		throw exception;
	}

	for (size_t size = 0, max = params.size() + 1; size < max; )
	{
		ssize_t written = ::write(socket, params.c_str() + size, max - size);
		if (written <= 0)
			break;
		size += size_t(written);
	}

	std::string result;
	for (char buffer[1024];;)
	{
		ssize_t readSize = ::read(socket, buffer, sizeof(buffer));
		if (readSize <= 0)
			break;
		result.append(buffer, readSize);
	}

	::close(socket);

	return result;
}
