#include <iostream>
#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include <indurop/indurop.h>

int main(int argc, char* argv[]){
   irp::initPeriodExe();
   struct timeval t, t0;
   double gap;

   //freopen("/dev/pts/2", "w", stdout);

   while(1){
      irp::waitPeriod();
      t0 = t;
      gettimeofday(&t,NULL);
      gap = (double)(t.tv_sec) + (double)(t.tv_usec)/1000000.0 -
         (double)(t0.tv_sec) - (double)(t0.tv_usec)/1000000.0;
      std::cout<< getpid() << ", t: "<< gap <<  std::endl;
   }

   return 0;
}
