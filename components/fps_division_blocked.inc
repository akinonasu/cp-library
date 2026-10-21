#ifndef CP_COMPONENTS_FPS_DIVISION_BLOCKED_HPP
#define CP_COMPONENTS_FPS_DIVISION_BLOCKED_HPP

#include "fps_blocked.hpp"
#include <limits>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

inline std::size_t quotient_size(std::size_t n, std::size_t m) { return n >= m ? n - m + 1 : 0; }
inline std::size_t remainder_size(std::size_t n, std::size_t m) { return std::min(n, m - 1); }

namespace division_detail {
using namespace detail;

inline constexpr std::size_t kDivisionTerms = 8;

inline constexpr std::size_t kLongDivision = std::size_t(1) << 12;

inline constexpr std::size_t kDirectRemainder = 64;

inline constexpr std::array<double, 26> kTransformNs = {0,    0,    0,    0,    0,    0,    0.45, 0.48, 0.51,
														0.55, 0.6,  0.65, 0.7,  0.76, 0.81, 0.88, 0.95, 1.02,
														1.09, 1.17, 1.23, 1.3,  1.37, 1.44, 1.51, 1.58};
inline constexpr double kLeafProductNs = 0.92, kDirectTermNs = 0.04, kFoldNs = 0.25;

inline std::size_t transform_length(std::size_t n) { return std::max<std::size_t>(64, std::bit_ceil(n)); }

inline double transform_ns(std::size_t len) {
	const auto lg = std::countr_zero(len);
	return std::size_t(lg) < kTransformNs.size() ? kTransformNs[lg] * double(len) : std::numeric_limits<double>::infinity();
}
inline double cyclic_ns(std::size_t len) { return 2 * transform_ns(len) + kLeafProductNs * double(len); }

inline double inverse_ns(std::size_t n) {
	double ns = 0;
	for (std::size_t k = kInverseBase; k < n; k *= 2) ns += transform_ns(2 * k) + 2 * cyclic_ns(2 * k);
	return ns;
}

struct QuotientPlan {
	std::size_t s = 0, blocks = 0, pad = 0, windows = 0, tail = 0;
	bool tails = false;
	double ns = std::numeric_limits<double>::infinity();

	QuotientPlan() = default;

	QuotientPlan(std::size_t k, std::size_t d, std::size_t block)
		: s(block), blocks((k + block - 1) / block), pad(blocks * block - k), tail(transform_length(2 * d)),
		  tails(d < block) {
		const std::size_t len = 2 * s;
		ns = inverse_ns(precision(k)) + transform_ns(len) + double(blocks) * cyclic_ns(len);
		if (blocks == 1) return;
		if (tails) {
			ns += transform_ns(tail) + double(blocks - 1) * cyclic_ns(tail);
			return;
		}
		const std::size_t reach = d / s + 1;
		windows = std::min(reach, blocks - 1);
		if (windows > kDivisionTerms) {
			ns = std::numeric_limits<double>::infinity();
			return;
		}
		double terms = 0;
		for (std::size_t j = 1; j < blocks; ++j) terms += double(std::min(j, reach));
		ns += double(windows + 2 * (blocks - 1)) * transform_ns(len) + terms * kLeafProductNs * double(len);
	}

	std::size_t precision(std::size_t k) const { return blocks == 1 ? k : s; }
};

enum class Remainder { kDirect, kWrap, kSplit, kTail };

struct RemainderPlan {
	Remainder method;
	std::size_t length = 0;
	double ns = std::numeric_limits<double>::infinity();

	RemainderPlan(Remainder how, std::size_t n, std::size_t k, std::size_t d, const QuotientPlan& quotient) : method(how) {
		const std::size_t wrap = transform_length(d);
		switch (how) {
			case Remainder::kDirect:
				if (k <= kDirectRemainder) ns = double(k) * double(d) * kDirectTermNs;
				break;
			case Remainder::kWrap:
				length = wrap;
				ns = cyclic_ns(length) + transform_ns(length) + double(n + k) * kFoldNs;
				break;
			case Remainder::kSplit:
				length = wrap / 2;
				if (length >= 64 && d > length && k + (d - length) <= length + 1)
					ns = transform_ns(length) + 2 * cyclic_ns(length) + double(n + d) * kFoldNs;
				break;
			case Remainder::kTail:
				if (!quotient.tails) break;
				ns = (quotient.blocks == 1 ? transform_ns(quotient.tail) : 0) + cyclic_ns(quotient.tail) + double(d) * kFoldNs;
				break;
		}
	}

