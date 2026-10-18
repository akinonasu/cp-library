#include <chrono>
#include <cstdio>
#include <random>
#include <vector>
int main(int argc,char**argv){const std::size_t n=argc>1?std::atoi(argv[1]):500000;std::mt19937 rng(20261019);std::vector<std::uint32_t> f(n);for(auto&x:f)x=rng()%poly::kModulus;f[0]=1;auto begin=std::chrono::steady_clock::now();poly::Arena arena(poly::Transform::words(poly::power_log(n))+poly::Arena::footprint(n)+poly::Arena::footprint(poly::power_scratch(n)));const poly::Transform transform(arena,poly::power_log(n));auto g=arena.take(n);poly::power(transform,f,1000000000000000000ULL%poly::kModulus,1,g,arena.take(poly::power_scratch(n)));auto end=std::chrono::steady_clock::now();unsigned long long h=0;for(auto x:g)h=h*0x9e3779b97f4a7c15ULL+x;std::printf("%.9f %llu\n",std::chrono::duration<double,std::milli>(end-begin).count(),h);}
