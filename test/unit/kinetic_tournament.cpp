#include "data_structure/advanced/kinetic_tournament.hpp"
#include <iostream>
#include <random>

std::int64_t sequence_score(const std::vector<std::int64_t> &a) {
	if (a.empty()) return 0;
	auto slopes = a;
	std::sort(slopes.begin(), slopes.end());
	slopes.erase(std::unique(slopes.begin(), slopes.end()), slopes.end());
	slopes.push_back(slopes.back() + 1);
	int n = static_cast<int>(slopes.size());
	KineticTournament<> tree(slopes, std::nullopt);
	tree.set(n - 1, 0);
	std::int64_t prefix = slopes.back();
	for (auto x : a) {
		if (x < prefix) {
			tree.add(0, n, x);
			int i = std::lower_bound(slopes.begin(), slopes.end(), prefix) - slopes.begin();
			tree.set(i, tree.all_prod()->value);
			prefix = x;
		} else {
			int k = std::upper_bound(slopes.begin(), slopes.end(), x) - slopes.begin();
			auto next = tree.prod(k, n);
			tree.advance(0, k);
			tree.add(k, n, prefix);
			if (next) {
				int i = k - 1;
				auto previous = tree.get(i);
				tree.set(i, previous ? std::max(*previous, next->value + x) : next->value + x);
			}
		}
	}
	return tree.all_prod()->value;
}

template<class T, bool Max> void check(KineticTournament<T, Max> &tree, const std::vector<std::optional<T>> &values) {
	int n = values.size(), active = 0;
	for (auto value : values) active += value.has_value();
	assert(tree.size() == n && tree.active_size() == active && tree.empty() == (active == 0));
	for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
		int index = -1;
		for (int i = l; i < r; ++i) if (values[i]) {
			if (index == -1 || (Max ? *values[i] >= *values[index] : *values[i] < *values[index])) index = i;
		}
		auto actual = tree.prod(l, r);
		assert(actual.has_value() == (index != -1));
		if (actual) assert(actual->index == index && actual->value == *values[index]);
	}
	auto result = tree.all_prod(), all = tree.prod(0, n);
	assert(bool(result) == bool(all));
	if (result) assert(result->index == all->index && result->value == all->value);
	assert(tree.values() == values);
	for (int i = 0; i < n; ++i) assert(tree.get(i) == values[i]);
}

template<class T, bool Max> void random() {
	std::mt19937_64 rng(278456 + Max);
	for (int trial = 0; trial < 80; ++trial) {
		int n = rng() % 35;
		std::vector<T> slopes(n);
		std::vector<std::optional<T>> values(n);
		for (auto &slope : slopes) slope = T(int(rng() % 31) - 15);
		std::sort(slopes.begin(), slopes.end());
		for (auto &value : values) if (rng() % 3) value = T(int(rng() % 201) - 100);
		KineticTournament<T, Max> tree(slopes, values);
		for (int step = 0; step < 1200; ++step) {
			int l = rng() % (n + 1), r = rng() % (n + 1), type = rng() % 10;
			if (l > r) std::swap(l, r);
			T a = rng() % 30, b = T(int(rng() % 101) - 50);
			if (type < 4) {
				if (!type) { a = 0; tree.add(l, r, b); }
				else if (type == 1) { b = 0; tree.advance(l, r, a); }
				else if (type == 2) { l = 0; r = n; tree.all_apply(a, b); }
				else tree.apply(l, r, a, b);
				for (int i = l; i < r; ++i) if (values[i]) *values[i] += T(a * slopes[i] + b);
			} else if (type == 4 && n) {
				int i = rng() % n; tree.set(i, b); values[i] = b;
			} else if (type == 5 && n) {
				int i = rng() % n; assert(tree.erase(i) == values[i].has_value()); values[i].reset();
			} else if (type == 6 && step % 100 == 0) {
				tree.clear(); std::fill(values.begin(), values.end(), std::nullopt);
			} else if (type == 7) {
				auto copy = tree;
				copy.all_apply(3, -5);
				assert(tree.values() == values);
				auto moved = std::move(copy);
				assert(copy.size() == 0 && copy.empty() && !copy.all_prod());
				copy = std::move(moved); assert(moved.size() == 0);
			}
			if (step % 100 == 0) check(tree, values);
			if (l < r) {
				auto actual = tree.prod(l, r);
				int index = -1;
				for (int i = l; i < r; ++i) if (values[i] && (index == -1 || (Max ? *values[i] >= *values[index] : *values[i] < *values[index]))) index = i;
				assert(bool(actual) == (index != -1));
				if (actual) assert(actual->index == index && actual->value == *values[index]);
			}
		}
		check(tree, values);
	}
}

std::int64_t brute(const std::vector<std::int64_t> &a) {
	std::int64_t best = 0;
	for (unsigned mask = 0; mask < (1U << a.size()); ++mask) {
		std::int64_t low[2] = {std::numeric_limits<std::int64_t>::max(), std::numeric_limits<std::int64_t>::max()}, score = 0;
		for (int i = 0; i < int(a.size()); ++i) { int side = (mask >> i) & 1; low[side] = std::min(low[side], a[i]); score += low[side]; }
		best = std::max(best, score);
	}
	return best;
}

int main() {
	random<int, true>(); random<int, false>(); random<std::int64_t, true>(); random<std::int64_t, false>();
	using T = std::int64_t;
	const T lo = std::numeric_limits<T>::lowest(), hi = std::numeric_limits<T>::max();
	for (bool minimum : {false, true}) {
		if (minimum) {
			KineticTournament<T, false> tree(std::vector<T>{lo, hi}, std::vector<T>{hi, lo});
			tree.all_apply(1); assert(tree.all_prod()->value == -1 && tree.all_prod()->index == 0);
		} else {
			KineticTournament<> wide(std::vector<T>{lo, hi}, std::vector<T>{hi, lo});
			wide.all_apply(1); assert(wide.get(0) == -1 && wide.get(1) == -1 && wide.all_prod()->index == 1);
			KineticTournament<> tree(std::vector<T>{0, 1}, std::vector<T>{hi, lo});
			tree.all_apply(hi); tree.all_apply(hi); assert(tree.all_prod()->index == 0);
			tree.all_apply(1); assert(tree.all_prod()->index == 1);
			assert(tree.get(0) == hi && tree.get(1) == hi);
		}
	}
	KineticTournament<> cancellation(std::vector<T>{hi, hi}, std::vector<T>{0, 1});
	for (int i = 0; i < 100; ++i) cancellation.all_apply(1, -hi);
	assert(cancellation.get(0) == 0 && cancellation.get(1) == 1);
	for (int n = 0; n <= 7; ++n) for (int mask = 0; mask < (1 << (2 * n)); ++mask) {
		std::vector<T> a(n);
		for (int i = 0; i < n; ++i) a[i] = 1 + ((mask >> (2 * i)) & 3);
		assert(sequence_score(a) == brute(a));
	}
	assert(sequence_score({5, 3, 6, 1, 4, 7}) == 23);
	assert(sequence_score(std::vector<T>(500000, 1000000000)) == 500000000000000LL);
	std::cout << "Kinetic tournament passed\n";
}
