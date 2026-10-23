#ifndef RANGE_AFFINE_SUM_SIMD_HPP
#define RANGE_AFFINE_SUM_SIMD_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace range_affine_sum_detail {
inline bool simd_available() {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	return __builtin_cpu_supports("avx2");
#else
	return false;
#endif
}

template<unsigned Mod>
class SimdTree {
	static_assert(Mod >= 3 && Mod < (1U << 30) && (Mod & 1));
	static constexpr unsigned clean = 1U << 31;
	static constexpr unsigned inverse = [] {
		unsigned x = Mod;
		for (int i = 0; i < 5; ++i) x *= 2 - Mod * x;
		return 0U - x;
	}();
	static constexpr unsigned one = (std::uint64_t(1) << 32) % Mod;
	static constexpr unsigned square = std::uint64_t(one) * one % Mod;
	struct alignas(64) Leaf { std::array<unsigned, 16> sums{}; };
	struct alignas(64) Branch {
		std::array<unsigned, 16> sums{}, multiplier{}, translation{};
		Branch() { multiplier.fill(clean | one); }
	};
	std::vector<Leaf> leaves;
	std::vector<Branch> branches;
	std::array<std::size_t, 9> offsets{};
	unsigned level = 0, total_ = 0;
	static unsigned reduce(std::uint64_t x) {
		return (x + std::uint64_t(unsigned(x) * inverse) * Mod) >> 32;
	}
	static unsigned plus(unsigned a, unsigned b) {
		unsigned sum = a + b;
		return sum < 2 * Mod ? sum : sum - 2 * Mod;
	}
	static unsigned minus(unsigned a, unsigned b) {
		return a >= b ? a - b : a + 2 * Mod - b;
	}
	static unsigned mapped(unsigned a, unsigned b, unsigned sum, unsigned length) {
		return plus(reduce(std::uint64_t(a) * sum), reduce(std::uint64_t(b) * length));
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	using Vec = __m256i;
	struct Factor { Vec multiplier, inverse_product; };
	[[gnu::target("avx2"), gnu::always_inline]] static inline Factor factor(unsigned a) {
		return {_mm256_set1_epi32(int(a)), _mm256_set1_epi32(int(a * inverse))};
	}
	[[gnu::target("avx2"), gnu::always_inline]] static inline Vec multiply(Vec x, const Factor &a) {
		Vec odd = _mm256_srli_epi64(x, 32), modulus = _mm256_set1_epi32(int(Mod));
		Vec lo = _mm256_mul_epu32(x, a.multiplier), hi = _mm256_mul_epu32(odd, _mm256_srli_epi64(a.multiplier, 32));
		lo = _mm256_add_epi64(lo, _mm256_mul_epu32(_mm256_mul_epu32(x, a.inverse_product), modulus));
		hi = _mm256_add_epi64(hi, _mm256_mul_epu32(_mm256_mul_epu32(odd, _mm256_srli_epi64(a.inverse_product, 32)), modulus));
		return _mm256_blend_epi32(_mm256_srli_epi64(lo, 32), hi, 0xaa);
	}
	[[gnu::target("avx2"), gnu::always_inline]] static inline Vec add(Vec a, Vec b) {
		Vec sum = _mm256_add_epi32(a, b);
		return _mm256_min_epu32(sum, _mm256_sub_epi32(sum, _mm256_set1_epi32(int(2 * Mod))));
	}
	[[gnu::target("avx2"), gnu::always_inline]] static inline Vec selection(int first, int last) {
		Vec index = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		return _mm256_and_si256(_mm256_cmpgt_epi32(index, _mm256_set1_epi32(first - 1)),
			_mm256_cmpgt_epi32(_mm256_set1_epi32(last), index));
	}
	[[gnu::target("avx2"), gnu::always_inline]] static inline unsigned horizontal(Vec x) {
		Vec sum = _mm256_add_epi64(_mm256_cvtepu32_epi64(_mm256_castsi256_si128(x)),
			_mm256_cvtepu32_epi64(_mm256_extracti128_si256(x, 1)));
		__m128i half = _mm_add_epi64(_mm256_castsi256_si128(sum), _mm256_extracti128_si256(sum, 1));
		return (std::uint64_t(_mm_cvtsi128_si64(half)) + std::uint64_t(_mm_extract_epi64(half, 1))) % Mod;
	}
	[[gnu::target("avx2"), gnu::always_inline]] static inline unsigned fold(const unsigned *sums, int first, int last) {
		if (last <= 8) return horizontal(_mm256_and_si256(selection(first, last), _mm256_load_si256(reinterpret_cast<const Vec *>(sums))));
		if (first >= 8) return horizontal(_mm256_and_si256(selection(first - 8, last - 8), _mm256_load_si256(reinterpret_cast<const Vec *>(sums + 8))));
		Vec a = _mm256_and_si256(selection(first, last), _mm256_load_si256(reinterpret_cast<const Vec *>(sums)));
		Vec b = _mm256_and_si256(selection(first - 8, last - 8), _mm256_load_si256(reinterpret_cast<const Vec *>(sums + 8)));
		return horizontal(_mm256_add_epi32(a, b));
	}
	template<bool Tags, bool Carried = false>
	[[gnu::target("avx2"), gnu::always_inline]] static inline unsigned transform(unsigned *sums, unsigned *multipliers, unsigned *translations,
		int first, int last, unsigned a, unsigned b, unsigned length, unsigned incoming_a = one, unsigned incoming_b = 0) {
		if constexpr (Carried) {
			b = plus(reduce(std::uint64_t(a) * incoming_b), b);
			a = reduce(std::uint64_t(a) * incoming_a);
		}
		Factor scale = factor(a);
		Vec offset = _mm256_set1_epi32(int(reduce(std::uint64_t(b) * length)));
		Vec translation = _mm256_set1_epi32(int(b)), value_mask = _mm256_set1_epi32(int(clean - 1));
		Factor before = factor(incoming_a);
		Vec old_offset = _mm256_set1_epi32(int(reduce(std::uint64_t(incoming_b) * length)));
		Vec old_translation = _mm256_set1_epi32(int(incoming_b));
		Vec previous = _mm256_setzero_si256();
		for (int i = Carried ? 0 : first & ~7; i < (Carried ? 16 : last); i += 8) {
			Vec mask = selection(first - i, last - i);
			auto *p = reinterpret_cast<Vec *>(sums + i);
			Vec old = _mm256_load_si256(p);
			previous = _mm256_add_epi32(previous, _mm256_and_si256(mask, old));
			Factor selected = scale;
			Vec selected_offset = offset, selected_translation = translation;
			if constexpr (Carried) {
				selected = {_mm256_blendv_epi8(before.multiplier, scale.multiplier, mask),
					_mm256_blendv_epi8(before.inverse_product, scale.inverse_product, mask)};
				selected_offset = _mm256_blendv_epi8(old_offset, offset, mask);
				selected_translation = _mm256_blendv_epi8(old_translation, translation, mask);
			}
			Vec next = add(multiply(old, selected), selected_offset);
			_mm256_store_si256(p, Carried ? next : _mm256_blendv_epi8(old, next, mask));
			if constexpr (Tags) {
				auto *m = reinterpret_cast<Vec *>(multipliers + i), *t = reinterpret_cast<Vec *>(translations + i);
				Vec old_m = _mm256_load_si256(m), old_t = _mm256_load_si256(t);
				Vec next_m = multiply(_mm256_and_si256(old_m, value_mask), selected);
				Vec next_t = add(multiply(old_t, selected), selected_translation);
				_mm256_store_si256(m, Carried ? next_m : _mm256_blendv_epi8(old_m, next_m, mask));
				_mm256_store_si256(t, Carried ? next_t : _mm256_blendv_epi8(old_t, next_t, mask));
			}
		}
		unsigned old = horizontal(previous);
		if constexpr (Carried) return mapped(minus(a, incoming_a), minus(b, incoming_b), old, unsigned(last - first) * length);
		else return minus(mapped(a, b, old, unsigned(last - first) * length), old);
	}
#else
	static unsigned fold(const unsigned *sums, int first, int last) {
		unsigned sum = 0;
		for (int i = first; i < last; ++i) sum = plus(sum, sums[i]);
		return sum;
	}
	template<bool Tags, bool Carried = false>
	static unsigned transform(unsigned *sums, unsigned *multipliers, unsigned *translations,
		int first, int last, unsigned a, unsigned b, unsigned length, unsigned incoming_a = one, unsigned incoming_b = 0) {
		unsigned delta = 0;
		unsigned after_a = reduce(std::uint64_t(a) * incoming_a), after_b = plus(reduce(std::uint64_t(a) * incoming_b), b);
		for (int i = Carried ? 0 : first; i < (Carried ? 16 : last); ++i) {
			unsigned scale = a, shift = b;
			if constexpr (Carried) scale = first <= i && i < last ? after_a : incoming_a, shift = first <= i && i < last ? after_b : incoming_b;
			unsigned next = mapped(scale, shift, sums[i], length);
			if constexpr (Carried) { if (first <= i && i < last) delta = plus(delta, minus(next, mapped(incoming_a, incoming_b, sums[i], length))); }
			else delta = plus(delta, minus(next, sums[i]));
			sums[i] = next;
			if constexpr (Tags) {
				multipliers[i] = reduce(std::uint64_t(scale) * (multipliers[i] & (clean - 1)));
				translations[i] = plus(reduce(std::uint64_t(scale) * translations[i]), shift);
			}
		}
		return delta;
	}
#endif
	template<bool Carried = false>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	unsigned apply_slots(unsigned depth, std::size_t block, int first, int last, unsigned a, unsigned b,
		unsigned incoming_a = one, unsigned incoming_b = 0) {
		if (!depth) return transform<false, Carried>(leaves[block].sums.data(), nullptr, nullptr, first, last, a, b, 1, incoming_a, incoming_b);
		auto &node = branches[offsets[depth] + block];
		return transform<true, Carried>(node.sums.data(), node.multiplier.data(), node.translation.data(),
			first, last, a, b, unsigned(1) << (4 * depth), incoming_a, incoming_b);
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	unsigned update(unsigned depth, std::size_t block, unsigned l, unsigned r, unsigned a, unsigned b,
		unsigned incoming_a = clean | one, unsigned incoming_b = 0) {
		if (!depth) return incoming_a & clean ? apply_slots<>(0, block, int(l), int(r), a, b)
			: apply_slots<true>(0, block, int(l), int(r), a, b, incoming_a, incoming_b);
		unsigned shift = 4 * depth, length = unsigned(1) << shift;
		int first = int(l >> shift), last = int(r >> shift);
		auto &node = branches[offsets[depth] + block];
		unsigned delta = 0;
		int full_first = first + bool(l & (length - 1));
		full_first = std::min(full_first, last);
		if (!(incoming_a & clean)) delta = apply_slots<true>(depth, block, full_first, last, a, b, incoming_a, incoming_b);
		else if (full_first < last) delta = apply_slots<>(depth, block, full_first, last, a, b);
		auto partial = [&](int slot, unsigned left, unsigned right) {
			unsigned change = update(depth - 1, block * 16 + unsigned(slot), left, right, a, b, node.multiplier[slot], node.translation[slot]);
			node.multiplier[slot] = clean | one;
			node.translation[slot] = 0;
			node.sums[slot] = plus(node.sums[slot], change);
			return change;
		};
		if (l & (length - 1)) {
			unsigned change = partial(first, l & (length - 1),
				first == int((r - 1) >> shift) ? ((r - 1) & (length - 1)) + 1 : length);
			delta = plus(delta, change);
			if (first == int((r - 1) >> shift)) return delta;
		}
		if (r & (length - 1)) {
			delta = plus(delta, partial(last, 0, r & (length - 1)));
		}
		return delta;
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	unsigned query(unsigned depth, std::size_t block, unsigned l, unsigned r) const {
		if (!depth) return fold(leaves[block].sums.data(), int(l), int(r));
		unsigned shift = 4 * depth, length = unsigned(1) << shift;
		int first = int(l >> shift), last = int(r >> shift);
		const auto &node = branches[offsets[depth] + block];
		unsigned sum = 0;
		auto partial = [&](int slot, unsigned left, unsigned right) {
			unsigned value = query(depth - 1, block * 16 + unsigned(slot), left, right);
			return node.multiplier[slot] & clean ? value
				: mapped(node.multiplier[slot], node.translation[slot], value, right - left);
		};
		if (l & (length - 1)) {
			if (first == int((r - 1) >> shift)) return partial(first, l & (length - 1), ((r - 1) & (length - 1)) + 1);
			sum = partial(first++, l & (length - 1), length);
		}
		if (r & (length - 1)) sum = plus(sum, partial(last, 0, r & (length - 1)));
		if (first < last) sum = plus(sum, fold(node.sums.data(), first, last));
		return sum;
	}
public:
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	void build(const std::vector<unsigned> &values) {
		leaves.clear(); branches.clear(); offsets.fill(0); level = total_ = 0;
		if (values.empty()) return;
		unsigned base = std::bit_ceil(unsigned(values.size()));
		level = (std::bit_width(base) - 1) / 4;
		if (base == (unsigned(1) << (4 * level)) && level) --level;
		unsigned active = base >> (4 * level);
		std::size_t count = (std::size_t(base) + 15) / 16;
		leaves.resize(count);
		for (std::size_t i = 0; i < values.size(); ++i) leaves[i >> 4].sums[i & 15] = values[i];
		std::size_t total = 0;
		for (unsigned d = 1; d <= level; ++d) {
			count = (count + 15) / 16;
			offsets[d] = total;
			total += count;
		}
		branches.resize(total);
		for (unsigned d = 1; d <= level; ++d) {
			std::size_t children = d == 1 ? leaves.size() : offsets[d] - offsets[d - 1];
			for (std::size_t i = 0; i < children; ++i) {
				unsigned sum = d == 1 ? fold(leaves[i].sums.data(), 0, 16)
					: fold(branches[offsets[d - 1] + i].sums.data(), 0, 16);
				branches[offsets[d] + (i >> 4)].sums[i & 15] = sum;
			}
		}
		total_ = level ? fold(branches[offsets[level]].sums.data(), 0, int(active)) : fold(leaves[0].sums.data(), 0, int(active));
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	void apply(unsigned l, unsigned r, unsigned a, unsigned b) {
		if (l == r || (a == 1 && !b)) return;
		a = reduce(std::uint64_t(a) * square);
		b = reduce(std::uint64_t(b) * square);
		total_ = plus(total_, update(level, 0, l, r, a, b));
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	unsigned prod(unsigned l, unsigned r) const {
		return l == r ? 0 : query(level, 0, l, r);
	}
	unsigned all_prod() const { return total_; }
	void swap(SimdTree &other) noexcept {
		leaves.swap(other.leaves); branches.swap(other.branches); offsets.swap(other.offsets);
		std::swap(level, other.level); std::swap(total_, other.total_);
	}
};
}

#endif
