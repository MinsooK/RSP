#include <iostream>
#include <ctime>
#include <cstdio>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstdlib>

#include <indurop/indurop.h>

int main(int argc, char* argv[]){
   irp::initSporadExe();
   struct timeval t = {}, t0 = {};
   double gap;

   srand( (unsigned int)(time(NULL)) );
   int r;

   freopen("/dev/pts/1", "w", stdout);

   usleep(3000000);
   while(1){
      irp::raiseSignal();
      r = 1000000 + (rand() % 3)*1000000;
      usleep(r);
      t0 = t;
      gettimeofday(&t,NULL);
      gap = (double)(t.tv_sec) + (double)(t.tv_usec)/1000000.0 -
         (double)(t0.tv_sec) - (double)(t0.tv_usec)/1000000.0;
      std::cout<< getpid() << ", t: "<< gap << " sporadic"<< std::endl;

   }

   return 0;
}
