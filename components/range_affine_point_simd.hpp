#ifndef RANGE_AFFINE_POINT_SIMD_HPP
#define RANGE_AFFINE_POINT_SIMD_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
#endif

namespace range_affine_point_detail {
inline bool simd_available() {
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__)) && !defined(RANGE_AFFINE_POINT_GET_DISABLE_AVX2)
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
	struct alignas(64) Leaf { std::array<unsigned, 32> values{}; };
	struct alignas(64) Branch {
		std::array<unsigned, 32> multiplier{}, translation{};
		Branch() { multiplier.fill(clean | one); }
	};
	std::vector<Leaf> leaves;
	std::vector<Branch> branches;
	std::array<std::size_t, 9> offsets{};
	unsigned level = 0;
	static unsigned reduce(std::uint64_t x) {
		return (x + std::uint64_t(unsigned(x) * inverse) * Mod) >> 32;
	}
	static unsigned plus(unsigned a, unsigned b) {
		unsigned sum = a + b;
		return sum < 2 * Mod ? sum : sum - 2 * Mod;
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
	template<bool Tags, bool Carried = false>
	[[gnu::target("avx2"), gnu::always_inline]] static inline void transform(unsigned *multipliers, unsigned *translations,
		int first, int last, unsigned a, unsigned b, unsigned incoming_a = one, unsigned incoming_b = 0) {
		if constexpr (Carried) {
			b = plus(reduce(std::uint64_t(a) * incoming_b), b);
			a = reduce(std::uint64_t(a) * incoming_a);
		}
		Factor scale = factor(a), before = factor(incoming_a);
		Vec translation = _mm256_set1_epi32(int(b)), old_translation = _mm256_set1_epi32(int(incoming_b));
		Vec value_mask = _mm256_set1_epi32(int(clean - 1));
		for (int i = Carried ? 0 : first & ~7; i < (Carried ? 32 : last); i += 8) {
			Vec mask = selection(first - i, last - i);
			Factor selected = scale;
			Vec shift = translation;
			if constexpr (Carried) {
				selected = {_mm256_blendv_epi8(before.multiplier, scale.multiplier, mask),
					_mm256_blendv_epi8(before.inverse_product, scale.inverse_product, mask)};
				shift = _mm256_blendv_epi8(old_translation, translation, mask);
			}
			auto *m = reinterpret_cast<Vec *>(multipliers + i);
			Vec old_m = _mm256_load_si256(m);
			Vec next_m;
			if constexpr (Tags) {
				next_m = multiply(_mm256_and_si256(old_m, value_mask), selected);
				auto *t = reinterpret_cast<Vec *>(translations + i);
				Vec old_t = _mm256_load_si256(t), next_t = add(multiply(old_t, selected), shift);
				_mm256_store_si256(t, Carried ? next_t : _mm256_blendv_epi8(old_t, next_t, mask));
			} else next_m = add(multiply(old_m, selected), shift);
			_mm256_store_si256(m, Carried ? next_m : _mm256_blendv_epi8(old_m, next_m, mask));
		}
	}
#else
	template<bool Tags, bool Carried = false>
	static void transform(unsigned *multipliers, unsigned *translations,
		int first, int last, unsigned a, unsigned b, unsigned incoming_a = one, unsigned incoming_b = 0) {
		unsigned after_a = reduce(std::uint64_t(a) * incoming_a), after_b = plus(reduce(std::uint64_t(a) * incoming_b), b);
		for (int i = Carried ? 0 : first; i < (Carried ? 32 : last); ++i) {
			unsigned scale = a, shift = b;
			if constexpr (Carried) scale = first <= i && i < last ? after_a : incoming_a, shift = first <= i && i < last ? after_b : incoming_b;
			if constexpr (Tags) {
				multipliers[i] = reduce(std::uint64_t(scale) * (multipliers[i] & (clean - 1)));
				translations[i] = plus(reduce(std::uint64_t(scale) * translations[i]), shift);
			} else multipliers[i] = plus(reduce(std::uint64_t(scale) * multipliers[i]), shift);
		}
	}
#endif
	template<bool Carried = false>
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	void apply_slots(unsigned depth, std::size_t block, int first, int last, unsigned a, unsigned b,
		unsigned incoming_a = one, unsigned incoming_b = 0) {
		if (!depth) return transform<false, Carried>(leaves[block].values.data(), nullptr, first, last, a, b, incoming_a, incoming_b);
		auto &node = branches[offsets[depth] + block];
		transform<true, Carried>(node.multiplier.data(), node.translation.data(), first, last, a, b, incoming_a, incoming_b);
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	void update(unsigned depth, std::size_t block, unsigned l, unsigned r, unsigned a, unsigned b,
		unsigned incoming_a = clean | one, unsigned incoming_b = 0) {
		if (!depth) return incoming_a & clean ? apply_slots<>(0, block, int(l), int(r), a, b)
			: apply_slots<true>(0, block, int(l), int(r), a, b, incoming_a, incoming_b);
		unsigned shift = 5 * depth, length = unsigned(1) << shift;
		int first = int(l >> shift), last = int(r >> shift);
		auto &node = branches[offsets[depth] + block];
		int full_first = std::min(first + bool(l & (length - 1)), last);
		if (!(incoming_a & clean)) apply_slots<true>(depth, block, full_first, last, a, b, incoming_a, incoming_b);
		else if (full_first < last) apply_slots<>(depth, block, full_first, last, a, b);
		auto partial = [&](int slot, unsigned left, unsigned right) {
			update(depth - 1, block * 32 + unsigned(slot), left, right, a, b, node.multiplier[slot], node.translation[slot]);
			node.multiplier[slot] = clean | one;
			node.translation[slot] = 0;
		};
		if (l & (length - 1)) {
			partial(first, l & (length - 1), first == int((r - 1) >> shift) ? ((r - 1) & (length - 1)) + 1 : length);
			if (first == int((r - 1) >> shift)) return;
		}
		if (r & (length - 1)) partial(last, 0, r & (length - 1));
	}
public:
	static unsigned encode(unsigned value) { return reduce(std::uint64_t(value) * square); }
	static unsigned decode(unsigned value) { unsigned x = reduce(value); return x >= Mod ? x - Mod : x; }
	void build(const std::vector<unsigned> &values) {
		leaves.clear(); branches.clear(); offsets.fill(0); level = 0;
		if (values.empty()) return;
		unsigned base = std::bit_ceil(unsigned(values.size()));
		level = (std::bit_width(base) - 1) / 5;
		if (base == (unsigned(1) << (5 * level)) && level) --level;
		std::size_t count = (std::size_t(base) + 31) / 32;
		leaves.resize(count);
		for (std::size_t i = 0; i < values.size(); ++i) leaves[i >> 5].values[i & 31] = values[i];
		std::size_t total = 0;
		for (unsigned d = 1; d <= level; ++d) {
			count = (count + 31) / 32;
			offsets[d] = total;
			total += count;
		}
		branches.resize(total);
	}
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
	[[gnu::target("avx2")]]
#endif
	void apply(unsigned l, unsigned r, unsigned a, unsigned b) {
		if (l == r || ((a == one || a == one + Mod) && (b == 0 || b == Mod))) return;
		update(level, 0, l, r, a, b);
	}
	unsigned get(unsigned p) const {
		unsigned value = leaves[p >> 5].values[p & 31];
		for (unsigned d = 1; d <= level; ++d) {
			const auto &node = branches[offsets[d] + ((p >> (5 * d)) >> 5)];
			unsigned slot = (p >> (5 * d)) & 31, a = node.multiplier[slot];
			if (!(a & clean)) value = plus(reduce(std::uint64_t(a) * value), node.translation[slot]);
		}
		return value;
	}
	void swap(SimdTree &other) noexcept {
		leaves.swap(other.leaves); branches.swap(other.branches); offsets.swap(other.offsets); std::swap(level, other.level);
	}
};
}

#endif
