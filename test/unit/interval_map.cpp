#include "data_structure/interval_map.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

template<class T> void boundaries() {
	using Map = IntervalMap<T, int>;
	using Size = typename Map::size_type;
	const T lo = (std::numeric_limits<T>::min)(), hi = (std::numeric_limits<T>::max)();
	const Size length = static_cast<Size>(hi) - static_cast<Size>(lo);
	Map map(lo, hi, 1);
	assert(map.size() == length && map.interval_count() == 1);
	assert(map.contains(lo, hi, 1) && !map.get(hi));
	map.assign(lo, T(1), 2);
	assert(map.size() == length && map.interval_count() == 2);
	assert(map.contains(lo, hi) && !map.contains(lo, hi, 2));
	assert(map.get(lo) == 2 && map.at(T(1)) == 1);
	assert(map.erase(hi) == 0 && map.erase(lo) == 1);
	assert(map.size() == length - 1 && !map.contains(lo));
	map.set(lo, 2);
	map.set(static_cast<T>(hi - 1), 1);
	assert(map.size() == length && map.interval_count() == 2);
	assert(map.count(lo, hi) == length && map.erase(lo, hi) == length);
	assert(map.empty() && map.size() == 0 && map.lower_bound(lo) == map.end());
}

void randomized() {
	using Entry = std::tuple<int, int, int>;
	std::mt19937 random(20261023);
	IntervalMap<int, int> map;
	std::array<std::optional<int>, 65> values{};
	for (int step = 0; step < 20000; ++step) {
		int l = int(random() % 66) - 32, r = int(random() % 66) - 32;
		if (l > r) std::swap(l, r);
		int value = int(random() % 7) - 3;
		unsigned operation = random() % 10;
		if (operation < 5) {
			map.assign(l, r, value);
			for (int x = l; x < r; ++x) values[x + 32] = value;
		} else if (operation < 7) {
			std::uint64_t removed = 0;
			for (int x = l; x < r; ++x) { removed += values[x + 32].has_value(); values[x + 32].reset(); }
			assert(map.erase(l, r) == removed);
		} else if (operation == 7) {
			int x = int(random() % 65) - 32;
			map.set(x, value); values[x + 32] = value;
		} else if (operation == 8) {
			int x = int(random() % 65) - 32;
			assert(map.erase(x) == std::uint64_t(values[x + 32].has_value()));
			values[x + 32].reset();
		} else if (step % 13 == 0) {
			map.clear(); values.fill(std::nullopt);
		} else {
			std::vector<IntervalMap<int, int>::interval_type> intervals;
			values.fill(std::nullopt);
			for (int j = 0; j < 5; ++j) {
				int a = int(random() % 66) - 32, b = int(random() % 66) - 32;
				if (a > b) std::swap(a, b);
				int v = int(random() % 7) - 3;
				intervals.push_back({a, b, v});
				for (int x = a; x < b; ++x) values[x + 32] = v;
			}
			map.assign(intervals);
		}
		std::vector<Entry> expected, actual, reversed;
		std::uint64_t size = 0;
		for (int x = -32; x < 33;) {
			if (!values[x + 32]) { ++x; continue; }
			int first = x, v = *values[x + 32];
			while (x < 33 && values[x + 32] == v) { ++x; ++size; }
			expected.emplace_back(first, x, v);
		}
		for (const auto &[left, segment] : map) actual.emplace_back(left, segment.right, segment.value);
		for (auto it = map.rbegin(); it != map.rend(); ++it) reversed.emplace_back(it->first, it->second.right, it->second.value);
		assert(actual == expected && reversed == std::vector<Entry>(expected.rbegin(), expected.rend()));
		assert(map.size() == size && map.interval_count() == expected.size() && map.empty() == expected.empty());
		for (int x = -34; x <= 34; ++x) {
			std::optional<int> expected_value = -32 <= x && x < 33 ? values[x + 32] : std::nullopt;
			assert(map.get(x) == expected_value && map.contains(x) == expected_value.has_value());
			auto it = map.find(x);
			assert((it != map.end()) == expected_value.has_value());
			if (expected_value) {
				assert(it->first <= x && x < it->second.right && it->second.value == *expected_value);
				assert(map.at(x) == *expected_value);
			}
			auto next = std::find_if(expected.begin(), expected.end(), [&](const auto &entry) { return x < std::get<1>(entry); });
			auto bound = map.lower_bound(x);
			assert((bound == map.end()) == (next == expected.end()));
			if (next != expected.end()) assert(bound->first == std::get<0>(*next));
		}
		for (int j = 0; j < 5; ++j) {
			int a = int(random() % 66) - 32, b = int(random() % 66) - 32;
			if (a > b) std::swap(a, b);
			std::uint64_t covered = 0;
			bool same = true;
			for (int x = a; x < b; ++x) {
				covered += values[x + 32].has_value();
				same &= values[x + 32] == value;
			}
			assert(map.count(a, b) == covered && map.intersects(a, b) == (covered != 0));
			assert(map.contains(a, b) == (covered == std::uint64_t(b - a)));
			assert(map.contains(a, b, value) == same);
			std::vector<Entry> clipped, enumerated;
			for (const auto &[left, right, v] : expected)
				if ((std::max)(a, left) < (std::min)(b, right)) clipped.emplace_back((std::max)(a, left), (std::min)(b, right), v);
			map.for_each(a, b, [&](int left, int right, const int &v) { enumerated.emplace_back(left, right, v); });
			assert(enumerated == clipped);
		}
		if (step % 257 == 0) {
			auto copy = map;
			auto moved = std::move(copy);
			assert(copy.empty() && copy.size() == 0 && moved.size() == size);
			copy.set(100, 9); assert(!map.contains(100));
			copy = moved;
			copy = std::move(moved);
			assert(moved.empty() && moved.size() == 0 && copy.size() == size);
			const auto &alias = copy; copy = alias;
			auto &same_map = copy; copy = std::move(same_map);
			assert(copy.size() == size);
		}
	}
}

