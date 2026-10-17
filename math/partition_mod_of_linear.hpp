#ifndef PARTITION_MOD_OF_LINEAR_HPP
#define PARTITION_MOD_OF_LINEAR_HPP

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <vector>
#include "prefix_min_of_mod_of_linear.hpp"

struct ModLinearSegment {
	std::uint64_t x, y, dx;
	std::int64_t dy;
	std::uint64_t count;
	bool operator==(const ModLinearSegment&) const = default;
};

template<class Emit>
void partition_mod_of_linear(std::uint64_t n, std::uint64_t m, std::uint64_t a, std::uint64_t b, Emit&& emit) {
	using U = std::uint64_t;
	assert(m > 0);
	if (!n) return;
	a %= m, b %= m;
	U limit = std::sqrt(static_cast<long double>(n));
	while (limit + 1 <= n / (limit + 1)) ++limit;
	while (limit > n / limit) --limit;
	limit = std::min(limit, m);
	U dx = 1, distance = m;
	bool negative = false;
	auto search = [&](U step, bool sign) {
		prefix_min_of_mod_of_linear(limit, m, step, step, [&](ModLinearPrefixMinSegment part) {
			U value = part.y - part.dy * (part.count - 1), index = part.x + part.dx * (part.count - 1) + 1;
			if (value < distance || (value == distance && index < dx))
				distance = value, dx = index, negative = sign;
		});
	};
	search(a, false);
	if (distance) search(a ? m - a : 0, true);
	std::int64_t dy = negative ? -std::int64_t(distance) : std::int64_t(distance);
	for (U residue = 0, initial = b; residue < dx; ++residue) {
		U x = residue, y = initial, remaining = (n - 1 - residue) / dx + 1;
		while (remaining) {
			U gap = negative ? y : m - 1 - y;
			U count = distance ? std::min(remaining, gap / distance + 1) : remaining;
			emit(ModLinearSegment{x, y, dx, dy, count});
			remaining -= count;
			if (!remaining) break;
			x += dx * count;
			U rest = gap % distance;
			y = negative ? m - distance + rest : distance - 1 - rest;
		}
		initial = initial >= m - a ? initial - (m - a) : initial + a;
	}
}

inline std::vector<ModLinearSegment> partition_mod_of_linear(std::uint64_t n, std::uint64_t m,
		std::uint64_t a, std::uint64_t b) {
	std::vector<ModLinearSegment> result;
	partition_mod_of_linear(n, m, a, b, [&](ModLinearSegment segment) { result.push_back(segment); });
	return result;
}

#endif
