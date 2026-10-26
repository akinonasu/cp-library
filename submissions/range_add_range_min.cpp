#line 1 "test/range_add_range_min.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_add_range_min"
#line 1 "data_structure/range_add_range_min.hpp"



#line 1 "components/aligned_allocator.hpp"



#include <cstddef>
#include <limits>
#include <new>

template<class T, std::size_t Alignment = 64> struct AlignedAllocator {
	static_assert(Alignment >= alignof(T) && !(Alignment & (Alignment - 1)));
	using value_type = T;
	template<class U> struct rebind { using other = AlignedAllocator<U, Alignment>; };
	AlignedAllocator() = default;
	template<class U> AlignedAllocator(const AlignedAllocator<U, Alignment> &) {}
	T *allocate(std::size_t n) {
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
		return static_cast<T *>(::operator new(n * sizeof(T), std::align_val_t(Alignment)));
	}
	void deallocate(T *p, std::size_t) { ::operator delete(p, std::align_val_t(Alignment)); }
	template<class U> bool operator==(const AlignedAllocator<U, Alignment> &) const { return true; }
};


#line 1 "components/range_add_min_node.hpp"



#include <algorithm>
#include <array>
#include <bit>
#line 8 "components/range_add_min_node.hpp"
#include <cstdint>
#line 10 "components/range_add_min_node.hpp"
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


#line 9 "data_structure/range_add_range_min.hpp"
#include <cassert>
#line 13 "data_structure/range_add_range_min.hpp"
#include <stdexcept>
#line 16 "data_structure/range_add_range_min.hpp"
#include <vector>

