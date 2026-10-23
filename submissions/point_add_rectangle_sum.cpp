#line 1 "test/weighted_wavelet_matrix_rectangle.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/point_add_rectangle_sum"
#line 1 "data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp"



#line 1 "components/wavelet_compression.hpp"



#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace wavelet_detail {
template<class T>
std::uint32_t lower_code(const std::vector<T> &values, bool dense, const T &x) {
	if (values.empty()) return 0;
	if constexpr (std::is_integral_v<T> && sizeof(T) <= 8)
		if (dense) {
			if (x <= values.front()) return 0;
			if (values.back() < x) return static_cast<int>(values.size());
			return std::uint64_t(x) - std::uint64_t(values.front());
		}
	int p = 0, length = static_cast<int>(values.size());
	while (length > 1) {
		int half = length / 2;
		p += (values[p + half] < x) * half;
		length -= half;
	}
	return p + (values[p] < x);
}

template<class T, bool WithPositions = true>
std::vector<std::uint32_t> compress(const std::vector<T> &a, std::vector<T> &values,
	int &ordered, std::vector<int> &positions, std::vector<int> &offsets) {
	assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
	const int n = static_cast<int>(a.size());
	ordered = 0;
	if constexpr (WithPositions) offsets.clear();
	std::vector<std::uint32_t> current(n);
	bool ascending = std::is_sorted(a.begin(), a.end());
	if (ascending || std::is_sorted(a.rbegin(), a.rend())) {
		ordered = ascending ? 1 : -1;
		values.clear();
		values.reserve(n);
		for (int i = 0; i < n; ++i) {
			int j = ascending ? i : n - 1 - i;
			if (values.empty() || values.back() != a[j]) values.push_back(a[j]);
			current[j] = static_cast<int>(values.size()) - 1;
		}
		if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
		if constexpr (WithPositions) positions.clear();
		return current;
	}
	if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && sizeof(T) <= 8) {
		if (n >= 1024) {
			using U = std::make_unsigned_t<T>;
			struct [[gnu::packed]] Item {
				U key;
				std::uint32_t index;
			};
			std::vector<Item> order(n);
			U lo = std::numeric_limits<U>::max(), hi = 0;
			constexpr U sign = std::is_signed_v<T> ? U(1) << (sizeof(T) * 8 - 1) : U(0);
			for (int i = 0; i < n; ++i) {
				U key = U(a[i]) ^ sign;
				order[i] = {key, std::uint32_t(i)};
				lo = std::min(lo, key);
				hi = std::max(hi, key);
			}
			if (hi - lo < 65536 && std::uint64_t(hi - lo) < unsigned(n)) {
				std::vector<int> codes(std::size_t(hi - lo) + 1, -1);
				for (const auto &v : order) codes[v.key - lo] = 0;
				values.clear();
				values.reserve(codes.size());
				for (std::size_t i = 0; i < codes.size(); ++i)
					if (codes[i] == 0) {
						codes[i] = static_cast<int>(values.size());
						values.push_back(T(U(lo + U(i)) ^ sign));
					}
				for (const auto &v : order) current[v.index] = codes[v.key - lo];
				if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
				if constexpr (WithPositions) positions.clear();
				return current;
			}
			std::vector<Item> next(n);
			int width = std::bit_width(U(lo ^ hi)), span_width = std::bit_width(U(hi - lo));
			if ((width + 10) / 11 > (span_width + 10) / 11) {
				for (auto &v : order) v.key -= lo;
				width = span_width;
			}
			for (int shift = 0; shift < width; shift += 11) {
				std::array<int, 2048> offsets{};
				for (const auto &v : order) ++offsets[(v.key >> shift) & 2047];
				int total = 0;
				for (int &c : offsets) {
					int t = c;
					c = total;
					total += t;
				}
				for (const auto &v : order) next[offsets[(v.key >> shift) & 2047]++] = v;
				order.swap(next);
			}
			values.clear();
			values.reserve(n);
			U previous = 0;
			if constexpr (WithPositions) {
				positions.resize(n);
				offsets.reserve(std::size_t(n) + 1);
			}
			for (int j = 0; j < n; ++j) {
				const auto &v = order[j];
				if (values.empty() || v.key != previous) {
					values.push_back(a[v.index]);
					if constexpr (WithPositions) offsets.push_back(j);
				}
				previous = v.key;
				current[v.index] = static_cast<int>(values.size()) - 1;
				if constexpr (WithPositions) positions[j] = v.index;
			}
			if constexpr (WithPositions) offsets.push_back(n);
			if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
			return current;
		}
	}
	values = a;
	std::sort(values.begin(), values.end());
	values.erase(std::unique(values.begin(), values.end()), values.end());
	if (values.capacity() > 2 * values.size()) values.shrink_to_fit();
	for (int i = 0; i < n; ++i) current[i] = lower_code<T>(values, false, a[i]);
	if constexpr (WithPositions) positions.clear();
	return current;
}

