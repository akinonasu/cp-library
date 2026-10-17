#include "../../components/static_range_min.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

void require(bool condition) { if (!condition) std::abort(); }

template <class T>
void check(const std::vector<T> &values) {
	StaticRangeMin range(values);
	require(range.size() == int(values.size()) && range.empty() == values.empty());
	for (int l = 0; l < range.size(); ++l) {
		require(!(range[l] < values[l]) && !(values[l] < range[l]));
		T expected = values[l];
		for (int r = l + 1; r <= range.size(); ++r) {
			if (values[r - 1] < expected) expected = values[r - 1];
			T actual = range.prod(l, r);
			require(!(actual < expected) && !(expected < actual));
		}
	}
	if (!values.empty()) {
		T expected = *std::min_element(values.begin(), values.end());
		T actual = range.all_prod();
		require(!(actual < expected) && !(expected < actual));
	}
	auto copy = range;
	auto moved = std::move(copy);
	require(moved.size() == range.size());
	copy.build(values);
	require(copy.size() == range.size());
	range.build({});
	require(range.empty() && range.size() == 0);
	range = moved;
	require(range.size() == int(values.size()));
}

struct Value {
	int value;
	explicit Value(int value) : value(value) {}
	friend bool operator<(const Value &a, const Value &b) { return a.value < b.value; }
};

int main() {
	std::mt19937 rng(37192);
	for (int n : {0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 64, 65, 95, 96, 97, 127, 128, 129, 1023}) {
		for (int kind = 0; kind < 5; ++kind) {
			std::vector<int> a(n);
			for (int i = 0; i < n; ++i) a[i] = kind == 0 ? i : kind == 1 ? -i : kind == 2 ? 7 : kind == 3 ? int(rng() % 5) : int(rng());
			check(a);
		}
	}
	check<std::uint64_t>({0, 1, std::numeric_limits<std::uint64_t>::max(), 0});
	check<std::string>({"z", "apple", "apple", "banana", ""});
	std::vector<bool> bits(130);
	for (int i = 0; i < int(bits.size()); ++i) bits[i] = rng() & 1;
	check(bits);
	std::vector<Value> values;
	for (int i = 0; i < 100; ++i) values.emplace_back(int(rng() % 7));
	check(values);
	StaticRangeMin<int> empty;
	require(empty.empty());
}
