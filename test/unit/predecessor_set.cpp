#include "components/predecessor_set.hpp"
#include <climits>
#include <cstdlib>
#include <random>
#include <set>

void require(bool ok) { if (!ok) std::abort(); }

void check(const PredecessorSet &set, const std::set<int> &oracle, int p) {
	require(set.size() == int(oracle.size()) && set.empty() == oracle.empty());
	auto it = oracle.lower_bound(p);
	require(set.next(p) == (it == oracle.end() ? set.universe_size() : *it));
	it = oracle.upper_bound(p);
	require(set.prev(p) == (it == oracle.begin() ? -1 : *--it));
}

int main() {
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
