#include "graph/difference_constraints.hpp"
#include <array>
#include <functional>
#include <random>

using Wide = __int128_t;
using System = DifferenceConstraints<>;
constexpr Wide infinity = Wide(1) << 120;

template<class Cost>
void check(const DifferenceConstraints<Cost> &system) {
	int n = system.size(), origin = system.origin();
	std::vector<std::vector<Wide>> d(n + 1, std::vector<Wide>(n + 1, infinity));
	for (int v = 0; v <= n; ++v) d[v][v] = 0;
	for (auto edge : system.constraints()) d[edge.from][edge.to] = std::min(d[edge.from][edge.to], Wide(edge.weight));
	for (int k = 0; k <= n; ++k) for (int u = 0; u <= n; ++u) for (int v = 0; v <= n; ++v)
		if (d[u][k] != infinity && d[k][v] != infinity) d[u][v] = std::min(d[u][v], d[u][k] + d[k][v]);
	bool feasible = true;
	for (int v = 0; v <= n; ++v) feasible &= d[v][v] >= 0;
	auto verify = [&](const auto &result) {
		assert(result.size() == n && result.origin() == origin && result.feasible() == feasible);
		if (!feasible) {
			const auto &cycle = result.negative_cycle();
			assert(!cycle.empty());
			Wide sum = 0;
			for (std::size_t i = 0; i < cycle.size(); ++i) {
				auto edge = cycle[i];
				assert(edge.to == cycle[(i + 1) % cycle.size()].from);
				bool present = false;
				for (auto input : system.constraints()) present |= input.from == edge.from && input.to == edge.to && input.weight == edge.weight;
				assert(present);
				sum += edge.weight;
			}
			assert(sum < 0);
			return;
		}
		assert(result.negative_cycle().empty() && result.values().size() == std::size_t(n));
		assert(result.value(origin) == 0);
		for (auto edge : system.constraints()) assert(Wide(result.value(edge.to)) - Wide(result.value(edge.from)) <= edge.weight);
		for (int source = 0; source <= n; ++source) {
			auto bounds = result.upper_bounds(source);
			assert(bounds.size() == std::size_t(n));
			for (int v = 0; v <= n; ++v) {
				auto maximum = result.max_difference(source, v), minimum = result.min_difference(source, v);
				assert(bool(maximum) == (d[source][v] != infinity));
				assert(bool(minimum) == (d[v][source] != infinity));
				if (maximum) assert(*maximum == d[source][v]);
				if (minimum) assert(*minimum == -d[v][source]);
				if (v < n) assert(bounds[v] == maximum);
			}
		}
		assert(result.upper_bounds() == result.upper_bounds(origin));
		for (int v = 0; v <= n; ++v) {
			assert(result.upper_bound(v) == result.max_difference(origin, v));
			assert(result.lower_bound(v) == result.min_difference(origin, v));
		}
	};
	verify(system.solve());
	DifferenceConstraints<Wide> wide(n);
	for (auto edge : system.constraints()) wide.add_le(edge.from, edge.to, edge.weight);
	verify(wide.solve());
}

void conversions() {
	for (int operation = 0; operation < 10; ++operation) for (int u = 0; u <= 2; ++u)
		for (int v = 0; v <= 2; ++v) for (int c = -3; c <= 3; ++c) {
			System system(2);
			std::function<bool(Wide, Wide)> predicate;
			switch (operation) {
				case 0: system.add_le(u, v, c); predicate = [=](Wide a, Wide b) { return b - a <= c; }; break;
				case 1: system.add_ge(u, v, c); predicate = [=](Wide a, Wide b) { return b - a >= c; }; break;
				case 2: system.add_lt(u, v, c); predicate = [=](Wide a, Wide b) { return b - a < c; }; break;
				case 3: system.add_gt(u, v, c); predicate = [=](Wide a, Wide b) { return b - a > c; }; break;
				case 4: system.add_eq(u, v, c); predicate = [=](Wide a, Wide b) { return b - a == c; }; break;
				case 5: system.add_range(u, v, c, 2); predicate = [=](Wide a, Wide b) { return c <= b - a && b - a <= 2; }; break;
				case 6: system.add_upper_bound(v, c); predicate = [=](Wide, Wide b) { return b <= c; }; break;
				case 7: system.add_lower_bound(v, c); predicate = [=](Wide, Wide b) { return b >= c; }; break;
				case 8: system.fix(v, c); predicate = [=](Wide, Wide b) { return b == c; }; break;
				default: system.add_range(v, c, 2); predicate = [=](Wide, Wide b) { return c <= b && b <= 2; };
			}
			for (int a = -5; a <= 5; ++a) for (int b = -5; b <= 5; ++b) {
				std::array<Wide, 3> x{a, b, 0};
				bool valid = true;
				for (auto edge : system.constraints()) valid &= x[edge.to] - x[edge.from] <= edge.weight;
				assert(valid == predicate(x[u], x[v]));
			}
			check(system);
		}
}

