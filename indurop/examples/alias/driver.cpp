
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>

#include <indurop/detail/thread.h>

#include <device.h>

int main()
{
	std::srand(std::time(NULL));
	for (;; irp::this_thread::sleep_for(1000))
	{
		int32_t state = std::rand() % 10;
		float base = std::rand() / 100;
		float data[] = {base + 1, base + 2, base + 3, base + 4};

		test::context.state = state;
		std::copy(data, data + sizeof(data) / sizeof(*data), test::context.data);

		std::time_t time = std::time(NULL);
		std::cout << "Running : ";
		std::cout << std::asctime(std::localtime(&time));
	}
	return 0;
}
