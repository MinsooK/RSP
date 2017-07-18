
/**
 *   스케줄러 클래스 선언
 *   @file scheduler.h
 *   @author Park Donghyeon
 *   @date   2017-04-20
 */
//#define DEBUG

#include <iostream>
#include <unistd.h>
// #include <cstdint> // -std=c++11
#include <vector>
#include <list>
#include <csignal>
#include <task.h> ///< xenomai-3.x.x library
#include <timer.h>
#include <utility>

#ifdef DEBUG
#include <fstream>
#endif

#include <bicomc/detail/atomic.h>

class Module;

/****************************
 *   @class Scheduler       *
 *   @author Park Donghyeon *
 *   @date   2017-05-01     *
 ****************************/

class Scheduler{
// public:
//    enum class MODE: uint8_t{
//       NORMAL = 0x00;
//       DEBUG = 0x01;
//    }

private:
   //RT_TASK _scheduler;

   // MODE _mode;   ///< 1:디버그 모드, 0:일반모드
   int _mode;   ///< 1:디버그 모드, 0:일반모드
   std::string _target;
   int _moduleNumber;
   std::vector<pid_t> _pidList;  ///< thread module pid = -1
   std::list<pid_t> _eventList;
   std::vector<Module> _moduleList;
   long long _macroPeriod; ///< LCM period of modules
   long long _basicPeriod; ///< GCD period of modules
   std::vector<std::vector<int> > _table;  ///< scheduling table
   std::vector<Module*> mTargetModules;   ///< 'thread module index':'run function ptr'

   // static void signal_handler(int signum); ///<교체예정
   // static void scheduler_main(void *arg);  ///<교체예정
   int loadModule();
   int loadPeriodic(Module& module);
   int loadSporadic(Module& module);
   Scheduler();
   ~Scheduler();

public:
   static Scheduler& getInstance(){
      static Scheduler instance;
      return instance;
   }
   /*반환 값
   0:오류없음
   -1:process파일type 실행파일 아니거나 경로틀림
   -2:thread 실행파일 아니거나 경로틀림
   -3 fork 오류
   */
   void pushModuleList(std::vector<Module> modules);
   int getModuleNumber();
   void setBasicPeriod();
   void setMacroPeriod();
   /*임시 함수*/
   long long getBasicPeriod(){return _basicPeriod;}
   std::vector<std::vector<int> > getTable(){return _table;}
   std::vector<Module*> const& getpRunList() const { return mTargetModules; }
   std::vector<pid_t> getpidList(){return _pidList;}
   /*임시 함수*/
   void setMode(int flag, std::vector<std::string> targetList);
   void setTarget(std::string target);
   void setTable();
   int pushModule(Module module);
   void showTable();
   void start();
   void stop();

private:
	void schedule();
	static void scheduler_main(void* arg);

private:
	bcc::detail::atomic_intptr_t mIsRequestedExit;
};
