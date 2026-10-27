#ifndef CP_COMPONENTS_FPS_COMPOSITION_COMMON_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_COMMON_HPP

#include "fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::composition_poly {
using Word = std::uint32_t;
using Words = std::span<Word>;
using ConstWords = std::span<const Word>;
using std::size_t, blocked_poly::Arena, blocked_poly::Transform, blocked_poly::Half, blocked_poly::kModulus;
namespace kernels {
using Vec = __m256i;
namespace column_detail = blocked_poly::detail;

template<bool Inverse>
[[gnu::noinline]] void columns(Vec *a, size_t h, size_t width,
		const Word *x, const Word *y) {
	using namespace column_detail;
	const Factor wx(x[0], x[8]), wy(y[0], y[8]), wz(y[1], y[9]);
	for (size_t c = 0; c < h; c += 2 * width)
		for (size_t j = c; j < c + width; ++j) {
			const Vec f0 = a[j], f1 = a[j + h], f2 = a[j + 2 * h], f3 = a[j + 3 * h];
			if constexpr (Inverse) {
				const Vec ab = low(add(f0, f1)), cd = low(add(f2, f3));
				const Vec amb = times(diff(f0, f1), wy), cmd = times(diff(f2, f3), wz);
				a[j] = low(add(ab, cd)), a[j + h] = low(add(amb, cmd));
				a[j + 2 * h] = times(diff(ab, cd), wx), a[j + 3 * h] = times(diff(amb, cmd), wx);
			} else {
				const Vec lo = low(f0), hi = low(f1), xc = times(f2, wx), xd = times(f3, wx);
				const Vec ac = low(add(lo, xc)), amc = low_difference(lo, xc);
				const Vec bd = times(add(hi, xd), wy), bmd = times(diff(hi, xd), wz);
				a[j] = add(ac, bd), a[j + h] = diff(ac, bd);
				a[j + 2 * h] = add(amc, bmd), a[j + 3 * h] = diff(amc, bmd);
			}
		}
}
inline void forward_columns(Vec *a, size_t h, size_t width, const Word *x, const Word *y) {
	columns<false>(a, h, width, x, y);
}
inline void inverse_columns(Vec *a, size_t h, size_t width, const Word *x, const Word *y) {
	columns<true>(a, h, width, x, y);
}
inline void forward_even_columns(Vec *a, size_t h, const Word *x, const Word *y) {
	columns<false>(a, h, 1, x, y);
}
inline void inverse_even_columns(Vec *a, size_t h, const Word *x, const Word *y) {
	columns<true>(a, h, 1, x, y);
}
}

namespace detail {
using namespace blocked_poly::detail;
using blocked_poly::detail::times;
struct Factors {
	Vec w, q;
};

inline Vec times(Vec x, const Factors& w) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, w.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(w.q, 32));
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline constexpr auto kInverseScales = [] {
	std::array<std::array<Word, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(Word(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Factors entries(const Word* table, size_t k) {
	const Word* e = table + slot(k);
	return {load(e), load(e + 8)};
}

inline constexpr size_t kComposeBase = 32;

constexpr Word minus_twice(Word x) {
	const Word y = x ? 2 * kP - 2 * x : 0;
	return y >= kP ? y - kP : y;
}

inline Vec negate(Vec x) {
	return _mm256_min_epu32(_mm256_sub_epi32(broadcast(kP), x), _mm256_sub_epi32(_mm256_setzero_si256(), x));
}

inline Vec even_lanes(Vec x, Vec y) {
	const Vec t = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), 0x88));
	return _mm256_permute4x64_epi64(t, 0xD8);
}

inline Vec twice_mod(Vec x) { return reduce(add(x, x), kP); }

inline void transpose(Vec (&r)[8]) {
	Vec t[8], u[8];
#pragma GCC unroll 4
	for (int i = 0; i < 8; i += 2) t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]), t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
#pragma GCC unroll 2
	for (int i = 0; i < 8; i += 4)
