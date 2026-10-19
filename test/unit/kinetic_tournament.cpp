#include "data_structure/segment_tree/kinetic_tournament.hpp"
#include <random>

using I = __int128_t;

template <bool Maximize = false, class Tree>
void equal(const Tree &tree, const std::vector<std::optional<typename Tree::Line>> &lines, int l, int r) {
	int index = -1;
	I value = 0;
	for (int i = l; i < r; ++i) if (lines[i]) {
		I current = I(lines[i]->a) * tree.time() + lines[i]->b;
		if (index == -1 || (Maximize ? current > value : current < value)) index = i, value = current;
	}
	auto actual = tree.prod(l, r);
	assert(bool(actual) == (index != -1));
	if (actual) assert(actual->index == index && actual->value == value);
}

template <class T, bool Maximize, class R>
void random_test(std::uint64_t seed) {
	using Tree = KineticTournament<T, Maximize, R>;
	std::mt19937_64 rng(seed);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 100}) {
		Tree tree(n, T(-100));
		std::vector<std::optional<typename Tree::Line>> lines(n);
		for (int step = 0; step < 2000; ++step) {
			int kind = rng() % 7;
			if (kind <= 1 && n) {
				int i = rng() % n;
				T a = T(rng() % 201) - 100, b = T(rng() % 401) - 200;
				lines[i] = typename Tree::Line{a, b};
				tree.set(i, a, b);
			} else if (kind == 2 && n) {
				int i = rng() % n;
				assert(tree.erase(i) == lines[i].has_value());
				lines[i].reset();
			} else if (kind == 3) tree.advance(T(tree.time() + rng() % 5));
			else if (kind == 4 && tree.next_event()) tree.advance(*tree.next_event());
			else if (kind == 5 && step % 71 == 0) {
				tree.clear();
				for (auto &line : lines) line.reset();
			}
			int count = 0;
			for (int i = 0; i < n; ++i) {
				count += lines[i].has_value();
				auto line = tree.get(i);
				assert(bool(line) == bool(lines[i]));
				if (line) assert(line->a == lines[i]->a && line->b == lines[i]->b);
			}
			assert(tree.size() == n && tree.active_size() == count && tree.empty() == (count == 0));
			if (tree.next_event()) assert(*tree.next_event() > tree.time());
			for (int trial = 0; trial < 3; ++trial) {
				int l = rng() % (n + 1), r = rng() % (n + 1);
				if (l > r) std::swap(l, r);
				int index = -1;
				I value = 0;
				for (int i = l; i < r; ++i) if (lines[i]) {
					I candidate = I(lines[i]->a) * tree.time() + lines[i]->b;
					if (index == -1 || (Maximize ? candidate > value : candidate < value)) index = i, value = candidate;
				}
				auto actual = tree.prod(l, r);
				assert(bool(actual) == (index != -1));
				if (actual) assert(actual->index == index && I(actual->value) == value);
			}
			auto a = tree.prod(0, n), b = tree.all_prod();
			assert(bool(a) == bool(b));
			if (a) assert(a->index == b->index && a->value == b->value);
		}
		auto copy = tree;
		auto moved = Tree(std::move(copy));
		assert(copy.size() == 0 && copy.empty() && !copy.all_prod() && !copy.next_event());
		copy = std::move(moved);
		assert(moved.empty() && moved.size() == 0 && copy.active_size() == tree.active_size());
		copy = tree;
		copy.reset(1, -1000);
		assert(copy.size() == 1 && copy.empty() && copy.time() == -1000);
		copy.build(std::vector<typename Tree::Line>{{1, 2}, {-3, 4}}, -2000);
		assert(copy.size() == 2 && copy.active_size() == 2 && copy.time() == -2000);
	}
}

