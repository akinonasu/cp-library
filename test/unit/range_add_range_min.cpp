#include "data_structure/segment_tree/range_add_range_min.hpp"
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

int main() {
	randomized<std::int16_t>();
	randomized<std::int32_t>();
	randomized<std::int64_t>();
	boundaries<std::int16_t>();
	boundaries<std::int32_t>();
	boundaries<std::int64_t>();
	full_width<std::int16_t>();
	full_width<std::int32_t>();
	full_width<std::int64_t>();
}