#pragma GCC unroll 2
		for (int j = 0; j < 2; ++j)
			u[i + 2 * j] = _mm256_unpacklo_epi64(t[i + j], t[i + j + 2]), u[i + 2 * j + 1] = _mm256_unpackhi_epi64(t[i + j], t[i + j + 2]);
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i)
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20), r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
}

inline Vec product(Vec x, Vec y) { return _mm256_mul_epu32(x, y); }
inline Vec twice(Vec x) { return _mm256_add_epi64(x, x); }
inline Vec plus(Vec x, Vec y) { return _mm256_add_epi64(x, y); }
inline Vec minus(Vec x, Vec y) { return _mm256_sub_epi64(x, y); }

inline Vec offset() { return _mm256_set1_epi64x(static_cast<long long>(4 * std::uint64_t(kP) * kP)); }

template <size_t N>
[[gnu::always_inline]] inline void odd_lanes(const Vec (&x)[N], Vec (&out)[N]) {
#pragma GCC unroll 8
	for (size_t i = 0; i < N; ++i) out[i] = _mm256_srli_epi64(x[i], 32);
}

inline Vec reduce_sums(Vec even, Vec odd) {
	return mint_detail::Ops::reduce8(even, odd);
}

[[gnu::always_inline]] inline void graeffe_sums(const Vec (&c)[8], const Vec (&tc)[8], Vec (&sum)[4]) {
	const auto x = [&c](int i, int j) { return product(c[i], c[j]); };
	const auto t = [&c, &tc](int i, int j) { return product(c[i], tc[j]); };
	sum[0] = minus(minus(plus(plus(x(0, 0), twice(t(2, 6))), plus(t(4, 4), offset())), twice(t(1, 7))), twice(t(3, 5)));
	sum[1] = minus(minus(plus(plus(twice(x(0, 2)), twice(t(4, 6))), offset()), x(1, 1)), plus(twice(t(3, 7)), t(5, 5)));
	sum[2] = minus(minus(plus(plus(twice(x(0, 4)), x(2, 2)), plus(t(6, 6), offset())), twice(x(1, 3))), twice(t(5, 7)));
	sum[3] = minus(minus(plus(plus(twice(x(0, 6)), twice(x(2, 4))), offset()), twice(x(1, 5))), plus(x(3, 3), t(7, 7)));
}

[[gnu::always_inline]] inline void graeffe_leaf(const Vec (&c)[8], const Vec (&tc)[8], Vec (&out)[4]) {
	Vec even[4], odd[4], c2[8], tc2[8];
	graeffe_sums(c, tc, even);
	odd_lanes(c, c2), odd_lanes(tc, tc2);
	graeffe_sums(c2, tc2, odd);
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) out[k] = low(reduce_sums(even[k], odd[k]));
}

[[gnu::always_inline]] inline void load_leaves(const Word* from, Vec (&c)[8]) {
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) c[i] = load(from + 16 * i);
	transpose(c);
}

template <bool kNegative>
[[gnu::always_inline]] inline void scaled_leaves(const Vec (&c)[8], const Factors& s, int first, Vec (&tc)[8]) {
#pragma GCC unroll 8
	for (int j = 0; j < 8; ++j) {
		const Vec x = reduce(times(c[j], s), kP);
		tc[j] = j < first ? _mm256_setzero_si256() : kNegative ? negate(x) : x;
	}
}

inline void combine_pairs(const Vec (&x)[4], const Vec (&y)[4], const Factors& inverse, Word* out) {
	Vec r[8];
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i) r[i] = low(add(x[i], y[i])), r[i + 4] = times(diff(x[i], y[i]), inverse);
	transpose(r);
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) store(out + 8 * i, r[i]);
}

struct Tables {
	const Word *roots, *inverse_roots;

	static size_t words(size_t m) { return 2 * Arena::footprint(blocked_ntt::detail::table_words(std::countr_zero(m) + 2)); }

