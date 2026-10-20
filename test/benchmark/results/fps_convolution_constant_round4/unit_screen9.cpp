#include "components/fps_convolution.hpp"
#include <algorithm>
#include <cstdlib>
#include <random>
#include <sys/mman.h>
#include <unistd.h>
using fps_detail::Mint;
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
int main() {
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
}
