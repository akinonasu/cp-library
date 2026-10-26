#include "components/predecessor_set.hpp"
#include <climits>
#include <cstdlib>
#include <random>
#include <set>
#include <string>

void require(bool ok) { if (!ok) std::abort(); }

void check(const PredecessorSet &set, const std::set<int> &oracle, int p) {
	require(set.size() == int(oracle.size()) && set.empty() == oracle.empty());
	auto it = oracle.lower_bound(p);
	require(set.next(p) == (it == oracle.end() ? set.universe_size() : *it));
	it = oracle.upper_bound(p);
	require(set.prev(p) == (it == oracle.begin() ? -1 : *--it));
}

void check_small(const PredecessorSet &set, const std::set<int> &oracle) {
	for (int p = -1; p <= set.universe_size(); ++p) check(set, oracle, p);
	check(set, oracle, INT_MIN);
	check(set, oracle, INT_MAX);
	require(set.to_vector() == std::vector<int>(oracle.begin(), oracle.end()));
}

void check_small_transitions() {
	const std::set<int> original{0, 1, 63, 64, 65, 127, 128, 129, 255, 256, 257, 511, 512, 1023, 1024, 4095, 4096};
	std::string bits(4097, '0');
	for (int p : original) bits[p] = '1';
	PredecessorSet base(bits);
	for (int removed : original) {
		auto set = base;
		auto oracle = original;
		require(set.set(removed, false)); oracle.erase(removed);
		require(!set.erase(removed) && !set.set(removed, false));
		require(!set.erase(2048) && !set.set(2048, false));
		check_small(set, oracle);
		for (int p : oracle) require(!set.insert(p) && !set.set(p, true));
		require(set.insert(removed)); oracle.insert(removed);
		check_small(set, oracle);
		require(set.erase(removed)); oracle.erase(removed);
		set.assign(set.universe_size(), set.words());
		check_small(set, oracle);
		PredecessorSet moved(std::move(set));
		check_small(set, {});
		check_small(moved, oracle);
		set = base;
		set.swap(moved);
		check_small(set, oracle);
		check_small(moved, original);
		moved = std::move(set);
		check_small(set, {});
		check_small(moved, oracle);
		moved.assign(bits);
		check_small(moved, original);
	}
	for (int n : {0, 1, 15, 16, 17, 18}) {
		PredecessorSet set(n, true);
		std::set<int> oracle;
		for (int p = 0; p < n; ++p) oracle.insert(p);
		check_small(set, oracle);
		set.clear(); check_small(set, {});
		set.fill(); check_small(set, oracle);
		for (int p = 0; p < n; ++p) {
			require(set.erase(p)); oracle.erase(p);
			check_small(set, oracle);
		}
		for (int p = n - 1; p >= 0; --p) {
			require(set.set(p, true)); oracle.insert(p);
			check_small(set, oracle);
		}
		set.assign(4097); check_small(set, {});
	}
}

int main() {
	check_small_transitions();
	for (int n = 0; n <= 8; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		PredecessorSet set(n);
		std::set<int> oracle;
		for (int i = 0; i < n; ++i) if (mask >> i & 1) require(set.insert(i)), oracle.insert(i);
		for (int p = -2; p <= n + 2; ++p) check(set, oracle, p);
		for (int i = 0; i < n; ++i) require(set.contains(i) == bool(mask >> i & 1));
	}
	std::mt19937 rng(393919);
	for (int n : {0, 1, 63, 64, 65, 4095, 4096, 4097, 262143, 262144, 262145}) {
		PredecessorSet set(n, true);
		require(set.universe_size() == n && set.size() == n);
		for (int i = 0; i < n; ++i) require(set.contains(i) && set.next(i) == i && set.prev(i) == i);
		set.clear(); require(set.empty() && set.next(INT_MIN) == n && set.prev(INT_MAX) == -1);
		std::set<int> oracle;
		for (int i = 0; i < 30000 && n; ++i) {
			int p = rng() % n;
			if (rng() & 1) require(set.insert(p) == oracle.insert(p).second);
			else require(set.erase(p) == bool(oracle.erase(p)));
			check(set, oracle, p);
			check(set, oracle, int(rng() % (n + 4)) - 2);
		}
		auto copy = set;
		PredecessorSet moved(std::move(copy));
		require(copy.empty() && !copy.universe_size() && copy.next(0) == 0);
		copy = moved;
		moved = std::move(copy);
		require(copy.empty() && !copy.universe_size());
		for (int p : {INT_MIN, -1, 0, n / 2, n, INT_MAX}) check(moved, oracle, p);
		moved.assign(n, true); require(moved.size() == n);
		moved.assign(0); require(moved.empty() && !moved.universe_size());
	}
}