template<class T = std::int64_t> class RangeAddRangeMin {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= 8);
	using U = std::make_unsigned_t<T>;
	using Node = range_add_min_detail::Node<T>;
	static constexpr unsigned branch = 8, bits = 3;
	static constexpr T infinity = std::numeric_limits<T>::max();
	static constexpr U umax = std::numeric_limits<U>::max();
	static constexpr U limit_ = U(infinity) / 3;
	std::vector<U, AlignedAllocator<U>> data_;
	std::array<std::size_t, (sizeof(int) * 8 + bits - 1) / bits + 1> offsets_{}, counts_{};
	int n_ = 0, minimum_pos_ = -1;
	unsigned height_ = 0;
	std::size_t width_ = 1;
	T root_ = infinity;
	U bound_ = 0;
	static T added(T value, U delta) { return std::bit_cast<T>(U(U(value) + delta)); }
	unsigned block_count(unsigned level, std::size_t index) const { return unsigned(std::min<std::size_t>(branch, counts_[level] - index * branch)); }
	template<bool Narrow>
	void propagate(unsigned level, std::size_t index, U change, bool negative) {
		for (; level < height_; ++level, index >>= bits) {
			if (!change) return;
			std::size_t block = index >> bits;
			change = Node::template change_one<Narrow>(data_.data() + offsets_[level] + block * branch, block_count(level, block), index & (branch - 1), change, negative);
		}
		root_ = added(root_, change);
	}
	template<bool Narrow>
	void update(std::size_t l, std::size_t r, U delta, bool negative) {
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
		if (a == b) {
			U change = first == last ? Node::template change_one<Narrow>(data_.data() + a * branch, block_count(0, a), first, delta, negative)
				: Node::template normalize<Narrow>(data_.data() + a * branch, block_count(0, a), negative, first, last + 1, delta);
			propagate<Narrow>(1, a, change, negative);
			return;
		}
		auto ends = Node::template normalize_boundaries<Narrow>(data_.data() + a * branch, data_.data() + b * branch, block_count(0, b), negative, first, last, delta, delta, delta);
		U left = ends.first, right = ends.second;
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			first = unsigned(l & (branch - 1));
			last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			U *values = data_.data() + offsets_[level];
			if (a == b) {
				U change;
				if (first + 1 == last) {
					change = Node::template change_one<Narrow>(values + a * branch, block_count(level, a), first, left, negative);
					change += Node::template change_one<Narrow>(values + a * branch, block_count(level, a), last, right, negative);
				} else change = Node::template normalize<Narrow>(values + a * branch, block_count(level, a), negative, first + 1, last, delta, first, left, last, right);
				propagate<Narrow>(level + 1, a, change, negative);
				return;
			}
			if (first + 1 < branch && last) {
				auto ends = Node::template normalize_boundaries<Narrow>(values + a * branch, values + b * branch, block_count(level, b), negative, first, last, delta, left, right);
				left = ends.first, right = ends.second;
			} else {
				left = first + 1 == branch ? Node::template change_one<Narrow>(values + a * branch, branch, first, left, negative)
					: Node::template normalize<Narrow>(values + a * branch, branch, negative, first + 1, branch, delta, first, left);
				right = !last ? Node::template change_one<Narrow>(values + b * branch, block_count(level, b), 0, right, negative)
					: Node::template normalize<Narrow>(values + b * branch, block_count(level, b), negative, 0, last, delta, last, right);
			}
		}
	}
	U restore(unsigned level, std::size_t index, U result) const {
		for (; level < height_; ++level, index >>= bits) result += data_[offsets_[level] + index];
		return result;
	}
	template<bool Narrow>
	U query(std::size_t l, std::size_t r) const {
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		if (a == b) return restore(1, a, Node::template minimum<Narrow>(data_.data() + a * branch, l & (branch - 1), (r & (branch - 1)) + 1));
		U left = Node::template minimum<Narrow>(data_.data() + a * branch, l & (branch - 1), branch);
		U right = Node::template minimum<Narrow>(data_.data() + b * branch, 0, (r & (branch - 1)) + 1);
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			left += data_[offsets_[level] + l];
			right += data_[offsets_[level] + r];
			unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			const U *values = data_.data() + offsets_[level];
			if (a == b) {
				U result = std::min(left, right);
				if (first + 1 < last) result = std::min(result, Node::template minimum<Narrow>(values + a * branch, first + 1, last));
				return restore(level + 1, a, result);
			}
			if (first + 1 < branch) left = std::min(left, Node::template minimum<Narrow>(values + a * branch, first + 1, branch));
			if (last) right = std::min(right, Node::template minimum<Narrow>(values + b * branch, 0, last));
		}
	}
#if defined(__AVX2__) && !defined(__AVX512F__) && defined(__x86_64__)
	static __m256i query_block(const U* values, unsigned first, unsigned last) {
		return Node::template minimum_vector<true>(values, first, last, 2 * limit_);
	}
	U query_vector(std::size_t l, std::size_t r) const {
		--r;
		std::size_t a = l >> bits, b = r >> bits;
		if (a == b) {
			auto value = query_block(data_.data() + a * branch, l & (branch - 1), (r & (branch - 1)) + 1);
			U carry = restore(1, a, 0);
			return Node::reduce_min(value) + carry;
		}
		auto left = query_block(data_.data() + a * branch, l & (branch - 1), branch);
		auto right = query_block(data_.data() + b * branch, 0, (r & (branch - 1)) + 1);
		for (unsigned level = 1;; ++level) {
			l >>= bits;
			r >>= bits;
			left = _mm256_add_epi64(left, _mm256_set1_epi64x(std::int64_t(data_[offsets_[level] + l])));
			right = _mm256_add_epi64(right, _mm256_set1_epi64x(std::int64_t(data_[offsets_[level] + r])));
			unsigned first = unsigned(l & (branch - 1)), last = unsigned(r & (branch - 1));
			a = l >> bits;
			b = r >> bits;
			const U* values = data_.data() + offsets_[level];
			if (a == b) {
				auto result = Node::min_signed(left, right);
				if (first + 1 < last) result = Node::min_signed(result, query_block(values + a * branch, first + 1, last));
				U carry = restore(level + 1, a, 0);
				return Node::reduce_min(result) + carry;
			}
			if (first + 1 < branch) left = Node::min_signed(left, query_block(values + a * branch, first + 1, branch));
			if (last) right = Node::min_signed(right, query_block(values + b * branch, 0, last));
		}
	}
