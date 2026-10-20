#include "fps/fps.hpp"
#include "components/fps_convolution_unbalanced.hpp"
#include <array>
#include <cstdlib>
#include <random>
#include <span>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <sys/mman.h>
#include <unistd.h>
static void require(bool ok) { if (!ok) std::abort(); }
using fps_detail::Mint;
namespace unbalanced = fps_detail::multiply_detail::unbalanced;
struct Guarded {
	void* region;
	std::size_t bytes;
	Mint* data;
	Guarded(std::span<const Mint> in, bool tail) {
		const std::size_t page = std::size_t(sysconf(_SC_PAGESIZE));
		const std::size_t body = (in.size_bytes()+page-1)/page*page;
		bytes=body+2*page;
		region=mmap(nullptr,bytes,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		require(region!=MAP_FAILED);
		auto* begin=static_cast<char*>(region)+page;
		require(mprotect(begin,body,PROT_READ|PROT_WRITE)==0);
		data=reinterpret_cast<Mint*>(tail?begin+body-in.size_bytes():begin);
		for(std::size_t i=0;i<in.size();++i) ::new(data+i) Mint(in[i]);
		require(mprotect(begin,body,PROT_READ)==0);
	}
	~Guarded() { require(munmap(region,bytes)==0); }
};
void check(std::span<const Mint> a,std::span<const Mint> b,std::span<const Mint> expected) {
	auto result=unbalanced::multiply(a,b);
	require(std::equal(result.begin(),result.end(),expected.begin(),expected.end()));
	for(auto x:result) require(x.v<Mint::mod());
}

#endif
int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 rng(284517);
	const auto fill=[&](std::vector<Mint>& a) {
		for(auto& x:a) { x=Mint::raw(rng()%Mint::mod()); x.v%=Mint::mod(); if(rng()&1)x.v+=Mint::mod(); }
	};
	for(unsigned m:{33,63,64,65,99,100,127,128,129,999,1000,1024,1025}) {
		const unsigned stride=8*std::bit_ceil(m)-m+1;
		for(unsigned n:{stride-1,stride,stride+1,2*stride-1,2*stride,2*stride+1}) {
			std::vector<Mint> a(n),b(m);fill(a);fill(b);
			const auto expected=fps_detail::multiply_detail::multiply(a,b);
			check(a,b,expected);check(b,a,expected);
			for(bool tail:{false,true}) {
				Guarded ga(a,tail),gb(b,!tail);
				check({ga.data,a.size()},{gb.data,b.size()},expected);
			}
		}
	}
	for(unsigned it=0;it<80;++it) {
		const unsigned n=33+rng()%128,m=33+rng()%128;
		std::vector<Mint> a(n),b(m),expected(n+m-1);fill(a);fill(b);
		for(unsigned i=0;i<n;++i)for(unsigned j=0;j<m;++j)expected[i+j]+=a[i]*b[j];
		check(a,b,expected);
	}
	for(unsigned n:{33,100,1000}) {
		std::vector<Mint> a(n);fill(a);
		const auto expected=fps_detail::multiply_detail::multiply(a,a);
		check(a,a,expected);
		std::fill(a.begin(),a.end(),Mint::word(Mint::mod()));
		check(a,a,std::vector<Mint>(2*n-1));
	}
	for(std::size_t n:{32767,32768,32769,100000,524288})for(std::size_t m:std::array<std::size_t,4>{32,33,n/32,n/32+1}) {
		require(unbalanced::fits(n,m)==(m>32&&n>=32768&&m<=n/32));
		require(unbalanced::fits(m,n)==unbalanced::fits(n,m));
	}
#endif
}
