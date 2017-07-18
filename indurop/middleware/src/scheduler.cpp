
/****************************
 *   스케줄러 클래스 정의     *
 *   @file scheduler.cpp    *
 *   @author Park Donghyeon *
 *   @date   2017-04-20     *
 *   @todo   RT_TASK 전역변수 처리 *
 *           콜백함수를 멤버함수로 변경 가능? *
 *                           *
 ****************************/


#include <iostream>
#include <cstddef>
#include <vector>
#include <dlfcn.h>
#include <csignal>
 #include <native/task.h>
 #include <native/timer.h>
// #include <task.h>
// #include <timer.h>
// #include <utility>
#include <cstdlib>
#include <sys/time.h>
#include <sys/resource.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>

#ifdef DEBUG
#include <fstream>
#include <iomanip>
#endif

#include <Module.hpp>
#include <scheduler.hpp>

using irp::ModuleType;
using irp::ScheduleType;
using irp::ModuleState;
using irp::ModuleAction;

static RT_TASK _scheduler;

#define SEND_SIGNAL_S SIGUSR2
#define SEND_SIGNAL_P SIGUSR1

#ifdef DEBUG
std::string lpath(getenv("INDUROP_PATH"));
std::ofstream gLogFile((lpath+"/middleware/log.txt").c_str());
#endif

/***********************************
 *   시그널 처리 함수                *
 *   @param  sig        시그널 번호  *
 *   @param  info       시그널 정보 구조체 *
 *   @param  arg        커널용 인자  *
 *   @author Park Donghyeon        *
 *   @date   2017-05-21            *
 ***********************************/

static void sigact_handler_periodic(int sig, siginfo_t* info, void *arg){
#ifdef DEBUG
   gLogFile<< "[" << rt_timer_read() <<"] get: "<< info->si_signo << ", from: " << info->si_pid << std::endl;
#endif
   rt_task_resume(&_scheduler);
}

static void sigact_handler_sporadic(int sig, siginfo_t* info, void *arg){
#ifdef DEBUG
   gLogFile<< "[" << rt_timer_read() <<"] get: "<< info->si_signo << ", from: " << info->si_pid << std::endl;
#endif

}

/***********************************************
 *   최대공약수 계산                            *
 *   @param  x          정수1                  *
 *   @param  y          정수2                  *
 *   @return            최대공약수 반환         *
 *                      인자가 0 이하라면 0 반환 *
 *   @author Park Donghyeon                    *
 *   @date   2017-05-01                        *
 ***********************************************/
long long cal_gcd(long long x, long long y){
   long long n = 0;
   if( y <= 0 || x <= 0)
      return 0;

   while(1){
      n = x % y;
      if(n == 0)
         return y;
      x = y;
      y = n;
   }
}


/****************************************
 *   최소공배수 계산                           *
 *   @param  x          정수1             *
 *   @param  y          정수2             *
 *   @return            최소공배수 반환        *
 *                      인자가 0 이하라면 0 반환 *
 *   @author Park Donghyeon             *
 *   @date   2017-05-01                 *
 ****************************************/
long long cal_lcm(long long x, long long y){
   if( y <= 0 || x <= 0)
      return 0;

   return (x*y)/cal_gcd(x,y);
}


int Scheduler::getModuleNumber(){
   return _moduleNumber;
}


/*****************************************************
 *   스케줄러의 basic 주기 설정                        *
 *   @brief basic period는 테이블의 한 행의 실행 주기   *
 *   @author JJunney                           *
 *   @date   2017-05-30                               *
 *   @todo   주기가 0인 모듈이 있으면 항상 0
 *           모듈 갯수가 0이면 return값?            *
 *           setMacro, setBasic 둘 다 같은 문제 있음   *
 *****************************************************/
