#include "components/fps_composition.hpp"
#include <cstdlib>
#include <random>
#include <iostream>
int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	using namespace fps_detail::composition_poly;
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 rng(192731);
	for (unsigned h : {4u,8u,16u,32u,64u,128u,256u,512u})
		for (unsigned width = 2; width < h; width *= 2) {
			Arena arena(2*Arena::footprint(32*h)+Transform::words(15));
			Transform t(arena,15);
			auto a=arena.take(32*h),b=arena.take(32*h);
			for (unsigned k=0;k<4;++k) {
				for(auto&x:a)x=rng()%(4*narrow::kP);
				std::copy(a.begin(),a.end(),b.begin());
				const auto* x=t.roots()+fps_detail::blocked_ntt::detail::slot(k);
				const auto* y=t.roots()+fps_detail::blocked_ntt::detail::slot(2*k);
				kernels::forward_columns(reinterpret_cast<__m256i*>(a.data()),h,width,x,y);
				narrow::forward_columns(b.data(),h,width,x,y);
				if(!std::equal(a.begin(),a.end(),b.begin())) {std::cerr<<"forward "<<h<<' '<<width<<' '<<k<<'\n';std::abort();}
				for(auto& value:a)value=rng()%(2*narrow::kP);
				std::copy(a.begin(),a.end(),b.begin());
				x=t.inverse_roots()+fps_detail::blocked_ntt::detail::slot(k);
				y=t.inverse_roots()+fps_detail::blocked_ntt::detail::slot(2*k);
				kernels::inverse_columns(reinterpret_cast<__m256i*>(a.data()),h,width,x,y);
				narrow::inverse_columns(b.data(),h,width,x,y);
				if(!std::equal(a.begin(),a.end(),b.begin())) {std::cerr<<"inverse "<<h<<' '<<width<<' '<<k<<'\n';std::abort();}
			}
		}
#endif
}