template<class T> struct ValueIndex {
	int n = 0;
	std::vector<T> values;
	std::vector<std::uint32_t> original;
	void build(const std::vector<T> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = static_cast<int>(a.size());
		int ordered = 0;
		std::vector<int> positions, offsets;
		original = compress<T, false>(a, values, ordered, positions, offsets);
	}
	std::uint32_t code(const T &x) const { return lower_code(values, false, x); }
};
}


#line 1 "components/quad_bit_vector.hpp"



#line 8 "components/quad_bit_vector.hpp"
#include <cstddef>
#line 11 "components/quad_bit_vector.hpp"
#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#endif

struct QuadBitVector {
	struct Block {
		std::uint64_t low = 0, high = 0;
		std::array<int, 4> prefix{};
	};
	int n;
	std::vector<Block> blocks;

#if defined(__x86_64__) || defined(__i386__)
	__attribute__((target("avx2"))) static void pack_avx2(const std::uint32_t *a, int n, int shift, Block *blocks) {
		auto low_shift = _mm_cvtsi32_si128(31 - shift), high_shift = _mm_cvtsi32_si128(30 - shift);
		for (int w = 0; w < (n >> 6) + bool(n & 63); ++w) {
			int start = w * 64;
			std::uint64_t low = 0, high = 0;
			int length = std::min(64, n - start), j = 0;
			for (; j + 8 <= length; j += 8) {
				auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(a + start + j));
				low |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, low_shift)))) << j;
				high |= std::uint64_t(_mm256_movemask_ps(_mm256_castsi256_ps(_mm256_sll_epi32(x, high_shift)))) << j;
			}
			for (; j < length; ++j) {
				low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
				high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
			}
			blocks[w].low = low;
			blocks[w].high = high;
		}
		blocks[n >> 6].low = n & 63 ? blocks[n >> 6].low : 0;
		blocks[n >> 6].high = n & 63 ? blocks[n >> 6].high : 0;
	}
