#ifndef PREFIX_MIN_OF_MOD_OF_LINEAR_HPP
#define PREFIX_MIN_OF_MOD_OF_LINEAR_HPP

#include <cassert>
#include <cstdint>
#include <vector>

struct ModLinearPrefixMinSegment {
	std::uint64_t x, y, dx, dy, count;
	bool operator==(const ModLinearPrefixMinSegment &) const = default;
};

template <class Emit>
void prefix_min_of_mod_of_linear(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b, Emit &&emit) {
	assert(m > 0);
	if (!n) return;
	if (a >= m) a %= m;
	if (b >= m) b %= m;
	bool first = true;
	if (a && b && n > 1) {
		std::uint64_t p = m - a, q = a, sp = 1, sq = 1, x = 0, remaining = n - 1;
		for (;;) {
			if (p <= b) {
				std::uint64_t k = b / p;
				bool final = __uint128_t(k) * sp > remaining;
				if (final) k = remaining / sp;
				if (k) {
					if (first) emit(ModLinearPrefixMinSegment{0, b, sp, p, k + 1});
					else emit(ModLinearPrefixMinSegment{x + sp, b - p, sp, p, k});
					first = false;
					x += k * sp;
					b -= k * p;
					remaining -= k * sp;
				}
				if (final || !b) break;
			}
			if (sp > remaining || sq > remaining - sp) break;
			if (p > q) {
				std::uint64_t k = (p - b - 1) / q + 1, d = k * q;
				if (d == p) break;
				if (d > p) --k;
				sp += k * sq;
				if (sp > remaining) break;
				p -= k * q;
			} else {
				std::uint64_t k = q / p;
				q %= p;
				if (!q) break;
				sq += k * sp;
			}
		}
	}
	if (first) emit(ModLinearPrefixMinSegment{0, b, 0, 0, 1});
}

inline std::vector<ModLinearPrefixMinSegment> prefix_min_of_mod_of_linear(std::uint64_t n, std::uint64_t m,
																	 std::uint64_t a, std::uint64_t b) {
	std::vector<ModLinearPrefixMinSegment> result;
	prefix_min_of_mod_of_linear(n, m, a, b, [&](ModLinearPrefixMinSegment part) { result.push_back(part); });
	return result;
}

#endif
