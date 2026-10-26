#include "data_structure/range_add_range_min.hpp"
#include <algorithm>
#include <cassert>
#include <bit>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

template<class T> void randomized() {
	std::mt19937 random(282022);
	for (int n : {0, 1, 2, 15, 16, 17, 31, 32, 33, 63, 65, 255, 256, 257, 4095, 4096, 4097}) {
		std::vector<T> values(n);
		for (auto &value : values) value = T(int(random() % 101) - 50);
		RangeAddRangeMin<T> tree(values);
		assert(tree.size() == n && tree.empty() == !n);
		assert(tree.memory_usage() <= (values.size() + 16 * 9) * 3 * sizeof(T));
		for (int step = 0; step < 1500; ++step) {
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			T value = T(int(random() % 21) - 10);
			if (step % 4 == 0) {
				tree.add(l, r, value);
				for (int i = l; i < r; ++i) values[i] = T(values[i] + value);
			} else if (step % 4 == 1 && n) {
				int p = random() % n;
				tree.set(p, value), values[p] = value;
			} else if (step % 4 == 2) {
				tree.apply(l, r, value);
				for (int i = l; i < r; ++i) values[i] = T(values[i] + value);
			}
			T expected = l == r ? std::numeric_limits<T>::max() : *std::min_element(values.begin() + l, values.begin() + r);
			const auto &view = tree;
			assert(view.prod(l, r) == expected && view.min(l, r) == expected);
			assert(view.all_prod() == (n ? *std::min_element(values.begin(), values.end()) : std::numeric_limits<T>::max()));
			assert(view.all_prod() == view.all_min());
			if (n) assert(view.get(l % n) == values[l % n]);
			if (step % 211 == 0) {
				assert(view.values() == values);
				auto copy = tree;
				auto moved = std::move(copy);
				assert(copy.empty() && copy.values().empty());
				copy = moved;
				auto *self = &moved;
				moved = std::move(*self);
				assert(copy.values() == values && moved.values() == values);
				tree.swap(moved);
			}
		}
		assert(tree.values() == values);
		tree.build({});
		assert(tree.empty() && tree.prod(0, 0) == std::numeric_limits<T>::max());
	}
}

template<class T> void boundaries() {
	const T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	RangeAddRangeMin<T> tree(4097, low);
	tree.add(0, 4097, high);
	tree.add(16, 4096, high);
	assert(tree.get(0) == -1 && tree.get(4096) == -1 && tree.get(16) == high - 1);
	assert(tree.prod(16, 4096) == high - 1 && tree.all_prod() == -1);
	tree.add(0, 4097, T(-high));
	assert(tree.get(0) == low && tree.get(16) == -1);
	tree.add(16, 4096, T(-high));
	for (T value : tree.values()) assert(value == low);
	tree.set(256, high);
	assert(tree.get(256) == high && tree.prod(256, 257) == high);
	RangeAddRangeMin<T> padding(17, high);
	padding.add(0, 17, T(-high));
	padding.add(0, 16, high);
	assert(padding.get(16) == 0 && padding.prod(0, 16) == high && padding.all_min() == 0);
	padding.add(16, 17, high);
	for (T value : padding.values()) assert(value == high);
	padding.add(0, 17, T(-high));
	for (T value : padding.values()) assert(value == 0);
}

template<class T> void full_width() {
	using U = std::make_unsigned_t<T>;
	std::mt19937_64 random(543785);
	const T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	for (int n : {1, 7, 8, 9, 63, 64, 65, 511, 512, 513}) {
		std::vector<T> values(n);
		for (auto &value : values) value = std::bit_cast<T>(U(random()));
		RangeAddRangeMin<T> tree(values);
		for (int step = 0; step < 1200; ++step) {
			int l = random() % n, r = random() % n;
			if (l > r) std::swap(l, r);
			++r;
			if (step % 3 == 0) {
				int p = random() % n;
				T value = std::bit_cast<T>(U(random()));
				tree.set(p, value), values[p] = value;
			} else {
				T minimum = *std::min_element(values.begin() + l, values.begin() + r);
				T maximum = *std::max_element(values.begin() + l, values.begin() + r);
				__int128_t lower = std::max<__int128_t>(low, __int128_t(low) - minimum);
				__int128_t upper = std::min<__int128_t>(high, __int128_t(high) - maximum);
				T delta = T(std::clamp<__int128_t>(std::bit_cast<T>(U(random())), lower, upper));
				tree.add(l, r, delta);
				for (int i = l; i < r; ++i) values[i] = T(__int128_t(values[i]) + delta);
			}
			const auto &view = tree;
			assert(view.prod(l, r) == *std::min_element(values.begin() + l, values.begin() + r));
			assert(view.all_min() == *std::min_element(values.begin(), values.end()));
			if (step % 37 == 0) assert(view.values() == values);
		}
		assert(tree.values() == values);
	}
}