#endif
	explicit QuadBitVector(int n = 0) : n(n), blocks((n >> 6) + 1) {}
	void build(const std::vector<std::uint32_t> &a, int shift) {
		assert(a.size() == std::size_t(n) && 0 <= shift && shift <= 30);
		[[maybe_unused]] bool vectorized = false;
#if defined(__x86_64__) || defined(__i386__)
		vectorized = n >= 64 && __builtin_cpu_supports("avx2");
		if (vectorized) pack_avx2(a.data(), n, shift, blocks.data());
#endif
		std::array<int, 4> counts{};
		for (int w = 0; w < static_cast<int>(blocks.size()); ++w) {
			std::uint64_t low = 0, high = 0;
			int start = w * 64, length = std::min(64, n - start);
			if (vectorized) low = blocks[w].low, high = blocks[w].high;
			else
				for (int j = 0; j < length; ++j) {
					low |= std::uint64_t((a[start + j] >> shift) & 1) << j;
					high |= std::uint64_t((a[start + j] >> (shift + 1)) & 1) << j;
				}
			blocks[w] = {low, high, counts};
			int c1 = std::popcount(low & ~high), c2 = std::popcount(~low & high), c3 = std::popcount(low & high);
			counts[0] += length - c1 - c2 - c3;
			counts[1] += c1;
			counts[2] += c2;
			counts[3] += c3;
		}
	}
	std::uint64_t mask(const Block &b, int digit) const {
		return ((digit & 1) ? b.low : ~b.low) & ((digit & 2) ? b.high : ~b.high);
	}

	int rank(int r, int digit) const {
		const auto &b = blocks[r >> 6];
		return b.prefix[digit] + std::popcount(mask(b, digit) & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	int below(int r, int digit) const {
		if (!digit) return 0;
		if (digit == 1) return rank(r, 0);
		if (digit == 3) return r - rank(r, 3);
		const auto &b = blocks[r >> 6];
		return b.prefix[0] + b.prefix[1] + std::popcount(~b.high & ((std::uint64_t(1) << (r & 63)) - 1));
	}
	std::array<int, 4> ranks(int r) const {
		const auto &b = blocks[r >> 6];
		std::uint64_t m = (std::uint64_t(1) << (r & 63)) - 1;
		std::array<int, 4> a;
		a[1] = b.prefix[1] + std::popcount(b.low & ~b.high & m);
		a[2] = b.prefix[2] + std::popcount(~b.low & b.high & m);
		a[3] = b.prefix[3] + std::popcount(b.low & b.high & m);
		a[0] = r - a[1] - a[2] - a[3];
		return a;
	}
};


#line 1 "components/fenwick_sum.hpp"



#line 8 "components/fenwick_sum.hpp"

template <class T>
class FenwickSum {
	std::vector<T> data;

  public:
	FenwickSum() = default;
	explicit FenwickSum(int n) { assert(n >= 0); data.resize(n); }
	explicit FenwickSum(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= static_cast<unsigned>(std::numeric_limits<int>::max()));
		data = a;
		for (unsigned i = 1; i <= data.size(); ++i) {
			unsigned p = i + (i & -i);
			if (p <= data.size()) data[p - 1] += data[i - 1];
		}
	}
	int size() const { return static_cast<int>(data.size()); }
	void add(int p, const T &x) {
		assert(0 <= p && p < size());
		for (unsigned i = unsigned(p) + 1; i <= data.size(); i += i & -i) data[i - 1] += x;
	}
	void add_pair(int p, const T &x, int q, const T &y) {
		assert(0 <= p && p < size() && 0 <= q && q < size());
		unsigned a = unsigned(p) + 1, b = unsigned(q) + 1;
		while (a != b && (a <= data.size() || b <= data.size())) {
			if (a < b) data[a - 1] += x, a += a & -a;
			else data[b - 1] += y, b += b & -b;
		}
		T both = x;
		both += y;
		for (; a <= data.size(); a += a & -a) data[a - 1] += both;
	}
	T prefix(int r) const {
		assert(0 <= r && r <= size());
		T result{};
		for (; r; r &= r - 1) result += data[r - 1];
		return result;
	}
	template <class Accum>
	Accum prod_as(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		Accum result{};
		if (l == r) return result;
		unsigned shared = unsigned(l) & (std::numeric_limits<unsigned>::max() << std::bit_width(unsigned(l ^ r)));
		for (; unsigned(l) != shared; l &= l - 1) result -= data[l - 1];
		for (; unsigned(r) != shared; r &= r - 1) result += data[r - 1];
		return result;
	}
	T prod(int l, int r) const { return prod_as<T>(l, r); }
};


