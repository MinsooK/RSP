/**********************************
 *   @mainpage 실시간 제조로봇 미들웨어 스케줄러 *
 *   @section Program middleware     *
 *   @section intro 소개             *
 *    - 설명 : 스케줄러 개발          *
 *   @section InOut 입력             *
 *    - 필요파일 : ./modules.xml     *
 *   @section CreateInfo 작성 정보   *
 *    - 작성자 : Park Donghyeon      *
 *    - 작성일 : 2017-05-01          *
 *   @file main.cpp main함수         *
 **********************************/

#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <cstring>

#include <scheduler.hpp>
#include <Module.hpp>
#include <xmlParsing.hpp>

#include "server/api_server.h"
#include "server/api_handler.h"

#include "util/environment.h"

int main(int argc, char const* argv[])
{
	irp::Environment::init(argc, argv);

	irp::ApiServer server;
	char const* profilePath = "./modules.xml";
	int flag = 0;
	std::vector<std::string> targetList;

	for (int i = 1; i < argc; ++i)
	{
		if (*argv[i] == '-')
		{
			if (std::strncmp("-D", argv[1], 2) == 0)
				flag = 1; // debug mode
			continue;
		}

		profilePath = argv[i];
		targetList.push_back(argv[i]);
	}

	std::cout<< getpid()<< ": scheduler" << std::endl;
	try
	{
		server.start(7789);

		mlockall(MCL_CURRENT|MCL_FUTURE);

		Scheduler& scheduler = Scheduler::getInstance();
		scheduler.setMode(flag, targetList);
		std::vector<Module> mv = getModuleFromXML(profilePath);
		scheduler.pushModuleList(mv);
		scheduler.setTable();
		scheduler.showTable();
		scheduler.start();

		while (std::cin.get() != 'q');

		scheduler.stop();
	}
	catch (std::exception const& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	catch (...)
	{
		std::cerr << "unknown error" << std::endl;
		return EXIT_FAILURE;
	}

	return 0;
}
