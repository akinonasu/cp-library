#ifndef CP_COMPONENTS_FPS_INVERSE_BLOCKED_HPP
#define CP_COMPONENTS_FPS_INVERSE_BLOCKED_HPP

#include "fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {
inline void append_scaled(Words a, std::vector<mint> &out, Word scale) {
	const detail::Factor factor(scale);
	if (a.size() > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + a.size());
	alignas(32) Word values[64];
	for (size_t i = 0; i < a.size(); i += 64) {
		const auto count = std::min<size_t>(64, a.size() - i);
		size_t j = 0;
		for (; j + 8 <= count; j += 8)
			detail::store(values + j, detail::reduce(detail::times(detail::load_unaligned(a.data() + i + j), factor), mint::mod()));
		for (; j < count; ++j) values[j] = blocked_ntt::detail::multiply_mod(a[i + j], scale);
		append_fps_words<true>(out, values, count);
	}
}

inline void native_inverse(const mint *in, size_t input, std::vector<mint> &out, size_t n) {
	const int lg = inverse_log(n);
	const size_t len = size_t(1) << lg, k = len / 2;
	const size_t c = std::min(input, n), scratch_words = std::max(len, inverse_scratch(k));
	Arena arena(Transform::words(lg) + Arena::footprint(len) + Arena::footprint(scratch_words) + Arena::footprint(k));
	const Transform t(arena, lg);
	auto a = arena.take(len), scratch = arena.take(scratch_words), g = arena.take(k);
	copy_coefficients(in, a.first(c));
	const ConstWords f = a.first(c);
	const size_t target = std::min(n, k), base = std::min(target, detail::kInverseBase);
	detail::inverse_direct(f, g.first(base));
	for (size_t i = 0; i < base; ++i) g[i] = blocked_ntt::detail::multiply_mod(g[i], 932051910);
	for (size_t count = base; count < target; count *= 2) {
		const size_t size = 2 * count;
		inverse_step(t, f, g.first(count), g.subspan(count, std::min(count, target - count)), scratch.first(size),
				scratch.subspan(Arena::footprint(size), size), 53925088);
	}
	if (n <= k) return append_fps_words<true>(out, g.data(), n);
	inverse_step(t, f, g, a.subspan(k, n - k), scratch.first(len), a, 53925088);
	prepare_fps_output(out, n);
	append_fps_words<true>(out, g.data(), k);
	append_fps_words<true>(out, a.data() + k, n - k);
}
}

#pragma GCC pop_options
#endif
#endif
