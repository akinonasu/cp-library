#include "components/fps_blocked.hpp"
#include <chrono>
#include <cstdio>
#include <random>
#include <array>
using mint;
using fps_detail::blocked_poly::detail::Vec;
using namespace fps_detail::blocked_poly;
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
template<bool montgomery> __attribute__((noinline)) std::vector<Mint> scalar(const unsigned* in, std::size_t n) {
	std::vector<Mint> out;out.reserve(n);
	for(std::size_t i=0;i<n;++i)out.emplace_back(montgomery?Mint::word(in[i]):Mint::raw(in[i]));
	return out;
}
template<bool montgomery> __attribute__((noinline)) std::vector<Mint> resize(const unsigned* in, std::size_t n) {
	std::vector<Mint> out;out.resize(n);
	if constexpr(montgomery)std::memcpy(static_cast<void *>(out.data()),in,n*sizeof(unsigned));
	else {
		const Vec r=_mm256_set1_epi32(932051910u);std::size_t i=0;
		for(;i+8<=n;i+=8){const Vec v=detail::montgomery(detail::load_unaligned(in+i),r);std::memcpy(static_cast<void *>(out.data()+i),&v,32);}
		for(;i<n;++i)out[i]=Mint::raw(in[i]);
	}
	return out;
}
template<int block,bool montgomery> __attribute__((noinline)) std::vector<Mint> insert(const unsigned* in, std::size_t n) {
	std::vector<Mint> out;out.reserve(n);
	const Vec r=_mm256_set1_epi32(932051910u);
	std::array<Mint,block> batch;
	std::size_t i=0;
	for(;i+block<=n;i+=block){
		if constexpr(montgomery)std::memcpy(static_cast<void *>(batch.data()),in+i,block*sizeof(unsigned));
		else for(int j=0;j<block;j+=8){const Vec v=detail::montgomery(detail::load_unaligned(in+i+j),r);std::memcpy(static_cast<void *>(batch.data()+j),&v,32);}
		out.insert(out.end(),batch.begin(),batch.end());
	}
	for(;i<n;++i)out.emplace_back(montgomery?Mint::word(in[i]):Mint::raw(in[i]));
	return out;
}
template<bool montgomery> __attribute__((noinline)) std::vector<Mint> helper(const unsigned* in, std::size_t n) {
	std::vector<Mint> out;
	fps_detail::append_fps_words<montgomery>(out, in, n);
	return out;
}
#pragma GCC pop_options
int main(){
	static_assert(sizeof(Mint)==sizeof(unsigned)&&std::is_trivially_copyable_v<Mint>);
	constexpr int n=500000;std::vector<unsigned> in(n);std::mt19937 rng(16);for(auto&x:in)x=rng()%998244353;
	using Fn=std::vector<Mint>(*)(const unsigned*,std::size_t);
	const Fn funcs[]={scalar<true>,resize<true>,insert<8,true>,insert<16,true>,insert<32,true>,insert<64,true>,insert<128,true>,scalar<false>,resize<false>,insert<8,false>,insert<16,false>,insert<32,false>,insert<64,false>,insert<128,false>,helper<true>,helper<false>};
	const char* names[]={"scalar_mont","resize_mont","block8_mont","block16_mont","block32_mont","block64_mont","block128_mont","scalar_raw","resize_raw","block8_raw","block16_raw","block32_raw","block64_raw","block128_raw","helper_mont","helper_raw"};
	std::vector<int> order(16);for(int i=0;i<16;++i)order[i]=i;
	for(int trial=0;trial<42;++trial){std::shuffle(order.begin(),order.end(),rng);
		for(int at:order){auto start=std::chrono::steady_clock::now();auto v=funcs[at](in.data(),n);auto stop=std::chrono::steady_clock::now();unsigned long long sum=0;for(auto x:v)sum=(sum^x.val())*0x9e3779b97f4a7c15ULL;
			if(trial>0)std::printf("%s %.9f %llu\n",names[at],std::chrono::duration<double,std::milli>(stop-start).count(),sum);
		}
	}
}
