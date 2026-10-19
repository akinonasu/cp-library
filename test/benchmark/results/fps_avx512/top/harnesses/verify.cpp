#include "components/fps_blocked.hpp"
#include <cstdio>
#include <random>
using namespace fps_detail::blocked_poly;
using namespace fps_detail::blocked_poly::detail;
[[gnu::target("avx2"),gnu::optimize("O3"),gnu::noinline]] static void reference(Vec* f,std::size_t h,Half output,const std::uint32_t* roots,const Factor& s){
	const Factor z(roots[1],roots[9]);
	for(std::size_t j=0;j<h;++j){const Vec p0=f[j],p1=f[j+h],p2=f[j+2*h],p3=f[j+3*h];const Vec ab=low(add(p0,p1)),cd=low(add(p2,p3)),amb=low(diff(p0,p1)),cmd=times(diff(p2,p3),z);
		if(output!=Half::kUpper){f[j]=reduce(times(add(ab,cd),s),kP);f[j+h]=reduce(times(add(amb,cmd),s),kP);}
		if(output!=Half::kLower){f[j+2*h]=reduce(times(diff(ab,cd),s),kP);f[j+3*h]=reduce(times(diff(amb,cmd),s),kP);}
	}
}
static void require(bool ok){if(!ok)std::abort();}
int main(){std::mt19937 rng(20261022);for(std::size_t h:{4u,16u,32u,64u,256u,1024u,16384u}){std::size_t n=32*h;int lg=std::bit_width(n)-1;Arena arena(Transform::words(lg)+2*Arena::footprint(n+16));Transform t(arena,lg);auto a=arena.take(n+16),b=arena.take(n+16);
	for(auto mode:{Half::kLower,Half::kUpper,Half::kBoth})for(unsigned scale:{0u,1u,kP-1,932051910u}){for(std::size_t i=0;i<n;++i)a[i]=b[i]=rng()%(2*kP);for(std::size_t i=n;i<n+16;++i)a[i]=b[i]=0xdeadbeef;Factor s(scale);reference(reinterpret_cast<Vec*>(a.data()),h,mode,t.inverse_roots(),s);inverse_top4(reinterpret_cast<Vec*>(b.data()),h,mode,t.inverse_roots(),s);for(std::size_t i=0;i<n+16;++i)require(a[i]==b[i]);}}
	std::puts("all radix4 inverse halves, zero/nonzero scales, and canaries passed");
}
