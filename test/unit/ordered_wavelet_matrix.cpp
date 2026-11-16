#include "data_structure/point_set_wavelet_matrix.hpp"
#include <algorithm>
#include <cassert>
#include <compare>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

struct Key {
	int first;
	std::uint64_t second;
	Key() = delete;
	Key(int a, std::uint64_t b) : first(a), second(b) {}
	auto operator<=>(const Key &) const = default;
};

struct LessOnly {
	int value;
	LessOnly() = delete;
	explicit LessOnly(int value) : value(value) {}
	friend bool operator<(const LessOnly &a, const LessOnly &b) { return a.value < b.value; }
};

struct PayloadKey {
	int key;
	std::string payload;
	friend bool operator<(const PayloadKey &a, const PayloadKey &b) { return a.key < b.key; }
};

template class WeightedWaveletMatrix<std::string>;
template class WeightedWaveletMatrix<LessOnly>;
template class PointSetWaveletMatrix<LessOnly>;

template <class T>
bool equal(const T &a, const T &b) { return !(a < b) && !(b < a); }

template <class Iterator, class T>
auto occurrences(Iterator first, Iterator last, const T &x) {
	return std::count_if(first, last, [&](const T &y) { return equal(x, y); });
}

template <class Matrix>
concept HasSum = requires(Matrix &matrix) { matrix.range_sum(0, 0); matrix.sum_smallest(0, 0, 0); };

template <class T>
void verify_weighted(const std::vector<T> &a, const std::vector<std::int64_t> &weights,
		const WeightedWaveletMatrix<T> &wm, const std::vector<T> &alphabet, std::mt19937_64 &random) {
	assert(wm.size() == int(a.size()) && wm.empty() == a.empty());
	for (int p = 0; p < int(a.size()); ++p) assert(equal(wm[p], a[p]) && wm.weight(p) == weights[p]);
	for (int trial = 0; trial < 30; ++trial) {
		int l = random() % (a.size() + 1), r = random() % (a.size() + 1);
		if (r < l) std::swap(l, r);
		std::vector<T> sorted;
		for (int p = l; p < r; ++p)
			for (std::int64_t j = 0; j < weights[p]; ++j) sorted.push_back(a[p]);
		std::sort(sorted.begin(), sorted.end());
		assert(wm.count(l, r) == std::int64_t(sorted.size()));
		for (const T &lower : alphabet) {
			assert(wm.count(l, r, lower) == occurrences(sorted.begin(), sorted.end(), lower));
			assert(wm.range_freq(l, r, lower) == std::lower_bound(sorted.begin(), sorted.end(), lower) - sorted.begin());
			for (const T &upper : alphabet) {
				std::int64_t count = 0;
				for (const T &x : sorted) count += !(x < lower) && x < upper;
				assert(wm.range_freq(l, r, lower, upper) == count);
			}
		}
		for (int k = 0; k < int(sorted.size()); ++k) {
			assert(equal(wm.kth_smallest(l, r, k), sorted[k]));
			assert(equal(wm.kth_largest(l, r, k), sorted[sorted.size() - 1 - k]));
		}
	}
}

