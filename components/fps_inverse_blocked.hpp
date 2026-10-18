#ifndef CP_COMPONENTS_FPS_INVERSE_BLOCKED_HPP
#define CP_COMPONENTS_FPS_INVERSE_BLOCKED_HPP

#include "fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {
inline void append_scaled(std::span<std::uint32_t> a, std::vector<Mint> &out, std::uint32_t scale) {
	const detail::Factor factor(scale);
	if (a.size() > out.max_size() - out.size()) throw std::length_error("fps output");
	prepare_fps_output(out, out.size() + a.size());
	alignas(32) std::uint32_t values[64];
	for (std::size_t i = 0; i < a.size(); i += 64) {
		const auto count = std::min<std::size_t>(64, a.size() - i);
		std::size_t j = 0;
		for (; j + 8 <= count; j += 8)
			detail::store(values + j, detail::reduce(detail::times(detail::load_unaligned(a.data() + i + j), factor), Mint::mod()));
		for (; j < count; ++j) values[j] = blocked_ntt::detail::multiply_mod(a[i + j], scale);
		append_fps_words<true>(out, values, count);
	}
}

inline void native_inverse(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n) {
	const int lg = inverse_log(n);
	const std::size_t len = std::size_t(1) << lg, k = len / 2;
	const std::size_t c = std::min(input, n), scratch_words = std::max(len, inverse_scratch(k));
	Arena arena(Transform::words(lg) + Arena::footprint(len) + Arena::footprint(scratch_words) + Arena::footprint(k));
	const Transform t(arena, lg);
	auto a = arena.take(len), scratch = arena.take(scratch_words), g = arena.take(k);
	for (std::size_t i = 0; i < c; ++i) {
		const unsigned v = in[i].v;
		a[i] = std::min(v, v - Mint::mod());
	}
	const std::span<const std::uint32_t> f = a.first(c);
	if (n <= k) {
		inverse(t, f, g.first(n), scratch);
		append_scaled(g.first(n), out, 932051910);
		return;
	}
	inverse(t, f, g, scratch);
	inverse_step(t, f, g, a.subspan(k, n - k), scratch.first(len), a);
	prepare_fps_output(out, n);
	append_scaled(g, out, 932051910);
	append_scaled(a.subspan(k, n - k), out, 932051910);
}
}

#pragma GCC pop_options
#endif
#endif
