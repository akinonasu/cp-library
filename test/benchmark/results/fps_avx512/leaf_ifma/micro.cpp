#include "leaf_ifma.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string_view>
#include <vector>
using namespace leaf_ifma_probe;
struct alignas(64) Item{Window w;alignas(32)unsigned b[8];};
[[gnu::target("avx2"),gnu::noinline]] unsigned long long run_base(const Item* a,std::size_t mask,std::size_t loops){Vec acc=_mm256_setzero_si256();for(std::size_t i=0;i<loops;++i)acc=_mm256_add_epi32(acc,leaf_product(a[i&mask].w,a[i&mask].b));alignas(32)unsigned v[8];store(v,acc);unsigned long long h=0;for(auto x:v)h=h*0x9e3779b97f4a7c15ULL+x;return h;}
template<int Groups>[[gnu::target("avx512f,avx512ifma"),gnu::noinline]] unsigned long long run_inline(const Item* a,std::size_t mask,std::size_t loops){Vec acc=_mm256_setzero_si256();for(std::size_t i=0;i<loops;++i)acc=_mm256_add_epi32(acc,core<Groups>(a[i&mask].w,a[i&mask].b));alignas(32)unsigned v[8];store(v,acc);unsigned long long h=0;for(auto x:v)h=h*0x9e3779b97f4a7c15ULL+x;return h;}
template<int mode>[[gnu::target("avx2"),gnu::noinline]] unsigned long long run_calls(const Item* a,std::size_t mask,std::size_t loops){Vec acc=_mm256_setzero_si256();for(std::size_t i=0;i<loops;++i){const auto&x=a[i&mask];Vec y;if constexpr(mode==0)y=original_call(x.w,x.b);else if constexpr(mode==1)y=call(x.w,x.b);else if constexpr(mode==2)y=dispatch(x.w,x.b);else y=forced_fallback(x.w,x.b);acc=_mm256_add_epi32(acc,y);}alignas(32)unsigned v[8];store(v,acc);unsigned long long h=0;for(auto x:v)h=h*0x9e3779b97f4a7c15ULL+x;return h;}
int main(int argc,char**argv){std::string_view mode=argc>1?argv[1]:"base";std::size_t count=argc>2?std::strtoull(argv[2],nullptr,10):128,loops=argc>3?std::strtoull(argv[3],nullptr,10):1048576;std::vector<Item>a(count);std::mt19937 rng(19512026);for(auto&x:a){for(int i=0;i<16;++i)x.w.word[i]=rng()%kP;for(auto&b:x.b)b=rng()%kP;}auto t=std::chrono::steady_clock::now();unsigned long long h;if(mode=="base")h=run_base(a.data(),count-1,loops);else if(mode=="ifma_inline")h=run_inline<4>(a.data(),count-1,loops);else if(mode=="ifma_inline_g1")h=run_inline<1>(a.data(),count-1,loops);else if(mode=="ifma_inline_g2")h=run_inline<2>(a.data(),count-1,loops);else if(mode=="base_call")h=run_calls<0>(a.data(),count-1,loops);else if(mode=="ifma_call")h=run_calls<1>(a.data(),count-1,loops);else if(mode=="dispatch")h=run_calls<2>(a.data(),count-1,loops);else h=run_calls<3>(a.data(),count-1,loops);auto e=std::chrono::steady_clock::now();std::printf("%.9f %llu\n",std::chrono::duration<double,std::nano>(e-t).count()/loops,h);}
