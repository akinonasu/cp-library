#ifndef DYNAMIC_QUAD_VECTOR_HPP
#define DYNAMIC_QUAD_VECTOR_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace dynamic_wavelet_detail {
template<int Block = 1024, int Branch = 32>
class QuadVector {
	static_assert(128 <= Block && Block <= 32768 && Block % 64 == 0);
	static_assert(Branch >= 4 && std::has_single_bit(unsigned(Branch)));
	using Word = std::uint64_t;
	using Counts = std::array<int, 4>;
	static constexpr int words = Block / 64;
	struct Leaf {
		std::array<Word, words> low{}, high{};
		std::array<std::array<std::uint16_t, 4>, words> prefix{};
		int length = 0;
		Counts counts{};
	};
	struct Node {
		std::array<int, Branch> ids{};
		std::array<int, Branch + 1> sizes{};
		std::array<std::array<int, Branch + 1>, 4> sums{};
		int count = 0;
	};
	std::vector<Leaf> leaves;
	std::vector<Node> nodes;
	int root = 0, n = 0, free_leaf = 0, free_node = 0, uniform = -1;
	Counts total{};
	void refresh_uniform() {
		uniform = -1;
		if (!n) return;
		for (int d = 0; d < 4; ++d) if (total[d] == n) uniform = d;
	}
	static int popcount(Word value) {
#if defined(__POPCNT__)
		return std::popcount(value);
#elif defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(DYNAMIC_QUAD_DISABLE_SIMD)
		if (__builtin_cpu_supports("popcnt")) {
			Word result;
			__asm__("popcnt {%1, %0|%0, %1}" : "=r"(result) : "r"(value) : "cc");
			return int(result);
		}
#endif
		return std::popcount(value);
	}
	static Word mask(int length) { return length == 64 ? ~Word(0) : (Word(1) << length) - 1; }
	static Word extract(const Word* data, int count, int p) {
		int w = p >> 6, shift = p & 63;
		Word value = w < count ? data[w] >> shift : 0;
		if (shift && w + 1 < count) value |= data[w + 1] << (64 - shift);
		return value;
	}
	int length(int id) const { return id & 1 ? leaves[id >> 1].length : nodes[id >> 1].sizes[nodes[id >> 1].count]; }
	int count(int id, int digit) const { return id & 1 ? leaves[id >> 1].counts[digit] : nodes[id >> 1].sums[digit][nodes[id >> 1].count]; }
	int make_leaf(const Leaf& leaf) {
		if (free_leaf) {
			int id = free_leaf;
			free_leaf = leaves[id >> 1].length;
			leaves[id >> 1] = leaf;
			return id;
		}
		assert(leaves.size() < (std::size_t(1) << 30));
		leaves.push_back(leaf);
		return (int(leaves.size()) - 1) * 2 + 1;
	}
	void release_leaf(int id) {
		leaves[id >> 1].length = free_leaf;
		free_leaf = id;
	}
	void pull(int id) {
		auto& node = nodes[id >> 1];
		int size = 0;
		Counts sums{};
		for (int i = 0; i < node.count; ++i) {
			node.sizes[i + 1] = size += length(node.ids[i]);
			for (int d = 0; d < 4; ++d) node.sums[d][i + 1] = sums[d] += count(node.ids[i], d);
		}
	}
	int make_node(const int* children, int size) {
		Node node;
		node.count = size;
		int length = 0;
		Counts counts{};
		for (int i = 0; i < size; ++i) {
			node.ids[i] = children[i];
			node.sizes[i + 1] = length += this->length(children[i]);
			for (int d = 0; d < 4; ++d) node.sums[d][i + 1] = counts[d] += count(children[i], d);
		}
		if (free_node) {
			int id = free_node;
			free_node = nodes[id >> 1].count;
			nodes[id >> 1] = node;
			return id;
		}
		if (nodes.empty()) nodes.emplace_back();
		assert(nodes.size() < (std::size_t(1) << 30));
		nodes.push_back(node);
		return (int(nodes.size()) - 1) * 2;
	}
	void release_node(int id) {
		nodes[id >> 1].count = free_node;
		free_node = id;
	}
	static bool has_avx2() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(DYNAMIC_QUAD_DISABLE_SIMD)
		return __builtin_cpu_supports("avx2") && __builtin_cpu_supports("popcnt");
#else
		return false;
#endif
	}
	static bool has_avx512() {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__)) && !defined(DYNAMIC_QUAD_DISABLE_SIMD) && !defined(DYNAMIC_QUAD_DISABLE_AVX512)
		return __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("popcnt");