template <class T>
void check(std::vector<T> alphabet) {
	std::mt19937_64 random(7148931);
	static_assert(!HasSum<WeightedWaveletMatrix<T>> && !HasSum<PointSetWaveletMatrix<T>>);
	WeightedWaveletMatrix<T> empty;
	PointSetWaveletMatrix<T> point_empty;
	assert(empty.empty() && empty.count(0, 0) == 0 && point_empty.empty());
	for (int n : {0, 1, 2, 7, 8, 31, 32, 63, 64, 65, 127, 257}) {
		std::vector<T> a;
		std::vector<std::int64_t> weights(n);
		for (int i = 0; i < n; ++i) a.push_back(alphabet[random() % (alphabet.size() - 1)]), weights[i] = random() % 4;
		WeightedWaveletMatrix<T> wm(a, weights);
		verify_weighted(a, weights, wm, alphabet, random);
		for (int trial = 0; trial < 35; ++trial) {
			if (n) {
				int p = random() % n, q = random() % n;
				std::int64_t value = random() % 5;
				wm.set_weight(p, value); weights[p] = value;
				wm.add_weight(p, 2); weights[p] += 2;
				std::int64_t amount = random() % (weights[p] + 1);
				wm.move_weight(p, q, amount); weights[p] -= amount; weights[q] += amount;
			}
			verify_weighted(a, weights, wm, alphabet, random);
		}
		auto copy = wm;
		wm.build({}, {});
		assert(wm.empty());
		verify_weighted(a, weights, copy, alphabet, random);
		wm.build(a, weights, false);
		verify_weighted(a, weights, wm, alphabet, random);
		WeightedWaveletMatrix<T> ones(a);
		assert(ones.count(0, n) == n);

		std::vector<std::pair<int, T>> candidates;
		for (int p = 0; p < n; ++p) for (std::size_t j = 0; j + 1 < alphabet.size(); ++j) candidates.emplace_back(p, alphabet[j]);
		auto duplicates = candidates;
		candidates.insert(candidates.end(), duplicates.begin(), duplicates.end());
		PointSetWaveletMatrix<T> points(a, candidates);
		for (int trial = 0; trial < 60; ++trial) {
			if (n) {
				int p = random() % n;
				const T &value = alphabet[random() % (alphabet.size() - 1)];
				assert(points.contains(p, value) && points.set(p, value)); a[p] = value;
				assert(equal(points[p], value) && !points.contains(p, alphabet.back()));
				assert(!points.set(p, alphabet.back()) && equal(points[p], value));
			}
			int l = random() % (n + 1), r = random() % (n + 1);
			if (r < l) std::swap(l, r);
			std::vector<T> sorted(a.begin() + l, a.begin() + r);
			std::sort(sorted.begin(), sorted.end());
			for (const T &lower : alphabet) {
				assert(points.count(l, r, lower) == occurrences(sorted.begin(), sorted.end(), lower));
				assert(points.rank(lower, r) == occurrences(a.begin(), a.begin() + r, lower));
				assert(points.range_freq(l, r, lower) == std::lower_bound(sorted.begin(), sorted.end(), lower) - sorted.begin());
				for (const T &upper : alphabet) {
					int count = 0;
					for (const T &x : sorted) count += !(x < lower) && x < upper;
					assert(points.range_freq(l, r, lower, upper) == count);
				}
			}
			for (int k = 0; k < int(sorted.size()); ++k) {
				assert(equal(points.kth_smallest(l, r, k), sorted[k]));
				assert(equal(points.kth_largest(l, r, k), sorted[sorted.size() - 1 - k]));
			}
			if (trial == 20) points = PointSetWaveletMatrix<T>(points);
			if (trial == 40) points.build(a, candidates);
		}
	}
}

int main() {
	static_assert(HasSum<WeightedWaveletMatrix<int>> && HasSum<PointSetWaveletMatrix<std::uint64_t>>);
	check<std::string>({"beta", "", std::string("a\0b", 3), "alpha", std::string(80, 'z'), "unregistered"});
	check<std::pair<int, std::uint64_t>>({{0, 0}, {-1, UINT64_MAX}, {0, UINT64_MAX}, {1, 3}, {5, 7}, {2, 2}});
	check<Key>({Key(0, 0), Key(-1, UINT64_MAX), Key(0, UINT64_MAX), Key(1, 3), Key(5, 7), Key(2, 2)});
	check<LessOnly>({LessOnly(0), LessOnly(-7), LessOnly(19), LessOnly(3), LessOnly(-11), LessOnly(23)});
	check<double>({-1e100, -0.0, 1.25, 1e100, 1e-100, 17.0});
	{
		std::vector<PayloadKey> keys{{2, "first"}, {1, "second"}, {2, "third"}};
		WeightedWaveletMatrix<PayloadKey> labels(keys, {2, 3, 4});
		assert(labels.count(0, 3, PayloadKey{2, "query"}) == 6);
		assert(labels[0].key == 2 && labels[2].key == 2);
		assert(labels.kth_smallest(0, 3, 0).key == 1 && labels.kth_largest(0, 3, 0).key == 2);
		PointSetWaveletMatrix<PayloadKey> points(keys, {{0, {3, "candidate"}}, {0, {3, "duplicate"}}, {1, {2, "other"}}});
		assert(points.set(0, {3, "equivalent"}) && points[0].key == 3);
		assert(points.set(1, {2, "equivalent"}) && points.count(0, 3, {2, "query"}) == 2);
		assert(points.kth_smallest(0, 3, 0).key == 2 && points.kth_largest(0, 3, 0).key == 3);
	}
	std::vector<std::string> many;
	for (int i = 0; i < 2053; ++i) many.push_back("key:" + std::to_string(i));
	std::reverse(many.begin(), many.end());
	WeightedWaveletMatrix<std::string> wm(many);
	std::sort(many.begin(), many.end());
	for (int k = 0; k < int(many.size()); ++k) assert(wm.kth_smallest(0, wm.size(), k) == many[k]);
}