	std::size_t buffers() const { return method == Remainder::kWrap ? 2 : method == Remainder::kSplit ? 3 : 0; }
};

struct DivisionPlan {
	std::size_t n, m, k, d;
	bool long_division;
	QuotientPlan quotient;
	RemainderPlan remainder;
	int lg = Transform::kMinLog;

	DivisionPlan(std::size_t n_, std::size_t m_)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(k * d <= kLongDivision),
		  remainder(Remainder::kWrap, n, k, d, quotient) {
		if (long_division) return;
		double best = std::numeric_limits<double>::infinity();
		for (std::size_t s = 32;; s *= 2) {
			const QuotientPlan p(k, d, s);
			for (Remainder how : {Remainder::kDirect, Remainder::kWrap, Remainder::kSplit, Remainder::kTail}) {
				const RemainderPlan r(how, n, k, d, p);
				if (p.ns + r.ns < best) best = p.ns + r.ns, quotient = p, remainder = r;
			}
			if (s >= k || 4 * s > (std::size_t(1) << Transform::kMaxLog)) break;
		}
		set_log();
	}

	DivisionPlan(std::size_t n_, std::size_t m_, std::size_t s, Remainder how)
		: n(n_), m(m_), k(quotient_size(n_, m_)), d(m_ - 1), long_division(false), quotient(k, d, s),
		  remainder(how, n, k, d, quotient) {
		set_log();
	}

	bool tails() const { return quotient.tails && (quotient.blocks > 1 || remainder.method == Remainder::kTail); }

	void set_log() { lg = std::countr_zero(std::max({2 * quotient.s, tails() ? quotient.tail : 0, remainder.length})); }

	std::size_t divisor() const {
		const QuotientPlan& p = quotient;
		return tails() ? m : std::min(m, std::max(p.precision(k), (p.windows + 1) * p.s));
	}

	std::size_t quotient_words() const {
		const QuotientPlan& p = quotient;
		const std::size_t len = 2 * p.s;
		std::size_t later = Arena::footprint(p.blocks * p.s + p.s);
		if (tails()) later += 2 * Arena::footprint(p.tail);
		if (p.windows) later += (2 * p.windows + 1) * Arena::footprint(len);
		return Arena::footprint(divisor()) + Arena::footprint(len) + std::max(inverse_scratch(p.precision(k)), later);
	}

	std::size_t scratch() const { return std::max(quotient_words(), remainder.buffers() * Arena::footprint(remainder.length)); }

	std::size_t words() const {
		return long_division ? Arena::footprint(n) : Transform::words(lg) + Arena::footprint(scratch());
	}
};

class Bump {
public:
	explicit Bump(std::span<std::uint32_t> memory) : memory_(memory) {}

	std::span<std::uint32_t> take(std::size_t n) {
		if (used_ > memory_.size() || Arena::footprint(n) > memory_.size() - used_) throw std::bad_alloc();
		const std::span<std::uint32_t> s = memory_.subspan(used_, n);
		used_ += Arena::footprint(n);
		return s;
	}

	std::span<std::uint32_t> rest() const { return memory_.subspan(used_); }

private:
	std::span<std::uint32_t> memory_;
	std::size_t used_ = 0;
};

inline Vec montgomery_reduce(Vec even, Vec odd) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

inline std::uint32_t input_word(std::uint32_t x) { return x; }
inline std::uint32_t input_word(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec load_any(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline Vec load_any(const Mint* p) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	Vec value;
	std::memcpy(&value, p, sizeof(value));
	return reduce(value, kP);
}
inline void store_any(std::uint32_t* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }
inline Vec reversed(Vec x) { return _mm256_permutevar8x32_epi32(x, _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0)); }

inline void append_words(std::vector<Mint>& out, const std::uint32_t* values, std::size_t n, std::uint32_t factor = 1) {
	const Factor scale(factor);
	std::size_t i = 0;
	for (; i + 8 <= n; i += 8) {
		Vec word = load_any(values + i);
		if (factor != 1) word = times(word, scale);
		Mint block[8];
		static_assert(sizeof(block) == sizeof(word) && std::is_trivially_copyable_v<Mint>);
		std::memcpy(static_cast<void*>(block), &word, sizeof(block));
		out.insert(out.end(), block, block + 8);
	}
	for (; i < n; ++i) out.emplace_back(Mint::word(factor == 1 ? values[i] : blocked_ntt::detail::multiply_mod(values[i], factor)));
}

inline void append_division_words(std::vector<Mint>& out, const std::uint32_t* values, std::size_t n, std::uint32_t factor = 1) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	const Factor scale(factor);
	std::array<Mint, 64> batch;
	std::size_t i = 0;
	for (; i + 64 <= n; i += 64) {
		if (factor == 1) std::memcpy(static_cast<void*>(batch.data()), values + i, sizeof(batch));
		else {
#pragma GCC unroll 8
			for (std::size_t j = 0; j < 64; j += 8) {
				const Vec word = times(load_any(values + i + j), scale);
				std::memcpy(static_cast<void*>(batch.data() + j), &word, sizeof(word));
			}
		}
		out.insert(out.end(), batch.begin(), batch.end());
	}
	if (i < n) append_words(out, values + i, n - i, factor);
}