#else
		return false;
#endif
	}
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
	__attribute__((target("avx2,popcnt")))
	static int child_avx2(const Node& node, int p) {
		auto value = _mm256_set1_epi32(p);
		auto a = _mm256_cmpgt_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(node.sizes.data())), value);
		auto b = _mm256_cmpgt_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(node.sizes.data() + 8)), value);
		auto c = _mm256_cmpgt_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(node.sizes.data() + 16)), value);
		auto d = _mm256_cmpgt_epi32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(node.sizes.data() + 24)), value);
		unsigned passed = ~unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(a)))
			& ~(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(b))) << 8)
			& ~(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(c))) << 16)
			& ~(unsigned(_mm256_movemask_ps(_mm256_castsi256_ps(d))) << 24);
		unsigned active = node.count == 32 ? ~0U : (1U << node.count) - 1;
		return std::popcount(passed & active) - 1;
	}
	__attribute__((target("avx512f,popcnt")))
	static int child_avx512(const Node& node, int p) {
		auto value = _mm512_set1_epi32(p);
		unsigned passed = unsigned(_mm512_cmple_epi32_mask(_mm512_loadu_si512(node.sizes.data()), value));
		passed |= unsigned(_mm512_cmple_epi32_mask(_mm512_loadu_si512(node.sizes.data() + 16), value)) << 16;
		unsigned active = node.count == 32 ? ~0U : (1U << node.count) - 1;
		return std::popcount(passed & active) - 1;
	}
	__attribute__((target("avx2,popcnt")))
	static void add_range_avx2(Node& node, int first, int last, int size_delta, int digit, int sum_delta) {
		int i = first;
		auto sizes = _mm256_set1_epi32(size_delta), sums = _mm256_set1_epi32(sum_delta);
		for (; i + 8 <= last; i += 8) {
			if (size_delta) {
				auto* p = reinterpret_cast<__m256i*>(node.sizes.data() + i + 1);
				_mm256_storeu_si256(p, _mm256_add_epi32(_mm256_loadu_si256(p), sizes));
			}
			if (sum_delta) {
				auto* p = reinterpret_cast<__m256i*>(node.sums[digit].data() + i + 1);
				_mm256_storeu_si256(p, _mm256_add_epi32(_mm256_loadu_si256(p), sums));
			}
		}
		for (; i < last; ++i) {
			node.sizes[i + 1] += size_delta;
			node.sums[digit][i + 1] += sum_delta;
		}
	}
	__attribute__((target("avx512f,popcnt")))
	static void add_range_avx512(Node& node, int first, int last, int size_delta, int digit, int sum_delta) {
		auto sizes = _mm512_set1_epi32(size_delta), sums = _mm512_set1_epi32(sum_delta);
		for (int i = first; i < last; i += 16) {
			int length = std::min(16, last - i);
			__mmask16 active = __mmask16((1U << length) - 1);
			if (size_delta) {
				auto* p = node.sizes.data() + i + 1;
				_mm512_mask_storeu_epi32(p, active, _mm512_add_epi32(_mm512_maskz_loadu_epi32(active, p), sizes));
			}
			if (sum_delta) {
				auto* p = node.sums[digit].data() + i + 1;
				_mm512_mask_storeu_epi32(p, active, _mm512_add_epi32(_mm512_maskz_loadu_epi32(active, p), sums));
			}
		}
	}
