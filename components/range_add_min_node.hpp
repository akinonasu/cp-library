#ifndef RANGE_ADD_MIN_NODE_HPP
#define RANGE_ADD_MIN_NODE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#if (defined(__AVX2__) || defined(__AVX512F__)) && defined(__x86_64__)
#include <immintrin.h>
#endif

namespace range_add_min_detail {
template<class T> struct Node {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	static constexpr unsigned branch = 8;
	static constexpr U umax = std::numeric_limits<U>::max();
	static constexpr U sign_bit = U(1) << (sizeof(U) * 8 - 1);
#if defined(__AVX2__) && defined(__x86_64__)
	alignas(64) inline static constexpr auto masks_ = [] {
		std::array<std::array<U, branch>, branch + 1> masks{};
		for (unsigned i = 0; i <= branch; ++i)
			for (unsigned j = 0; j < branch; ++j) masks[i][j] = j < i ? umax : 0;
		return masks;
	}();
	alignas(64) inline static constexpr auto slots_ = [] {
		std::array<std::array<U, branch>, branch> slots{};
		for (unsigned i = 0; i < branch; ++i) slots[i][i] = umax;
		return slots;
	}();
	static __m256i min_signed(__m256i x, __m256i y) { return _mm256_blendv_epi8(x, y, _mm256_cmpgt_epi64(x, y)); }
	static U reduce_min(__m256i x) {
		x = min_signed(x, _mm256_permute2x128_si256(x, x, 1));
		x = min_signed(x, _mm256_permute4x64_epi64(x, 0xb1));
		return U(_mm_cvtsi128_si64(_mm256_castsi256_si128(x)));
	}
#endif
	template<bool Narrow>
	static U minimum(const U* values, unsigned l, unsigned r) {
#if defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			unsigned mask = ((1u << r) - 1) ^ ((1u << l) - 1);
			auto value = _mm512_loadu_si512(values);
			if constexpr (Narrow) return U(_mm512_mask_reduce_min_epi64(mask, value));
			else return U(_mm512_mask_reduce_min_epu64(mask, value));
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto identity = _mm256_set1_epi64x(std::numeric_limits<T>::max());
			auto result = identity;
			for (unsigned i = 0; i < branch; i += 4) {
				auto first = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[l].data() + i));
				auto last = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[r].data() + i));
				auto mask = _mm256_xor_si256(first, last);
				auto value = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
				if constexpr (!Narrow) value = _mm256_xor_si256(value, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
				value = _mm256_xor_si256(identity, _mm256_and_si256(_mm256_xor_si256(value, identity), mask));
				result = min_signed(result, value);
			}
			if constexpr (Narrow) return reduce_min(result);
			else return reduce_min(result) ^ sign_bit;
		}
#endif
		U result = umax;
		for (unsigned i = l; i < r; ++i) result = std::min(result, values[i]);
		return result;
	}
	template<bool Narrow>
	static U normalize(U *values, unsigned count, bool negative, unsigned first, unsigned last, U delta, unsigned x = 0, U dx = 0, unsigned y = 0, U dy = 0) {
#if defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto old = _mm512_loadu_si512(values);
			unsigned mask = ((1u << last) - 1) ^ ((1u << first) - 1), valid = (1u << count) - 1;
			auto next = _mm512_mask_add_epi64(old, mask, old, _mm512_set1_epi64(std::bit_cast<T>(delta)));
			next = _mm512_mask_add_epi64(next, 1u << x, next, _mm512_set1_epi64(std::bit_cast<T>(dx)));
			next = _mm512_mask_add_epi64(next, 1u << y, next, _mm512_set1_epi64(std::bit_cast<T>(dy)));
			U change;
			if constexpr (Narrow) {
				change = U(_mm512_reduce_min_epi64(next));
				next = _mm512_sub_epi64(next, _mm512_set1_epi64(std::bit_cast<T>(change)));
			} else {
				unsigned wrap = negative ? unsigned(_mm512_cmp_epu64_mask(next, old, _MM_CMPINT_GT)) & valid : 0;
				change = U(_mm512_mask_reduce_min_epu64(wrap ? wrap : valid, next));
				next = _mm512_mask_sub_epi64(next, valid, next, _mm512_set1_epi64(std::bit_cast<T>(change)));
			}
			_mm512_storeu_si512(values, next);
			return change;
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto result = _mm256_set1_epi64x(std::numeric_limits<T>::max()), wrapped = result;
			auto underflow = _mm256_setzero_si256();
			__m256i next[branch / 4];
			for (unsigned i = 0; i < branch; i += 4) {
				auto a = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[first].data() + i));
				auto b = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[last].data() + i));
				auto old = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
				auto value = _mm256_add_epi64(old, _mm256_and_si256(_mm256_xor_si256(a, b), _mm256_set1_epi64x(std::bit_cast<T>(delta))));

				value = _mm256_add_epi64(value, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[x].data() + i)), _mm256_set1_epi64x(std::bit_cast<T>(dx))));
				value = _mm256_add_epi64(value, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[y].data() + i)), _mm256_set1_epi64x(std::bit_cast<T>(dy))));
				next[i / 4] = value;
				if constexpr (Narrow) {
					result = min_signed(result, value);
				} else {
					auto biased = _mm256_xor_si256(value, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
					result = min_signed(result, biased);
					if (negative) {
						auto wrap = _mm256_cmpgt_epi64(biased, _mm256_xor_si256(old, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit))));
						underflow = _mm256_or_si256(underflow, wrap);
						wrapped = min_signed(wrapped, _mm256_blendv_epi8(_mm256_set1_epi64x(std::numeric_limits<T>::max()), biased, wrap));
					}
				}
			}
			U change;
			if constexpr (Narrow) change = reduce_min(result);
			else change = reduce_min(negative && _mm256_movemask_epi8(underflow) ? wrapped : result) ^ sign_bit;
			for (unsigned i = 0; i < branch; i += 4) {
				auto shift = _mm256_set1_epi64x(std::bit_cast<T>(change));
				if constexpr (!Narrow) {
					auto valid = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data() + i));
					shift = _mm256_and_si256(valid, shift);
				}
				auto value = _mm256_sub_epi64(next[i / 4], shift);
				_mm256_store_si256(reinterpret_cast<__m256i*>(values + i), value);
			}
			return change;
		}
