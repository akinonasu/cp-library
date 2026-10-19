#include "fps/fps.hpp"
#include <cstdio>
#include <random>
int main(){std::mt19937 rng(19512026);unsigned long long hash=0;auto absorb=[&](const fps&f){for(auto x:f)hash=hash*0x9e3779b97f4a7c15ULL+x.val();};absorb(fps{1}.inv(4097));absorb(fps(4097).exp());for(int n:{257,4097,32768,131073,500000}){fps a(n);for(auto&x:a)x=fps_mint::word(2*fps_mint::mod()-1-rng()%1000);a[0]=1;absorb(a.inv());absorb(a.log());absorb(a.sqrt());absorb(a.pow(1000000000000000000LL));a[0]=0;absorb(a.exp());std::fprintf(stderr,"range n=%d done\n",n);}using namespace fps_detail::blocked_poly::detail;std::printf("calls=%llu max_weighted=%u max_unweighted=%u max_b=%u hash=%llu\n",(unsigned long long)leaf_probe_calls,leaf_probe_weighted,leaf_probe_unweighted,leaf_probe_b,hash);}