void Scheduler::setBasicPeriod(){
   long long gcd;
   if (_moduleNumber == 0){
      _basicPeriod = 1;
   } else if (_moduleNumber == 1){
      if (_moduleList[0].getOperationType() == ScheduleType::PERIODIC)
         gcd = _moduleList[0].getPeriod();
      else
         gcd = 1;
   } else{
      for(int i = 0; i<_moduleNumber; ++i){
    	  if(_moduleList[i].getOperationType() == ScheduleType::PERIODIC){
           gcd = _moduleList[i].getPeriod();
           break;
        }
      }
      for(int i=0;i<_moduleNumber; ++i){
    	  if(_moduleList[i].getOperationType()== ScheduleType::PERIODIC)
    		  gcd = cal_gcd(gcd,_moduleList[i].getPeriod());
      }
   }
   _basicPeriod = gcd;
}


/************************************************
 *   스케줄러의 macro 주기 설정                   *
 *   @brief macro period는 테이블 전체 실행 주기  *
 *   @author Sim namwook                          *
 *   @date   2017-05-29                           *
 ************************************************/
void Scheduler::setMacroPeriod(){
   long long lcm;
   if( _moduleNumber == 0){
      lcm = 1000000000;
   } else if( _moduleNumber == 1){
      if (_moduleList[0].getOperationType() == ScheduleType::PERIODIC)
         lcm = _moduleList[0].getPeriod();
      else
         lcm = 1000000000;
   } else {
	   for (int i = 0; i < _moduleNumber; i++){
		   if (_moduleList[i].getOperationType() == ScheduleType::PERIODIC){
			   lcm = _moduleList[i].getPeriod();
			   break;
		   }
	   }
      for(int i = 0; i<_moduleNumber; ++i){
		   if(_moduleList[i].getOperationType() == ScheduleType::PERIODIC)
            lcm = cal_lcm(lcm, _moduleList[i].getPeriod());
      }
   }
   _macroPeriod = lcm;
}


/**********************************
 *   스케줄러 body                 *
 *   @param  arg        사용자 인자 *
 *   @author Park Donghyeon    *
 *   @date   2017-05-21        *
 *   @todo   _pidList가 비어있으면 세그먼트폴트 *
 *           임시 함수들 사용 안하는 방향으로 수정    *
 *           thead의 init, close, error 함수 구현 *
 **********************************/
void Scheduler::scheduler_main(void* arg)
{
	if (arg)
		static_cast<Scheduler*>(arg)->schedule();
}

void Scheduler::schedule()
{
	for (size_t i = 0, size = mTargetModules.size(); i < size; ++i)
	{
		Module* p = mTargetModules[i];
		if (!p)
			continue;

		try
		{
			p->start();
		}
		catch (std::exception const& e)
		{
			std::cerr << "'" << e.what() << "' error occured in starting '";
			std::cerr << p->getName() << "'" << std::endl;
			try { p->destroy(); } catch (...) {}
			mTargetModules[i] = nullptr;
		}
		catch (...)
		{
			std::cerr << "unknown error occured in starting '";
			std::cerr << p->getName() << "'" << std::endl;
			try { p->destroy(); } catch (...) {}
			mTargetModules[i] = nullptr;
		}
	}

	rt_task_sleep(100000000);
	/*
	*   비실시간 모듈 load
	*/
	rt_task_set_periodic(&_scheduler, TM_NOW, _basicPeriod); ///< 충분히 큰 주기가 아니면 오류

	int i, j;
	int n, m;
	int currentTask = 0;
	RTIME t = RTIME(), t0 = RTIME();

	while (!bcc::detail::atomic_load(&mIsRequestedExit))
	{
		for (i = 0, n = _table.size(); i < n; ++i)
		{
			rt_task_wait_period(NULL);

			t0 = t;
			t = rt_timer_read();

#ifdef DEBUG /* DEBUG start */
			gLogFile << "[" << t << "] basicPeriod: " << std::setw(15) << t - t0 << std::endl;
#endif /* DEBUG end */

			for (j = 0, m = _table[i].size(); j < m; ++j)
			{
				currentTask = _table[i][j];
				if (_pidList[currentTask] == -1)
				{
					try
					{
						Module* pModule = mTargetModules[currentTask];
						if (pModule && pModule->state() == ModuleState::EXECUTING)
							pModule->execute(static_cast<bcc::int64_t>(t));
					}
					catch (...)
					{}
				}
				else
				{
					kill(_pidList[currentTask], SEND_SIGNAL_P);
#ifdef DEBUG /* DEBUG start */
					gLogFile << "[" << rt_timer_read() << "] send: " << _pidList[currentTask] << std::endl;
#endif /* DEBUG end */
					rt_task_suspend(&_scheduler);
				}
			}
		} /* end for */
	}	 /* end while */

	for (size_t i = 0, size = mTargetModules.size(); i < size; ++i)
	{
		Module* p = mTargetModules[i];
		if (!p)
			continue;

		try
		{
			p->destroy();
		}
		catch (std::exception const& e)
		{
			std::cerr << "'" << e.what() << "' error occured in destroying '";
			std::cerr << p->getName() << "'" << std::endl;
		}
		catch (...)
		{
			std::cerr << "unknown error occured in destroying '";
			std::cerr << p->getName() << "'" << std::endl;
		}
	}
}

