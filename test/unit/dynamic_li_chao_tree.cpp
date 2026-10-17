#include "data_structure/dynamic_li_chao_tree.hpp"
#include <array>
#include <random>

template<class T, bool maximize> void random_test() {
	using Tree = DynamicLiChaoTree<T, maximize>;
	using B = typename Tree::bound_type;
	using R = typename Tree::value_type;
	struct Segment { B l, r; T a, b; };
	std::mt19937_64 rng(731);
	for (int trial = 0; trial < 140; ++trial) {
		int low = int(rng() % 61) - 60, high = low + 1 + rng() % 100;
		Tree tree(low, high);
		assert(tree.empty() && tree.lower() == low && tree.upper() == high && tree.node_count() == 0);
		std::vector<Segment> lines;
		for (int step = 0; step < 250; ++step) {
			int kind = rng() % 20;
			if (!kind) tree.clear(), lines.clear();
			else {
				T a = T(int(rng() % 61) - 30), b = T(int(rng() % 101) - 50);
				B l = int(rng() % 241) - 120, r = int(rng() % 241) - 120;
				if (l > r) std::swap(l, r);
				if (kind < 6) {
					auto before = tree.node_count();
					tree.add_line(a, b), l = low, r = high;
					assert(tree.node_count() <= before + 1);
				} else tree.add_segment(l, r, a, b);
				lines.push_back({l, r, a, b});
			}
			bool nonempty = false;
			for (const auto& f : lines) nonempty |= std::max(B(low), f.l) < std::min(B(high), f.r);
			assert(tree.empty() == !nonempty);
			for (int repeat = 0; repeat < 4; ++repeat) {
				T x = T(low + rng() % (high - low));
				std::optional<R> expected;
				for (const auto& f : lines) if (f.l <= B(x) && B(x) < f.r) {
					R value = R(f.a) * R(x) + R(f.b);
					if (!expected || (maximize ? value > *expected : value < *expected)) expected = value;
				}
				assert(tree.query(x) == expected);
			}
		}
		tree.build(-1, 2);
		assert(tree.empty() && tree.node_count() == 0 && tree.lower() == -1 && tree.upper() == 2);
		tree.add_segment(-100, 0, 3, 4);
		assert(tree.query(-1) == R(1) && !tree.query(0) && !tree.query(1));
		auto copy = tree;
		tree.clear();
		assert(!tree.query(-1) && copy.query(-1) == R(1));
		auto moved = std::move(copy);
		assert(moved.query(-1) == R(1));
	}
}

template<bool maximize> void extreme_test() {
	using T = std::int64_t;
	using R = __int128_t;
	constexpr T low = std::numeric_limits<T>::lowest(), high = std::numeric_limits<T>::max();
	std::array<T, 9> xs{low, low + 1, -1000000000, -1, 0, 1, 1000000000, high - 1, high};
	DynamicLiChaoTree<T, maximize> tree;
	assert(tree.lower() == R(low) && tree.upper() == R(high) + 1);
	for (T x : xs) assert(!tree.query(x));
	std::vector<std::pair<T, T>> lines;
	for (T a : {low, low + 1, T(-1), T(0), T(1), high - 1, high})
		for (T b : {low, low + 1, T(-1), T(0), T(1), high - 1, high}) {
			tree.add_line(a, b), lines.emplace_back(a, b);
			for (T x : xs) {
				std::optional<R> expected;
				for (auto [c, d] : lines) {
					R value = R(c) * R(x) + R(d);
					if (!expected || (maximize ? value > *expected : value < *expected)) expected = value;
				}
				assert(tree.query(x) == expected);
			}
		}
	for (T x : xs) {
		tree.clear();
		tree.add_segment(R(x), R(x) + 1, high, low);
		assert(tree.query(x) == R(high) * R(x) + R(low));
		for (T y : xs) if (y != x) assert(!tree.query(y));
	}
	tree.clear();
	tree.add_segment(2, 3, high, low);
	assert(tree.query(2) == R(high) - 1 && !tree.query(1) && !tree.query(3));
	tree.add_segment(R(high) + 1, R(high) + 100, low, high);
	tree.add_segment(R(low) - 100, R(low), high, low);
	assert(tree.query(2) == R(high) - 1);
}

int main() {
	random_test<std::int8_t, false>();
	random_test<std::int16_t, true>();
	random_test<int, false>();
	random_test<int, true>();
	random_test<std::int64_t, false>();
	random_test<std::int64_t, true>();
	extreme_test<false>(), extreme_test<true>();
	DynamicLiChaoTree<> same(-10, 11);
	same.reserve(1);
	same.add_line(3, 4);
	for (int i = 0; i < 1000; ++i) same.add_line(3, 5), same.add_line(3, -i);
	assert(same.node_count() == 1 && same.query(0) == -999);
	DynamicLiChaoTree<> singleton(7, 8);
	singleton.add_line(3, 4), singleton.add_segment(7, 8, -3, 2);
	assert(singleton.query(7) == -19);
	DynamicLiChaoTree<std::int64_t, false, std::int64_t> narrow(-100, 101);
	narrow.add_line(7, -9);
	assert(narrow.query(-100) == -709 && narrow.query(100) == 691);
	constexpr int n = 30000;
	DynamicLiChaoTree<> hull(0, n);
	hull.reserve(n);
	for (int i = 0; i < n; ++i) hull.add_line(-2LL * i, 1LL * i * i);
	for (int i = 0; i < n; ++i) assert(hull.query(i) == -__int128_t(i) * i);
	assert(hull.node_count() <= n);
}