#endif
	static int child(const Node& node, int p) {
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Branch == 32) {
			if (has_avx512()) return child_avx512(node, p);
			if (has_avx2()) return child_avx2(node, p);
		}
#endif
		if constexpr (Branch <= 8) {
			int j = 0;
			while (j + 1 < node.count && p >= node.sizes[j + 1]) ++j;
			return j;
		} else {
			int low = 0, high = node.count - 1;
			while (low < high) {
				int middle = (low + high) / 2;
				if (p < node.sizes[middle + 1]) high = middle;
				else low = middle + 1;
			}
			return low;
		}
	}
	static void add_range(Node& node, int first, int last, int size_delta, int digit, int sum_delta) {
		if (first == last) return;
#if (defined(__x86_64__) || defined(__i386__)) && (defined(__GNUC__) || defined(__clang__))
		if constexpr (Branch == 32) {
			if (has_avx512()) { add_range_avx512(node, first, last, size_delta, digit, sum_delta); return; }
			if (has_avx2()) { add_range_avx2(node, first, last, size_delta, digit, sum_delta); return; }
		}
#endif
		for (int i = first; i < last; ++i) {
			node.sizes[i + 1] += size_delta;
			node.sums[digit][i + 1] += sum_delta;
		}
	}

	static int access_leaf(const Leaf& leaf, int p) {
		return int((leaf.low[p >> 6] >> (p & 63)) & 1) | (int((leaf.high[p >> 6] >> (p & 63)) & 1) << 1);
	}
	static int rank_leaf(const Leaf& leaf, int p, int digit) {
		if (p == leaf.length) return leaf.counts[digit];
		int w = p >> 6;
		Word selected = ((digit & 1) ? leaf.low[w] : ~leaf.low[w]) & ((digit & 2) ? leaf.high[w] : ~leaf.high[w]);
		return leaf.prefix[w][digit] + popcount(selected & mask(p & 63));
	}
	static Counts ranks_leaf(const Leaf& leaf, int p) {
		if (p == leaf.length) return leaf.counts;
		int w = p >> 6;
		Word selected = mask(p & 63), low = leaf.low[w], high = leaf.high[w];
		Counts result;
		result[1] = leaf.prefix[w][1] + popcount(low & ~high & selected);
		result[2] = leaf.prefix[w][2] + popcount(~low & high & selected);
		result[3] = leaf.prefix[w][3] + popcount(low & high & selected);
		result[0] = p - result[1] - result[2] - result[3];
		return result;
	}
	int rank_node(int id, int p, int digit) const {
		int before = 0;
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			p -= node.sizes[j], before += node.sums[digit][j];
			id = node.ids[j];
		}
		return before + rank_leaf(leaves[id >> 1], p, digit);
	}
	Counts ranks_node(int id, int p) const {
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			p -= node.sizes[j];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
			id = node.ids[j];
		}
		auto result = ranks_leaf(leaves[id >> 1], p);
		for (int d = 0; d < 4; ++d) result[d] += before[d];
		return result;
	}
	static void insert_leaf(Leaf& leaf, int p, int digit) {
		int w = p >> 6, shift = p & 63;
		if (!(leaf.length & 63)) for (int d = 0; d < 4; ++d) leaf.prefix[leaf.length >> 6][d] = leaf.counts[d];
		for (int i = leaf.length >> 6; i > w; --i) {
			int carry = int(leaf.low[i - 1] >> 63) | (int(leaf.high[i - 1] >> 63) << 1);
			leaf.low[i] = (leaf.low[i] << 1) | (leaf.low[i - 1] >> 63);
			leaf.high[i] = (leaf.high[i] << 1) | (leaf.high[i - 1] >> 63);
			++leaf.prefix[i][digit];
			--leaf.prefix[i][carry];
		}
		Word low = mask(shift), a = leaf.low[w], b = leaf.high[w];
		leaf.low[w] = (a & low) | ((a & ~low) << 1) | (Word(digit & 1) << shift);
		leaf.high[w] = (b & low) | ((b & ~low) << 1) | (Word(digit >> 1) << shift);
		++leaf.length, ++leaf.counts[digit];
	}
	static void erase_leaf(Leaf& leaf, int p, int digit) {
		int w = p >> 6, last = (leaf.length - 1) >> 6;
		Word low = mask(p & 63), a = leaf.low[w], b = leaf.high[w];
		leaf.low[w] = (a & low) | ((a >> 1) & ~low);
		leaf.high[w] = (b & low) | ((b >> 1) & ~low);
		if (w < last) leaf.low[w] |= leaf.low[w + 1] << 63, leaf.high[w] |= leaf.high[w + 1] << 63;
		for (int i = w + 1; i <= last; ++i) {
			int carry = int(leaf.low[i] & 1) | (int(leaf.high[i] & 1) << 1);
			--leaf.prefix[i][digit];
			++leaf.prefix[i][carry];
			leaf.low[i] = (leaf.low[i] >> 1) | (i < last ? leaf.low[i + 1] << 63 : 0);
			leaf.high[i] = (leaf.high[i] >> 1) | (i < last ? leaf.high[i + 1] << 63 : 0);
		}
		--leaf.length, --leaf.counts[digit];
	}
	static void assign_leaf(Leaf& leaf, const Word* low, const Word* high, int count, int start, int length) {
		leaf = Leaf{};
		leaf.length = length;
		for (int i = 0; i < (length + 63) / 64; ++i) {
			int size = std::min(64, length - i * 64);
			Word a = extract(low, count, start + i * 64) & mask(size);
			Word b = extract(high, count, start + i * 64) & mask(size);
			leaf.low[i] = a, leaf.high[i] = b;
			Counts added;
			added[1] = popcount(a & ~b), added[2] = popcount(~a & b), added[3] = popcount(a & b);
			added[0] = size - added[1] - added[2] - added[3];
			for (int d = 0; d < 4; ++d) leaf.prefix[i][d] = leaf.counts[d], leaf.counts[d] += added[d];
		}
	}
	bool rebalance(int parent, int j) {
		auto& node = nodes[parent >> 1];
		int l = j ? j - 1 : 0, r = l + 1;
		int left = node.ids[l], right = node.ids[r];
		bool merge;
		if (left & 1) {
			auto& a = leaves[left >> 1];
			auto& b = leaves[right >> 1];
			std::array<Word, 2 * words> low{}, high{};
			std::copy(a.low.begin(), a.low.end(), low.begin());
			std::copy(a.high.begin(), a.high.end(), high.begin());
			int offset = a.length >> 6, shift = a.length & 63, size = a.length + b.length;
			for (int i = 0; i < (b.length + 63) / 64; ++i) {
				low[offset + i] |= b.low[i] << shift;
				high[offset + i] |= b.high[i] << shift;
				if (shift) low[offset + i + 1] |= b.low[i] >> (64 - shift), high[offset + i + 1] |= b.high[i] >> (64 - shift);
			}
			merge = size <= Block;
			int first = merge ? size : size / 2;
			assign_leaf(a, low.data(), high.data(), int(low.size()), 0, first);
			if (merge) release_leaf(right);
			else assign_leaf(b, low.data(), high.data(), int(low.size()), first, size - first);
		} else {
			auto& a = nodes[left >> 1];
			auto& b = nodes[right >> 1];
			std::array<int, 2 * Branch> children;
			int size = a.count + b.count;
			for (int i = 0; i < a.count; ++i) children[i] = a.ids[i];
			for (int i = 0; i < b.count; ++i) children[a.count + i] = b.ids[i];
			merge = size <= Branch;
			a.count = merge ? size : size / 2;
			for (int i = 0; i < a.count; ++i) a.ids[i] = children[i];
			pull(left);
			if (merge) release_node(right);
			else {
				b.count = size - a.count;
				for (int i = 0; i < b.count; ++i) b.ids[i] = children[a.count + i];
				pull(right);
			}
		}
		if (merge) {
			for (int i = r; i + 1 < node.count; ++i) node.ids[i] = node.ids[i + 1];
			--node.count;
		}
		pull(parent);
		return merge;
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(DYNAMIC_QUAD_DISABLE_SIMD) && !defined(__BMI2__)
	__attribute__((target("bmi2")))
	static int select_bmi2(Word value, int k) {
		return std::countr_zero(_pdep_u64(Word(1) << k, value));
	}
#endif
	static int select_word(Word value, int k) {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(DYNAMIC_QUAD_DISABLE_SIMD)
#ifdef __BMI2__
		return std::countr_zero(_pdep_u64(Word(1) << k, value));
#else
		if (__builtin_cpu_supports("bmi2")) return select_bmi2(value, k);
#endif
#endif
		int result = 0;
		for (int shift : {32, 16, 8}) {
			int count = popcount(value & mask(shift));
			if (k >= count) k -= count, value >>= shift, result += shift;
			else value &= mask(shift);
		}
		while (k--) value &= value - 1;
		return result + std::countr_zero(value);
	}

  public:
	QuadVector() = default;
	explicit QuadVector(const std::vector<std::uint8_t>& values) { build(values); }
	QuadVector(const QuadVector&) = default;
	QuadVector& operator=(const QuadVector&) = default;
	QuadVector(QuadVector&& other) noexcept { swap(other); }
	QuadVector& operator=(QuadVector&& other) noexcept {
		if (this != &other) clear(), swap(other);
		return *this;
	}
	void swap(QuadVector& other) noexcept {
		leaves.swap(other.leaves), nodes.swap(other.nodes);
		std::swap(root, other.root), std::swap(n, other.n), std::swap(total, other.total);
		std::swap(free_leaf, other.free_leaf), std::swap(free_node, other.free_node), std::swap(uniform, other.uniform);
	}
	int size() const { return n; }
	int uniform_digit() const { return uniform; }
	bool empty() const { return !n; }
	std::size_t memory_usage() const { return leaves.capacity() * sizeof(Leaf) + nodes.capacity() * sizeof(Node); }
	void clear() {
		leaves.clear(), nodes.clear();
		root = n = free_leaf = free_node = 0;
		total.fill(0);
		uniform = -1;
	}
	void reserve(int size) {
		assert(size >= 0);
		if (!size) return;
		std::size_t capacity = std::size_t(size) / (Block / 2) + 2;
		leaves.reserve(capacity);
		if (size > Block) nodes.reserve(capacity / (Branch / 2 - 1) + 4);
	}
	void build(const std::vector<std::uint8_t>& values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		clear();
		if (values.empty()) return;
		n = int(values.size());
		std::vector<Word> low((values.size() + 63) / 64), high(low.size());
		assert(values.front() < 4);
		if (std::all_of(values.begin(), values.end(), [&](std::uint8_t value) { return value == values.front(); })) {
			std::fill(low.begin(), low.end(), Word(0) - Word(values.front() & 1));
			std::fill(high.begin(), high.end(), Word(0) - Word(values.front() >> 1));
		} else for (int i = 0; i < n; ++i) {
			assert(values[i] < 4);
			low[i >> 6] |= Word(values[i] & 1) << (i & 63);
			high[i >> 6] |= Word(values[i] >> 1) << (i & 63);
		}
		int target = Block * 3 / 4, count = n <= Block ? 1 : (n - 1) / target + 1;
		leaves.reserve(count);
		std::vector<int> current;
		current.reserve(count);
		for (int i = 0, start = 0; i < count; ++i) {
			int length = n / count + (i < n % count);
			Leaf leaf;
			assign_leaf(leaf, low.data(), high.data(), int(low.size()), start, length);
			for (int d = 0; d < 4; ++d) total[d] += leaf.counts[d];
			current.push_back(make_leaf(leaf));
			start += length;
		}
		while (current.size() > 1) {
			int count = (int(current.size()) - 1) / Branch + 1;
			std::vector<int> next;
			next.reserve(count);
			for (int i = 0, start = 0; i < count; ++i) {
				int length = int(current.size()) / count + (i < int(current.size()) % count);
				next.push_back(make_node(current.data() + start, length));
				start += length;
			}
			current.swap(next);
		}
		root = current[0];
		refresh_uniform();
	}
	int rank(int p, int digit) const {
		assert(0 <= p && p <= n && 0 <= digit && digit < 4);
		if (!p || !total[digit]) return 0;
		if (p == n) return total[digit];
		if (total[digit] == n) return p;
		return rank_node(root, p, digit);
	}
	std::pair<int, int> rank(int l, int r, int digit) const {
		assert(0 <= l && l <= r && r <= n && 0 <= digit && digit < 4);
		if (!total[digit]) return {0, 0};
		if (total[digit] == n) return {l, r};
		if (!l) return {0, rank(r, digit)};
		if (r == n) return {rank(l, digit), total[digit]};
		int id = root, before = 0;
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int a = child(node, l), b = child(node, r);
			if (a != b) return {before + node.sums[digit][a] + rank_node(node.ids[a], l - node.sizes[a], digit),
				before + node.sums[digit][b] + rank_node(node.ids[b], r - node.sizes[b], digit)};
			l -= node.sizes[a], r -= node.sizes[a], before += node.sums[digit][a];
			id = node.ids[a];
		}
		const auto& leaf = leaves[id >> 1];
		return {before + rank_leaf(leaf, l, digit), before + rank_leaf(leaf, r, digit)};
	}
	Counts ranks(int p) const {
		assert(0 <= p && p <= n);
		if (!p) return {};
		if (uniform >= 0) { Counts result{}; result[uniform] = p; return result; }
		if (p == n) return total;
		return ranks_node(root, p);
	}
	std::pair<Counts, Counts> ranks(int l, int r) const {
		assert(0 <= l && l <= r && r <= n);
		if (uniform >= 0) { Counts x{}, y{}; x[uniform] = l, y[uniform] = r; return {x, y}; }
		if (!l) return {Counts{}, ranks(r)};
		if (r == n) return {ranks(l), total};
		int id = root;
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int a = child(node, l), b = child(node, r);
			if (a != b) {
				auto x = ranks_node(node.ids[a], l - node.sizes[a]);
				auto y = ranks_node(node.ids[b], r - node.sizes[b]);
				for (int d = 0; d < 4; ++d) x[d] += before[d] + node.sums[d][a], y[d] += before[d] + node.sums[d][b];
				return {x, y};
			}
			l -= node.sizes[a], r -= node.sizes[a];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][a];
			id = node.ids[a];
		}
		auto x = ranks_leaf(leaves[id >> 1], l), y = ranks_leaf(leaves[id >> 1], r);
		for (int d = 0; d < 4; ++d) x[d] += before[d], y[d] += before[d];
		return {x, y};
	}
	std::pair<int, int> access_rank(int p) const {
		assert(0 <= p && p < n);
		if (uniform >= 0) return {p, uniform};
		int id = root;
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			p -= node.sizes[j];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
			id = node.ids[j];
		}
		const auto& leaf = leaves[id >> 1];
		int digit = access_leaf(leaf, p);
		return {before[digit] + rank_leaf(leaf, p, digit), digit};
	}
	int insert(int p, int digit) {
		assert(0 <= p && p <= n && n < std::numeric_limits<int>::max() && 0 <= digit && digit < 4);
		if (!root) {
			Leaf leaf;
			insert_leaf(leaf, 0, digit);
			root = make_leaf(leaf);
			++n, ++total[digit];
			uniform = total[digit] == n ? digit : -1;
			return 0;
		}
		std::array<int, 32> path, slots;
		int id = root, depth = 0, before = 0;
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			path[depth] = id, slots[depth++] = j;
			p -= node.sizes[j], before += node.sums[digit][j];
			id = node.ids[j];
		}
		before += rank_leaf(leaves[id >> 1], p, digit);
		int split = 0;
		if (leaves[id >> 1].length == Block) {
			Leaf original = leaves[id >> 1], right;
			assign_leaf(leaves[id >> 1], original.low.data(), original.high.data(), words, 0, Block / 2);
			assign_leaf(right, original.low.data(), original.high.data(), words, Block / 2, Block - Block / 2);
			split = make_leaf(right);
			if (p <= Block / 2) insert_leaf(leaves[id >> 1], p, digit);
			else insert_leaf(leaves[split >> 1], p - Block / 2, digit);
		} else insert_leaf(leaves[id >> 1], p, digit);
		while (depth) {
			id = path[--depth];
			int j = slots[depth];
			auto& node = nodes[id >> 1];
			if (!split) {
				add_range(node, j, node.count, 1, digit, 1);
				continue;
			}
			std::array<int, Branch + 1> children;
			for (int i = 0; i <= j; ++i) children[i] = node.ids[i];
			children[j + 1] = split;
			for (int i = j + 1; i < node.count; ++i) children[i + 1] = node.ids[i];
			int count = node.count + 1;
			node.count = count <= Branch ? count : count / 2;
			for (int i = 0; i < node.count; ++i) node.ids[i] = children[i];
			pull(id);
			int first = node.count;
			split = count <= Branch ? 0 : make_node(children.data() + first, count - first);
		}
		if (split) {
			std::array<int, 2> children{root, split};
			root = make_node(children.data(), 2);
		}
		++n, ++total[digit];
		uniform = total[digit] == n ? digit : -1;
		return before;
	}
	std::pair<int, int> erase_rank(int p) {
		assert(0 <= p && p < n);
		std::array<int, 32> path, slots;
		int id = root, depth = 0;
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			path[depth] = id, slots[depth++] = j;
			p -= node.sizes[j];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
			id = node.ids[j];
		}
		auto& leaf = leaves[id >> 1];
		int digit = access_leaf(leaf, p), rank = before[digit] + rank_leaf(leaf, p, digit);
		erase_leaf(leaf, p, digit);
		--n, --total[digit];
		refresh_uniform();
		if (!n) {
			release_leaf(root);
			root = 0;
			return {rank, digit};
		}
		for (int d = 0; d < depth; ++d) {
			auto& node = nodes[path[d] >> 1];
			add_range(node, slots[d], node.count, -1, digit, -1);
		}
		while (depth && (id & 1 ? leaves[id >> 1].length < Block / 2 : nodes[id >> 1].count < Branch / 2)) {
			int parent = path[--depth];
			if (!rebalance(parent, slots[depth])) break;
			id = parent;
		}
		if (!(root & 1) && nodes[root >> 1].count == 1) {
			int old = root;
			root = nodes[old >> 1].ids[0];
			release_node(old);
		}
		return {rank, digit};
	}
	std::tuple<int, int, int> set_rank(int p, int digit) {
		assert(0 <= p && p < n && 0 <= digit && digit < 4);
		if (total[digit] == n) return {p, digit, p};
		std::array<int, 32> path, slots;
		int id = root, depth = 0;
		Counts before{};
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int j = child(node, p);
			path[depth] = id, slots[depth++] = j;
			p -= node.sizes[j];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
			id = node.ids[j];
		}
		auto& leaf = leaves[id >> 1];
		int old = access_leaf(leaf, p);
		int old_rank = before[old] + rank_leaf(leaf, p, old);
		if (old == digit) return {old_rank, old, old_rank};
		int new_rank = before[digit] + rank_leaf(leaf, p, digit);
		Word bit = Word(1) << (p & 63);
		if ((old ^ digit) & 1) leaf.low[p >> 6] ^= bit;
		if ((old ^ digit) & 2) leaf.high[p >> 6] ^= bit;
		--leaf.counts[old], ++leaf.counts[digit], --total[old], ++total[digit];
		uniform = total[digit] == n ? digit : -1;
		for (int i = (p >> 6) + 1; i <= (leaf.length - 1) / 64; ++i) --leaf.prefix[i][old], ++leaf.prefix[i][digit];
		for (int d = 0; d < depth; ++d) {
			auto& node = nodes[path[d] >> 1];
			add_range(node, slots[d], node.count, 0, old, -1);
			add_range(node, slots[d], node.count, 0, digit, 1);
		}
		return {old_rank, old, new_rank};
	}
	std::tuple<int, int, int> move_rank(int from, int to, int digit) {
		assert(0 <= from && from < n && 0 <= to && to < n && 0 <= digit && digit < 4);
		if (uniform == digit) return {from, digit, to};
		if (from == to) return set_rank(from, digit);
		std::array<int, 32> left_path, right_path, left_slots, right_slots;
		int a = root, b = root, p = from, q = to + (from < to), left_depth = 0, right_depth = 0, next = 0;
		Counts before{};
		while (!(a & 1)) {
			const auto& node = nodes[a >> 1];
			int j = child(node, p);
			left_path[left_depth] = a, left_slots[left_depth++] = j;
			p -= node.sizes[j];
			for (int d = 0; d < 4; ++d) before[d] += node.sums[d][j];
			a = node.ids[j];
		}
		while (!(b & 1)) {
			const auto& node = nodes[b >> 1];
			int j = child(node, q);
			right_path[right_depth] = b, right_slots[right_depth++] = j;
			q -= node.sizes[j], next += node.sums[digit][j];
			b = node.ids[j];
		}
		auto& left = leaves[a >> 1];
		auto& right = leaves[b >> 1];
		if (a != b && (left.length <= Block / 2 || right.length == Block)) {
			auto [rank, old] = erase_rank(from);
			return {rank, old, insert(to, digit)};
		}
		int old = access_leaf(left, p), rank = before[old] + rank_leaf(left, p, old);
		next += rank_leaf(right, q, digit) - int(from < to && old == digit);
		erase_leaf(left, p, old);
		insert_leaf(right, q - int(a == b && from < to), digit);
		--total[old], ++total[digit];
		uniform = total[digit] == n ? digit : -1;
		int common = 0;
		while (common < left_depth && common < right_depth && left_path[common] == right_path[common]) {
			auto& node = nodes[left_path[common] >> 1];
			int l = left_slots[common], r = right_slots[common];
			if (old == digit) add_range(node, std::min(l, r), std::max(l, r), l < r ? -1 : 1, digit, l < r ? -1 : 1);
			else {
				add_range(node, std::min(l, r), std::max(l, r), l < r ? -1 : 1, digit, 0);
				add_range(node, l, node.count, 0, old, -1);
				add_range(node, r, node.count, 0, digit, 1);
			}
			++common;
		}
		for (int d = common; d < left_depth; ++d) {
			auto& node = nodes[left_path[d] >> 1];
			add_range(node, left_slots[d], node.count, -1, old, -1);
		}
		for (int d = common; d < right_depth; ++d) {
			auto& node = nodes[right_path[d] >> 1];
			add_range(node, right_slots[d], node.count, 1, digit, 1);
		}
		return {rank, old, next};
	}

	int select(int digit, int k) const {
		assert(0 <= digit && digit < 4);
		if (k < 0 || k >= total[digit]) return -1;
		if (total[digit] == n) return k;
		int id = root, before = 0;
		while (!(id & 1)) {
			const auto& node = nodes[id >> 1];
			int low = 0, high = node.count - 1;
			while (low < high) {
				int middle = (low + high) / 2;
				if (k < node.sums[digit][middle + 1]) high = middle;
				else low = middle + 1;
			}
			before += node.sizes[low], k -= node.sums[digit][low];
			id = node.ids[low];
		}
		const auto& leaf = leaves[id >> 1];
		for (int i = 0; i < (leaf.length + 63) / 64; ++i) {
			Word word = ((digit & 1) ? leaf.low[i] : ~leaf.low[i]) & ((digit & 2) ? leaf.high[i] : ~leaf.high[i]);
			word &= mask(std::min(64, leaf.length - i * 64));
			int count = popcount(word);
			if (k < count) return before + i * 64 + select_word(word, k);
			k -= count;
		}
		return -1;
	}
};
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC pop_options
#endif

#endif