/*****************************
 *   스케줄러 생성자           *
 *   @author  Park Donghyeon *
 *   @data    2017-05-22     *
 *   @todo    환경변수 path를 멤버변수에서 소유? *
 *****************************/
Scheduler::Scheduler()
	: mIsRequestedExit()
{
   _moduleNumber = 0;
   _macroPeriod  = 0;
   _basicPeriod  = 0;

   struct sigaction sigact_periodic;
   sigact_periodic.sa_sigaction = sigact_handler_periodic;
   sigact_periodic.sa_flags = SA_SIGINFO;
   sigemptyset(&sigact_periodic.sa_mask);
   sigaction(SEND_SIGNAL_P, &sigact_periodic, NULL);

   struct sigaction sigact_sporadic;
   sigact_sporadic.sa_sigaction = sigact_handler_sporadic;
   sigact_sporadic.sa_flags = SA_SIGINFO;
   sigemptyset(&sigact_sporadic.sa_mask);
   sigaction(SEND_SIGNAL_S, &sigact_sporadic, NULL);

   rt_task_create(&_scheduler, "MIDDLEWARE_RT_THREAD", 0, 99, T_JOINABLE);
}

Scheduler::~Scheduler(){
	stop();
	rt_task_delete(&_scheduler);

#ifdef DEBUG /* DEBUG start */
   gLogFile.close();
#endif /* DEBUG end */

}


/**********************************
 *   스케줄러에 모듈 삽입           *
 *   @brief  모듈을 moduleList에 삽입하고 load한다. *
 *   @param  module     모듈 객체  *
 *   @return            성공 시 0 *
 *   @author Park Donghyeon       *
 *   @date   2017-05-04           *
 *   @todo  thread타입 모듈의 run함수 외 나머지 함수 구현 예정 *
 **********************************/
int Scheduler::pushModule(Module module){
   _moduleList.push_back(module);
   ++_moduleNumber;

   return 1;
}

/*******************************
 *   list에 담은 모듈을 load한다. *
 *   모듈의 경로는 $INDUROP_PATH/모듈명 *
 *   $INDUROP_PATH = ~/indurop/build *
 *   @return            없음     *
 *   @author Park Donghyeon    *
 *   @date   2017-05-26        *
 *   @todo   모듈경로 환경변수 셋팅방법 문서정리. *
 *           없는 모듈일 경우 에러처리. *
 *                             *
 *******************************/
