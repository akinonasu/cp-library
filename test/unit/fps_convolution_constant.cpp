#include "components/fps_convolution.hpp"
#include "convolution/convolution.hpp"
#include <algorithm>
#include <cstdlib>
#include <random>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <sys/mman.h>
#include <unistd.h>
using mint;
namespace constant = fps_detail::multiply_detail::constant;
static void require(bool ok) { if (!ok) std::abort(); }
static void protect(const std::vector<Mint>& a,int mode) {
	const auto page=std::uintptr_t(sysconf(_SC_PAGESIZE)),first=reinterpret_cast<std::uintptr_t>(a.data()),last=first+a.size()*sizeof(Mint);
	const auto lo=(first+page-1)&~(page-1),hi=last&~(page-1);
	if(lo<hi)require(mprotect(reinterpret_cast<void*>(lo),hi-lo,mode)==0);
}
static std::vector<Mint> make(std::size_t n,unsigned v) {
	std::vector<Mint> a(n);unsigned raw=Mint::raw(v).v%Mint::mod();
	for(std::size_t i=0;i<n;++i)a[i]=Mint::word(raw+(i%2?Mint::mod():0));
	return a;
}
static void check(const std::vector<Mint>& a,const std::vector<Mint>& b) {
	const auto sa=a,sb=b;protect(a,PROT_READ);protect(b,PROT_READ);
	auto out=constant::multiply(a,b);const auto factor=a[0]*b[0];
	for(std::size_t i=0;i<out.size();++i) {
		const auto count=std::min({i+1,a.size(),b.size(),out.size()-i});
		require(out[i]==factor*Mint(count)&&out[i].v<Mint::mod());
	}
	for(std::size_t i=0;i<a.size();++i)require(a[i].v==sa[i].v);
	for(std::size_t i=0;i<b.size();++i)require(b[i].v==sb[i].v);
	protect(a,PROT_READ|PROT_WRITE);protect(b,PROT_READ|PROT_WRITE);
}
static void check_public(const std::vector<Mint>& a,const std::vector<Mint>& b) {
	const auto expected=fps_detail::multiply_detail::run<fps_detail::multiply_detail::ntt::Convolution>(a,b);
	protect(a,PROT_READ);protect(b,PROT_READ);
	const auto direct=fps_detail::multiply_detail::multiply(a,b);
	require(direct==expected);
	const auto convolved=convolution(a,b);
	require(convolved==expected&&convolution(b,a)==expected);
	const fps x(a),y(b);
	const auto product=fps::multiply(x,y);
	require(product==expected);
	for(auto v:direct)require(v.v<Mint::mod());
	for(auto v:convolved)require(v.v<Mint::mod());
	for(auto v:product)require(v.v<Mint::mod());
	protect(a,PROT_READ|PROT_WRITE);protect(b,PROT_READ|PROT_WRITE);
}
struct Guarded {
	void* region;
	std::size_t bytes;
	Mint* data;
	Guarded(std::span<const Mint> values,bool tail) {
		const auto page=std::size_t(sysconf(_SC_PAGESIZE));
		const auto readable=(values.size_bytes()+page-1)/page*page;
		bytes=readable+2*page;
		region=mmap(nullptr,bytes,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);require(region!=MAP_FAILED);
		auto* begin=static_cast<char*>(region)+page;
		require(mprotect(begin,readable,PROT_READ|PROT_WRITE)==0);
		data=reinterpret_cast<Mint*>(tail?begin+readable-values.size_bytes():begin);
		for(std::size_t i=0;i<values.size();++i)::new(data+i) Mint(values[i]);
		require(mprotect(begin,readable,PROT_READ)==0);
	}
	~Guarded() { require(munmap(region,bytes)==0); }
};
#endif
int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 rng(28467);
	for(unsigned n:{1,2,7,8,9,15,16,17,33,4095,4096,4097,4103,8192})for(unsigned m:{1,7,8,9,33,4096}) {
		auto a=make(n,rng()%Mint::mod()),b=make(m,rng()%Mint::mod());
		require(constant::fits(a,b)==(std::min(n,m)>=4096));check(a,b);check(b,a);
		if(n>=4096&&m>=4096)for(std::size_t at:{std::size_t(0),std::size_t(7),std::size_t(8),a.size()-2,a.size()-1}) {
			a[at]+=Mint(1);require(!constant::fits(a,b));a[at]-=Mint(1);
		}
	}
	for(unsigned n:{4096,4097,4103,524288}) {
		auto a=make(n,0),b=make(n+7,17);require(constant::fits(a,b));check(a,b);
		a=make(n,23);require(constant::fits(a,a));check(a,a);
	}
	for(unsigned n:{4095,4096,4097,8192})for(unsigned m:{4095,4096,4097,8192}) {
		auto a=make(n,264704197),b=make(m,120999146);
		check_public(a,b);
		if(n==m)check_public(a,a);
		a.back()+=Mint(1);
		require(!constant::fits(a,b));
		check_public(a,b);
		a.back()-=Mint(1);
		b.back()+=Mint(1);
		require(!constant::fits(a,b));
		check_public(a,b);
	}
	for(unsigned n:{4096,4097,4103,4104,8192})for(bool tail:{false,true}) {
		auto a=make(n,17);
		{
			Guarded g(a,tail);require(constant::same_eight(g.data,constant::canonical(a[0])));
			require(constant::remaining({g.data,a.size()},constant::canonical(a[0])));
		}
		a.back()+=Mint(1);
		Guarded g(a,tail);require(!constant::remaining({g.data,a.size()},constant::canonical(a[0])));
	}
#endif
}
