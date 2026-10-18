#include "components/fps_exp_blocked.hpp"
#include <chrono>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

#pragma GCC push_options
#pragma GCC optimize("O3")
#pragma GCC target("avx2")
using namespace fps_detail::blocked_poly;
using namespace fps_detail::blocked_poly::detail;
[[gnu::noinline]] void old_import(const fps_detail::Mint* in, unsigned* out, std::size_t n) {
	const Factor undo(fps_detail::blocked_ntt::detail::power(kR, kP - 2));
	Indices index(0);
	for (std::size_t i = 0; i < n; i += 8, index.next()) {
		Vec value;std::memcpy(&value,in+i,sizeof(value));
		value=reduce(times(value,undo),kP);
		store_unaligned(out+i,canonical(montgomery(value,index.value())));
	}
}
[[gnu::noinline]] void new_import(const fps_detail::Mint* in, unsigned* out, std::size_t n) {
	auto index=_mm256_setr_epi32(0,1,2,3,4,5,6,7);
	for (std::size_t i = 0; i < n; i += 8,index=add(index,broadcast(8))) {
		Vec value;std::memcpy(&value,in+i,sizeof(value));
		store_unaligned(out+i,reduce(montgomery(value,index),kP));
	}
}
#pragma GCC pop_options
int main(int argc,char**argv) {
	if(argc!=2)return 1;
	constexpr std::size_t n=500000,repetitions=64;
	std::mt19937 rng(731);
	std::vector<fps_detail::Mint> in(n);
	std::vector<unsigned> a(n),b(n);
	for(auto&x:in)x=fps_detail::Mint::word(rng()%(2*kP));
	old_import(in.data(),a.data(),n);new_import(in.data(),b.data(),n);
	if(a!=b)return 2;
	auto f=argv[1][0]=='o'?old_import:new_import;
	auto begin=std::chrono::steady_clock::now();
	for(std::size_t i=0;i<repetitions;++i){f(in.data(),a.data(),n);asm volatile(""::"r"(a.data()):"memory");}
	auto end=std::chrono::steady_clock::now();
	unsigned long long checksum=0;for(auto x:a)checksum=checksum*1000003+x;
	std::printf("%.9f %llu\n",std::chrono::duration<double,std::milli>(end-begin).count()/repetitions,checksum);
}
