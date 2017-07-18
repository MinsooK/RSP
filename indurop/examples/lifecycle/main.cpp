
#include <iostream>

#include <indurop/property.h>

namespace {
	std::string message;
}

extern "C" void initialize(irp::Property const& property)
{
	std::cout << "initialize() in " << __FILE__ <<std::endl;
	message = property.get("message", __FILE__);
}

extern "C" void start()
{
	std::cout << "start() in " << __FILE__ <<std::endl;
}

extern "C" void run()
{
	static size_t i = 0;
	std::cout << "run() " << ++i << " : " << message <<std::endl;
}

extern "C" void stop()
{
	std::cout << "stop() in " << __FILE__ <<std::endl;
}

extern "C" void destroy()
{
	std::cout << "destroy() in " << __FILE__ <<std::endl;
}