int main() {
	for (int n = 0; n <= 3; ++n) {
		int vertices = n == 3 ? 3 : n + 1, count = 1;
		for (int i = 0; i < vertices * vertices; ++i) count *= 3;
		for (int mask = 0; mask < count; ++mask) {
			System system(n);
			int state = mask;
			for (int u = 0; u < vertices; ++u) for (int v = 0; v < vertices; ++v) {
				int choice = state % 3;
				state /= 3;
				if (choice) system.add_le(u, v, choice == 1 ? -1 : 1);
			}
			check(system);
		}
	}
	conversions();
	std::mt19937_64 random(20261022);
	for (int trial = 0; trial < 1000; ++trial) {
		int n = 1 + random() % 9, m = random() % 40;
		System system(n);
		system.reserve(m * 2);
		for (int i = 0; i < m; ++i) {
			int u = random() % (n + 1), v = random() % (n + 1), c = int(random() % 21) - 10;
			switch (random() % 7) {
				case 0: system.add_le(u, v, c); break;
				case 1: system.add_ge(u, v, c); break;
				case 2: system.add_lt(u, v, c); break;
				case 3: system.add_gt(u, v, c); break;
				case 4: system.add_eq(u, v, c); break;
				case 5: system.fix(v, c); break;
				default: system.add_range(u, v, c, int(random() % 21) - 10);
			}
		}
		check(system);
	}
	for (int trial = 0; trial < 500; ++trial) {
		System system(2);
		int m = random() % 10;
		for (int i = 0; i < m; ++i) system.add_le(random() % 3, random() % 3, int(random() % 7) - 3);
		auto result = system.solve();
		bool found = false;
		std::array<std::array<Wide, 3>, 3> maximum;
		for (auto &row : maximum) row.fill(-infinity);
		for (int a = -12; a <= 12; ++a) for (int b = -12; b <= 12; ++b) {
			std::array<Wide, 3> x{a, b, 0};
			bool valid = true;
			for (auto edge : system.constraints()) valid &= x[edge.to] - x[edge.from] <= edge.weight;
			if (!valid) continue;
			found = true;
			for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) maximum[u][v] = std::max(maximum[u][v], x[v] - x[u]);
		}
		assert(result.feasible() == found);
		if (found) for (int u = 0; u < 3; ++u) for (int v = 0; v < 3; ++v) {
			auto upper = result.max_difference(u, v);
			if (upper) assert(*upper == maximum[u][v]);
		}
	}
	constexpr std::int64_t low = std::numeric_limits<std::int64_t>::min(), high = std::numeric_limits<std::int64_t>::max();
	DifferenceConstraints<Wide> repeated_negative(1);
	for (int i = 0; i < 32; ++i) repeated_negative.add_le(0, 0, -high / 8);
	check(repeated_negative);
	for (int n = 1; n <= 9; ++n) for (int delta = -1; delta <= 1; ++delta) {
		std::int64_t cost = high / (n + 1) + delta;
		System positive(n), negative(n);
		positive.fix(0, cost);
		negative.fix(0, -cost);
		for (int v = 1; v < n; ++v) {
			positive.add_eq(v - 1, v, cost);
			negative.add_eq(v - 1, v, -cost);
		}
		check(positive);
		check(negative);
		assert(positive.solve().upper_bound(n - 1) == Wide(cost) * n);
		assert(negative.solve().lower_bound(n - 1) == -Wide(cost) * n);
	}
	for (auto c : {low, low + 1, std::int64_t(-1), std::int64_t(0), high - 1, high}) {
		DifferenceConstraints<Wide> system(2);
		system.fix(0, c);
		system.add_eq(0, 1, c);
		check(system);
		auto result = system.solve();
		assert(result.value(0) == Wide(c) && result.value(1) == Wide(c) * 2);
		DifferenceConstraints<Wide> less(2), greater(2);
		less.fix(0, 0);
		less.add_lt(0, 1, c);
		greater.fix(0, 0);
		greater.add_gt(0, 1, c);
		check(less);
		check(greater);
		assert(less.solve().upper_bound(1) == Wide(c) - 1);
		assert(greater.solve().lower_bound(1) == Wide(c) + 1);
		less.add_ge(0, 1, c);
		greater.add_le(0, 1, c);
		check(less);
		check(greater);
	}
	auto overflow = [](auto operation) {
		bool caught = false;
		try { operation(); } catch (const std::overflow_error &) { caught = true; }
		assert(caught);
	};
	for (Wide cost : {Wide(1) << 100, -(Wide(1) << 100)}) {
		DifferenceConstraints<Wide> large(2);
		large.fix(0, cost);
		large.add_eq(0, 1, cost);
		check(large);
		assert(large.solve().value(1) == cost * 2);
	}
	DifferenceConstraints<Wide> wide_conversion(2);
	Wide wide_low = std::numeric_limits<Wide>::lowest();
	overflow([&] { wide_conversion.add_ge(0, 1, wide_low); });
	overflow([&] { wide_conversion.add_lt(0, 1, wide_low); });
	assert(wide_conversion.constraint_count() == 0);
	System cancellation(2);
	cancellation.fix(0, high - 1);
	cancellation.fix(1, high - 1);
	cancellation.add_le(0, 1, high - 1);
	check(cancellation);
	System conversion(2);
	overflow([&] { conversion.add_ge(0, 1, low); });
	overflow([&] { conversion.add_lt(0, 1, low); });
	overflow([&] { conversion.add_eq(0, 1, low); });
	overflow([&] { conversion.add_range(0, 1, low, 0); });
	assert(conversion.constraint_count() == 0);
	System excessive(1);
	for (int i = 0; i < 32; ++i) excessive.add_le(0, 0, -high / 8);
	overflow([&] { excessive.solve(); });
	System long_distance(3);
	long_distance.add_le(0, 1, high / 2 + 1);
	long_distance.add_le(1, 2, high / 2 + 1);
	auto long_result = long_distance.solve();
	overflow([&] { long_result.max_difference(0, 2); });
	overflow([&] { long_result.upper_bounds(0); });
	System lowest(2);
	lowest.add_le(0, 1, low);
	auto lowest_result = lowest.solve();
	assert(lowest_result.max_difference(0, 1) == low);
	overflow([&] { lowest_result.min_difference(1, 0); });
	System normalization(2);
	normalization.fix(0, high);
	normalization.add_eq(0, 1, high);
	overflow([&] { normalization.solve(); });
	System reduced(2);
	reduced.add_le(0, 1, high);
	overflow([&] { reduced.solve(); });
	DifferenceConstraints<Wide> wide_distance(3);
	wide_distance.add_le(0, 1, high / 2 + 1);
	wide_distance.add_le(1, 2, high / 2 + 1);
	assert(wide_distance.solve().max_difference(0, 2) == Wide(high) + 1);
	static_assert(std::is_same_v<System::value_type, std::int64_t>);
	static_assert(std::is_same_v<DifferenceConstraints<Wide>::value_type, Wide>);

	DifferenceConstraints<int> small(2);
	small.add_lt(0, 1, std::numeric_limits<int>::min());
	check(small);
	DifferenceConstraints<std::int8_t> tiny(2);
	tiny.add_gt(0, 1, std::numeric_limits<std::int8_t>::max());
	check(tiny);
	System snapshot(3);
	assert(snapshot.add_le(0, 1, 5) == 0);
	assert(snapshot.add_ge(0, 1, -2) == 1);
	assert(snapshot.constraint_count() == 2);
	auto saved = snapshot.solve(), copy = saved;
	snapshot.add_lt(2, 2, 0);
	assert(!snapshot.solve().feasible() && saved.feasible());
	assert(saved.max_difference(0, 1) == 5 && saved.min_difference(0, 1) == -2);
	assert(!saved.upper_bound(0) && !saved.lower_bound(0) && !saved.max_difference(1, 2));
	auto moved = std::move(copy);
	assert(moved.values() == saved.values() && moved.upper_bounds(0) == saved.upper_bounds(0));
	assert(copy.size() == 0 && copy.feasible() && copy.upper_bounds().empty() && copy.max_difference(0, 0) == 0);
	copy = snapshot.solve();
	assert(!copy.feasible());
	copy = saved;
	assert(copy.feasible() && copy.upper_bounds(0) == saved.upper_bounds(0));
	const int n = 200000;
	System chain(n);
	chain.reserve(n + 1);
	chain.fix(0, 0);
	for (int v = 1; v < n; ++v) chain.add_le(v - 1, v, -1);
	auto result = chain.solve();
	assert(result.feasible() && result.upper_bound(n - 1) == -(n - 1) && !result.lower_bound(n - 1));
	auto bounds = result.upper_bounds(0);
	for (int v = 0; v < n; ++v) assert(bounds[v] == -v);
}
