
#include <ctime>
#include <iostream>

#include <indurop/detail/thread.h>

#include <device.h>

int main()
{
	for (;; irp::this_thread::sleep_for(1000))
	{
		if (test::state == 0)
			continue;

		std::time_t time = std::time(NULL);
		std::cout << "Error Detected : ";
		std::cout << std::asctime(std::localtime(&time));
	}
	return 0;
}
