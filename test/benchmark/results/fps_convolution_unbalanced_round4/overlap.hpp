#ifndef ROUND4_OVERLAP_HPP
#define ROUND4_OVERLAP_HPP
#include "snapshot/components/fps_blocked.hpp"
#include <algorithm>
#include <bit>
#include <vector>
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")
namespace round4_overlap {
using fps_detail::Mint;
using namespace fps_detail::blocked_poly;
inline std::vector<Mint> multiply(const std::vector<Mint>& a, const std::vector<Mint>& b, std::size_t len) {
	const auto& x = a.size() >= b.size() ? a : b;
	const auto& y = a.size() >= b.size() ? b : a;
	const std::size_t n = x.size(), m = y.size(), stride = len - m + 1;
	const int lg = std::countr_zero(len);
	Arena arena(Transform::words(lg) + 3 * Arena::footprint(len));
	const Transform t(arena, lg);
	auto bt = arena.take(len), in = arena.take(len), out = arena.take(len);
	for (std::size_t i = 0; i < m; ++i) in[i] = y[i].val();
	t.forward(in.first(m), 0, bt);
	std::vector<Mint> result(n + m - 1);
	for (std::size_t p = 0; p < n; p += stride) {
		const std::size_t count = std::min(stride, n - p);
		for (std::size_t i = 0; i < count; ++i) {
			const unsigned v = x[p + i].v;
			in[i] = std::min(v, v - Mint::mod());
		}
		t.cyclic_product(in.first(count), 0, out, bt);
		for (std::size_t i = 0; i < count + m - 1; ++i) {
			const unsigned v = result[p + i].v + out[i];
			result[p + i] = Mint::word(std::min(v, v - Mint::mod()));
		}
	}
	return result;
}
}
#pragma GCC pop_options
#endif
