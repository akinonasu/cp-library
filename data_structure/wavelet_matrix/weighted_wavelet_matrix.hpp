#ifndef WEIGHTED_WAVELET_MATRIX_HPP
#define WEIGHTED_WAVELET_MATRIX_HPP

#include "../../components/wavelet_compression.hpp"
#include "../../components/quad_bit_vector.hpp"
#include "../../components/fenwick_sum.hpp"
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
	std::vector<FenwickSum<Node>> rows;
	int levels = 0;
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= size()); }
	template <bool Single = false, class Choose>
	std::pair<std::uint32_t, Node> walk(int l, int r, Node total, Choose choose) const {
		std::uint32_t x = 0;
		for (int d = 0; d < levels && total.count; ++d) {
			if constexpr (Single) if (total.count == 1) return {x, total};
			int base = 3 * d + 1;
			int a = bits[d].below(l, 2), b = bits[d].below(r, 2);
			Node zero = rows[base].prod(a, b);
			bool high = choose(zero, total);
			total = high ? total - zero : zero;
			int high_l = high ? l - a : a, high_r = high ? r - b : b;
			int c = bits[d].rank(l, 2 * high), e = bits[d].rank(r, 2 * high);
			zero = rows[base + 1 + high].prod(c, e);
			bool low = choose(zero, total);
			total = low ? total - zero : zero;
			int digit = 2 * high + low;
			x = (x << 2) | digit;
			l = middle[d][digit] + (low ? high_l - c : c);
			r = middle[d][digit] + (low ? high_r - e : e);
		}
		return {x, total};
	}
	Node less_than(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return {};
		Node total = rows[0].prod(l, r), answer{};
		if (x >= index.values.size()) return total;
		int shift = 2 * levels;
		walk(l, r, total, [&](Node zero, Node) {
			bool one = (x >> --shift) & 1;
			if (one) answer += zero;
			return one;
		});
		return answer;
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
		Node total = rows[0].prod(l, r);
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
	WeightedWaveletMatrix(const std::vector<T> &a, const std::vector<Count> &w) { build(a, w); }
	void build(const std::vector<T> &a, const std::vector<Count> &w) {
		assert(a.size() == w.size() && a.size() <= std::size_t(std::numeric_limits<int>::max()));
		index.build(a);
		weights = w;
		levels = index.values.size() <= 1 ? 0 : (std::bit_width(unsigned(index.values.size() - 1)) + 1) / 2;
		bits.clear();
		bits.reserve(levels);
		middle.resize(levels);
		rows.clear();
		rows.resize(3 * levels + 1);
		std::vector<Node> root(a.size());
		for (int i = 0; i < size(); ++i) {
			assert(w[i] >= 0);
			root[i] = {w[i], Sum(a[i]) * w[i]};
		}
		rows[0].build(root);
		std::vector<std::uint32_t> current = index.original, next(a.size());
		std::vector<int> order(a.size()), next_order(a.size());
		std::iota(order.begin(), order.end(), 0);
		for (int d = 0; d < levels; ++d) {
			int shift = 2 * (levels - 1 - d);
			bits.emplace_back(size());
			bits.back().build(current, shift);
			auto counts = bits.back().ranks(size());
			middle[d] = {0, counts[0], counts[0] + counts[1], counts[0] + counts[1] + counts[2]};
			std::array<std::vector<Node>, 3> zero;
			zero[0].reserve(middle[d][2]);
			zero[1].reserve(counts[0]);
			zero[2].reserve(counts[2]);
			auto cursor = middle[d];
			for (int i = 0; i < size(); ++i) {
				int digit = (current[i] >> shift) & 3;
				Node node = root[order[i]];
				if (!(digit & 2)) zero[0].push_back(node);
				if (!(digit & 1)) zero[1 + (digit >> 1)].push_back(node);
				int p = cursor[digit]++;
				next[p] = current[i], next_order[p] = order[i];
			}
			for (int j = 0; j < 3; ++j) rows[3 * d + 1 + j].build(zero[j]);
			current.swap(next), order.swap(next_order);
		}
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
		Node change{delta, Sum(index.values[x]) * delta};
		rows[0].add(p, change);
		for (int d = 0; d < levels; ++d) {
			int digit = (x >> (2 * (levels - 1 - d))) & 3;
			int rank = bits[d].rank(p, digit);
			if (!(digit & 2)) rows[3 * d + 1].add(bits[d].below(p, 2), change);
			if (!(digit & 1)) rows[3 * d + 2 + (digit >> 1)].add(rank, change);
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
		Node minus{-amount, -Sum(index.values[x]) * amount}, plus{amount, Sum(index.values[y]) * amount};
		rows[0].add_pair(from, minus, to, plus);
		for (int d = 0; d < levels; ++d) {
			int shift = 2 * (levels - 1 - d), a = (x >> shift) & 3, b = (y >> shift) & 3;
			int p = bits[d].rank(from, a), q = bits[d].rank(to, b), base = 3 * d + 1;
			if (!(a & 2) && !(b & 2)) rows[base].add_pair(bits[d].below(from, 2), minus, bits[d].below(to, 2), plus);
			else if (!(a & 2)) rows[base].add(bits[d].below(from, 2), minus);
			else if (!(b & 2)) rows[base].add(bits[d].below(to, 2), plus);
			if (!(a & 1) && a == b) rows[base + 1 + (a >> 1)].add_pair(p, minus, q, plus);
			else {
				if (!(a & 1)) rows[base + 1 + (a >> 1)].add(p, minus);
				if (!(b & 1)) rows[base + 1 + (b >> 1)].add(q, plus);
			}
			from = middle[d][a] + p, to = middle[d][b] + q;
		}
	}
	Count count(int l, int r) const { check(l, r); return rows[0].prod(l, r).count; }
	Count count(int l, int r, const T &x) const {
		check(l, r);
		auto c = index.code(x);
		if (c == index.values.size() || index.values[c] != x) return 0;
		return less_than(l, r, c + 1).count - less_than(l, r, c).count;
	}
	Count range_freq(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, index.code(upper)).count; }
	Count range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? less_than(l, r, index.code(upper)).count - less_than(l, r, index.code(lower)).count : 0;
	}
	Sum range_sum(int l, int r) const { check(l, r); return rows[0].prod(l, r).sum; }
	Sum range_sum(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, index.code(upper)).sum; }
	Sum range_sum(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? less_than(l, r, index.code(upper)).sum - less_than(l, r, index.code(lower)).sum : 0;
	}
	T kth_smallest(int l, int r, Count k) const {
		check(l, r);
		Node total = rows[0].prod(l, r);
		assert(0 <= k && k < total.count);
		auto [x, leaf] = walk<true>(l, r, total, [&](Node zero, Node) {
			bool one = k >= zero.count;
			if (one) k -= zero.count;
			return one;
		});
		return leaf.count == 1 ? T(leaf.sum) : index.values[x];
	}
	T kth_largest(int l, int r, Count k) const { return kth_smallest(l, r, count(l, r) - 1 - k); }
	Sum sum_smallest(int l, int r, Count k) const { check(l, r); return selected_sum<false>(l, r, k, rows[0].prod(l, r)); }
	Sum sum_largest(int l, int r, Count k) const { check(l, r); return selected_sum<true>(l, r, k, rows[0].prod(l, r)); }
	Count min_count_smallest(int l, int r, Sum target) const { return sum_bound<false, true>(l, r, target); }
	Count min_count_largest(int l, int r, Sum target) const { return sum_bound<true, true>(l, r, target); }
	Count max_count_smallest(int l, int r, Sum budget) const { return sum_bound<false, false>(l, r, budget); }
	Count max_count_largest(int l, int r, Sum budget) const { return sum_bound<true, false>(l, r, budget); }
};

#endif
