#ifndef PREFIX_MIN_OF_MOD_OF_LINEAR_HPP
#define PREFIX_MIN_OF_MOD_OF_LINEAR_HPP

#include <cassert>
#include <cstdint>
#include <limits>
#include <vector>
#include "../../components/mod_linear_prefix_min.hpp"

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
	if (n > m) n = m;
	auto output = [&](auto x, auto y, auto dx, auto dy, auto count) {
		emit(ModLinearPrefixMinSegment{x, y, dx, dy, count});
	};
	if (m <= std::numeric_limits<std::uint32_t>::max())
		mod_linear_detail::prefix_min<std::uint32_t>(n, m, a, b, output);
	else mod_linear_detail::prefix_min(n, m, a, b, output);
}

inline std::vector<ModLinearPrefixMinSegment> prefix_min_of_mod_of_linear(std::uint64_t n, std::uint64_t m,
		std::uint64_t a, std::uint64_t b) {
	std::vector<ModLinearPrefixMinSegment> result;
	prefix_min_of_mod_of_linear(n, m, a, b, [&](ModLinearPrefixMinSegment part) { result.push_back(part); });
	return result;
}

#endif
