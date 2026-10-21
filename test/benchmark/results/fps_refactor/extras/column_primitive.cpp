#include "components/fps_composition.hpp"
namespace fps_detail::composition_poly {
namespace candidate_kernels {
using Vec = __m256i;
namespace column_detail = blocked_poly::detail;

template<bool Inverse>
[[gnu::noinline]] void columns(Vec *a, std::size_t h, std::size_t width,
		const std::uint32_t *x, const std::uint32_t *y) {
	using namespace column_detail;
	const Factor wx(x[0], x[8]), wy(y[0], y[8]), wz(y[1], y[9]);
	for (std::size_t c = 0; c < h; c += 2 * width)
		for (std::size_t j = c; j < c + width; ++j) {
			const Vec f0 = a[j], f1 = a[j + h], f2 = a[j + 2 * h], f3 = a[j + 3 * h];
			if constexpr (Inverse) {
				const Vec ab = low(add(f0, f1)), cd = low(add(f2, f3));
				const Vec amb = times(diff(f0, f1), wy), cmd = times(diff(f2, f3), wz);
				a[j] = low(add(ab, cd)), a[j + h] = low(add(amb, cmd));
				a[j + 2 * h] = times(diff(ab, cd), wx), a[j + 3 * h] = times(diff(amb, cmd), wx);
			} else {
				const Vec lo = low(f0), hi = low(f1), xc = times(f2, wx), xd = times(f3, wx);
				const Vec ac = low(add(lo, xc)), amc = low_difference(lo, xc);
				const Vec bd = times(add(hi, xd), wy), bmd = times(diff(hi, xd), wz);
				a[j] = add(ac, bd), a[j + h] = diff(ac, bd);
				a[j + 2 * h] = add(amc, bmd), a[j + 3 * h] = diff(amc, bmd);
			}
		}
}
inline void forward_columns(Vec *a, std::size_t h, std::size_t width, const std::uint32_t *x, const std::uint32_t *y) {
	columns<false>(a, h, width, x, y);
}
inline void inverse_columns(Vec *a, std::size_t h, std::size_t width, const std::uint32_t *x, const std::uint32_t *y) {
	columns<true>(a, h, width, x, y);
}
inline void forward_even_columns(Vec *a, std::size_t h, const std::uint32_t *x, const std::uint32_t *y) {
	columns<false>(a, h, 1, x, y);
}
inline void inverse_even_columns(Vec *a, std::size_t h, const std::uint32_t *x, const std::uint32_t *y) {
	columns<true>(a, h, 1, x, y);
}
}

}
#include <cstdlib>
#include <random>
#include <iostream>
int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	using namespace fps_detail::composition_poly;
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 rng(192731);
	for (unsigned h : {4u,8u,16u,32u,64u,128u,256u,512u})
		for (unsigned width = 1; width < h; width *= 2) {
			Arena arena(2*Arena::footprint(32*h)+Transform::words(15));
			Transform t(arena,15);
			auto a=arena.take(32*h),b=arena.take(32*h);
			for (unsigned k=0;k<4;++k) {
				for(auto&x:a)x=rng()%(4*998244353u);
				std::copy(a.begin(),a.end(),b.begin());
				const auto* x=t.roots()+fps_detail::blocked_ntt::detail::slot(k);
				const auto* y=t.roots()+fps_detail::blocked_ntt::detail::slot(2*k);
				if(width==1) kernels::forward_even_columns(reinterpret_cast<__m256i*>(a.data()),h,x,y); else kernels::forward_columns(reinterpret_cast<__m256i*>(a.data()),h,width,x,y);
				candidate_kernels::forward_columns(reinterpret_cast<__m256i*>(b.data()),h,width,x,y);
				if(!std::equal(a.begin(),a.end(),b.begin())) {std::cerr<<"forward "<<h<<' '<<width<<' '<<k<<'\n';std::abort();}
				for(auto& value:a)value=rng()%(2*998244353u);
				std::copy(a.begin(),a.end(),b.begin());
				x=t.inverse_roots()+fps_detail::blocked_ntt::detail::slot(k);
				y=t.inverse_roots()+fps_detail::blocked_ntt::detail::slot(2*k);
				if(width==1) kernels::inverse_even_columns(reinterpret_cast<__m256i*>(a.data()),h,x,y); else kernels::inverse_columns(reinterpret_cast<__m256i*>(a.data()),h,width,x,y);
				candidate_kernels::inverse_columns(reinterpret_cast<__m256i*>(b.data()),h,width,x,y);
				if(!std::equal(a.begin(),a.end(),b.begin())) {std::cerr<<"inverse "<<h<<' '<<width<<' '<<k<<'\n';std::abort();}
			}
		}
#endif
}
