#line 1 "test/range_affine_range_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum"
#line 1 "data_structure/range_affine_range_sum.hpp"



#line 1 "components/range_affine_sum_simd.hpp"



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


#line 1 "math/mint.hpp"


#line 4 "math/mint.hpp"
#include <istream>
#include <ostream>
#include <type_traits>

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace mint_detail {
template <class I>
constexpr bool mint_integral = std::is_integral_v<I>
#if defined(__SIZEOF_INT128__)
							   || std::is_same_v<I, __int128> || std::is_same_v<I, unsigned __int128>
#endif
	;
struct Mint {
	using mint = Mint;
	unsigned v = 0;
	static constexpr unsigned mod() { return 998244353; }
	[[gnu::always_inline]] static unsigned reduce(unsigned long long x) {
		return (x + (unsigned long long)(unsigned(x) * 998244351u) * mod()) >> 32;
	}
	[[gnu::always_inline]] Mint() = default;
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	Mint(I x) {
		long long r = x % (long long)mod();
		if (r < 0) r += mod();
		v = reduce((unsigned long long)r * 932051910);
	}
	[[gnu::always_inline]] static Mint raw(unsigned x) {
		Mint a;
		a.v = reduce((unsigned long long)x * 932051910);
		return a;
	}
	[[gnu::always_inline]] static Mint word(unsigned x) {
		Mint a;
		a.v = x;
		return a;
	}
	[[gnu::always_inline]] unsigned val() const {
		unsigned r = reduce(v);
		return r < mod() ? r : r - mod();
	}
	friend std::istream &operator>>(std::istream &is, Mint &x) {
		long long value;
		if (is >> value) x = Mint(value);
		return is;
	}
	friend std::ostream &operator<<(std::ostream &os, const Mint &x) {
		return os << x.val();
	}
	[[gnu::always_inline]] Mint &operator+=(Mint b) {
		v += b.v;
		if (v >= 2 * mod()) v -= 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator-=(Mint b) {
		v -= b.v;
		if (v >> 31) v += 2 * mod();
		return *this;
	}
	[[gnu::always_inline]] Mint &operator*=(Mint b) {
		v = reduce((unsigned long long)v * b.v);
		return *this;
	}
	Mint &operator/=(Mint b) { return *this *= b.inv(); }
	Mint &operator++() { return *this += Mint(1); }
	Mint &operator--() { return *this -= Mint(1); }
	Mint operator++(int) {
		Mint a = *this;
		++*this;
		return a;
	}
	Mint operator--(int) {
		Mint a = *this;
		--*this;
		return a;
	}
	[[gnu::always_inline]] Mint operator+() const { return *this; }
	[[gnu::always_inline]] Mint operator-() const { return word(v ? 2 * mod() - v : 0); }
	[[gnu::always_inline]] Mint operator+(Mint b) const { return Mint(*this) += b; }
	[[gnu::always_inline]] Mint operator-(Mint b) const { return Mint(*this) -= b; }
	[[gnu::always_inline]] Mint operator*(Mint b) const { return Mint(*this) *= b; }
	Mint operator/(Mint b) const { return Mint(*this) /= b; }
	[[gnu::always_inline]] bool operator==(Mint b) const { return (v < mod() ? v : v - mod()) == (b.v < mod() ? b.v : b.v - mod()); }
	[[gnu::always_inline]] bool operator!=(Mint b) const { return !(*this == b); }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator+(I a, Mint b) { return Mint(a) + b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator-(I a, Mint b) { return Mint(a) - b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator*(I a, Mint b) { return Mint(a) * b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend Mint operator/(I a, Mint b) { return Mint(a) / b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator==(I a, Mint b) { return Mint(a) == b; }
	template <class I, std::enable_if_t<mint_integral<I>, int> = 0>
	friend bool operator!=(I a, Mint b) { return Mint(a) != b; }
	Mint pow(long long n) const {
		assert(n >= 0);
		if (!n) return Mint(1);
		Mint r(1), a = *this;
		for (; n > 1; n >>= 1, a *= a)
			if (n & 1) r *= a;
		return r * a;
	}
	[[gnu::noinline]] Mint inv() const {
		assert(*this != Mint(0));
		Mint a2 = *this * *this, a3 = a2 * *this;
		Mint a7 = a3 * a3 * *this;
		Mint a31 = a7;
		for (int i = 0; i < 2; ++i) a31 *= a31;
		a31 *= a3;
		Mint r = a7;
		for (int i = 0; i < 3; ++i) r *= r;
		r *= a3;
		r *= r;
		for (int j = 0; j < 4; ++j) {
			for (int i = 0; i < 5; ++i) r *= r;
			r *= a31;
		}
		for (int i = 0; i < 3; ++i) r *= r;
		return r * a7;
	}
};
} // namespace mint_detail

using mint = mint_detail::Mint;
using fps_mint = mint;

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif


#line 1 "data_structure/lazy_segtree.hpp"



#line 7 "data_structure/lazy_segtree.hpp"
#include <limits>
#line 10 "data_structure/lazy_segtree.hpp"

template<class S, auto op, auto e, class F, auto mapping, auto composition, auto id>
class LazySegtree {
	int n_ = 0;
	unsigned base_ = 1, height_ = 0;
	std::vector<S> data_;
	std::vector<F> lazy_;
	std::vector<unsigned char> dirty_;
	void pull(unsigned k) { data_[k] = op(data_[k * 2], data_[k * 2 + 1]); }
	void all_apply(unsigned k, const F &f) {
		data_[k] = mapping(f, data_[k]);
		if (k < base_) {
			lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f;
			dirty_[k] = true;
		}
	}
	void push(unsigned k) {
		if (!dirty_[k]) return;
		all_apply(k * 2, lazy_[k]);
		all_apply(k * 2 + 1, lazy_[k]);
		dirty_[k] = false;
	}
	void push_path(unsigned p) {
		for (unsigned i = height_; i; --i) push(p >> i);
	}
	void push_boundaries(unsigned l, unsigned r) {
		for (unsigned i = height_; i; --i) {
			if ((l >> i) << i != l) push(l >> i);
			if ((r >> i) << i != r) push((r - 1) >> i);
		}
	}
	void allocate(int n) {
		assert(n >= 0);
		n_ = n;
		base_ = std::bit_ceil(unsigned(n) ? unsigned(n) : 1U);
		height_ = std::countr_zero(base_);
		data_.assign(std::size_t(base_) * 2, e());
		lazy_.assign(base_, id());
		dirty_.assign(base_, false);
	}
	void vertex([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	S prod_rec(unsigned k, unsigned l, unsigned r, unsigned ql, unsigned qr) const {
		if (ql <= l && r <= qr) return data_[k];
		unsigned m = l + (r - l) / 2;
		S value = qr <= m ? prod_rec(k * 2, l, m, ql, qr)
			: m <= ql ? prod_rec(k * 2 + 1, m, r, ql, qr)
			: op(prod_rec(k * 2, l, m, ql, qr), prod_rec(k * 2 + 1, m, r, ql, qr));
		return dirty_[k] ? mapping(lazy_[k], value) : value;
	}

  public:
	explicit LazySegtree(int n = 0) { allocate(n); }
	explicit LazySegtree(const std::vector<S> &a) {
		assert(a.size() <= std::size_t(std::numeric_limits<int>::max()));
		allocate(static_cast<int>(a.size()));
		for (int i = 0; i < n_; ++i) data_[base_ + unsigned(i)] = a[i];
		for (unsigned k = base_ - 1; k; --k) pull(k);
	}
	int size() const { return n_; }
	void set(int p, S value) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = std::move(value);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	S get(int p) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		return data_[k];
	}
	S all_prod() const { return data_[1]; }
	S prod(int l, int r) {
		range(l, r);
		if (l == r) return e();
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r);
		push_boundaries(a, b);
		S left = e(), right = e();
		while (a < b) {
			if (a & 1) left = op(left, data_[a++]);
			if (b & 1) right = op(data_[--b], right);
			a >>= 1, b >>= 1;
		}
		return op(left, right);
	}
	S prod(int l, int r) const {
		range(l, r);
		if (l == r) return e();
		return prod_rec(1, 0, base_, unsigned(l), unsigned(r));
	}
	void apply(int p, F f) {
		vertex(p);
		unsigned k = base_ + unsigned(p);
		push_path(k);
		data_[k] = mapping(f, data_[k]);
		for (k >>= 1; k; k >>= 1) pull(k);
	}
	void apply(int l, int r, F f) {
		range(l, r);
		if (l == r) return;
		unsigned a = base_ + unsigned(l), b = base_ + unsigned(r), first = a, last = b;
		push_boundaries(a, b);
		while (a < b) {
			if (a & 1) all_apply(a++, f);
			if (b & 1) all_apply(--b, f);
			a >>= 1, b >>= 1;
		}
		for (unsigned i = 1; i <= height_; ++i) {
			if ((first >> i) << i != first) pull(first >> i);
			if ((last >> i) << i != last) pull((last - 1) >> i);
		}
	}
	template<auto predicate> int max_right(int l) { return max_right(l, predicate); }
	template<class Pred> int max_right(int l, Pred predicate) {
		range(l, l);
		assert(predicate(e()));
		if (l == n_) return n_;
		unsigned k = base_ + unsigned(l);
		push_path(k);
		S acc = e();
		do {
			while (!(k & 1)) k >>= 1;
			S next = op(acc, data_[k]);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k *= 2;
					S part = op(acc, data_[k]);
					if (predicate(part)) acc = std::move(part), ++k;
				}
				return static_cast<int>(k - base_);
			}
			acc = std::move(next);
			++k;
		} while ((k & -k) != k);
		return n_;
	}
	template<auto predicate> int min_left(int r) { return min_left(r, predicate); }
	template<class Pred> int min_left(int r, Pred predicate) {
		range(r, r);
		assert(predicate(e()));
		if (!r) return 0;
		unsigned k = base_ + unsigned(r);
		push_path(k - 1);
		S acc = e();
		do {
			--k;
			while (k > 1 && (k & 1)) k >>= 1;
			S next = op(data_[k], acc);
			if (!predicate(next)) {
				while (k < base_) {
					push(k);
					k = k * 2 + 1;
					S part = op(data_[k], acc);
					if (predicate(part)) acc = std::move(part), --k;
				}
				return static_cast<int>(k + 1 - base_);
			}
			acc = std::move(next);
		} while ((k & -k) != k);
		return 0;
	}
};


#line 10 "data_structure/range_affine_range_sum.hpp"
#include <memory>
#line 14 "data_structure/range_affine_range_sum.hpp"

template<class Mint = mint>
class RangeAffineRangeSum {
	struct Summary { Mint sum; int length; };
	struct Affine { Mint a, b; };
	static Summary op(Summary a, Summary b) { return {a.sum + b.sum, a.length + b.length}; }
	static Summary e() { return {Mint(0), 0}; }
	static Summary mapping(Affine f, Summary x) { return {f.a * x.sum + f.b * Mint(x.length), x.length}; }
	static Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
	static Affine id() { return {Mint(1), Mint(0)}; }
	using Fallback = LazySegtree<Summary, op, e, Affine, mapping, composition, id>;
	static constexpr unsigned modulus = [] {
		if constexpr (requires { std::integral_constant<unsigned, Mint::mod()>{}; }) return unsigned(Mint::mod());
		else return 0U;
	}();
	static constexpr bool supported = modulus >= 3 && modulus < (1U << 30) && (modulus & 1);
	using Fast = range_affine_sum_detail::SimdTree<supported ? modulus : 998244353>;
	int n = 0;
	Fast fast;
	std::unique_ptr<Fallback> fallback;
	void range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n); }
	static Mint import(unsigned value) {
		if constexpr (supported) return Mint(value >= modulus ? value - modulus : value);
		else return Mint(value);
	}
	static std::vector<Mint> repeated(int size, const Mint &value) {
		assert(size >= 0);
		return std::vector<Mint>(std::size_t(size), value);
	}
public:
	using value_type = Mint;
	RangeAffineRangeSum() = default;
	explicit RangeAffineRangeSum(int size, Mint value = Mint(0)) : RangeAffineRangeSum(repeated(size, value)) {}
	explicit RangeAffineRangeSum(const std::vector<Mint> &values) {
		assert(values.size() <= std::size_t(std::numeric_limits<int>::max()));
		n = int(values.size());
		if (supported && range_affine_sum_detail::simd_available()) {
			std::vector<unsigned> canonical(values.size());
			for (std::size_t i = 0; i < values.size(); ++i) canonical[i] = unsigned(values[i].val());
			fast.build(canonical);
		} else {
			std::vector<Summary> a;
			a.reserve(values.size());
			for (const auto &value : values) a.push_back({value, 1});
			fallback = std::make_unique<Fallback>(a);
		}
	}
	RangeAffineRangeSum(const RangeAffineRangeSum &other)
		: n(other.n), fast(other.fast), fallback(other.fallback ? std::make_unique<Fallback>(*other.fallback) : nullptr) {}
	RangeAffineRangeSum(RangeAffineRangeSum &&other) noexcept { swap(other); }
	RangeAffineRangeSum &operator=(RangeAffineRangeSum other) noexcept { swap(other); return *this; }
	int size() const { return n; }
	void apply(int l, int r, Mint a, Mint b) {
		range(l, r);
		if (l == r) return;
		if (fallback) fallback->apply(l, r, {a, b});
		else fast.apply(unsigned(l), unsigned(r), unsigned(a.val()), unsigned(b.val()));
	}
	void apply(int p, Mint a, Mint b) { assert(0 <= p && p < n); apply(p, p + 1, a, b); }
	Mint prod(int l, int r) const {
		range(l, r);
		if (l == r) return Mint(0);
		if (fallback) return std::as_const(*fallback).prod(l, r).sum;
		return import(fast.prod(unsigned(l), unsigned(r)));
	}
	Mint sum(int l, int r) const { return prod(l, r); }
	Mint all_prod() const { return fallback ? fallback->all_prod().sum : import(fast.all_prod()); }
	Mint all_sum() const { return all_prod(); }
	Mint get(int p) const { assert(0 <= p && p < n); return prod(p, p + 1); }
	Mint operator[](int p) const { return get(p); }
	void set(int p, Mint value) { assert(0 <= p && p < n); apply(p, p + 1, Mint(0), value); }
	void swap(RangeAffineRangeSum &other) noexcept {
		std::swap(n, other.n); fast.swap(other.fast); fallback.swap(other.fallback);
	}
	friend void swap(RangeAffineRangeSum &a, RangeAffineRangeSum &b) noexcept { a.swap(b); }
};


#line 1 "components/fast_io.hpp"



#line 8 "components/fast_io.hpp"
#include <charconv>
#line 11 "components/fast_io.hpp"
#include <cstdio>
#include <cstring>
#line 14 "components/fast_io.hpp"
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


#line 4 "test/range_affine_range_sum.test.cpp"

int main() {
	FastInput input;
	FastOutput output;
	int n = 0, q = 0;
	input.read(n), input.read(q);
	std::vector<mint> a(n);
	for (auto &x : a) { unsigned value = 0; input.read(value); x = mint::raw(value); }
	RangeAffineRangeSum tree(a);
	while (q--) {
		int type = 0, l = 0, r = 0;
		input.read(type), input.read(l), input.read(r);
		if (!type) {
			unsigned b = 0, c = 0;
			input.read(b), input.read(c);
			tree.apply(l, r, mint::raw(b), mint::raw(c));
		} else output.println(tree.prod(l, r).val());
	}
}
