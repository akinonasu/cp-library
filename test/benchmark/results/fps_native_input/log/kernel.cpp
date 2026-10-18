#include "fps/fps.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
int main(int argc,char**argv){int n=argc>1?std::atoi(argv[1]):500000;std::mt19937 rng(20261019);fps a(n);for(auto&x:a)x=rng();a[0]=1;auto t=std::chrono::steady_clock::now();fps r=a.log();auto e=std::chrono::steady_clock::now();unsigned long long h=0;for(auto x:r)h=h*0x9e3779b97f4a7c15ULL+x.val();std::printf("%.9f %llu\n",std::chrono::duration<double,std::milli>(e-t).count(),h);}