template<class T>
inline void subtract(const T* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) store_any(out + i, difference(load_any(a + i), load_any(b + i)));
	for (; i < count; ++i) out[i] = (input_word(a[i]) + kP - b[i]) % kP;
}

template<class T>
inline void reverse_difference(std::span<T> a, std::ptrdiff_t top, std::uint32_t* out, std::size_t count,
							   const std::uint32_t* sub = nullptr) {
	const std::ptrdiff_t size = std::ptrdiff_t(a.size());
	const auto value = [&](std::size_t i) {
		const std::ptrdiff_t p = top - std::ptrdiff_t(i);
		const std::uint32_t x = p >= 0 && p < size ? input_word(a[std::size_t(p)]) : 0;
		return sub ? (x + kP - sub[i]) % kP : x;
	};
	std::size_t i = top >= size ? std::min(count, std::size_t(top - size + 1)) : 0;
	if (sub) {
		for (std::size_t j = 0; j < i; ++j) out[j] = value(j);
	} else {
		std::fill_n(out, i, 0);
	}
	for (; i + 8 <= count && top - std::ptrdiff_t(i) >= 7; i += 8) {
		const Vec x = reversed(load_any(a.data() + (top - std::ptrdiff_t(i) - 7)));
		store_any(out + i, sub ? difference(x, load_any(sub + i)) : x);
	}
	for (; i < count; ++i) out[i] = value(i);
}

template<class T>
inline void fold(std::span<T> a, std::span<std::uint32_t> out) {
	const std::size_t len = out.size();
	std::fill(out.begin(), out.end(), 0);
	for (std::size_t from = 0; from < a.size(); from += len) {
		const std::size_t count = std::min(len, a.size() - from), full = count / 8 * 8;
		const T* x = a.data() + from;
		for (std::size_t j = 0; j < full; j += 8) {
			store(out.data() + j, reduce(add(load(out.data() + j), load_any(x + j)), kP));
		}
		for (std::size_t j = full; j < count; ++j) out[j] = (out[j] + input_word(x[j])) % kP;
	}
}

template<class T>
inline void long_division(std::span<const T> f, std::span<const std::uint32_t> g, std::span<std::uint32_t> q,
						  std::span<std::uint32_t> r, std::span<std::uint32_t> rest) {
	using blocked_ntt::detail::multiply_mod;
	const std::size_t d = g.size() - 1;
	if constexpr (std::is_same_v<T, std::uint32_t>) std::copy(f.begin(), f.end(), rest.begin());
	else for (std::size_t i = 0; i < f.size(); ++i) rest[i] = input_word(f[i]);
	const std::uint32_t lead = blocked_ntt::detail::power(g[d], kP - 2);
	for (std::size_t i = q.size(); i-- > 0;) {
		const std::uint32_t c = multiply_mod(rest[i + d], lead);
		q[i] = c;
		for (std::size_t t = 0; t < d; ++t) rest[i + t] = (rest[i + t] + kP - multiply_mod(c, g[t])) % kP;
	}
	std::copy_n(rest.begin(), r.size(), r.begin());
}

