
#include <indurop/initPeriod.hpp>

#include <csignal>
#include <unistd.h>
#include <sys/types.h>

#define SEND_SIGNAL_P SIGUSR1

namespace {
   void sigact_handler(int sig, siginfo_t* info, void *arg){
      if (info->si_signo == SIGTERM){

      }
   }
}

void irp::initPeriodExe(){
   static struct sigaction act;
   //struct sigset_t set;

   act.sa_sigaction = sigact_handler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&act.sa_mask);
   sigaction(SEND_SIGNAL_P,&act, NULL);
}

void irp::waitPeriod(){
   static pid_t schedulerPID = getppid();
   static int flag = 0;
   if( flag == 0){
      flag = 1;
   } else{
      kill(schedulerPID, SEND_SIGNAL_P);
   }
   pause();
}