template<class T> void aligned_extremes() {
	const T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	for (int n : {1, 7, 8, 9, 63, 64, 65, 511, 512, 513}) {
		std::vector<int> edges{0, n};
		for (int p : {1, 6, 7, 8, 9, 62, 63, 64, 65, 510, 511, 512})
			if (p < n) edges.push_back(p);
		std::sort(edges.begin(), edges.end());
		edges.erase(std::unique(edges.begin(), edges.end()), edges.end());
		for (int l : edges) for (int r : edges) if (l < r) {
			std::vector<T> values(n);
			RangeAddRangeMin<T> tree(values);
			auto check = [&] {
				assert(tree.values() == values);
				assert(tree.all_min() == *std::min_element(values.begin(), values.end()));
				for (int a : edges) for (int b : edges) if (a <= b) {
					T expected = a == b ? high : *std::min_element(values.begin() + a, values.begin() + b);
					assert(tree.prod(a, b) == expected);
				}
			};
			auto add = [&](T delta) {
				tree.add(l, r, delta);
				for (int i = l; i < r; ++i) values[i] = T(__int128_t(values[i]) + delta);
				check();
			};
			add(low);
			for (int p : {l, r - 1}) {
				tree.set(p, high), values[p] = high;
				check();
				tree.set(p, low), values[p] = low;
				check();
			}
			add(high);
			add(T(1));
			add(high);
			add(low);
			add(T(1));
		}
	}
}

void padding_transitions() {
	using T = std::int64_t;
	const T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	const T limit = high / 3, delta = (limit - 1) / 2;
	for (int n : {11, 88, 704}) {
		int start = n / 11 * 8;
		std::vector<T> values(n);
		RangeAddRangeMin<T> tree(values);
		auto check = [&](const RangeAddRangeMin<T>& view) {
			assert(view.values() == values);
			T prefix = high, suffix = high;
			for (int i = 0; i < n; ++i) {
				prefix = std::min(prefix, values[i]);
				suffix = std::min(suffix, values[n - 1 - i]);
				assert(view.get(i) == values[i]);
				assert(view.prod(0, i + 1) == prefix);
				assert(view.prod(n - 1 - i, n) == suffix);
			}
			assert(view.all_min() == prefix);
			assert(view.prod(0, 0) == high && view.prod(n, n) == high);
		};
		auto add = [&](int l, int r, T value) {
			tree.add(l, r, value);
			for (int i = l; i < r; ++i) {
				__int128_t next = __int128_t(values[i]) + value;
				assert(low <= next && next <= high);
				values[i] = T(next);
			}
			check(tree);
		};
		auto set = [&](int p, T value) {
			tree.set(p, value), values[p] = value;
			check(tree);
		};
		add(start, n, delta);
		add(start, n, -delta);
		add(start, n, -1);
		add(start, n, 1);
		add(start, n, high);
		add(0, start, high);
		set(start, low);
		auto copy = tree;
		auto moved = std::move(copy);
		assert(copy.empty());
		check(moved);
		tree = std::move(moved);
		assert(moved.empty());
		add(start + 1, n, -1);
		set(start, high);
		add(start, n, low);
		add(start, n, 1);
		values.assign(n, 0);
		values.back() = limit - 1;
		tree.build(values);
		check(tree);
		add(start, n, 1);
		add(start, n, high - limit);
		add(0, start, high);
		set(start, -limit);
		add(start, n, -1);
		values.assign(n, 0);
		tree.build(values);
		add(0, n, low);
		add(start, n, delta);
		add(0, n, high);
		add(start, n, -delta);
		add(start, n, -1);
		add(start, n, 1);
		add(0, n, high);
		add(start, n, 1);
		add(0, n, -high);
	}
}

