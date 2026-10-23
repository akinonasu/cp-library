#include "data_structure/offline_rectangle_sum.hpp"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <tuple>
#include <vector>

void require(bool ok) { if (!ok) std::abort(); }

struct Coordinate {
	int value;
	explicit Coordinate(int value) : value(value) {}
	friend bool operator<(Coordinate a, Coordinate b) { return a.value < b.value; }
};
struct ModSum {
	int value = 0;
	ModSum() = default;
	ModSum(int x) : value((x % 97 + 97) % 97) {}
	ModSum &operator+=(ModSum other) { value = (value + other.value) % 97; return *this; }
	ModSum &operator-=(ModSum other) { value = (value + 97 - other.value) % 97; return *this; }
};

int main() {
	std::mt19937 rng(312484);
	for (int round = 0; round < 150; ++round) {
		int n = rng() % 100, q = rng() % 100;
		OfflineRectangleSum<int, int, std::int64_t> solver(n, q);
		std::vector<std::array<int, 3>> points(n);
		for (auto &p : points) {
			p = {int(rng() % 17) - 8, int(rng() % 17) - 8, int(rng() % 2001) - 1000};
			solver.add_point(p[0], p[1], p[2]);
		}
		std::vector<std::int64_t> expected;
		for (int i = 0; i < q; ++i) {
			int l = int(rng() % 21) - 10, r = int(rng() % 21) - 10;
			int d = int(rng() % 21) - 10, u = int(rng() % 21) - 10;
			require(solver.add_query(l, d, r, u) == i);
			std::int64_t answer = 0;
			for (auto p : points) if (l <= p[0] && p[0] < r && d <= p[1] && p[1] < u) answer += p[2];
			expected.push_back(answer);
		}
		require(solver.solve() == expected && solver.solve() == expected);
		auto copy = solver;
		copy.add_point(0, 0, 7);
		require(solver.solve() == expected);
	}
	OfflineRectangleSum<std::int64_t, std::uint64_t, __int128_t> extremes;
	extremes.add_point(INT64_MIN, 0, __int128_t(1) << 100);
	extremes.add_point(INT64_MAX, UINT64_MAX, -(__int128_t(1) << 90));
	extremes.add_point(0, UINT64_MAX - 1, -3);
	extremes.add_query(INT64_MIN, 0, INT64_MAX, UINT64_MAX);
	extremes.add_query(INT64_MAX, 0, INT64_MAX, UINT64_MAX);
	auto answers = extremes.solve();
	require(answers == std::vector<__int128_t>{(__int128_t(1) << 100) - 3, 0});
	OfflineRectangleSum<int> intermediate;
	intermediate.add_point(-2, 0, INT64_MIN);
	intermediate.add_point(-1, 0, INT64_MAX);
	intermediate.add_point(0, 0, INT64_MAX);
	intermediate.add_query(-1, 0, 0, 1);
	intermediate.add_query(0, 0, 1, 1);
	intermediate.add_query(-2, 0, 0, 1);
	require(intermediate.solve() == std::vector<std::int64_t>{INT64_MAX, INT64_MAX, -1});
	OfflineRectangleSum<int> narrow_intermediate;
	narrow_intermediate.add_point(-1, 0, INT64_MIN);
	narrow_intermediate.add_point(0, 1, 1);
	narrow_intermediate.add_query(0, 0, 1, 2);
	narrow_intermediate.add_query(-1, 1, 1, 2);
	require(narrow_intermediate.solve() == std::vector<std::int64_t>{1, 1});

	OfflineRectangleSum<int, int, std::uint64_t> unsigned_intermediate;
	unsigned_intermediate.add_point(-1, 0, UINT64_MAX);
	unsigned_intermediate.add_point(0, 0, UINT64_MAX);
	unsigned_intermediate.add_query(0, 0, 1, 1);
	require(unsigned_intermediate.solve() == std::vector<std::uint64_t>{UINT64_MAX});

	OfflineRectangleSum<std::string, Coordinate, ModSum> generic;
	generic.add_point("b", Coordinate(-3), ModSum(100));
	generic.add_point("b", Coordinate(5), ModSum(-10));
	generic.add_query("a", Coordinate(-5), "c", Coordinate(5));
	require(generic.solve()[0].value == 3);
	OfflineRectangleSum<int, Coordinate> mixed;
	mixed.add_point(4, Coordinate(5), 7);
	mixed.add_query(0, Coordinate(0), 10, Coordinate(10));
	require(mixed.solve() == std::vector<std::int64_t>{7});
	OfflineRectangleSum<int> radix;
	for (int i = 0; i < 3000; ++i) radix.add_point((i * 17) % 97 - 50, (i * 19) % 101 - 40, i);
	std::int64_t answer = 0;
	for (int i = 0; i < 3000; ++i) if ((i * 17) % 97 - 50 >= -7 && (i * 17) % 97 - 50 < 18 && (i * 19) % 101 - 40 >= 5 && (i * 19) % 101 - 40 < 23) answer += i;
	for (int i = 0; i < 300; ++i) radix.add_query(-7, 5, 18, 23);
	for (auto value : radix.solve()) require(value == answer);
}
