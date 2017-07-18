
#include <iostream>

#include <hello.h>

int main()
{
	bcc::int64_t time = test::time; 
	std::cout << "Hello world! : " << time << std::endl; 
	test::time = time + 1; 
	return 0;
}