	Tables(Words memory, size_t m, const Transform* shared = nullptr) {
		if (shared && shared->lg_max() >= std::countr_zero(m) + 2) {
			roots = shared->roots(), inverse_roots = shared->inverse_roots();
			return;
		}
		auto* const forward = memory.data();
		auto* const inverse = forward + Arena::footprint(blocked_ntt::detail::table_words(std::countr_zero(m) + 2));
		roots = forward, inverse_roots = inverse;
		std::fill(memory.begin(), memory.end(), 0u);
		blocked_ntt::detail::build_table(forward, m / 4, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse, m / 4, blocked_ntt::detail::kRoots[1]);
	}
};

struct PairWeights {
	Factors s, inverse;
	PairWeights(const Tables& tables, size_t k) : s(entries(tables.roots, k)), inverse(entries(tables.inverse_roots, k)) {}
};

template <int kParity>
[[gnu::always_inline]] inline void product_sums(const Vec (&a)[8], const Vec (&c)[8], const Vec (&tc)[8],
												Vec (&sum)[4]) {
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) {
		Vec s = offset();
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) {
			const int j = 2 * k + kParity - i, l = j + 8;
			if (j >= 0) s = j % 2 ? minus(s, product(a[i], c[j])) : plus(s, product(a[i], c[j]));
			if (l < 8) s = l % 2 ? minus(s, product(a[i], tc[l])) : plus(s, product(a[i], tc[l]));
		}
		sum[k] = s;
	}
}

template <int kParity>
[[gnu::always_inline]] inline void product_leaf(const Vec (&a)[8], const Vec (&c)[8], const Vec (&tc)[8],
												Vec (&out)[4]) {
	Vec even[4], odd[4], a2[8], c2[8], tc2[8];
	product_sums<kParity>(a, c, tc, even);
	odd_lanes(a, a2), odd_lanes(c, c2), odd_lanes(tc, tc2);
	product_sums<kParity>(a2, c2, tc2, odd);
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) out[k] = low(reduce_sums(even[k], odd[k]));
}

template <bool kNumerator>
struct FirstLevelProducts {
	static constexpr int kOutputs = kNumerator ? 5 : 3;
	const Tables* tables;
	const Word *q1, *q2, *p1;
	Word* out[kOutputs];

	[[gnu::flatten]] void pairs(size_t k) const {
		const PairWeights weights(*tables, k);
		Vec a[kOutputs][4], b[kOutputs][4];
		leaves<false>(16 * k, weights.s, a);
		leaves<true>(16 * k + 8, weights.s, b);
#pragma GCC unroll 5
		for (int i = 0; i < kOutputs; ++i) combine_pairs(a[i], b[i], weights.inverse, out[i] + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] void leaves(size_t at, const Factors& s, Vec (&r)[kOutputs][4]) const {
		Vec c1[8], c2[8], a[8], t1[8], t2[8];
		load_leaves(q1 + at, c1), load_leaves(q2 + at, c2);
		if constexpr (kNumerator) load_leaves(p1 + at, a);
		scaled_leaves<kNegative>(c1, s, kNumerator ? 2 : 4, t1);
		scaled_leaves<kNegative>(c2, s, 1, t2);
		graeffe_leaf(c1, t1, r[0]);
		graeffe_leaf(c2, t2, r[1]);
		product_leaf<0>(c1, c2, t2, r[2]);
		if constexpr (kNumerator) {
			product_leaf<1>(a, c1, t1, r[3]);
			product_leaf<1>(a, c2, t2, r[4]);
		}
	}
};

inline Word graeffe_scale(int lg) {
	return blocked_ntt::detail::multiply_mod(kInverseScales[0][lg], Word((std::uint64_t(1) << 31) % kP));
}

struct LevelBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Tables* tables;
	Word* graeffe;

	void operator()(Word* a, size_t count, size_t first) const {
		ForwardBottom{tables->roots}(a, count, first);
		if (graeffe)
			for (size_t i = 0; i < count; i += 4) pairs(a + 32 * i, 2 * (first + i));
	}

