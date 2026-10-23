#ifndef CLOSEST_PAIR_COMPONENT_HPP
#define CLOSEST_PAIR_COMPONENT_HPP

#include "splitmix64.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace closest_pair_detail {
inline std::optional<std::pair<std::uint64_t, std::uint64_t>> packed(std::vector<std::uint64_t> points, std::uint32_t max_x, std::uint64_t budget) {
	using U = std::uint64_t;
	assert(points.size() >= 2 && points.size() <= INT32_MAX);
	std::uint32_t n = points.size();
	auto distance = [](U a, U b) {
		std::int64_t x = std::int64_t(std::uint32_t(a)) - std::uint32_t(b), y = std::int64_t(a >> 32) - std::int64_t(b >> 32);
		return U(x * x) + U(y * y);
	};
	U a = points[0], b = points[1], best = distance(a, b);
	if (!best) return std::pair{a, b};
	SplitMix64 random(n);
	for (std::uint32_t i = 0; i < (n + 3) / 4; ++i) {
		auto x = std::uint32_t((__uint128_t(random()) * n) >> 64), y = std::uint32_t((__uint128_t(random()) * n) >> 64);
		if (x == y) continue;
		U d = distance(points[x], points[y]);
		if (d < best) {
			a = points[x], b = points[y], best = d;
			if (!d) return std::pair{a, b};
		}
	}
	auto limit_for = [](U d) {
		U r = std::sqrt(double(d - 1));
		while ((r + 1) * (r + 1) < d) ++r;
		while (r * r >= d) --r;
		return r;
	};
	U limit = limit_for(best);
	unsigned shift = std::bit_width(limit), x_bits = std::bit_width(max_x);
	auto key = [=](U p) { return ((p >> 32 >> shift) << x_bits) | std::uint32_t(p); };
	U maximum = 0;
	for (U p : points) maximum = std::max(maximum, key(p));
	unsigned bits = std::min(11U, unsigned(std::bit_width(n))), buckets = 1U << bits, mask = buckets - 1;
	std::vector<U> temporary(n);
	for (unsigned digit = 0; digit < 64 && (maximum >> digit); digit += bits) {
		std::array<std::uint32_t, 2048> count{};
		for (U p : points) ++count[(key(p) >> digit) & mask];
		std::uint32_t sum = 0;
		for (unsigned i = 0; i < buckets; ++i) { auto old = count[i]; count[i] = sum; sum += old; }
		for (U p : points) temporary[count[(key(p) >> digit) & mask]++] = p;
		points.swap(temporary);
	}
	auto attempt = [&](U x, U y) {
		U d = distance(x, y);
		if (d < best) {
			a = x, b = y, best = d;
			if (d) limit = limit_for(d);
		}
	};
	std::uint32_t prev_begin = 0, prev_end = 0;
	U prev_lane = UINT64_MAX;
	for (std::uint32_t first = 0; first < n;) {
		U lane = points[first] >> 32 >> shift;
		std::uint32_t end = first + 1;
		while (end < n && (points[end] >> 32 >> shift) == lane) ++end;
		std::uint32_t left = prev_begin;
		bool adjacent = prev_begin < prev_end && prev_lane + 1 == lane;
		for (std::uint32_t i = first; i < end; ++i) {
			U p = points[i], x = std::uint32_t(p);
			for (std::uint32_t j = i; j > first;) {
				--j;
				if (x - std::uint32_t(points[j]) > limit) break;
				if (!budget--) return std::nullopt;
				attempt(p, points[j]);
				if (!best) return std::pair{a, b};
			}
			if (!adjacent || (p >> 32) - (lane << shift) + 1 > limit) continue;
			while (left < prev_end && U(std::uint32_t(points[left])) + limit < x) ++left;
			for (std::uint32_t j = left; j < prev_end && U(std::uint32_t(points[j])) <= x + limit; ++j) {
				if (!budget--) return std::nullopt;
				attempt(p, points[j]);
				if (!best) return std::pair{a, b};
			}
		}
		prev_begin = first, prev_end = end, prev_lane = lane, first = end;
	}
	return std::pair{a, b};
}
}

#endif