#line 7 "data_structure/wavelet_matrix/weighted_wavelet_matrix.hpp"
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
	Node less_than(int l, int r, std::uint32_t x) const {
		if (!x || l == r) return {};
		if constexpr (WithSum) assert(sum_ready);
		if (x >= index.values.size()) return row<WithSum>(0, l, r);
		Node answer{};
		for (int d = 0; d < levels && l < r; ++d) {
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
		return less_than(l, r, c + 1).count - less_than(l, r, c).count;
	}
	Count range_freq(int l, int r, const T &upper) const { check(l, r); return less_than(l, r, index.code(upper)).count; }
	Count range_freq(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? less_than(l, r, index.code(upper)).count - less_than(l, r, index.code(lower)).count : 0;
	}
	Sum range_sum(int l, int r) const { check(l, r); assert(sum_ready); return sums[0].prod(l, r); }
	Sum range_sum(int l, int r, const T &upper) const { check(l, r); return less_than<true>(l, r, index.code(upper)).sum; }
	Sum range_sum(int l, int r, const T &lower, const T &upper) const {
		check(l, r);
		return lower < upper ? less_than<true>(l, r, index.code(upper)).sum - less_than<true>(l, r, index.code(lower)).sum : 0;
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


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#line 18 "components/fast_io.hpp"
#if defined(__unix__) || defined(__APPLE__)
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

namespace fast_io_detail { struct BulkInput; struct BulkOutput; }

class FastInput {
	friend struct fast_io_detail::BulkInput;
	static constexpr std::size_t capacity = 1 << 16;
	FILE *file = nullptr;
	std::unique_ptr<char[]> buffer;
	const char *current = "", *end = current;
	std::string scratch;
	void *mapping = nullptr;
	std::size_t mapped_size = 0;
	bool refill() {
		if (!file) return false;
		std::size_t n = std::fread(buffer.get(), 1, capacity, file);
		current = buffer.get();
		end = current + n;
		return n;
	}
	static const char *scan(const char *p, const char *end) {
		while (end - p >= 8) {
			std::uint64_t x;
			std::memcpy(&x, p, 8);
			if ((x - 0x2121212121212121ULL) & ~x & 0x8080808080808080ULL) break;
			p += 8;
		}
		while (p != end && static_cast<unsigned char>(*p) > ' ') ++p;
		return p;
	}

public:
	explicit FastInput(FILE *input = stdin) {
#if defined(__unix__) || defined(__APPLE__)
		struct stat status;
		int fd = fileno(input);
		auto offset = ftello(input);
		if (fd >= 0 && offset >= 0 && !fstat(fd, &status) && S_ISREG(status.st_mode) && status.st_size > offset) {
			mapped_size = status.st_size;
			mapping = mmap(nullptr, mapped_size, PROT_READ, MAP_PRIVATE, fd, 0);
			if (mapping != MAP_FAILED) {
				current = static_cast<const char *>(mapping) + offset;
				end = static_cast<const char *>(mapping) + mapped_size;
				return;
			}
			mapping = nullptr;
		}
#endif
		file = input;
		buffer = std::make_unique_for_overwrite<char[]>(capacity);
	}
	explicit FastInput(std::string_view input) : current(input.empty() ? "" : input.data()), end(current + input.size()) {}
	FastInput(const FastInput &) = delete;
	FastInput &operator=(const FastInput &) = delete;
	~FastInput() {
#if defined(__unix__) || defined(__APPLE__)
		if (mapping) munmap(mapping, mapped_size);
#endif
	}
	std::string_view token() {
		while (true) {
			while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
			if (current != end) break;
			if (!refill()) return {};
		}
		const char *begin = current;
		current = scan(current, end);
		if (current != end || !file) return {begin, std::size_t(current - begin)};
		scratch.assign(begin, end);
		while (refill()) {
			current = scan(current, end);
			scratch.append(buffer.get(), std::size_t(current - buffer.get()));
			if (current != end) break;
		}
		return scratch;
	}
	template <class T>
	bool read(T &value) {
		if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
			while (true) {
				while (current != end && static_cast<unsigned char>(*current) <= ' ') ++current;
				if (current != end) break;
				if (!refill()) return false;
			}
			if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4 && std::endian::native == std::endian::little) {
				if (end - current >= 11) {
					std::uint64_t x;
					std::memcpy(&x, current, 8);
					if (!(((x + 0x4646464646464646ULL) | (x - 0x3030303030303030ULL)) & 0x8080808080808080ULL)) {
						x &= 0x0f0f0f0f0f0f0f0fULL;
						x = (x * 10 + (x >> 8)) & 0x00ff00ff00ff00ffULL;
						x = (x * 100 + (x >> 16)) & 0x0000ffff0000ffffULL;
						std::uint64_t v = std::uint32_t(x * 10000 + (x >> 32));
						int n = 8;
						for (; n < 10 && static_cast<unsigned char>(current[n]) - unsigned('0') < 10; ++n)
							v = v * 10 + (current[n] - '0');
						if (static_cast<unsigned char>(current[n]) <= ' ' && v <= 0xffffffffULL) {
							current += n;
							value = T(v);
							return true;
						}
					}
				}
			}
			T parsed{};
			auto result = std::from_chars(current, end, parsed);
			if (result.ec == std::errc{} && (result.ptr != end ? static_cast<unsigned char>(*result.ptr) <= ' ' : !file)) {
				current = result.ptr;
				value = parsed;
				return true;
			}
		}
		auto s = token();
		if (s.empty()) return false;
		if constexpr (std::is_same_v<T, bool>) {
			assert(s == "0" || s == "1");
			value = s[0] == '1';
		} else if constexpr (std::is_integral_v<T>) {
			auto result = std::from_chars(s.data(), s.data() + s.size(), value);
			assert(result.ec == std::errc{} && result.ptr == s.data() + s.size());
			(void)result;
		} else value = T(s);
		return true;
	}
};

