
#include <indurop/initSporad.hpp>

#include <csignal>
#include <unistd.h>
#include <sys/types.h>

#define SEND_SIGNAL_S SIGUSR2

namespace {
   void sigact_handler(int sig, siginfo_t* info, void *arg){
      if (info->si_signo == SIGTERM){

      }
   }
}

void irp::initSporadExe(){
   static struct sigaction act;
   //struct sigset_t set;

   act.sa_sigaction = sigact_handler;
   act.sa_flags = SA_SIGINFO;
   sigemptyset(&act.sa_mask);
   sigaction(SEND_SIGNAL_S, &act, NULL);
}

void irp::raiseSignal(){
   static pid_t schedulerPID = getppid();

   kill(schedulerPID, SEND_SIGNAL_S);
}
