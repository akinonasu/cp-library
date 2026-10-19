#include "math/floor_sum/prefix_min_of_mod_of_linear.hpp"
#include "math/floor_sum/min_of_mod_of_linear.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;
using Segment = ModLinearPrefixMinSegment;

U value(const std::vector<Segment> &parts, U x) {
	auto it = std::upper_bound(parts.begin(), parts.end(), x, [](U x, const Segment &part) { return x < part.x; });
	assert(it != parts.begin());
	const auto &[start, y, dx, dy, count] = *--it;
	return y - (dx ? std::min(count - 1, (x - start) / dx) : 0) * dy;
}

void check(U n, U m, U a, U b, bool exhaustive = false) {
	auto parts = prefix_min_of_mod_of_linear(n, m, a, b);
	std::vector<Segment> callback;
	prefix_min_of_mod_of_linear(n, m, a, b, [&](Segment part) { callback.push_back(part); });
	assert(parts == callback);
	if (!n) {
		assert(parts.empty());
		return;
	}
	std::size_t bits = std::bit_width(b % m);
	assert(!parts.empty() && parts.size() <= (bits ? bits : 1));
	assert(parts[0].x == 0 && parts[0].y == b % m);
	U last_x = 0, last_y = m;
	for (const auto &[x, y, dx, dy, count] : parts) {
		U previous_y = last_y;
		assert(count > 0 && x < n && y < m);
		assert(W(dx) * (count - 1) <= n - 1 - x);
		assert(W(dy) * (count - 1) <= y);
		if (x) assert(x > last_x && y < last_y && dx && dy);
		else assert(count == 1 || (dx && dy));
		last_x = x + dx * (count - 1);
		last_y = y - dy * (count - 1);
		for (U j : std::array<U, 3>{0, (count - 1) / 2, count - 1}) {
			U position = x + j * dx, actual = y - j * dy;
			assert((W(a) * position + b) % m == actual);
			assert(min_of_mod_of_linear(position + 1, m, a, b) == actual);
			if (position) assert(min_of_mod_of_linear(position, m, a, b) == (j ? actual + dy : previous_y));
		}
		if (last_x + 1 < n) assert(value(parts, last_x + 1) == min_of_mod_of_linear(last_x + 2, m, a, b));
	}
	assert(value(parts, n - 1) == min_of_mod_of_linear(n, m, a, b));
	if (n >= m) assert(last_y == b % std::gcd(a, m));
	if (exhaustive || n <= 128) {
		std::vector<std::pair<U, U>> records, actual;
		for (U i = 0, best = m; i < n; ++i) {
			U y = (W(a) * i + b) % m;
			if (y < best) records.emplace_back(i, y), best = y;
			assert(value(parts, i) == best);
		}
		for (auto [x, y, dx, dy, count] : parts)
			for (U j = 0; j < count; ++j) actual.emplace_back(x + dx * j, y - dy * j);
		assert(actual == records);
	}
}

int main() {
	const auto example = prefix_min_of_mod_of_linear(100, 100, 71, 85);
	std::vector<std::pair<U, U>> actual;
	for (auto [x, y, dx, dy, count] : example)
		for (U j = 0; j < count; ++j) actual.emplace_back(x + dx * j, y - dy * j);
	assert((actual == std::vector<std::pair<U, U>>{{0, 85}, {1, 56}, {2, 27}, {6, 11},
		{13, 8}, {20, 5}, {27, 2}, {65, 0}}));
	for (U m = 1; m <= 32; ++m)
		for (U a = 0; a < m; ++a)
			for (U b = 0; b < m; ++b) {
				check(2 * m + 2, m, a, b, true);
				for (U n = 0; n <= m + 1; ++n) check(n, m, a + 3 * m, b + 5 * m, true);
			}
	std::mt19937_64 random(713);
	for (int i = 0; i < 20000; ++i) check(random() % 128, random() | 1, random(), random(), true);
	constexpr U maximum = std::numeric_limits<U>::max();
	for (U m : std::array<U, 10>{1, 2, 3, (U(1) << 32) - 1, U(1) << 32, (U(1) << 32) + 1,
		(U(1) << 63) - 1, U(1) << 63, maximum - 1, maximum})
		for (U n : std::array<U, 8>{0, 1, 2, 3, m, m - 1, maximum / 2, maximum})
			for (U a : std::array<U, 7>{0, 1, m / 2, m - 1, m, maximum - 1, maximum})
				for (U b : std::array<U, 6>{0, 1, m / 2, m - 1, m, maximum}) check(n, m, a, b);
	for (int i = 0; i < 10000; ++i) {
		U m = random() | 1;
		check(random(), m, random(), random());
		check(random() % m + 1, m, random(), random());
	}
	std::vector<U> fibonacci{1, 1};
	while (W(fibonacci.back()) + fibonacci[fibonacci.size() - 2] <= maximum)
		fibonacci.push_back(fibonacci.back() + fibonacci[fibonacci.size() - 2]);
	for (std::size_t i = 2; i < fibonacci.size(); ++i)
		for (U n : std::array<U, 5>{1, 2, fibonacci[i] / 2, fibonacci[i], maximum})
			for (U b : std::array<U, 4>{0, 1, fibonacci[i] / 3, fibonacci[i] - 1})
				check(n, fibonacci[i], fibonacci[i - 1], b);
	auto descend = prefix_min_of_mod_of_linear(maximum, maximum, maximum - 1, maximum - 1);
	assert(descend.size() == 1 && descend[0].count == maximum);
	assert(descend[0].dx == 1 && descend[0].dy == 1);
}
