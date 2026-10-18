#include "components/fps_inverse_blocked.hpp"
#include <chrono>
#include <cstdio>
#include <random>
int main(int argc,char**argv){std::size_t n=argc>1?std::atoi(argv[1]):500000;unsigned scale=argc>2?std::strtoul(argv[2],nullptr,10):932051910;std::mt19937 rng(20261019);std::vector<unsigned>a(n);for(auto&x:a)x=rng()%fps_detail::Mint::mod();std::vector<fps_detail::Mint>out;out.reserve(n);auto begin=std::chrono::steady_clock::now();for(int trial=0;trial<32;++trial){out.clear();fps_detail::blocked_poly::append_scaled(a,out,scale);}auto end=std::chrono::steady_clock::now();unsigned long long h=0;for(auto x:out)h=h*0x9e3779b97f4a7c15ULL+x.val();std::printf("%.9f %llu\n",std::chrono::duration<double,std::milli>(end-begin).count()/32,h);}