template<class T>
inline void remainder_direct(std::span<const T> f, std::span<const std::uint32_t> g,
							 std::span<const std::uint32_t> q, std::span<std::uint32_t> r) {
	const std::size_t d = r.size(), k = q.size();
	Vec factor[kDirectRemainder];
	for (std::size_t i = 0; i < k; ++i) factor[i] = _mm256_set1_epi64x(std::int64_t((std::uint64_t(q[i]) << 32) % kP));
	const auto scalar = [&](std::size_t j) {
		std::uint64_t sum = 0;
		for (std::size_t i = 0; i <= std::min(j, k - 1); ++i) sum = (sum + std::uint64_t(q[i]) * g[j - i]) % kP;
		r[j] = std::uint32_t((input_word(f[j]) + kP - sum) % kP);
	};
	std::size_t j = 0;
	for (; j < std::min(d, k); ++j) scalar(j);
	for (; j + 8 <= d; j += 8) {
		Vec total = load_any(f.data() + j);
		for (std::size_t i = 0; i < k; i += 4) {
			Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
			for (std::size_t t = i; t < std::min(k, i + 4); ++t) {
				const Vec x = load_any(g.data() + (j - t));
				even = _mm256_add_epi64(even, _mm256_mul_epu32(x, factor[t]));
				odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_srli_epi64(x, 32), factor[t]));
			}
			total = reduce(_mm256_sub_epi32(add(total, broadcast(2 * kP)), montgomery_reduce(even, odd)), 2 * kP);
		}
		store_any(r.data() + j, reduce(total, kP));
	}
	for (; j < d; ++j) scalar(j);
}

template<class T>
inline void remainder_wrap(const Transform& t, std::span<const T> f, std::span<const std::uint32_t> g,
						   std::span<const std::uint32_t> q, std::span<std::uint32_t> r, std::span<std::uint32_t> a,
						   std::span<std::uint32_t> b) {
	const std::size_t len = a.size();
	if (q.size() <= len) {
		t.forward(q, 0, a);
	} else {
		fold(q, a);
		t.forward(a);
	}
	std::span<const std::uint32_t> gl = g;
	if (g.size() > len) {
		std::copy_n(g.begin(), len, b.begin());
		b[0] = (b[0] + g[len]) % kP;
		gl = b;
	}
	t.cyclic_product(gl, 0, b, a);
	fold(f, a);
	subtract(a.data(), b.data(), r.data(), r.size());
}

template<class T>
inline void remainder_split(const Transform& t, std::span<const T> f, std::span<const std::uint32_t> g,
							std::span<const std::uint32_t> q, std::span<std::uint32_t> r, std::span<std::uint32_t> a,
							std::span<std::uint32_t> b, std::span<std::uint32_t> c) {
	const std::size_t len = a.size(), k = q.size(), e = r.size() - len;
	t.forward(q, 0, a);
	t.cyclic_product(g.subspan(len + 1 - k, k + e - 1), 0, c, a);
	subtract(f.data() + len, c.data() + (k - 1), r.data() + len, e);
	fold(g, b);
	t.cyclic_product(b, a);
	fold(f, c);
	subtract(c.data(), b.data(), r.data(), len);
	subtract(r.data(), r.data() + len, r.data(), e);
}

template <std::size_t K>
void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, std::span<std::uint32_t> out) {
	InverseProductSumBottom<K> bottom{};
	for (std::size_t i = 0; i < K; ++i)
		bottom.terms[i] = {{t.roots(), t.inverse_roots(), pairs[i].b.data()}, pairs[i].a.data()};
	t.inverse_with(bottom, out, Half::kUpper);
}

inline void residual_sum(const Transform& t, std::span<const Transform::Pair> pairs, std::span<std::uint32_t> out) {
	switch (pairs.size()) {
		case 1:
		case 2:
		case 3: return t.inverse_product_sum(pairs, out, Half::kUpper);
		case 4: return residual_sum<4>(t, pairs, out);
		case 5: return residual_sum<5>(t, pairs, out);
		case 6: return residual_sum<6>(t, pairs, out);
		case 7: return residual_sum<7>(t, pairs, out);
		case 8: return residual_sum<8>(t, pairs, out);
		default: std::abort();
	}
}

