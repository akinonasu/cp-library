#ifndef RANGE_ADD_MIN_NODE_HPP
#define RANGE_ADD_MIN_NODE_HPP

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
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
	static __m256i broadcast_min(__m256i value) {
		value = min_signed(value, _mm256_permute2x128_si256(value, value, 1));
		return min_signed(value, _mm256_permute4x64_epi64(value, 0xb1));
	}
	static U reduce_min(__m256i value) { return U(_mm_cvtsi128_si64(_mm256_castsi256_si128(broadcast_min(value)))); }
	static __m256i broadcast_min32(__m256i value) {
		value = _mm256_min_epu32(value, _mm256_permute2x128_si256(value, value, 1));
		value = _mm256_min_epu32(value, _mm256_shuffle_epi32(value, 0x4e));
		return _mm256_min_epu32(value, _mm256_shuffle_epi32(value, 0xb1));
	}
	static U reduce_min32(__m256i value) { return U(std::uint32_t(_mm_cvtsi128_si32(_mm256_castsi256_si128(broadcast_min32(value))))); }
	template<bool Narrow>
	static __m256i minimum_vector(const U* values, unsigned l, unsigned r, U neutral) {
		auto identity = _mm256_set1_epi64x(std::bit_cast<T>(neutral));
		__m256i result;
		for (unsigned i = 0; i < branch; i += 4) {
			auto first = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[l].data() + i));
			auto last = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[r].data() + i));
			auto value = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
			if constexpr (!Narrow) value = _mm256_xor_si256(value, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
			value = _mm256_blendv_epi8(identity, value, _mm256_xor_si256(first, last));
			result = i ? min_signed(result, value) : value;
		}
		return result;
	}
#endif
	static T build(U* values, unsigned count) {
		if (!count) return std::numeric_limits<T>::max();
#if defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto value = _mm512_loadu_si512(values);
			unsigned valid = (1u << count) - 1;
			T minimum = T(_mm512_mask_reduce_min_epi64(valid, value));
			value = _mm512_mask_sub_epi64(value, valid, value, _mm512_set1_epi64(minimum));
			_mm512_storeu_si512(values, value);
			return minimum;
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8) {
			auto first = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data()));
			auto last = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data() + 4));
			auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			auto y = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values + 4));
			auto identity = _mm256_set1_epi64x(std::numeric_limits<T>::max());
			auto a = _mm256_blendv_epi8(identity, x, first), b = _mm256_blendv_epi8(identity, y, last);
			auto minimum = broadcast_min(min_signed(a, b));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(values), _mm256_sub_epi64(x, _mm256_and_si256(first, minimum)));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(values + 4), _mm256_sub_epi64(y, _mm256_and_si256(last, minimum)));
			return std::bit_cast<T>(U(_mm_cvtsi128_si64(_mm256_castsi256_si128(minimum))));
		}
		if constexpr (sizeof(T) == 4) {
			auto valid = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data()));
			auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			auto sign = _mm256_set1_epi32(std::bit_cast<T>(sign_bit));
			auto key = _mm256_xor_si256(value, sign);
			key = _mm256_or_si256(key, _mm256_andnot_si256(valid, _mm256_set1_epi32(-1)));
			auto minimum = _mm256_xor_si256(broadcast_min32(key), sign);
			value = _mm256_sub_epi32(value, _mm256_and_si256(valid, minimum));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(values), value);
			return std::bit_cast<T>(U(std::uint32_t(_mm_cvtsi128_si32(_mm256_castsi256_si128(minimum)))));
		}