	[[gnu::flatten]] void pairs(const Word* q, size_t k) const {
		const PairWeights weights(*tables, k);
		Vec va[4], vb[4];
		leaves<false>(q, weights.s, va);
		leaves<true>(q + 8, weights.s, vb);
		combine_pairs(va, vb, weights.inverse, graeffe + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] static void leaves(const Word* q, const Factors& s, Vec (&v)[4]) {
		Vec c[8], tc[8];
		load_leaves(q, c);
		scaled_leaves<kNegative>(c, s, 4, tc);
		graeffe_leaf(c, tc, v);
	}
};

template <class Bottom>
void forward_with(Words a, size_t size, const Tables& tables, const Bottom& bottom) {
	const Recursion recursion(tables.roots, tables.inverse_roots, bottom);
	const Source in(a.data(), size, 0);
	const size_t nv = a.size() / 8;
	auto* v = reinterpret_cast<Vec*>(a.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		forward_top4(in, v, h, tables.roots);
		for (size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
	} else {
		const size_t h = nv / 2;
		forward_top2(in, v, h);
		recursion.visit(a.data(), h, 0);
		recursion.visit(a.data() + 8 * h, h, 1);
	}
}

template <class Bottom>
void inverse_with(Words a, const Tables& tables, const Bottom& bottom, Word scale, Half output) {
	const Recursion recursion(tables.roots, tables.inverse_roots, bottom);
	const size_t nv = a.size() / 8;
	auto* v = reinterpret_cast<Vec*>(a.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		for (size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
		inverse_top4(v, h, output, tables.inverse_roots, Factor(scale));
	} else {
		const size_t h = nv / 2;
		recursion.visit(a.data(), h, 0);
		recursion.visit(a.data() + 8 * h, h, 1);
		inverse_top2(v, h, output, scale);
	}
}

inline void forward_columns(Word* a, size_t h, size_t b, const Word* roots, size_t k) {
	Vec* const v = reinterpret_cast<Vec*>(a);
	if (b == 0) return kernels::forward_even_columns(v, h, roots + slot(k), roots + slot(2 * k));
	kernels::forward_columns(v, h, size_t(1) << b, roots + slot(k), roots + slot(2 * k));
}

inline void inverse_columns(Word* a, size_t h, size_t b, const Word* inverse_roots,
							size_t k) {
	Vec* const v = reinterpret_cast<Vec*>(a);
	if (b == 0) return kernels::inverse_even_columns(v, h, inverse_roots + slot(k), inverse_roots + slot(2 * k));
	kernels::inverse_columns(v, h, size_t(1) << b, inverse_roots + slot(k), inverse_roots + slot(2 * k));
}

inline void forward_half(Word* a, size_t h, bool high, const Group& w) {
	for (Word* x = a; x < a + 8 * h; x += 8) {
		const Vec u = low(load(x));
		if (high) {
			const Vec b = load(x + 8 * h), yb = times(b, w.y), zb = times(b, w.z);
			store(x, add(u, yb)), store(x + 8 * h, diff(u, yb));
			store(x + 16 * h, add(u, zb)), store(x + 24 * h, diff(u, zb));
		} else {
			const Vec xc = times(load(x + 16 * h), w.x), sum = low(add(u, xc)), difference = low_difference(u, xc);
			store(x, sum), store(x + 8 * h, sum), store(x + 16 * h, difference), store(x + 24 * h, difference);
		}
	}
}

inline void inverse_half(Word* a, size_t h, bool high, const Group& w) {
	for (Word* x = a; x < a + 8 * h; x += 8) {
		const Vec f0 = load(x), f1 = load(x + 8 * h), f2 = load(x + 16 * h), f3 = load(x + 24 * h);
		const Vec ab = low(add(f0, f1)), cd = low(add(f2, f3));
		store(x, low(add(ab, cd)));
		if (high) store(x + 8 * h, low(add(times(diff(f0, f1), w.y), times(diff(f2, f3), w.z))));
		else store(x + 16 * h, times(diff(ab, cd), w.x));
	}
}

template <class Bottom>
class Pruned {
public:
	Pruned(const Tables& tables, const Bottom& bottom, size_t b)
		: recursion_(tables.roots, tables.inverse_roots, bottom), tables_(tables), b_(b) {}

	void forward(Word* a, size_t nv, size_t k) const {
		const size_t h = nv / 4, low = size_t(std::countr_zero(h));
		if (nv <= kForwardFull || low + 1 < b_) return recursion_.visit(a, nv, k);
		if (low > b_) {
			forward_columns(a, h, b_, tables_.roots, k);
			for (size_t t = 0; t < 4; ++t) forward(a + 8 * t * h, h, 4 * k + t);
			return;
		}
		forward_half(a, h, low + 1 == b_, Group(tables_.roots, k));
		for (size_t t = 0; t < 4; ++t) recursion_.visit(a + 8 * t * h, h, 4 * k + t);
	}

	void inverse(Word* a, size_t nv, size_t k) const {
		const size_t h = nv / 4, low = size_t(std::countr_zero(h));
		if (nv <= kInverseFull || low + 1 < b_) return recursion_.visit(a, nv, k);
		if (low > b_) {
			for (size_t t = 0; t < 4; ++t) inverse(a + 8 * t * h, h, 4 * k + t);
			return inverse_columns(a, h, b_, tables_.inverse_roots, k);
		}
		for (size_t t = 0; t < 4; ++t) recursion_.visit(a + 8 * t * h, h, 4 * k + t);
		inverse_half(a, h, low + 1 == b_, Group(tables_.inverse_roots, k));
	}

private:
	static constexpr size_t kForwardFull = 64, kInverseFull = 16;

	Recursion<Bottom> recursion_;
	const Tables& tables_;
	size_t b_;
};

template <class Bottom>
void forward_pruned(Words a, size_t size, size_t b, const Tables& tables,
					const Bottom& bottom) {
	const Pruned<Bottom> pruned(tables, bottom, b);
	const Source in(a.data(), size, 0);
	const size_t nv = a.size() / 8, width = size_t(1) << b;
	auto* v = reinterpret_cast<Vec*>(a.data());
	const Vec p = broadcast(kP), zero = _mm256_setzero_si256();
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		const Factor z(tables.roots[1], tables.roots[9]);
		for (size_t c = 0; c < h; c += 2 * width) {
			for (size_t j = c; j < c + width; ++j) {
				const Vec x0 = in(j), x1 = in(j + h), x2 = in(j + 2 * h), x3 = in(j + 3 * h);
				const Vec ac = add(x0, x2), amc = _mm256_sub_epi32(add(x0, p), x2);
				const Vec bd = add(x1, x3), zbmd = times(_mm256_sub_epi32(add(x1, p), x3), z);
				v[j] = add(ac, bd), v[j + h] = diff(ac, bd);
				v[j + 2 * h] = add(amc, zbmd), v[j + 3 * h] = diff(amc, zbmd);
			}
			for (size_t j = c + width; j < c + 2 * width; ++j)
				v[j] = v[j + h] = v[j + 2 * h] = v[j + 3 * h] = zero;
		}
		for (size_t t = 0; t < 4; ++t) pruned.forward(a.data() + 8 * t * h, h, t);
	} else {
		const size_t h = nv / 2;
		for (size_t c = 0; c < h; c += 2 * width) {
			for (size_t j = c; j < c + width; ++j) {
				const Vec x0 = in(j), x1 = in(j + h);
				v[j] = add(x0, x1), v[j + h] = _mm256_sub_epi32(add(x0, p), x1);
			}
			for (size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = zero;
		}
		pruned.forward(a.data(), h, 0);
		pruned.forward(a.data() + 8 * h, h, 1);
	}
}

template <class Bottom>
void inverse_pruned(Words a, size_t b, const Tables& tables, const Bottom& bottom,
					Word scale, Half output = Half::kBoth) {
	const Pruned<Bottom> pruned(tables, bottom, b);
	const size_t nv = a.size() / 8, width = size_t(1) << b;
	auto* v = reinterpret_cast<Vec*>(a.data());
	const Factor s(scale);
	const auto scaled = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	const bool lower = output != Half::kUpper, upper = output != Half::kLower;
	if (std::countr_zero(nv) % 2 == 0) {
		const size_t h = nv / 4;
		for (size_t t = 0; t < 4; ++t) pruned.inverse(a.data() + 8 * t * h, h, t);
		const Factor z(tables.inverse_roots[1], tables.inverse_roots[9]);
		for (size_t c = 0; c < h; c += 2 * width)
			for (size_t j = c; j < c + width; ++j) {
				const Vec x0 = v[j], x1 = v[j + h], x2 = v[j + 2 * h], x3 = v[j + 3 * h];
				const Vec ab = low(add(x0, x1)), cd = low(add(x2, x3));
				const Vec amb = low(diff(x0, x1)), cmd = times(diff(x2, x3), z);
				if (lower) v[j] = scaled(add(ab, cd)), v[j + h] = scaled(add(amb, cmd));
				if (upper) v[j + 2 * h] = scaled(diff(ab, cd)), v[j + 3 * h] = scaled(diff(amb, cmd));
			}
	} else {
		const size_t h = nv / 2;
		pruned.inverse(a.data(), h, 0);
		pruned.inverse(a.data() + 8 * h, h, 1);
		for (size_t c = 0; c < h; c += 2 * width)
			for (size_t j = c; j < c + width; ++j) {
				const Vec x0 = v[j], x1 = v[j + h];
				if (lower) v[j] = scaled(add(x0, x1));
				if (upper) v[j + h] = scaled(diff(x0, x1));
			}
	}
}

class Carve {
public:
	explicit Carve(Words scratch) : rest_(scratch) {}

	static constexpr size_t words(size_t n) { return Arena::footprint(n) + 8; }

	Words take(size_t n) {
		const size_t skip = (0 - reinterpret_cast<std::uintptr_t>(rest_.data()) / 4) % 16;
		if (skip + Arena::footprint(n) > rest_.size()) std::abort();
		const Words s = rest_.subspan(skip, n);
		rest_ = rest_.subspan(skip + Arena::footprint(n));
		return s;
	}

private:
	Words rest_;
};

inline void next_level(Words a, size_t stride, size_t rows) {
	const size_t half = stride / 2, top = rows * stride;
	std::copy(a.begin(), a.begin() + std::ptrdiff_t(half), a.begin() + std::ptrdiff_t(top));
	a[top] = a[top] ? a[top] - 1 : kP - 1;
	a[0] = 1;
	std::fill(a.begin() + 1, a.begin() + std::ptrdiff_t(half), 0);
}

template <size_t K>
void inverse_product_sum(const Tables& tables, const Transform::Pair* pairs, Words out, Half output) {
	InverseProductSumBottom<K> bottom{};
	for (size_t k = 0; k < K; ++k) bottom.terms[k] = {{tables.roots, tables.inverse_roots, pairs[k].b.data()}, pairs[k].a.data()};
	inverse_with(out, tables, bottom, kInverseScales[1][std::countr_zero(out.size())], output);
}

inline void multiply_constant(ConstWords a, Word c, Words t) {
	const Factor f(c);
	for (size_t i = 0; i < a.size(); i += 8) store(t.data() + i, reduce(times(load(a.data() + i), f), kP));
}

}

inline unsigned coefficient_value(const mint &x) {
	const unsigned value = x.v;
	const unsigned reduced = (static_cast<unsigned long long>(unsigned(value * 998244351u)) * 998244353u + value) >> 32;
	return std::min(reduced, reduced - 998244353u);
}
inline void append_word(std::vector<mint> &out, unsigned value) {
	out.emplace_back();
	out.back().v = value;
}

}
#pragma GCC pop_options
#endif
#endif