template<class T>
inline void subtract_reversed(const T* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {
	std::size_t j = 0;
	for (; j + 8 <= count; j += 8) store_any(out + j, difference(load_any(a + j), reversed(load_any(b + count - 8 - j))));
	for (; j < count; ++j) out[j] = (input_word(a[j]) + kP - b[count - 1 - j]) % kP;
}

template<class T>
inline void divide_blocks(const Transform& t, const DivisionPlan& plan, std::span<const T> f,
						  std::span<const std::uint32_t> g, std::span<std::uint32_t> q, std::span<std::uint32_t> r,
						  std::span<std::uint32_t> scratch) {
	const QuotientPlan& p = plan.quotient;
	const std::size_t k = q.size(), s = p.s, len = 2 * s, d = plan.d;
	const std::ptrdiff_t top = std::ptrdiff_t(f.size() - 1 + p.pad);
	Bump bump(scratch);
	const std::span<std::uint32_t> G = bump.take(plan.divisor()), ht = bump.take(len);
	std::reverse_copy(g.end() - std::ptrdiff_t(G.size()), g.end(), G.begin());
	const std::size_t precision = p.precision(k);
	inverse(t, G.first(std::min(G.size(), precision)), ht.first(precision), bump.rest());
	t.forward(ht.first(precision), 0, ht);

	const std::span<std::uint32_t> qs = bump.take(p.blocks * s + s);
	std::span<std::uint32_t> gt, mp, work;
	std::array<std::span<std::uint32_t>, kDivisionTerms> windows{}, ring{};
	if (plan.tails()) {
		gt = bump.take(p.tail), mp = bump.take(p.tail);
		t.forward(G, 0, gt);
	}
	if (p.windows) {
		work = bump.take(len);
		for (std::size_t i = 0; i < p.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(G.subspan(i * s, std::min(len, G.size() - i * s)), 0, windows[i]);
		}
	}

	const auto tail_product = [&](std::size_t j) {
		t.cyclic_product(qs.subspan(j * s - d, d), p.tail - 2 * d, mp, gt, Half::kUpper);
		return mp.data() + (p.tail - d);
	};
	for (std::size_t j = 0; j < p.blocks; ++j) {
		std::uint32_t* const block = qs.data() + j * s;
		const std::ptrdiff_t from = top - std::ptrdiff_t(j * s);
		if (j == 0) {
			reverse_difference(f, from, block, s);
		} else if (p.tails) {
			reverse_difference(f, from, block, d, tail_product(j));
			reverse_difference(f, from - std::ptrdiff_t(d), block + d, s - d);
		} else {
			const std::size_t terms = std::min(j, d / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (std::size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % p.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			reverse_difference(f, from, block, s, work.data() + s);
		}
		t.cyclic_product(qs.subspan(j * s, s), 0, qs.subspan(j * s, len), ht, Half::kLower);
		if (p.windows && j + 1 < p.blocks) t.forward(qs.subspan(j * s, s), 0, ring[j % p.windows]);

		reverse_difference(qs.first((j + 1) * s), std::ptrdiff_t((j + 1) * s - 1), q.data() + (p.blocks - 1 - j) * s,
						   j == 0 ? s - p.pad : s);
	}

	if (plan.remainder.method == Remainder::kTail) subtract_reversed(f.data(), tail_product(p.blocks), r.data(), d);
}

template<class T>
inline void divide(Arena& arena, const DivisionPlan& plan, std::span<const T> f, std::span<const std::uint32_t> g,
				   std::span<std::uint32_t> q, std::span<std::uint32_t> r) {
	if (plan.long_division) return long_division(f, g, q, r, arena.take(f.size()));
	const Transform t(arena, plan.lg);
	const std::span<std::uint32_t> scratch = arena.take(plan.scratch());
	divide_blocks(t, plan, f, g, q, r, scratch);
	Bump bump(scratch);
	const std::size_t len = plan.remainder.length;
	switch (plan.remainder.method) {
		case Remainder::kDirect: return remainder_direct(f, g, q, r);
		case Remainder::kWrap: return remainder_wrap(t, f, g, q, r, bump.take(len), bump.take(len));
		case Remainder::kSplit: return remainder_split(t, f, g, q, r, bump.take(len), bump.take(len), bump.take(len));
		case Remainder::kTail: return;
	}
}

}

inline std::size_t divide_words(std::size_t n, std::size_t m) { return division_detail::DivisionPlan(n, m).words(); }

inline void divide(Arena& arena, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
				   std::span<std::uint32_t> q, std::span<std::uint32_t> r) {
	const division_detail::DivisionPlan plan(f.size(), g.size());
	if (plan.k == 0) {
		std::copy(f.begin(), f.end(), r.begin());
		return;
	}
	division_detail::divide(arena, plan, f, g, q, r);
}

}


namespace fps_detail::blocked_poly {
inline void native_divide(const Mint* a, std::size_t n, const Mint* b, std::size_t m,
						  std::vector<Mint>& quotient, std::vector<Mint>& remainder) {
	const division_detail::DivisionPlan plan(n, m);
	const std::size_t k = quotient_size(n, m), d = remainder_size(n, m);
	Arena arena(plan.words() + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));
	const std::span<const Mint> f(a, n);
	auto g = arena.take(m), q = arena.take(k), r = arena.take(d);
	for (std::size_t i = 0; i < m; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	division_detail::divide(arena, plan, f, g, q, r);
	prepare_fps_output(quotient, k), prepare_fps_output(remainder, d);
	division_detail::append_division_words(quotient, q.data(), q.size(), blocked_ntt::detail::kR);
	division_detail::append_division_words(remainder, r.data(), r.size());
}

inline void native_div_series(const Mint* a, std::size_t input, const Mint* b, std::size_t divisor,
							  std::vector<Mint>& out, std::size_t n) {
	using namespace division_detail;
	const std::size_t count = std::min(divisor, n), degree = count - 1;
	QuotientPlan plan;
	for (std::size_t s = 32;; s *= 2) {
		QuotientPlan candidate(n, degree, s);
		if (candidate.ns < plan.ns) plan = candidate;
		if (s >= n || 4 * s > (std::size_t(1) << Transform::kMaxLog)) break;
	}
	const std::size_t s = plan.s, len = 2 * s;
	const bool tails = plan.tails && plan.blocks > 1;
	const std::size_t precision = plan.precision(n), numerator = std::min(input, n);
	const int lg = std::countr_zero(std::max(len, tails ? plan.tail : 0));
	const std::size_t later = Arena::footprint(plan.blocks * s + s) +
		(tails ? 2 * Arena::footprint(plan.tail) : 0) +
		(plan.windows ? (2 * plan.windows + 1) * Arena::footprint(len) : 0);
	Arena arena(Transform::words(lg) + Arena::footprint(numerator) + Arena::footprint(count) +
		Arena::footprint(len) + Arena::footprint(std::max(inverse_scratch(precision), later)));
	const Transform t(arena, lg);
	auto f = arena.take(numerator), g = arena.take(count), ht = arena.take(len);
	for (std::size_t i = 0; i < numerator; ++i) f[i] = std::min(a[i].v, a[i].v - Mint::mod());
	for (std::size_t i = 0; i < count; ++i) g[i] = std::min(b[i].v, b[i].v - Mint::mod());
	auto scratch = arena.take(std::max(inverse_scratch(precision), later));
	inverse(t, g.first(std::min(count, precision)), ht.first(precision), scratch);
	t.forward(ht.first(precision), 0, ht);
	Bump bump(scratch);
	auto qs = bump.take(plan.blocks * s + s);
	std::span<std::uint32_t> gt, mp, work;
	std::array<std::span<std::uint32_t>, kDivisionTerms> windows{}, ring{};
	if (tails) {
		gt = bump.take(plan.tail), mp = bump.take(plan.tail);
		t.forward(g, 0, gt);
	}
	if (plan.windows) {
		work = bump.take(len);
		for (std::size_t i = 0; i < plan.windows; ++i) {
			windows[i] = bump.take(len), ring[i] = bump.take(len);
			t.forward(g.subspan(i * s, std::min(len, count - i * s)), 0, windows[i]);
		}
	}
	prepare_fps_output(out, n);
	for (std::size_t j = 0; j < plan.blocks; ++j) {
		auto block = qs.subspan(j * s, len);
		const std::size_t from = j * s, available = from < numerator ? std::min(s, numerator - from) : 0;
		if (available) std::copy_n(f.data() + from, available, block.data());
		std::fill(block.begin() + available, block.begin() + s, 0u);
		if (j && tails) {
			t.cyclic_product(qs.subspan(j * s - degree, degree), plan.tail - 2 * degree, mp, gt, Half::kUpper);
			subtract(block.data(), mp.data() + plan.tail - degree, block.data(), degree);
		} else if (j) {
			const std::size_t terms = std::min(j, degree / s + 1);
			std::array<Transform::Pair, kDivisionTerms> pairs;
			for (std::size_t i = 0; i < terms; ++i) pairs[i] = {windows[i], ring[(j - 1 - i) % plan.windows]};
			residual_sum(t, std::span<const Transform::Pair>(pairs.data(), terms), work);
			subtract(block.data(), work.data() + s, block.data(), s);
		}
		t.cyclic_product(block.first(s), 0, block, ht, Half::kLower);
		if (plan.windows && j + 1 < plan.blocks) t.forward(block.first(s), 0, ring[j % plan.windows]);
		append_division_words(out, block.data(), std::min(s, n - from), blocked_ntt::detail::kR);
	}
}
}

#pragma GCC pop_options
#endif
#endif
