#include "fps/fps.hpp"
#include "components/fps_horner.hpp"
#include <chrono>
#include <random>
#include <cstdio>
#include <cstdlib>
using Func = mint(*)(const mint*,std::size_t,mint);
__attribute__((noinline)) mint scalar1(const mint*a,std::size_t n,mint x){mint r=0;while(n)r=r*x+a[--n];return r;}
__attribute__((noinline)) mint scalar4(const mint*a,std::size_t n,mint x){return fps_detail::horner_detail::scalar<4>(a,n,x);}
__attribute__((noinline)) mint scalar8(const mint*a,std::size_t n,mint x){return fps_detail::horner_detail::scalar<8>(a,n,x);}
__attribute__((noinline)) mint vector1(const mint*a,std::size_t n,mint x){return fps_detail::horner_detail::vector<1>(a,n,x);}
__attribute__((noinline)) mint vector2(const mint*a,std::size_t n,mint x){return fps_detail::horner_detail::vector<2>(a,n,x);}
__attribute__((noinline)) mint vector4(const mint*a,std::size_t n,mint x){return fps_detail::horner_detail::vector<4>(a,n,x);}
__attribute__((noinline)) mint adopted(const mint*a,std::size_t n,mint x){return fps_detail::horner(a,n,x);}
int main(int, char**argv){int mode=std::atoi(argv[1]),n=std::atoi(argv[2]);std::mt19937 rng(20261019);fps a(n);for(auto&x:a)x=rng();Func f[]={scalar1,scalar4,scalar8,vector1,vector2,vector4,adopted};int repeat=std::min(4096,std::max(8,1000000/std::max(n,1)));mint sum=0;auto t=std::chrono::steady_clock::now();for(int i=0;i<repeat;++i)sum+=f[mode](a.data(),n,mint(i+17));auto end=std::chrono::steady_clock::now();std::printf("%.9f %u\n",std::chrono::duration<double,std::milli>(end-t).count()/repeat,sum.val());}
