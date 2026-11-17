#include "components/fenwick_sum.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

struct AddOnly {
	int value = 0;
	AddOnly &operator+=(const AddOnly &x) { value += x.value; return *this; }
	AddOnly &operator-=(const AddOnly &x) { value -= x.value; return *this; }
};

template class FenwickSum<AddOnly>;

template<class T>
concept HasSearch = requires(const FenwickSum<T> &tree, T value) { tree.lower_bound(value); };

static_assert(HasSearch<int> && !HasSearch<AddOnly>);

template<class T>
void check(std::vector<T> values, std::mt19937_64 &random) {
	FenwickSum<T> tree(values);
	for (int repeat = 0; repeat < 200; ++repeat) {
		if (!values.empty()) {
			int p = random() % values.size();
			T next = random() % 7;
			tree.add(p, next - values[p]);
			values[p] = next;
		}
		T total{};
		for (T value : values) total += value;
		for (T target = -1; target <= total + 1; ++target) {
			T sum{};
			int expected = 0;
			if (target > 0) {
				while (expected < int(values.size()) && sum + values[expected] < target) sum += values[expected++];
			}
			assert(std::as_const(tree).lower_bound(target) == expected);
		}
	}
}

int main() {
	FenwickSum<AddOnly> sums(std::vector<AddOnly>{{3}, {1}});
	sums.add(0, {2});
	assert(sums.prod(0, 2).value == 6);
	std::mt19937_64 random(7638619);
	for (int n : {0, 1, 2, 3, 7, 8, 15, 16, 17, 31, 32, 63, 64, 65}) {
		check(std::vector<int>(n), random);
		check(std::vector<std::int64_t>(n), random);
	}
	const auto maximum = std::numeric_limits<std::int64_t>::max();
	FenwickSum<std::int64_t> tree({0, maximum - 1, 0, 1, 0});
	assert(tree.lower_bound(maximum - 1) == 1);
	assert(tree.lower_bound(maximum) == 3);
}
