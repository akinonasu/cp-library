#ifndef POINT_SET_WAVELET_MATRIX_HPP
#define POINT_SET_WAVELET_MATRIX_HPP

#include "weighted_wavelet_matrix.hpp"

template <class T = std::int64_t, class Sum = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>>
class PointSetWaveletMatrix {
	std::vector<T> keys;
	std::vector<int> offsets{0}, current;
	WeightedWaveletMatrix<T, int, Sum> wm;
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }
	int find(int p, const T &x) const {
		assert(0 <= p && p < size());
		return std::lower_bound(keys.begin() + offsets[p], keys.begin() + offsets[p + 1], x) - keys.begin();
	}

  public:
	using value_type = T;
	using sum_type = Sum;
	PointSetWaveletMatrix() = default;
	explicit PointSetWaveletMatrix(const std::vector<T> &a, const std::vector<std::pair<int, T>> &candidates = {}) { build(a, candidates); }
	void build(const std::vector<T> &a, const std::vector<std::pair<int, T>> &candidates = {}) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		std::vector<std::pair<int, T>> points = candidates;
		points.reserve(a.size() + candidates.size());
		for (int i = 0; i < static_cast<int>(a.size()); ++i) points.emplace_back(i, a[i]);
		std::sort(points.begin(), points.end());
		points.erase(std::unique(points.begin(), points.end()), points.end());
		assert(points.size() <= std::size_t(std::numeric_limits<int>::max()));
		offsets.assign(a.size() + 1, 0);
		keys.clear();
		keys.reserve(points.size());
		for (const auto &[p, x] : points) {
			assert(0 <= p && p < static_cast<int>(a.size()));
			++offsets[p + 1];
			keys.push_back(x);
		}
		for (int i = 0; i < static_cast<int>(a.size()); ++i) offsets[i + 1] += offsets[i];
		current.resize(a.size());
		std::vector<int> weights(points.size());
		for (int i = 0; i < size(); ++i) weights[current[i] = find(i, a[i])] = 1;
		wm.build(keys, weights);
	}
	int size() const { return static_cast<int>(current.size()); }
	bool empty() const { return current.empty(); }
	T access(int p) const { assert(0 <= p && p < size()); return keys[current[p]]; }
	T operator[](int p) const { return access(p); }
	bool contains(int p, const T &x) const {
		int q = find(p, x);
		return q < offsets[p + 1] && keys[q] == x;
	}
	bool set(int p, const T &x) {
		int q = find(p, x);
		if (q == offsets[p + 1] || keys[q] != x) return false;
		wm.move_weight(current[p], q, 1);
		current[p] = q;
		return true;
	}
	int count(int l, int r, const T &x) const { check(l, r); return wm.count(offsets[l], offsets[r], x); }
	int rank(const T &x, int r) const { return count(0, r, x); }
	int range_freq(int l, int r, const T &upper) const { check(l, r); return wm.range_freq(offsets[l], offsets[r], upper); }
	int range_freq(int l, int r, const T &lower, const T &upper) const { check(l, r); return wm.range_freq(offsets[l], offsets[r], lower, upper); }
	Sum range_sum(int l, int r) const { check(l, r); return wm.range_sum(offsets[l], offsets[r]); }
	Sum range_sum(int l, int r, const T &upper) const { check(l, r); return wm.range_sum(offsets[l], offsets[r], upper); }
	Sum range_sum(int l, int r, const T &lower, const T &upper) const { check(l, r); return wm.range_sum(offsets[l], offsets[r], lower, upper); }
	T kth_smallest(int l, int r, int k) const { check(l, r); return wm.kth_smallest(offsets[l], offsets[r], k); }
	T kth_largest(int l, int r, int k) const { check(l, r); return wm.kth_largest(offsets[l], offsets[r], k); }
	Sum sum_smallest(int l, int r, int k) const { check(l, r); return wm.sum_smallest(offsets[l], offsets[r], k); }
	Sum sum_largest(int l, int r, int k) const { check(l, r); return wm.sum_largest(offsets[l], offsets[r], k); }
	int min_count_smallest(int l, int r, Sum target) const { check(l, r); return wm.min_count_smallest(offsets[l], offsets[r], target); }
	int min_count_largest(int l, int r, Sum target) const { check(l, r); return wm.min_count_largest(offsets[l], offsets[r], target); }
	int max_count_smallest(int l, int r, Sum budget) const { check(l, r); return wm.max_count_smallest(offsets[l], offsets[r], budget); }
	int max_count_largest(int l, int r, Sum budget) const { check(l, r); return wm.max_count_largest(offsets[l], offsets[r], budget); }
};

#endif