struct Value {
	int value;
	Value() = delete;
	explicit Value(int x) : value(x) {}
};
struct Equal {
	int modulus;
	explicit Equal(int modulus) : modulus(modulus) {}
	bool operator()(const Value &a, const Value &b) const { return a.value % modulus == b.value % modulus; }
};

int main() {
	boundaries<std::int8_t>(); boundaries<std::uint8_t>();
	boundaries<int>(); boundaries<std::uint32_t>();
	boundaries<std::int64_t>(); boundaries<std::uint64_t>();
	boundaries<__int128_t>(); boundaries<__uint128_t>();
	static_assert(std::is_same_v<IntervalMap<>::size_type, std::uint64_t>);
	IntervalMap<int, std::string> colors(0, 10, "red");
	colors.assign(3, 7, "blue");
	assert(colors.interval_count() == 3 && colors.get(4) == "blue");
	colors.assign(2, 8, colors.at(4));
	assert(colors.interval_count() == 3 && colors.contains(2, 8, "blue"));
	colors.assign(0, 10, colors.at(4));
	assert(colors.interval_count() == 1 && colors.at(9) == "blue");
	colors.assign(-2, 0, "blue"); colors.assign(10, 12, "blue");
	assert(colors.interval_count() == 1 && colors.size() == 14);
	assert(colors.erase(3, 6) == 3 && colors.interval_count() == 2);
	assert(!colors.get(4) && !colors.contains(-2, 12));
	colors.assign(3, 6, "blue");
	assert(colors.interval_count() == 1 && colors.contains(-2, 12, "blue"));
	colors.assign(4, 4, "red");
	assert(colors.erase(4, 4) == 0 && colors.contains(4, 4, "red"));
	assert(!colors.intersects(4, 4) && colors.count(4, 4) == 0);
	std::uint64_t covered = 0;
	colors.for_each(0, 20, [token = std::make_unique<int>(1), &covered](int l, int r, const std::string &) { covered += (r - l) * *token; });
	assert(covered == 12);
	static_assert(std::is_const_v<std::remove_reference_t<decltype((colors.begin()->second.value))>>);
	static_assert(std::is_same_v<decltype(std::declval<const decltype(colors)&>().at(0)), const std::string&>);
	IntervalMap<int, int> initialized{{0, 8, 1}, {2, 5, 2}, {5, 8, 2}, {3, 3, 9}};
	assert(initialized.interval_count() == 2 && initialized.contains(2, 8, 2));
	initialized.assign({}); assert(initialized.empty() && initialized.size() == 0);
	IntervalMap<int, Value, std::uint64_t, Equal> custom(Equal(5));
	custom.assign(0, 2, Value(1)); custom.assign(2, 4, Value(6));
	assert(custom.interval_count() == 1 && custom.contains(0, 4, Value(11)));
	auto moved = std::move(custom);
	assert(custom.empty() && custom.size() == 0 && moved.get(3)->value % 5 == 1);
	custom.assign(0, 1, Value(2)); custom.assign(1, 2, Value(7));
	assert(custom.interval_count() == 1);
	IntervalMap<int, Value, std::uint64_t, Equal> other(Equal(3));
	swap(other, moved);
	other.assign(4, 6, Value(11));
	assert(other.interval_count() == 1);
	moved.assign(0, 1, Value(2)); moved.assign(1, 2, Value(5));
	assert(moved.interval_count() == 1);
	randomized();
}
