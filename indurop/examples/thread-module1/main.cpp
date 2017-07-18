#include <iostream>
#include <ctime>
#include <cstdio>
#include <sys/types.h>
#include <sys/time.h>

void run(void){
   static struct timeval t = {};
   static struct timeval t0 = {};
   static double gap;
   t0 = t;
   gettimeofday(&t,NULL);
   gap = (double)(t.tv_sec) + (double)(t.tv_usec)/1000000.0 -
      (double)(t0.tv_sec) - (double)(t0.tv_usec)/1000000.0;
   std::cout<< getpid() << ", t: "<< gap <<  std::endl;
}
