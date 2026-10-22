#ifndef CP_COMPONENTS_FPS_REVERSION_HPP
#define CP_COMPONENTS_FPS_REVERSION_HPP

#include "fps_projection_blocked.hpp"
#include "fps_exp_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::composition_poly {
inline void native_reversion(const mint *in, std::size_t input, std::vector<mint> &out, std::size_t n) {
	input = std::min(input, n);
	const int lg = std::max(projection_log(n), blocked_poly::power_log(n - 1));
	const std::size_t rest = std::max(projection_scratch(n), Arena::footprint(n - 1) + blocked_poly::power_scratch(n - 1));
	Arena arena(Transform::words(lg) + Arena::footprint(input) + 2 * Arena::footprint(n) + Arena::footprint(rest));
	const Transform t(arena, lg);
	auto f = arena.take(input), g = arena.take(n), a = arena.take(n), scratch = arena.take(rest);
	for (std::size_t i = 0; i < input; ++i) f[i] = coefficient_value(in[i]);
	power_projection(t, f, a, scratch.first(projection_scratch(n)));
	auto h = scratch.first(n - 1);
	blocked_poly::divide_by_index(a.subspan(1), 1, h);
	std::reverse(h.begin(), h.end());
	const auto e = kModulus - blocked_ntt::detail::power(std::uint32_t(n - 1), kModulus - 2);
	const auto c = in[1].inv();
	blocked_poly::power(t, h, e, std::min(c.v, c.v - mint::mod()), g.subspan(1),
						scratch.subspan(Arena::footprint(n - 1), blocked_poly::power_scratch(n - 1)));
	prepare_fps_output(out, n);
	out.emplace_back(0);
	append_fps_words<true>(out, g.data() + 1, g.size() - 1);
}
}
#pragma GCC pop_options
#endif
#endif
