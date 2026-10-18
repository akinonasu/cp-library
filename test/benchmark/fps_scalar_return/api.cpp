#include "api.hpp"
#include <bit>
#define NOINLINE __attribute__((noinline, noipa))
template<class F> std::uint64_t checksum_impl(const F &f) {
	std::uint64_t a = 0x243f6a8885a308d3ULL, b = f.size();
	for (std::size_t i = 0; i < f.size(); ++i) {
		const std::uint64_t v = f[i].val();
		a += v * (i + 1);
		b += v * v;
	}
	return a ^ std::rotl(b, 17);
}
#define DEFINE(T, F, prefix) \
NOINLINE F prefix##_add(const F &f, T x) { return f + x; } \
NOINLINE F prefix##_sub(const F &f, T x) { return f - x; } \
NOINLINE F prefix##_mul(const F &f, T x) { return f * x; } \
NOINLINE F prefix##_div(const F &f, T x) { return f / x; } \
NOINLINE std::uint64_t prefix##_checksum(const F &f) { return checksum_impl(f); }
DEFINE(fps_mint, PublicF, public_fps)
DEFINE(fps_detail::Mint, InternalF, internal_fps)
#undef DEFINE