int main() {
	using Tree = KineticTournament<>;
	for (int a = -2; a <= 2; ++a) for (int b = -2; b <= 2; ++b)
		for (int c = -2; c <= 2; ++c) for (int d = -2; d <= 2; ++d) {
			std::vector<Tree::Line> values{{a, b}, {c, d}};
			Tree tree(values, -5);
			std::vector<std::optional<Tree::Line>> lines{values[0], values[1]};
			for (int t = -5; t <= 5; ++t) {
				tree.advance(t);
				equal(tree, lines, 0, 2);
				auto event = tree.next_event();
				int now = tree.all_prod()->index;
				std::optional<std::int64_t> expected;
				for (int x = t + 1; x <= 20; ++x) {
					I p = I(a) * x + b, q = I(c) * x + d;
					if ((q < p ? 1 : 0) != now) { expected = x; break; }
				}
				assert(event == expected);
			}
		}
	constexpr auto lo = std::numeric_limits<std::int64_t>::min(), hi = std::numeric_limits<std::int64_t>::max();
	std::vector<std::int64_t> ends{lo, lo + 1, -2, -1, 0, 1, 2, hi - 1, hi};
	for (auto a : ends) for (auto b : ends) for (auto c : ends) for (auto d : ends) {
		std::vector<Tree::Line> values{{a, b}, {c, d}};
		Tree tree(values, lo);
		std::vector<std::optional<Tree::Line>> lines{values[0], values[1]};
		for (auto t : ends) { tree.advance(t); equal(tree, lines, 0, 2); }
		using Max = KineticTournament<std::int64_t, true>;
		std::vector<Max::Line> max_values{{a, b}, {c, d}};
		Max max_tree(max_values, lo);
		std::vector<std::optional<Max::Line>> max_lines{max_values[0], max_values[1]};
		for (auto t : ends) { max_tree.advance(t); equal<true>(max_tree, max_lines, 0, 2); }
	}
	Tree last(std::vector<Tree::Line>{{-1, hi}, {0, 0}});
	assert(last.next_event() == hi);
	last.advance(hi);
	assert(last.all_prod()->index == 0 && last.all_prod()->value == 0 && !last.next_event());
	Tree first(std::vector<Tree::Line>{{0, 0}, {1, hi}}, lo);
	assert(first.next_event() == lo + 1);
	first.advance(lo + 1);
	assert(first.all_prod()->index == 0 && !first.next_event());
	Tree wide(std::vector<Tree::Line>{{lo, hi}, {hi, lo}}, lo);
	assert(wide.next_event() == 1);
	wide.advance(1);
	assert(wide.all_prod()->index == 0 && wide.all_prod()->value == -1);
	using Narrow = KineticTournament<int>;
	Narrow narrow(std::vector<Narrow::Line>{{-1, std::numeric_limits<int>::max()}, {0, 0}});
	assert(narrow.next_event() == std::numeric_limits<int>::max());
	narrow.advance(std::numeric_limits<int>::max());
	assert(narrow.all_prod()->index == 0 && !narrow.next_event());
	KineticTournament<std::int64_t, false, std::int64_t> cancellation(2, 2);
	cancellation.set(0, hi, -hi);
	cancellation.set(1, 0, hi);
	assert(cancellation.all_prod()->index == 0 && cancellation.all_prod()->value == hi);
	cancellation.erase(0);
	cancellation.advance(3);
	assert(cancellation.all_prod()->value == hi);
	KineticTournament<std::int64_t, false, int> small_result(2, 1);
	small_result.set(0, -2000000000LL, 3000000000LL);
	small_result.set(1, 0, 2000000000LL);
	assert(small_result.all_prod()->index == 0 && small_result.all_prod()->value == 1000000000);
	random_test<std::int64_t, false, I>(20261020);
	random_test<std::int64_t, true, I>(20261021);
	random_test<int, false, std::int64_t>(20261022);
	random_test<int, true, std::int64_t>(20261023);
	random_test<std::int64_t, false, std::int64_t>(20261024);
	std::vector<Tree::Line> values;
	for (int i = 0; i < 200000; ++i) values.push_back({-2 * i, std::int64_t(i) * i});
	Tree large(values, 0);
	large.advance(199999);
	assert(large.all_prod()->index == 199999);
	large.clear();
	assert(large.size() == 200000 && large.empty() && !large.next_event());
}