class FastOutput {
	friend struct fast_io_detail::BulkOutput;
	inline static constexpr auto quads = [] {
		std::array<std::array<char, 4>, 10000> table{};
		for (int i = 0; i < 10000; ++i)
			table[i] = {char('0' + i / 1000), char('0' + i / 100 % 10), char('0' + i / 10 % 10), char('0' + i % 10)};
		return table;
	}();
	FILE *file;
	std::unique_ptr<char[]> buffer = std::make_unique_for_overwrite<char[]>(1 << 16);
	std::size_t capacity = 1 << 16, length = 0;
	char *reserve(std::size_t n) {
		if (n > capacity - length) {
			if (!flush()) throw std::runtime_error("output failed");
			if (n > capacity) {
				std::size_t next = std::max(n, 2 * capacity);
				buffer = std::make_unique_for_overwrite<char[]>(next);
				capacity = next;
			}
		}
		return buffer.get() + length;
	}

public:
	explicit FastOutput(FILE *output = stdout) : file(output) {}
	FastOutput(const FastOutput &) = delete;
	FastOutput &operator=(const FastOutput &) = delete;
	~FastOutput() { flush(); }
	bool flush() {
		std::size_t offset = 0;
		while (offset < length) {
			auto n = std::fwrite(buffer.get() + offset, 1, length - offset, file);
			if (!n) {
				std::memmove(buffer.get(), buffer.get() + offset, length - offset);
				length -= offset;
				return false;
			}
			offset += n;
		}
		length = 0;
		return std::fflush(file) == 0;
	}
	void write(char c) { *reserve(1) = c; ++length; }
	void write(__uint128_t value) {
		char digits[40];
		char *end = digits + sizeof(digits), *begin = end;
		do { *--begin = char('0' + value % 10); value /= 10; } while (value);
		write(std::string_view(begin, end - begin));
	}
	void write(__int128_t value) {
		__uint128_t magnitude = __uint128_t(value);
		if (value < 0) write('-'), magnitude = 0 - magnitude;
		write(magnitude);
	}
	void write(std::string_view s) {
		char *out = reserve(s.size());
		if (!s.empty()) std::memcpy(out, s.data(), s.size());
		length += s.size();
	}
	template <class T> requires std::is_integral_v<T>
	void write(T value) {
		char *out = reserve(32);
		if constexpr (std::is_same_v<T, bool>) *out = '0' + value, ++length;
		else if constexpr (std::is_unsigned_v<T> && sizeof(T) == 4) {
			char *p = out;
			if (value >= 100000000) {
				auto high = value / 100000000, low = value % 100000000;
				p = std::to_chars(p, p + 2, high).ptr;
				std::memcpy(p, quads[low / 10000].data(), 4);
				std::memcpy(p + 4, quads[low % 10000].data(), 4);
				p += 8;
			} else if (value >= 10000) {
				p = std::to_chars(p, p + 4, value / 10000).ptr;
				std::memcpy(p, quads[value % 10000].data(), 4);
				p += 4;
			} else p = std::to_chars(p, p + 4, value).ptr;
			length += p - out;
		} else length += std::to_chars(out, out + 32, value).ptr - out;
	}
	template <class T> requires requires(const T &value, char *out) { value.decimal_capacity(); value.to_chars(out); }
	void write(const T &value) {
		char *out = reserve(value.decimal_capacity());
		length += value.to_chars(out) - out;
	}
	template <class T>
	void println(const T &value) { write(value); write('\n'); }
};


#line 4 "test/weighted_wavelet_matrix_rectangle.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n); input.read(q);
	struct Point { int x, y; std::int64_t weight; };
	struct Query { int type, x, y, r, u; std::int64_t weight; };
	std::vector<Point> points(n);
	for (auto &p : points) input.read(p.x), input.read(p.y), input.read(p.weight);
	std::vector<Query> queries(q);
	for (auto &z : queries) {
		input.read(z.type); input.read(z.x); input.read(z.y);
		if (!z.type) input.read(z.weight), points.push_back({z.x, z.y, 0});
		else input.read(z.r), input.read(z.u);
	}
	std::sort(points.begin(), points.end(), [](auto a, auto b) { return std::pair(a.x, a.y) < std::pair(b.x, b.y); });
	std::vector<std::pair<int, int>> coordinates;
	std::vector<int> values;
	std::vector<std::int64_t> weights;
	for (auto p : points) {
		if (!coordinates.empty() && coordinates.back() == std::pair(p.x, p.y)) weights.back() += p.weight;
		else coordinates.emplace_back(p.x, p.y), values.push_back(p.y), weights.push_back(p.weight);
	}
	WeightedWaveletMatrix<int, std::int64_t, __int128_t> wm(values, weights, false);
	for (auto z : queries) {
		if (!z.type) {
			int p = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.x, z.y)) - coordinates.begin();
			wm.add_weight(p, z.weight);
		} else {
			int l = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.x, -1)) - coordinates.begin();
			int r = std::lower_bound(coordinates.begin(), coordinates.end(), std::pair(z.r, -1)) - coordinates.begin();
			output.println(wm.range_freq(l, r, z.y, z.u));
		}
	}
}