#endif
	template<bool Narrow>
	U assign(unsigned level, std::size_t index, std::size_t pos, U delta, bool negative) {
		unsigned child = unsigned((pos >> (bits * level)) & (branch - 1));
		U change = level ? assign<Narrow>(level - 1, index * branch + child, pos, delta, negative) : delta;
		unsigned count = unsigned(std::min<std::size_t>(branch, counts_[level] - index * branch));
		return Node::template change_one<Narrow>(data_.data() + offsets_[level] + index * branch, count, child, change, negative);
	}
	void collect(unsigned level, std::size_t index, std::size_t start, std::size_t width, U carry, std::vector<T> &result) const {
		const U *values = data_.data() + offsets_[level] + index * branch;
		std::size_t part = width / branch;
		for (unsigned i = 0; i < branch && start + i * part < std::size_t(n_); ++i) {
			U next = U(carry + values[i]);
			if (!level) result[start + i] = added(root_, next);
			else collect(level - 1, index * branch + i, start + i * part, part, next, result);
		}
	}
	void padding(U value) {
		for (unsigned level = 0; level < height_; ++level) {
			std::size_t count = counts_[level], remainder = count & (branch - 1);
			if (remainder) {
				U* first = data_.data() + offsets_[level] + count;
				std::fill(first, first + branch - remainder, value);
			}
		}
	}
	void track(U amount) {
		if constexpr (sizeof(T) == 8) {
			bool narrow = bound_ < limit_;
			bound_ = narrow && amount < limit_ - bound_ ? U(bound_ + amount) : umax;
			if (narrow && bound_ == umax) padding(umax);
		}
	}
	void check([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
public:
	using value_type = T;
	RangeAddRangeMin() = default;
	explicit RangeAddRangeMin(int n, T value = 0) { assert(n >= 0); build(std::vector<T>(n, value)); }
	explicit RangeAddRangeMin(const std::vector<T> &values) { build(values); }
	RangeAddRangeMin(const RangeAddRangeMin &) = default;
	RangeAddRangeMin(RangeAddRangeMin &&other) noexcept { swap(other); }
	RangeAddRangeMin &operator=(RangeAddRangeMin other) noexcept { swap(other); return *this; }
	void build(const std::vector<T> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		RangeAddRangeMin next;
		next.n_ = int(values.size());
		if (!values.empty()) {
			std::size_t capacity = 0;
			for (std::size_t count = values.size();;) {
				std::size_t groups = (count + branch - 1) / branch;
				capacity += groups * branch;
				if (groups == 1) break;
				count = groups;
			}
			next.data_.reserve(capacity);
			std::vector<T> current = values;
			for (unsigned level = 0;; ++level) {
				std::size_t count = current.size(), groups = (count + branch - 1) / branch;
				next.offsets_[level] = next.data_.size();
				next.counts_[level] = count;
				next.data_.resize(next.data_.size() + groups * branch, umax);
				std::vector<T> upper(groups);
				for (std::size_t group = 0; group < groups; ++group) {
					std::size_t first = group * branch, last = std::min(count, first + branch);
					T minimum = *std::min_element(current.begin() + first, current.begin() + last);
					upper[group] = minimum;
					for (std::size_t i = first; i < last; ++i) next.data_[next.offsets_[level] + i] = U(U(current[i]) - U(minimum));
				}
				if (next.width_ > std::numeric_limits<std::size_t>::max() / branch) throw std::length_error("RangeAddRangeMin size");
				next.width_ *= branch;
				if (groups == 1) { next.height_ = level + 1; next.root_ = upper[0]; break; }
				current.swap(upper);
			}
			auto extremes = std::minmax_element(values.begin(), values.end());
			next.minimum_pos_ = values[values.size() / 2] == next.root_ ? int(values.size() / 2)
				: int(extremes.first - values.begin());
			if constexpr (sizeof(T) == 8) {
				next.bound_ = U(U(*extremes.second) - U(next.root_));
				if (next.bound_ < limit_) next.padding(2 * limit_);
			}
		}
		swap(next);
	}
	int size() const { return n_; }
	bool empty() const { return !n_; }
	void add(int l, int r, T value) {
		check(l, r);
		if (l == r || !value) return;
		if (l == 0 && r == n_) { root_ = added(root_, U(value)); return; }
		T candidate = root_;
		if (l <= minimum_pos_ && minimum_pos_ < r) candidate = added(root_, U(value));
		if constexpr (sizeof(T) == 8) {
			track(value < 0 ? U(0) - U(value) : U(value));
			if (bound_ < limit_) update<true>(l, r, U(value), value < 0);
			else update<false>(l, r, U(value), value < 0);
		} else update<false>(l, r, U(value), value < 0);
		if (minimum_pos_ >= 0 && root_ != candidate) minimum_pos_ = -1;
	}
	void apply(int l, int r, T value) { add(l, r, value); }
	T prod(int l, int r) const {
		check(l, r);
		if (l == r) return infinity;
		if ((l == 0 && r == n_) || (l <= minimum_pos_ && minimum_pos_ < r)) return root_;
		if constexpr (sizeof(T) == 8) if (bound_ < limit_) {
#if defined(__AVX2__) && !defined(__AVX512F__) && defined(__x86_64__)
			return added(root_, query_vector(l, r));
#else
			return added(root_, query<true>(l, r));
#endif
		}
		return added(root_, query<false>(l, r));
	}
	T min(int l, int r) const { return prod(l, r); }
	T all_prod() const { return root_; }
	T all_min() const { return root_; }
	T get(int p) const { assert(0 <= p && p < n_); return added(root_, restore(0, p, 0)); }
	void set(int p, T value) {
		assert(0 <= p && p < n_);
		T old = get(p);
		if (value == old) { if (value == root_) minimum_pos_ = p; return; }
		track(value < old ? U(U(old) - U(value)) : U(U(value) - U(old)));
		U delta = U(U(value) - U(old)), change;
		if constexpr (sizeof(T) == 8) {
			change = bound_ < limit_ ? assign<true>(height_ - 1, 0, p, delta, value < old)
				: assign<false>(height_ - 1, 0, p, delta, value < old);
		} else change = assign<false>(height_ - 1, 0, p, delta, value < old);
		root_ = added(root_, change);
		if (value == root_) minimum_pos_ = p;
		else if (minimum_pos_ == p) minimum_pos_ = -1;
	}
	std::vector<T> values() const { std::vector<T> result(n_); if (n_) collect(height_ - 1, 0, 0, width_, 0, result); return result; }
	std::size_t memory_usage() const { return data_.capacity() * sizeof(U); }
	void swap(RangeAddRangeMin &other) noexcept {
		data_.swap(other.data_), offsets_.swap(other.offsets_), counts_.swap(other.counts_);
		std::swap(n_, other.n_), std::swap(minimum_pos_, other.minimum_pos_), std::swap(height_, other.height_), std::swap(width_, other.width_), std::swap(root_, other.root_), std::swap(bound_, other.bound_);
	}
	friend void swap(RangeAddRangeMin &a, RangeAddRangeMin &b) noexcept { a.swap(b); }
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#include <memory>
#line 15 "components/fast_io.hpp"
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


#line 6 "test/range_add_range_min.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<std::int64_t> values(n);
	for (auto &value : values) input.read(value);
	RangeAddRangeMin tree(values);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (type == 0) { std::int64_t value = 0; input.read(value); tree.add(l, r, value); }
		else output.println(tree.prod(l, r));
	}
}