#endif
		T minimum = std::bit_cast<T>(values[0]);
		for (unsigned i = 1; i < count; ++i) minimum = std::min(minimum, std::bit_cast<T>(values[i]));
		for (unsigned i = 0; i < count; ++i) values[i] = U(values[i] - U(minimum));
		return minimum;
	}
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
			U result = reduce_min(minimum_vector<Narrow>(values, l, r, U(std::numeric_limits<T>::max())));
			if constexpr (Narrow) return result;
			else return result ^ sign_bit;
		}
		if constexpr (sizeof(T) == 4) {
			auto first = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[l].data()));
			auto last = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[r].data()));
			auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			auto excluded = _mm256_xor_si256(_mm256_xor_si256(first, last), _mm256_set1_epi32(-1));
			return reduce_min32(_mm256_or_si256(value, excluded));
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
			auto identity = _mm256_set1_epi64x(std::numeric_limits<T>::max());
			__m256i next[2], result, wrapped = identity;
			auto underflow = _mm256_setzero_si256();
			for (unsigned i = 0; i < branch; i += 4) {
				auto a = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[first].data() + i));
				auto b = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[last].data() + i));
				auto old = _mm256_load_si256(reinterpret_cast<const __m256i*>(values + i));
				auto value = _mm256_add_epi64(old, _mm256_and_si256(_mm256_xor_si256(a, b), _mm256_set1_epi64x(std::bit_cast<T>(delta))));
				value = _mm256_add_epi64(value, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[x].data() + i)), _mm256_set1_epi64x(std::bit_cast<T>(dx))));
				value = _mm256_add_epi64(value, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[y].data() + i)), _mm256_set1_epi64x(std::bit_cast<T>(dy))));
				next[i / 4] = value;
				if constexpr (!Narrow) {
					auto biased = _mm256_xor_si256(value, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit)));
					result = i ? min_signed(result, biased) : biased;
					if (negative) {
						auto wrap = _mm256_cmpgt_epi64(biased, _mm256_xor_si256(old, _mm256_set1_epi64x(std::bit_cast<T>(sign_bit))));
						underflow = _mm256_or_si256(underflow, wrap);
						wrapped = min_signed(wrapped, _mm256_blendv_epi8(identity, biased, wrap));
					}
				}
			}
			__m256i shift;
			U change;
			if constexpr (Narrow) {
				shift = broadcast_min(min_signed(next[0], next[1]));
				change = U(_mm_cvtsi128_si64(_mm256_castsi256_si128(shift)));
			} else {
				change = reduce_min(negative && _mm256_movemask_epi8(underflow) ? wrapped : result) ^ sign_bit;
				shift = _mm256_set1_epi64x(std::bit_cast<T>(change));
			}
			for (unsigned i = 0; i < branch; i += 4) {
				auto delta = shift;
				if constexpr (!Narrow) delta = _mm256_and_si256(delta, _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data() + i)));
				_mm256_store_si256(reinterpret_cast<__m256i*>(values + i), _mm256_sub_epi64(next[i / 4], delta));
			}
			return change;
		}
		if constexpr (sizeof(T) == 4) {
			auto first_mask = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[first].data()));
			auto last_mask = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[last].data()));
			auto old = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			auto next = _mm256_add_epi32(old, _mm256_and_si256(_mm256_xor_si256(first_mask, last_mask), _mm256_set1_epi32(std::bit_cast<T>(delta))));
			next = _mm256_add_epi32(next, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[x].data())), _mm256_set1_epi32(std::bit_cast<T>(dx))));
			next = _mm256_add_epi32(next, _mm256_and_si256(_mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[y].data())), _mm256_set1_epi32(std::bit_cast<T>(dy))));
			auto candidates = next;
			if (negative) {
				auto sign = _mm256_set1_epi32(std::bit_cast<T>(sign_bit));
				auto wrapped = _mm256_cmpgt_epi32(_mm256_xor_si256(next, sign), _mm256_xor_si256(old, sign));
				if (_mm256_movemask_epi8(wrapped)) candidates = _mm256_or_si256(next, _mm256_andnot_si256(wrapped, _mm256_set1_epi32(-1)));
			}
			U change = reduce_min32(candidates);
			auto valid = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data()));
			next = _mm256_sub_epi32(next, _mm256_and_si256(valid, _mm256_set1_epi32(std::bit_cast<T>(change))));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(values), next);
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
	static std::pair<U, U> normalize_boundaries(U* left, U* right, unsigned right_count, bool negative, unsigned first, unsigned last, U delta, U left_change, U right_change) {
#if defined(__AVX2__) && !defined(__AVX512F__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 8 && Narrow) {
			__m256i x[2], y[2];
			auto common = _mm256_set1_epi64x(std::bit_cast<T>(delta));
			auto dx = _mm256_set1_epi64x(std::bit_cast<T>(left_change));
			auto dy = _mm256_set1_epi64x(std::bit_cast<T>(right_change));
			for (unsigned i = 0; i < branch; i += 4) {
				auto prefix = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[first].data() + i));
				auto suffix = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[last + 1].data() + i));
				auto lc = _mm256_andnot_si256(prefix, common);
				auto rc = _mm256_and_si256(suffix, common);
				lc = _mm256_blendv_epi8(lc, dx, _mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[first].data() + i)));
				rc = _mm256_blendv_epi8(rc, dy, _mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[last].data() + i)));
				x[i / 4] = _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(left + i)), lc);
				y[i / 4] = _mm256_add_epi64(_mm256_load_si256(reinterpret_cast<const __m256i*>(right + i)), rc);
			}
			auto a = min_signed(x[0], x[1]), b = min_signed(y[0], y[1]);
			auto minimum = min_signed(_mm256_unpacklo_epi64(a, b), _mm256_unpackhi_epi64(a, b));
			minimum = min_signed(minimum, _mm256_permute2x128_si256(minimum, minimum, 1));
			auto lm = _mm256_permute4x64_epi64(minimum, 0x00), rm = _mm256_permute4x64_epi64(minimum, 0x55);
			_mm256_store_si256(reinterpret_cast<__m256i*>(left), _mm256_sub_epi64(x[0], lm));
			_mm256_store_si256(reinterpret_cast<__m256i*>(left + 4), _mm256_sub_epi64(x[1], lm));
			_mm256_store_si256(reinterpret_cast<__m256i*>(right), _mm256_sub_epi64(y[0], rm));
			_mm256_store_si256(reinterpret_cast<__m256i*>(right + 4), _mm256_sub_epi64(y[1], rm));
			return {U(_mm_cvtsi128_si64(_mm256_castsi256_si128(minimum))), U(_mm_extract_epi64(_mm256_castsi256_si128(minimum), 1))};
		}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
		if constexpr (sizeof(T) == 4) {
			auto common = _mm256_set1_epi32(std::bit_cast<T>(delta));
			auto prefix = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[first].data()));
			auto suffix = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[last + 1].data()));
			auto lc = _mm256_andnot_si256(prefix, common);
			auto rc = _mm256_and_si256(suffix, common);
			lc = _mm256_blendv_epi8(lc, _mm256_set1_epi32(std::bit_cast<T>(left_change)), _mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[first].data())));
			rc = _mm256_blendv_epi8(rc, _mm256_set1_epi32(std::bit_cast<T>(right_change)), _mm256_load_si256(reinterpret_cast<const __m256i*>(slots_[last].data())));
			auto old_left = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(left));
			auto old_right = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(right));
			auto x = _mm256_add_epi32(old_left, lc), y = _mm256_add_epi32(old_right, rc);
			auto a = x, b = y;
			if (negative) {
				auto sign = _mm256_set1_epi32(std::bit_cast<T>(sign_bit));
				auto lx = _mm256_cmpgt_epi32(_mm256_xor_si256(x, sign), _mm256_xor_si256(old_left, sign));
				auto ry = _mm256_cmpgt_epi32(_mm256_xor_si256(y, sign), _mm256_xor_si256(old_right, sign));
				if (_mm256_movemask_epi8(lx)) a = _mm256_or_si256(x, _mm256_andnot_si256(lx, _mm256_set1_epi32(-1)));
				if (_mm256_movemask_epi8(ry)) b = _mm256_or_si256(y, _mm256_andnot_si256(ry, _mm256_set1_epi32(-1)));
			}
			auto minimum = _mm256_min_epu32(_mm256_unpacklo_epi32(a, b), _mm256_unpackhi_epi32(a, b));
			minimum = _mm256_min_epu32(minimum, _mm256_shuffle_epi32(minimum, 0x4e));
			minimum = _mm256_min_epu32(minimum, _mm256_permute2x128_si256(minimum, minimum, 1));
			auto lm = _mm256_shuffle_epi32(minimum, 0x00), rm = _mm256_shuffle_epi32(minimum, 0x55);
			x = _mm256_sub_epi32(x, lm);
			y = _mm256_sub_epi32(y, _mm256_and_si256(rm, _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[right_count].data()))));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(left), x);
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(right), y);
			auto low = _mm256_castsi256_si128(minimum);
			return {U(std::uint32_t(_mm_cvtsi128_si32(low))), U(std::uint32_t(_mm_extract_epi32(low, 1)))};
		}
