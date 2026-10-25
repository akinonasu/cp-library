#ifndef WEIGHTED_WAVELET_MATRIX_HPP
#define WEIGHTED_WAVELET_MATRIX_HPP

#include "../components/wavelet_compression.hpp"
#include "../components/quad_bit_vector.hpp"
#include "../components/fenwick_sum.hpp"
#include <numeric>

template <class T = std::int64_t, class Count = std::int64_t,
		class Sum = std::conditional_t<(sizeof(T) < 8), std::int64_t, __int128_t>>
class WeightedWaveletMatrix {
	static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8);
	static_assert(std::is_integral_v<Count> && std::is_signed_v<Count> && sizeof(Count) <= 8);
	static_assert(std::numeric_limits<Sum>::is_integer && std::numeric_limits<Sum>::is_signed);
	struct Node {
		Count count{};
		Sum sum{};
		Node &operator+=(const Node &x) { count += x.count; sum += x.sum; return *this; }
		Node &operator-=(const Node &x) { count -= x.count; sum -= x.sum; return *this; }
		friend Node operator-(Node a, const Node &b) { return a -= b; }
	};
	wavelet_detail::ValueIndex<T> index;
	std::vector<Count> weights;
	std::vector<QuadBitVector> bits;
	std::vector<std::array<int, 4>> middle;
	std::vector<FenwickSum<Count>> counts;
	std::vector<FenwickSum<Sum>> sums;
	bool sum_ready = false;
	int levels = 0;
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }
	template <bool WithSum>
	Node row(int d, int l, int r) const {
		if constexpr (WithSum) return {counts[d].prod(l, r), sums[d].prod(l, r)};
		else return {counts[d].prod(l, r), Sum(0)};
	}
	void add(int d, int p, Node delta) {
		counts[d].add(p, delta.count);
		if (sum_ready) sums[d].add(p, delta.sum);
	}
	void add_pair(int d, int p, Node a, int q, Node b) {
		counts[d].add_pair(p, a.count, q, b.count);
		if (sum_ready) sums[d].add_pair(p, a.sum, q, b.sum);
	}
	void prepare_sum_impl() {
		if (sum_ready) return;
		sums.resize(counts.size());
		std::vector<Sum> current(size()), next(size());
		for (int i = 0; i < size(); ++i) current[i] = Sum(index.values[index.original[i]]) * weights[i];
		sums[0].build(current);
		for (int d = 0; d < levels; ++d) {
			std::array<std::vector<Sum>, 3> zero;
			zero[0].reserve(middle[d][2]);
			zero[1].reserve(middle[d][1]);
			zero[2].reserve(middle[d][3] - middle[d][2]);
			auto cursor = middle[d];
			for (int i = 0; i < size(); ++i) {
				const auto &block = bits[d].blocks[i >> 6];
				int digit = ((block.low >> (i & 63)) & 1) | (((block.high >> (i & 63)) & 1) << 1);
				if (!(digit & 2)) zero[0].push_back(current[i]);
				if (!(digit & 1)) zero[1 + (digit >> 1)].push_back(current[i]);
				next[cursor[digit]++] = current[i];
			}
			for (int j = 0; j < 3; ++j) sums[3 * d + 1 + j].build(zero[j]);
			current.swap(next);
		}
		sum_ready = true;
	}

	template <bool Single = false, bool WithSum = true, class Choose>
	std::pair<std::uint32_t, Node> walk(int l, int r, Node total, Choose choose) const {
		std::uint32_t x = 0;
		for (int d = 0; d < levels && total.count; ++d) {
			if constexpr (Single) if (total.count == 1) return {x, total};
			int base = 3 * d + 1;
			int a = bits[d].below(l, 2), b = bits[d].below(r, 2);
			Node zero = row<WithSum>(base, a, b);
			bool high = choose(zero, total);
			total = high ? total - zero : zero;
			int high_l = high ? l - a : a, high_r = high ? r - b : b;
			int c = bits[d].rank(l, 2 * high), e = bits[d].rank(r, 2 * high);
			zero = row<WithSum>(base + 1 + high, c, e);
			bool low = choose(zero, total);
			total = low ? total - zero : zero;
			int digit = 2 * high + low;
			x = (x << 2) | digit;
			l = middle[d][digit] + (low ? high_l - c : c);
			r = middle[d][digit] + (low ? high_r - e : e);
		}
		return {x, total};
	}
	template <bool WithSum = false>
	Node less_than(int l, int r, std::uint32_t x, int first = 0) const {
		if (!x || l == r) return {};
		if constexpr (WithSum) assert(sum_ready);
		if (x >= index.values.size()) return row<WithSum>(0, l, r);
		Node answer{};
		for (int d = first; d < levels && l < r; ++d) {
			int digit = (x >> (2 * (levels - 1 - d))) & 3, high = digit >> 1, low = digit & 1;
			int a = bits[d].below(l, 2), b = bits[d].below(r, 2), base = 3 * d + 1;
			if (high) answer += row<WithSum>(base, a, b);
			int c = bits[d].rank(l, 2 * high), e = bits[d].rank(r, 2 * high);
			if (low) answer += row<WithSum>(base + 1 + high, c, e);
			l = middle[d][digit] + (low ? (high ? l - a : a) - c : c);
			r = middle[d][digit] + (low ? (high ? r - b : b) - e : e);
		}
		return answer;
	}

	template <bool WithSum = false>
	Node between(int l, int r, std::uint32_t low, std::uint32_t high) const {
		if (low == high || l == r) return {};
		if (!low || high == index.values.size()) return less_than<WithSum>(l, r, high) - less_than<WithSum>(l, r, low);
		int common = levels - (std::bit_width(low ^ high) + 1) / 2;
		for (int d = 0; d < common; ++d) {
			int digit = (low >> (2 * (levels - 1 - d))) & 3;
			l = middle[d][digit] + bits[d].rank(l, digit);
			r = middle[d][digit] + bits[d].rank(r, digit);
		}
		return less_than<WithSum>(l, r, high, common) - less_than<WithSum>(l, r, low, common);
	}

	template <bool Largest>
	Sum selected_sum(int l, int r, Count k, Node total) const {
		assert(0 <= k && k <= total.count);
		if (!k) return 0;
		if (k == total.count) return total.sum;
		Sum answer = 0;
		auto [x, leaf] = walk<true>(l, r, total, [&](Node zero, Node all) {
			Node preferred = Largest ? all - zero : zero;
			bool skip = k >= preferred.count;
			if (skip) k -= preferred.count, answer += preferred.sum;
			return Largest != skip;
		});
		return answer + (leaf.count == 1 ? leaf.sum : Sum(index.values[x])) * k;
	}
	template <bool Largest, bool Lower>
	Count sum_bound(int l, int r, Sum target) const {
		check(l, r);
		if constexpr (Lower) {
			if (target <= 0) return 0;
		} else if (target < 0) return -1;
		if (l == r) return Lower ? -1 : 0;
		assert(index.values.front() >= 0);
		assert(sum_ready);
		Node total = row<true>(0, l, r);
		if constexpr (Lower) {
			if (total.sum < target) return -1;
		} else if (total.sum <= target) return total.count;
		Count answer = 0;
		auto [x, leaf] = walk<true>(l, r, total, [&](Node zero, Node all) {
			Node preferred = Largest ? all - zero : zero;
			bool skip = Lower ? preferred.sum < target : preferred.sum <= target;
			if (skip) target -= preferred.sum, answer += preferred.count;
			return Largest != skip;
		});
		Sum value = leaf.count == 1 ? leaf.sum : Sum(index.values[x]);
		return answer + Count(target / value) + (Lower && target % value != 0);
	}

  public:
	using value_type = T;
	using count_type = Count;
	using sum_type = Sum;
	WeightedWaveletMatrix() { build({}, {}); }
	explicit WeightedWaveletMatrix(const std::vector<T> &a) { build(a, std::vector<Count>(a.size(), 1)); }
	WeightedWaveletMatrix(const std::vector<T> &a, const std::vector<Count> &w, bool with_sum = true) { build(a, w, with_sum); }
	void prepare_sum() { prepare_sum_impl(); }
	void build(const std::vector<T> &a, const std::vector<Count> &w, bool with_sum = true) {
		assert(a.size() == w.size() && a.size() <= std::size_t(std::numeric_limits<int>::max()));
		index.build(a);
		weights = w;
		levels = index.values.size() <= 1 ? 0 : (std::bit_width(unsigned(index.values.size() - 1)) + 1) / 2;
		bits.clear();
		bits.reserve(levels);
		middle.resize(levels);
		counts.clear();
		counts.resize(3 * levels + 1);
		sums.clear();
		sum_ready = false;
		std::vector<Count> root = w;
		for (int i = 0; i < size(); ++i) {
			assert(w[i] >= 0);
		}
		counts[0].build(root);
		std::vector<std::uint32_t> current = index.original, next(a.size());
		std::vector<int> order(a.size()), next_order(a.size());
		std::iota(order.begin(), order.end(), 0);
		for (int d = 0; d < levels; ++d) {
			int shift = 2 * (levels - 1 - d);
			bits.emplace_back(size());
			bits.back().build(current, shift);
			auto totals = bits.back().ranks(size());
			middle[d] = {0, totals[0], totals[0] + totals[1], totals[0] + totals[1] + totals[2]};
			std::array<std::vector<Count>, 3> zero;
			zero[0].reserve(middle[d][2]);
			zero[1].reserve(totals[0]);
			zero[2].reserve(totals[2]);
			auto cursor = middle[d];
			for (int i = 0; i < size(); ++i) {
				int digit = (current[i] >> shift) & 3;
				Count node = root[order[i]];
				if (!(digit & 2)) zero[0].push_back(node);
				if (!(digit & 1)) zero[1 + (digit >> 1)].push_back(node);
				int p = cursor[digit]++;
				next[p] = current[i], next_order[p] = order[i];
			}
			for (int j = 0; j < 3; ++j) counts[3 * d + 1 + j].build(zero[j]);
			current.swap(next), order.swap(next_order);
		}
		if (with_sum) prepare_sum();
	}
	void build(const std::vector<T> &a) { build(a, std::vector<Count>(a.size(), 1)); }
	int size() const { return index.n; }
	bool empty() const { return size() == 0; }
	T access(int p) const {
		assert(0 <= p && p < size());
		return index.values[index.original[p]];
	}
	T operator[](int p) const { return access(p); }
	Count weight(int p) const {
		assert(0 <= p && p < size());
		return weights[p];
	}
	void add_weight(int p, Count delta) {
		assert(0 <= p && p < size());
		assert(delta >= -weights[p] && (delta <= 0 || weights[p] <= std::numeric_limits<Count>::max() - delta));
		if (!delta) return;
		weights[p] += delta;
		std::uint32_t x = index.original[p];
		Node change{delta, sum_ready ? Sum(index.values[x]) * delta : Sum(0)};
		add(0, p, change);
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (2 * (levels - 1 - d))) & 3;
			int rank = bits[d].rank(p, digit);
			if (!(digit & 2)) add(3 * d + 1, bits[d].below(p, 2), change);
			if (!(digit & 1)) add(3 * d + 2 + (digit >> 1), rank, change);
			p = middle[d][digit] + rank;
		}
	}
	void set_weight(int p, Count value) { assert(value >= 0); add_weight(p, value - weight(p)); }
	void move_weight(int from, int to, Count amount) {
		assert(0 <= from && from < size() && 0 <= to && to < size());
		assert(0 <= amount && amount <= weights[from]);
		if (from == to || !amount) return;
		assert(weights[to] <= std::numeric_limits<Count>::max() - amount);
		weights[from] -= amount;
		weights[to] += amount;
		std::uint32_t x = index.original[from], y = index.original[to];
		Node minus{-amount, sum_ready ? -Sum(index.values[x]) * amount : Sum(0)}, plus{amount, sum_ready ? Sum(index.values[y]) * amount : Sum(0)};
		add_pair(0, from, minus, to, plus);
		for (int d = 0; d < levels; ++d) {
			int shift = 2 * (levels - 1 - d), a = (x >> shift) & 3, b = (y >> shift) & 3;
			int p = bits[d].rank(from, a), q = bits[d].rank(to, b), base = 3 * d + 1;
			if (!(a & 2) && !(b & 2)) add_pair(base, bits[d].below(from, 2), minus, bits[d].below(to, 2), plus);
			else if (!(a & 2)) add(base, bits[d].below(from, 2), minus);
			else if (!(b & 2)) add(base, bits[d].below(to, 2), plus);
			if (!(a & 1) && a == b) add_pair(base + 1 + (a >> 1), p, minus, q, plus);
			else {
				if (!(a & 1)) add(base + 1 + (a >> 1), p, minus);
				if (!(b & 1)) add(base + 1 + (b >> 1), q, plus);
			}
			from = middle[d][a] + p, to = middle[d][b] + q;
		}
	}
	Count count(int l, int r) const { check(l, r); return counts[0].prod(l, r); }
	Count count(int l, int r, const T &x) const {
		check(l, r);
		auto c = index.code(x);
		if (c == index.values.size() || index.values[c] != x) return 0;
		return between(l, r, c, c + 1).count;
	}
	Count range_freq(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, index.code(upper)).count; }
	Count range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? between(l, r, index.code(lower), index.code(upper)).count : 0;
	}
	Sum range_sum(int l, int r) const { check(l, r); assert(sum_ready); return sums[0].prod(l, r); }
	Sum range_sum(int l, int r, const T &upper) const { check(l, r); return less_than<true>(l, r, index.code(upper)).sum; }
	Sum range_sum(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? between<true>(l, r, index.code(lower), index.code(upper)).sum : 0;
	}
	T kth_smallest(int l, int r, Count k) const {
		check(l, r);
		Node total{counts[0].prod(l, r), Sum(0)};
		assert(0 <= k && k < total.count);
		auto [x, leaf] = walk<false, false>(l, r, total, [&](Node zero, Node) {
			bool one = k >= zero.count;
			if (one) k -= zero.count;
			return one;
		});
		return index.values[x];
	}
	T kth_largest(int l, int r, Count k) const { return kth_smallest(l, r, count(l, r) - 1 - k); }
	Sum sum_smallest(int l, int r, Count k) const { check(l, r); assert(sum_ready); return selected_sum<false>(l, r, k, row<true>(0, l, r)); }
	Sum sum_largest(int l, int r, Count k) const { check(l, r); assert(sum_ready); return selected_sum<true>(l, r, k, row<true>(0, l, r)); }
	Count min_count_smallest(int l, int r, Sum target) const { return sum_bound<false, true>(l, r, target); }
	Count min_count_largest(int l, int r, Sum target) const { return sum_bound<true, true>(l, r, target); }
	Count max_count_smallest(int l, int r, Sum budget) const { return sum_bound<false, false>(l, r, budget); }
	Count max_count_largest(int l, int r, Sum budget) const { return sum_bound<true, false>(l, r, budget); }
};

#endif
