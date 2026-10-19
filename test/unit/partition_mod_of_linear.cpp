#include "math/floor_sum/partition_mod_of_linear.hpp"
#include "test/benchmark/mod_linear_partition_baseline.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

using U = std::uint64_t;
using W = __uint128_t;
using S = __int128_t;

void check(U n, U m, U a, U b) {
	auto parts = partition_mod_of_linear(n, m, a, b);
	std::vector<ModLinearSegment> streamed;
	partition_mod_of_linear(n, m, a, b, [&](ModLinearSegment part) { streamed.push_back(part); });
	assert(parts == streamed);
	if (!n) { assert(parts.empty()); return; }
	assert(!parts.empty());
	U dx = parts[0].dx;
	std::int64_t dy = parts[0].dy;
	assert(dx && W(dx) * dx <= n);
	assert(W(a) * dx % m == (dy < 0 ? m - U(-dy) : U(dy)));
	assert(W(parts.size()) <= 6 * (U(std::sqrt(static_cast<long double>(n))) + 1));
	std::vector<U> next(dx);
	for (U i = 0; i < dx; ++i) next[i] = i;
	std::vector<int> seen(n <= 4096 ? n : 0);
	W total = 0;
	U previous_residue = 0;
	for (auto part : parts) {
		auto [x, y, sx, sy, count] = part;
		assert(sx == dx && sy == dy && count && y < m);
		U residue = x % dx;
		assert(residue >= previous_residue && x == next[residue]);
		previous_residue = residue;
		assert(W(x) + W(dx) * (count - 1) < n);
		assert(S(y) + S(dy) * (count - 1) >= 0 && S(y) + S(dy) * (count - 1) < S(m));
		for (U j : std::array<U, 3>{0, (count - 1) / 2, count - 1}) {
			U index = x + dx * j;
			assert(S(y) + S(dy) * j == S((W(a) * index + b) % m));
		}
		if (!seen.empty()) for (U j = 0; j < count; ++j) {
			U index = x + dx * j;
			assert(++seen[index] == 1);
			assert(S(y) + S(dy) * j == S((W(a) * index + b) % m));
		}
		total += count;
		W after = W(x) + W(dx) * count;
		next[residue] = after >= n ? n : U(after);
	}
	assert(total == n);
	for (U x : next) assert(x == n);
	for (int count : seen) assert(count == 1);
	if (n <= 1000000) {
		U best = m, stride = 1;
		std::int64_t step = 0;
		for (U i = 1; W(i) * i <= n; ++i) {
			U value = W(a) * i % m, distance = std::min(value, m - value);
			if (distance < best) best = distance, stride = i, step = value > m - value ? -S(distance) : S(distance);
		}
		assert(dx == stride && dy == step);
	}
	if (n <= 10000 && m <= 1000000000) {
		auto [sx, sy, old] = sqrt_floor(n, m, a % m, b % m);
		assert(U(sx) == dx && sy == dy && old.size() == parts.size());
		for (std::size_t i = 0; i < old.size(); ++i)
			assert((old[i] == std::tuple<long, long, long>(parts[i].x, parts[i].y, parts[i].count)));
	}
}

int main() {
	for (U n = 0; n <= 24; ++n) for (U m = 1; m <= 24; ++m)
		for (U a = 0; a < m; ++a) for (U b = 0; b < m; ++b) check(n, m, a, b);
	std::mt19937_64 random(78419);
	for (int i = 0; i < 10000; ++i) check(random() % 1000, random() | 1, random(), random());
	for (int i = 0; i < 200; ++i) check(random() % 1000000, random() | 1, random(), random());
	constexpr U maximum = std::numeric_limits<U>::max();
	for (U m : std::array<U, 9>{1, 2, 3, U(1) << 32, (U(1) << 32) - 1, U(1) << 63,
		(U(1) << 63) - 1, maximum - 1, maximum})
		for (U a : std::array<U, 6>{0, 1, m / 2, m - 1, m, maximum})
			for (U b : std::array<U, 4>{0, m / 2, m - 1, maximum}) check(4096, m, a, b);
	for (U n : std::array<U, 8>{1, 2, (U(1) << 32) - 1, U(1) << 32, (U(1) << 63) - 1,
		U(1) << 63, maximum - 1, maximum})
		for (U m : std::array<U, 4>{1, U(1) << 63, maximum - 1, maximum})
			for (U a : std::array<U, 4>{0, 1, m - 1, m - (m > 1 ? 2 : 1)})
				for (U b : std::array<U, 4>{0, m / 2, m - 1, maximum}) check(n, m, a, b);
	auto parts = partition_mod_of_linear(4, 5, 2, 1);
	assert((parts == std::vector<ModLinearSegment>{{0, 1, 2, -1, 2}, {1, 3, 2, -1, 2}}));
}