#endif
		U wrapped = umax, result = umax;
		bool underflow = false;
		for (unsigned i = 0; i < count; ++i) {
			U old = values[i], change = first <= i && i < last ? delta : 0;
			if (i == x) change += dx;
			if (i == y) change += dy;
			U next = U(old + change);
			values[i] = next;
			if (negative && next > old) { wrapped = std::min(wrapped, next); underflow = true; }
			result = std::min(result, next);
		}
		if (underflow) result = wrapped;
		for (unsigned i = 0; i < count; ++i) values[i] -= result;
		return result;
	}
	template<bool Narrow>
	static U full_minimum(const U* values) {
#if defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto value = _mm512_loadu_si512(values);
			if constexpr (Narrow) return U(_mm512_reduce_min_epi64(value));
			else return U(_mm512_reduce_min_epu64(value));
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto result = _mm256_load_si256(reinterpret_cast<const __m256i*>(values));
			if constexpr (!Narrow) result = _mm256_xor_si256(result, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
			for (unsigned i = 4; i < branch; i += 4) {
				auto value = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
				if constexpr (!Narrow) value = _mm256_xor_si256(value, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
				result = min_signed(result, value);
			}
			if constexpr (Narrow) return reduce_min(result);
			else return reduce_min(result) ^ sign_bit;
		}
#endif
		U result = umax;
		for (unsigned i = 0; i < branch; ++i) result = std::min(result, values[i]);
		return result;
	}
	static void subtract(U* values, unsigned count, U change) {
#if defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto value = _mm512_loadu_si512(values);
			value = _mm512_mask_sub_epi64(value, (1u << count) - 1, value, _mm512_set1_epi64(std::bit_cast<T>(change)));
			_mm512_storeu_si512(values, value);
			return;
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto delta = _mm256_set1_epi64x(std::bit_cast<T>(change));
			for (unsigned i = 0; i < branch; i += 4) {
				auto valid = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data() + i));
				auto old = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
				_mm256_store_si256(reinterpret_cast<__m256i*>(values + i), _mm256_sub_epi64(old, _mm256_and_si256(valid, delta)));
			}
			return;
		}
#endif
		for (unsigned i = 0; i < count; ++i) values[i] -= change;
	}
	template<bool Narrow>
	static U change_one(U *values, unsigned count, unsigned index, U delta, bool negative) {
		if (!delta) return 0;
		U old = values[index], next = U(old + delta);
		values[index] = next;
		if ((Narrow && sizeof(T) == 8) ? std::bit_cast<T>(next) < 0 : negative && next > old) {
			subtract(values, count, next);
			return next;
		}
		if (((Narrow && sizeof(T) == 8) || !negative) && !old) {
			U result = full_minimum<Narrow>(values);
			if (!result) return 0;
			subtract(values, count, result);
			return result;
		}
		return 0;
	}
};
}

#endif