int Scheduler::loadModule()
{
	for (int i = 0; i < _moduleNumber; ++i)
	{
		Module& module = _moduleList[i];
		std::string const& file = module.getName();

		pid_t child_id;
		void *handle;
		int ret = 0;
		switch (module.getModuleType())
		{
		case ModuleType::THREAD:
			handle = dlopen(file.c_str(), RTLD_NOW);
			if (!handle)
			{
				std::cerr << "dlopen fail:" << dlerror() << std::endl;
				return 1;
			}

			module.setOwner(handle, &dlclose);
			if (void* pFunc = dlsym(handle, "initialize"))
			{
				module.setOnInitialize(reinterpret_cast<Module::OnInitializeCallback>(pFunc));
			}
			else
			{
				module.setOnInitialize(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'initialize' is set dummy." << std::endl;
#endif
			}

			if (void* pFunc = dlsym(handle, "start"))
			{
				module.setOnStart(reinterpret_cast<Module::OnStartCallback>(pFunc));
			}
			else
			{
				module.setOnStart(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'start' is set dummy." << std::endl;
#endif
			}

			if (void* pFunc = dlsym(handle, "run"))
			{
				module.setOnExecute(reinterpret_cast<Module::OnExecuteCallback>(pFunc));
			}
			else
			{
				module.setOnExecute(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'execute' is set dummy." << std::endl;
#endif
			}

			if (void* pFunc = dlsym(handle, "stop"))
			{
				module.setOnStop(reinterpret_cast<Module::OnStopCallback>(pFunc));
			}
			else
			{
				module.setOnStop(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'stop' is set dummy." << std::endl;
#endif
			}

			if (void* pFunc = dlsym(handle, "destroy"))
			{
				module.setOnDestroy(reinterpret_cast<Module::OnDestroyCallback>(pFunc));
			}
			else
			{
				module.setOnDestroy(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'destroy' is set dummy." << std::endl;
#endif
			}

			if (void* pFunc = dlsym(handle, "error"))
			{
				module.setOnError(reinterpret_cast<Module::OnErrorCallback>(pFunc));
			}
			else
			{
				module.setOnError(nullptr);
#ifdef DEBUG
				std::clog << module.getName() << "'s 'error' is set dummy." << std::endl;
#endif
			}

			module.initialize();
			if (module.state() != ModuleState::PAUSED)
			{
				std::cerr << module.getName() << "'s 'initialize' fail : " << module.getName() << std::endl;
				return 1;
			}

			_pidList.push_back(-1);
			mTargetModules.push_back(&module);
			break;

		case ModuleType::PROCESS:
			//file.compare(path + _target); // 타겟비교?
			child_id = fork();
			switch (child_id)
			{
			case -1: //fork error

				break;
			case 0: //child
				ret = execl(file.c_str(), file.c_str(), NULL);
				std::cout << getpid() << " " << std::endl;
				if (ret == -1)
				{
					std::perror(file.c_str());
					exit(EXIT_FAILURE);
				}
				break;
			default: // parent
				_pidList.push_back(child_id);
				mTargetModules.push_back(nullptr);
				break;
			}
			// 모듈의 커널 우선순위를 최상위로 설정한다. 이는 우선순위 구현 시 약간의 조정이 필요하다.
			// (최상)-20 ~ 20(최하) : 스케줄러가 root권한으로 실행되야되.
			//setpriority(PRIO_PROCESS, child_id, -20);
			break;
		case ScheduleType::SPORADIC:
			loadSporadic(module);
			break;
		} /* end swtich */

	} /* end for */
}
/*******************************************
 *   Sporadic 모듈 load                      *
 *   @param  Module       로드 할 Sporadic 모듈 *
 *   @return                               *
 *   @author Park Donghyeon                *
 *   @date   2017-06-23                    *
 *******************************************/

int Scheduler::loadSporadic(Module& module) {
   //file.compare(path + _target); // 타겟비교?

   std::string const& file = module.getName();
   pid_t child_id = fork();
   int ret;
   switch (child_id) {
      case -1: //fork error

         break;
      case 0: //child
         ret = execl(file.c_str(), file.c_str(), NULL);
         std::cout<< getpid() << " " << std::endl;
         if( ret == -1){
            std::perror(file.c_str());
            exit(EXIT_FAILURE);
         }
         break;
      default: // parent
         _pidList.push_back(child_id);
         mTargetModules.push_back(nullptr);
         break;
   }
   // 모듈의 커널 우선순위를 최상위로 설정한다. 이는 우선순위 구현 시 약간의 조정이 필요하다.
   // (최상)-20 ~ 20(최하) : 스케줄러가 root권한으로 실행되야되.
   //setpriority(PRIO_PROCESS, child_id, -20);

   return 0;
}


/**********************************************
 *   스케줄링 테이블 설정                              *
 *   @brief 2차원 배열 테이블. 테이블 값은 module의 index. *
 *   @author Park Donghyeon                   *
 *   @date   2017-06-26                       *
 *   @warning 테이블을 갱신할 때 이전 테이블에
 *            덧붙여질 가능성 있음
 *            _table[i].push_back       *
 **********************************************/
void Scheduler::setTable(){
   setBasicPeriod();
   setMacroPeriod();
   int col = _moduleNumber;
   int row = _macroPeriod / _basicPeriod;
   int i=0,j=0;
   int colEnd = 0;
   _table.resize(row);
   for(i=0; i<row; ++i){
      long long bpi = _basicPeriod * i;
      for(j=0; j<col; ++j){
         Module module = _moduleList[j];
         if (module.getOperationType() == ScheduleType::PERIODIC && bpi % module.getPeriod() == 0) {
            _table[i].push_back(j);
             ++colEnd;
         }
      }
      _table[i].resize(colEnd);
      colEnd = 0;
   }
}

/****************************
 *   스케줄링 테이블 출력. 디버깅 용 *
 *   @author Park Donghyeon *
 *   @date   2017-05-21     *
 ****************************/
void Scheduler::showTable(){
   for(int i=0; i<_table.size(); ++i){
      for(int j=0, m=_table[i].size(); j<m; ++j)
         std::cout<< _table[i][j] << " ";
      std::cout<< std::endl;
   }
}

/*****************************
 *   스케줄링 시작            *
 *   @author Park Donghyeon  *
 *   @date   2017-05-01      *
 *****************************/
void Scheduler::start(){
   loadModule();

   setTable();

   bcc::detail::atomic_store(&mIsRequestedExit, 0);
   rt_task_start(&_scheduler, &Scheduler::scheduler_main, this);
}

/*********************************
 *   스케줄러 실행 모드 설정          *
 *   @param  flag       0:일반 모드  *
 *                      1:디버그 모드 *
 *   @author Park Donghyeon      *
 *   @date   2017-06-11          *
 *********************************/
void Scheduler::setMode(int flag, std::vector<std::string> targetList){
   _mode = flag;  ///< 0:일반모드, 1:디버그모드
}

/***************************************
 *   디버그 타겟 설정                         *
 *   @param  target     디버깅 타겟 모듈 path *
 *   @author Park Donghyeon            *
 *   @date   2017-06-11                *
 ***************************************/

void Scheduler::setTarget(std::string target){
   _target = target;
}

/****************************
 *   스케줄링 정지                *
 *   @author Park Donghyeon *
 *   @date   2017-05-01     *
 *   @todo 구현               *
 ****************************/
void Scheduler::stop(){
	bcc::detail::atomic_store(&mIsRequestedExit, 1);
	rt_task_join(&_scheduler);
}

/************************************
 *   스케줄러 모듈을 리스트로 받는다.  *
 *   @param  v          Module List *
 *   @author Park Donghyeon         *
 *   @date   2017-05-28             *
 ************************************/
void Scheduler::pushModuleList(std::vector<Module> modules){
   for(int i=0, n=modules.size(); i < n ; ++i)
      pushModule(modules[i]);
}