void minimum_witness() {
	using T = std::int64_t;
	const int n = 65;
	std::vector<T> values(n);
	RangeAddRangeMin<T> tree(values);
	auto check = [&](const RangeAddRangeMin<T>& view) {
		assert(view.values() == values);
		for (int l = 0; l <= n; ++l) {
			T expected = std::numeric_limits<T>::max();
			assert(view.prod(l, l) == expected);
			for (int r = l + 1; r <= n; ++r) {
				expected = std::min(expected, values[r - 1]);
				assert(view.prod(l, r) == expected);
			}
		}
	};
	auto add = [&](int l, int r, T value) {
		tree.add(l, r, value);
		for (int i = l; i < r; ++i) values[i] += value;
		check(tree);
	};
	auto set = [&](int p, T value) {
		tree.set(p, value), values[p] = value;
		check(tree);
	};
	auto move_copy = [&] {
		auto copy = tree;
		auto moved = std::move(copy);
		assert(copy.empty());
		check(moved);
		tree = std::move(moved);
		assert(moved.empty());
		check(tree);
	};
	check(tree);
	add(0, n, 17);
	add(16, 49, 5);
	move_copy();
	set(24, -11);
	move_copy();
	set(24, 40);
	set(64, 17);
	add(0, 32, -7);
	add(0, n, -10);
	set(0, 0);
	set(0, 100);
}

void narrow_query_boundaries() {
	using T = std::int64_t;
	const T low = std::numeric_limits<T>::min(), high = std::numeric_limits<T>::max();
	const T limit = high / 3, delta = (limit - 1) / 4;
	for (int n : {73, 521, 4097}) for (bool mirror : {false, true}) {
		std::vector<T> values(n);
		RangeAddRangeMin<T> tree(values);
		std::vector<int> positions{0, 1, 2, 7, 8, 9, 63, 64, 65,
			n - 65, n - 64, n - 9, n - 8, n - 2, n - 1, n};
		std::sort(positions.begin(), positions.end());
		positions.erase(std::unique(positions.begin(), positions.end()), positions.end());
		auto check = [&](const RangeAddRangeMin<T>& view) {
			assert(view.values() == values);
			for (int i = 0; i < n; ++i) assert(view.get(i) == values[i]);
			for (int l : positions) for (int r : positions) if (l <= r) {
				T expected = l == r ? high : *std::min_element(values.begin() + l, values.begin() + r);
				assert(view.prod(l, r) == expected && view.min(l, r) == expected);
			}
			assert(view.all_min() == *std::min_element(values.begin(), values.end()));
		};
		auto add = [&](int l, int r, T value) {
			tree.add(l, r, value);
			for (int i = l; i < r; ++i) {
				__int128_t next = __int128_t(values[i]) + value;
				assert(low <= next && next <= high);
				values[i] = T(next);
			}
			check(tree);
		};
		check(tree);
		int l = mirror ? 0 : 1, r = mirror ? n - 1 : n;
		for (int i = 0; i < 4; ++i) add(l, r, delta);
		assert(tree.prod(l, r) == limit - 2);
		add(0, n, 2 * limit);
		assert(tree.prod(l, r) == high - 3);
		add(l, r, 3);
		assert(tree.prod(l, r) == high && tree.get(l) == high);
		assert(tree.prod(l, l) == high && tree.prod(r, r) == high);
	}
}

int main() {
	randomized<std::int16_t>();
	randomized<std::int32_t>();
	randomized<std::int64_t>();
	boundaries<std::int16_t>();
	boundaries<std::int32_t>();
	boundaries<std::int64_t>();
	boundaries<std::int8_t>();
	full_width<std::int8_t>();
	full_width<std::int16_t>();
	full_width<std::int32_t>();
	full_width<std::int64_t>();
	aligned_extremes<std::int8_t>();
	aligned_extremes<std::int16_t>();
	aligned_extremes<std::int32_t>();
	aligned_extremes<std::int64_t>();
	padding_transitions();
	minimum_witness();
	narrow_query_boundaries();
}