#endif
		U a = first + 1 == branch ? change_one<Narrow>(left, branch, first, left_change, negative)
			: normalize<Narrow>(left, branch, negative, first + 1, branch, delta, first, left_change);
		U b = !last ? change_one<Narrow>(right, right_count, 0, right_change, negative)
			: normalize<Narrow>(right, right_count, negative, 0, last, delta, last, right_change);
		return {a, b};
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
		if constexpr (sizeof(T) == 4) return reduce_min32(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(values)));
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
		if constexpr (sizeof(T) == 4) {
			auto valid = _mm256_load_si256(reinterpret_cast<const __m256i*>(masks_[count].data()));
			auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
			value = _mm256_sub_epi32(value, _mm256_and_si256(valid, _mm256_set1_epi32(std::bit_cast<T>(change))));
			_mm256_storeu_si256(reinterpret_cast<__m256i*>(values), value);
			return;
		}
#endif
		for (unsigned i = 0; i < count; ++i) values[i] -= change;
	}

	static void materialize(const U* values, T* result, unsigned count, U base) {
		if (count == branch) {
#if defined(__AVX512F__) && defined(__x86_64__)
			if constexpr (sizeof(T) == 8) {
				auto value = _mm512_loadu_si512(values);
				value = _mm512_add_epi64(value, _mm512_set1_epi64(std::bit_cast<T>(base)));
				_mm512_storeu_si512(result, value);
				return;
			}
#endif
#if defined(__AVX2__) && defined(__x86_64__)
			if constexpr (sizeof(T) == 8) {
				auto delta = _mm256_set1_epi64x(std::bit_cast<T>(base));
				for (unsigned i = 0; i < branch; i += 4) {
					auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values + i));
					_mm256_storeu_si256(reinterpret_cast<__m256i*>(result + i), _mm256_add_epi64(value, delta));
				}
				return;
			}
			if constexpr (sizeof(T) == 4) {
				auto value = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(values));
				value = _mm256_add_epi32(value, _mm256_set1_epi32(std::bit_cast<T>(base)));
				_mm256_storeu_si256(reinterpret_cast<__m256i*>(result), value);
				return;
			}
			if constexpr (sizeof(T) == 2) {
				auto value = _mm_loadu_si128(reinterpret_cast<const __m128i*>(values));
				value = _mm_add_epi16(value, _mm_set1_epi16(std::bit_cast<T>(base)));
				_mm_storeu_si128(reinterpret_cast<__m128i*>(result), value);
				return;
			}
			if constexpr (sizeof(T) == 1) {
				auto value = _mm_loadl_epi64(reinterpret_cast<const __m128i*>(values));
				value = _mm_add_epi8(value, _mm_set1_epi8(std::bit_cast<T>(base)));
				_mm_storel_epi64(reinterpret_cast<__m128i*>(result), value);
				return;
			}
#endif
		}
		for (unsigned i = 0; i < count; ++i) result[i] = std::bit_cast<T>(U(values[i] + base));
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
