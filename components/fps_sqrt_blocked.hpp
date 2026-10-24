#ifndef CP_COMPONENTS_FPS_SQRT_BLOCKED_HPP
#define CP_COMPONENTS_FPS_SQRT_BLOCKED_HPP
#include "fps_inverse_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::blocked_poly {

namespace detail {

inline constexpr size_t kSqrtBase = 64;

inline void sqrt_direct(ConstWords f, Word c, Words g) {
	using blocked_ntt::detail::multiply_mod;
	const Word inverse = blocked_ntt::detail::power(multiply_mod(2, c), kP - 2);
	g[0] = c;
	for (size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = i < f.size() ? f[i] : 0;
		for (size_t j = 1; j < i; ++j) sum += kP - std::uint64_t(g[j]) * g[i - j] % kP;
		g[i] = multiply_mod(Word(sum % kP), inverse);
	}
}

inline Vec load_padded(ConstWords f, size_t at) {
	if (at + 8 <= f.size()) return load_unaligned(f.data() + at);
	alignas(32) Word x[8] = {};
	if (at < f.size()) std::copy(f.begin() + std::ptrdiff_t(at), f.end(), x);
	return load(x);
}

inline Vec first_lanes(size_t count) {
	return _mm256_cmpgt_epi32(broadcast(Word(std::min<size_t>(count, 8))),
							_mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
}
inline void store_first(Word* p, Vec x, size_t count) {
	if (count >= 8) return store_unaligned(p, x);
	_mm256_maskstore_epi32(reinterpret_cast<int*>(p), first_lanes(count), x);
}
inline Vec load_first(const Word* p, size_t count) {
	if (count >= 8) return load_unaligned(p);
	return _mm256_maskload_epi32(reinterpret_cast<const int*>(p), first_lanes(count));
}

[[gnu::always_inline]] inline Vec leaf_square(const Window& window) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2;
	asm(R"(
vpbroadcastd 32(%[w]), %[bi]
	vpmuludq 36(%[w]), %[bi], %[odd]
	vpmuludq 32(%[w]), %[bi], %[even]
	vpbroadcastd 36(%[w]), %[bi]
	vpmuludq 28(%[w]), %[bi], %[t0]
	vpbroadcastd 40(%[w]), %[bi]
	vpmuludq 28(%[w]), %[bi], %[t1]
	vpmuludq 24(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 44(%[w]), %[bi]
	vpmuludq 20(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpbroadcastd 48(%[w]), %[bi]
	vpmuludq 20(%[w]), %[bi], %[t1]
	vpmuludq 16(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 52(%[w]), %[bi]
	vpmuludq 12(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpbroadcastd 56(%[w]), %[bi]
	vpmuludq 12(%[w]), %[bi], %[t1]
	vpmuludq 8(%[w]), %[bi], %[t2]
	vpaddq %[t0], %[even], %[even]
	vpbroadcastd 60(%[w]), %[bi]
	vpmuludq 4(%[w]), %[bi], %[t0]
	vpaddq %[t1], %[odd], %[odd]
	vpaddq %[t2], %[even], %[even]
	vpaddq %[t0], %[even], %[even]
	vpaddq %[odd], %[odd], %[odd]
	vpmuludq %[ni], %[even], %[t0]
	vpmuludq %[ni], %[odd], %[t1]
	vpmuludq %[p], %[t0], %[t0]
	vpmuludq %[p], %[t1], %[t1]
	vpaddq %[t0], %[even], %[even]
	vpaddq %[t1], %[odd], %[odd]
	vpsrlq $32, %[even], %[even]
	vpblendd $0xaa, %[odd], %[even], %[even]
	vpsubd %[p2], %[even], %[t0]
	vpminud %[t0], %[even], %[even]
)"
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2)
		: [w] "r"(window.word), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const Word(*)[17]>(window.word)));
	return even;
}

struct InverseSquareBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Word* roots;
	const Word* inverse_roots;
	const Word* a;

	[[gnu::always_inline]] void prepare(size_t g, Window (&window)[4]) const {
		const Word* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, roots, g);
	}

	void operator()(Word* out, size_t count, size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			Vec f[4];
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) f[t] = leaf_square(window[j & 1][t]);
			inverse_h1(f, Group(inverse_roots, first + j));
			for (int t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline bool radix4(size_t nv) { return std::countr_zero(nv) % 2 == 0; }

template <class Bottom>
void subtrees(const Recursion<Bottom>& recursion, Word* a, size_t nv) {
	if (!radix4(nv)) {
		recursion.visit(a, nv / 2, 0);
		return recursion.visit(a + 4 * nv, nv / 2, 1);
	}
	for (size_t q = 0; q < 4; ++q) recursion.visit(a + 2 * q * nv, nv / 4, q);
}

template <class Bottom>
void from_lower_source(const Transform& t, const Recursion<Bottom>& recursion, Word* a, size_t nv,
					bool written = true) {
	const Source in(a, 4 * nv, 0);
	if (!radix4(nv)) return radix2_halves(recursion, in, a, nv, t.roots());
	if (!written) forward_top4(in, reinterpret_cast<Vec*>(a), nv / 4, t.roots());
	subtrees(recursion, a, nv);
}

inline void inverse_top(const Transform& t, Vec* v, size_t nv, Half output, Word scale) {
	if (radix4(nv)) return inverse_top4(v, nv / 4, output, t.inverse_roots(), Factor(scale));
	inverse_top2(v, nv / 2, output, scale);
}

template <Half kOutput, class Visit>
[[gnu::always_inline]] inline void inverse_columns(const Transform& t, Vec* v, size_t nv, Word scale,
												Visit visit) {
	constexpr bool kLower = kOutput != Half::kUpper, kUpper = kOutput != Half::kLower;
	const Factor s(scale);
	if (radix4(nv)) {
		const size_t h = nv / 4;
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (size_t j = 0; j < h; ++j) {
			const Vec p0 = v[j], p1 = v[j + h], p2 = v[j + 2 * h], p3 = v[j + 3 * h];
			const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
			const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
			Vec lo[2] = {}, hi[2] = {};
			if constexpr (kLower) lo[0] = times(add(ab, cd), s), lo[1] = times(add(amb, cmd), s);
			if constexpr (kUpper) hi[0] = times(diff(ab, cd), s), hi[1] = times(diff(amb, cmd), s);
			visit(std::true_type{}, j, h, lo, hi);
		}
		return;
	}
	const size_t h = nv / 2;
	for (size_t j = 0; j < h; ++j) {
		Vec lo[1] = {}, hi[1] = {};
		if constexpr (kLower) lo[0] = times(add(v[j], v[j + h]), s);
		if constexpr (kUpper) hi[0] = times(diff(v[j], v[j + h]), s);
		visit(std::false_type{}, j, h, lo, hi);
	}
}

[[gnu::always_inline]] inline void forward_lower4(Vec* v, size_t j, size_t h, Vec a, Vec b, const Factor& z) {
	const Vec zb = times(b, z);
	v[j] = add(a, b), v[j + h] = diff(a, b), v[j + 2 * h] = add(a, zb), v[j + 3 * h] = diff(a, zb);
}

[[gnu::always_inline]] inline void forward_upper4(Vec* v, size_t j, size_t h, Vec c, Vec d, const Factor& z) {
	const Vec mc = _mm256_sub_epi32(broadcast(kP), c), zmd = times(_mm256_sub_epi32(broadcast(2 * kP), d), z);
	v[j] = add(c, d), v[j + h] = diff(c, d), v[j + 2 * h] = add(mc, zmd), v[j + 3 * h] = diff(mc, zmd);
}

inline Factor top_root(const Transform& t) { return Factor(t.roots()[1], t.roots()[9]); }

inline Vec* vectors(Words a) { return reinterpret_cast<Vec*>(a.data()); }

inline Word product_scale(size_t n) { return kInverseScales[1][std::countr_zero(n)]; }

inline Vec half_residual(ConstWords f, size_t m, size_t at, Vec s) {
	const Factor half((kP + 1) / 2);
	const Vec fs = add(load_padded(f, at), load_padded(f, m + at));
	return reduce(times(_mm256_sub_epi32(add(s, broadcast(2 * kP)), fs), half), kP);
}

inline void half_residual(ConstWords f, size_t m, size_t first, const Word* s,
						Word* out, size_t count) {
	for (size_t i = 0; i < count; i += 8) store_first(out + i, half_residual(f, m, first + i, load(s + i)), count - i);
}

inline void sqrt_step(const Transform& t, ConstWords f, size_t m, Word* g,
					Word* h, Words b, Words ht) {
	const size_t half = m / 2, nv = m / 8;
	const Words gt = b.first(m), w = b.subspan(m, m), work = b.first(2 * m);
	const Factor z = top_root(t);
	const ProductBottom with_h{t.roots(), t.inverse_roots(), ht.data()};
	const Word scale = product_scale(m), scale2 = product_scale(2 * m);
	t.forward(ConstWords(g, m), 0, gt);
	Vec* v = vectors(w);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseProductBottom{with_h, gt.data()}), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* e) {
		if constexpr (radix4) forward_upper4(v, j, q, reduce(e[0], kP), e[1], z);
		else v[j] = e[0], v[j + q] = _mm256_sub_epi32(broadcast(2 * kP), e[0]);
	});
	subtrees(Recursion(t.roots(), t.inverse_roots(), with_h), w.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* x) {
		store_unaligned(h + half + 8 * j, reduce(x[0], kP));
		if constexpr (radix4) store_unaligned(h + half + 8 * (j + q), reduce(x[1], kP));
	});
	t.forward(ConstWords(h, m), 0, ht);

	v = vectors(gt);
	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}), gt.data(),
			nv);
	inverse_columns<Half::kBoth>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* lo, const Vec* hi) {
		if constexpr (radix4) {
			v[j] = half_residual(f, m, 8 * j, lo[0]), v[j + q] = half_residual(f, m, 8 * (j + q), lo[1]);
			v[j + 2 * q] = half_residual(f, m, half + 8 * j, hi[0]);
			v[j + 3 * q] = half_residual(f, m, half + 8 * (j + q), hi[1]);
		} else {
			forward_lower4(v, j, q, half_residual(f, m, 8 * j, lo[0]), half_residual(f, m, half + 8 * j, hi[0]), z);
		}
	});
	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_h), work.data(), 2 * nv);
	inverse_columns<Half::kLower>(t, vectors(work), 2 * nv, scale2,
								[&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
									store_unaligned(g + m + 8 * j, reduce(d[0], kP));
									if constexpr (radix4) store_unaligned(g + m + 8 * (j + q), reduce(d[1], kP));
								});
}

}

inline int sqrt_log(size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<size_t>(n, 2) - 1)) - 1);
}

inline size_t sqrt_scratch(size_t n) {
	if (n <= detail::kSqrtBase) return 0;
	return 3 * Arena::footprint(size_t(1) << sqrt_log(n));
}

inline void sqrt_steps(const Transform& t, ConstWords f, Word c, Words a,
					Words b, Words ht) {
	using namespace detail;
	const size_t len = a.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0] || len < kSqrtBase) std::abort();
	sqrt_direct(f, c, a.first(kSqrtBase));
	Word* const h = a.data() + len / 2;
	alignas(32) Word h0[kSqrtBase / 2];
	inverse_direct(a.first(kSqrtBase / 2), h0);
	for (Word& x : h0) x = x ? kP - x : 0;
	t.forward(h0, 0, ht.first(kSqrtBase));
	if (len > kSqrtBase) std::copy_n(h0, kSqrtBase / 2, h);
	for (size_t m = kSqrtBase; m < len; m *= 2) sqrt_step(t, f, m, a.data(), h, b.first(2 * m), ht.first(2 * m));
}

inline void sqrt_last_step(const Transform& t, ConstWords f, ConstWords gt,
						ConstWords ht, Words work, Words hi) {
	using namespace detail;
	const size_t m = gt.size(), half = m / 2, rest = hi.size(), nv = m / 8;
	if (rest == 0 || rest > m) std::abort();
	const Word scale = product_scale(m);
	const Factor z = top_root(t);
	const ProductBottom with_g{t.roots(), t.inverse_roots(), gt.data()}, with_h{t.roots(), t.inverse_roots(), ht.data()};
	const Recursion product_h(t.roots(), t.inverse_roots(), with_h);
	Vec* v = vectors(work);

	const auto source = [&](auto radix4, size_t j, size_t q, auto x) {
		if constexpr (radix4) forward_lower4(v, j, q, x(0, j), x(1, j + q), z);
		else v[j] = x(0, j);
	};

	const auto each = [](auto radix4, size_t j, size_t q, auto put) {
		put(0, j);
		if constexpr (radix4) put(1, j + q);
	};
	const auto put = [&](Word* out, size_t count, size_t p, Vec x) __attribute__((target("avx2"))) {
		if (8 * p < count) store_first(out + 8 * p, x, count - 8 * p);
	};

	subtrees(Recursion(t.roots(), t.inverse_roots(), InverseSquareBottom{t.roots(), t.inverse_roots(), gt.data()}),
			work.data(), nv);
	const bool split = rest > half;
	inverse_top(t, v, nv, split ? Half::kBoth : Half::kLower, scale);
	half_residual(f, m, 0, work.data(), work.data(), std::min(rest, half));
	const size_t count = split ? rest - half : 0;
	Word* const upper = hi.data() + (rest - count);
	half_residual(f, m, half, work.data() + half, upper, count);
	from_lower_source(t, product_h, work.data(), nv, false);
	if (!split) {
		inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
			each(radix4, j, q, [&](int k, size_t p) { put(hi.data(), rest, p, reduce(d[k], kP)); });
		});
		return;
	}

	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
		source(radix4, j, q, [&](int k, size_t p) {
			const Vec d0 = reduce(d[k], kP);
			store_unaligned(hi.data() + 8 * p, d0);
			return d0;
		});
	});

	from_lower_source(t, Recursion(t.roots(), t.inverse_roots(), with_g), work.data(), nv);
	inverse_columns<Half::kUpper>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec*, const Vec* x) {
		source(radix4, j, q, [&](int k, size_t p) {
			const size_t at = 8 * p;
			return canonical(add(x[k], load_first(upper + std::min(at, count), at < count ? count - at : 0)));
		});
	});
	from_lower_source(t, product_h, work.data(), nv);
	inverse_columns<Half::kLower>(t, v, nv, scale, [&](auto radix4, size_t j, size_t q, const Vec* d, const Vec*) {
		each(radix4, j, q, [&](int k, size_t p) { put(upper, count, p, reduce(d[k], kP)); });
	});
}

inline void sqrt(const Transform& t, ConstWords f, Word c, Words g,
				Words scratch) {
	using namespace detail;
	const size_t n = g.size();
	if (f.empty() || f[0] == 0 || blocked_ntt::detail::multiply_mod(c, c) != f[0]) std::abort();
	if (n <= kSqrtBase) return sqrt_direct(f, c, g);
	const size_t m = size_t(1) << sqrt_log(n), words = Arena::footprint(m);
	const Words b = scratch.first(m), ht = scratch.subspan(words, m), work = scratch.subspan(2 * words, m);
	sqrt_steps(t, f, c, g.first(m), b, ht);
	t.forward(g.first(m), 0, b);
	sqrt_last_step(t, f, b, ht, work, g.subspan(m));
}

}
namespace fps_detail::blocked_poly {
inline void native_sqrt(const mint *in, size_t input, std::vector<mint> &out, size_t n, mint constant) {
	const int lg = sqrt_log(n);
	const size_t c = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(c) + Arena::footprint(n) + Arena::footprint(sqrt_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(c), g = arena.take(n), scratch = arena.take(sqrt_scratch(n));
	copy_coefficients(in, f);
	const Word ct = blocked_ntt::detail::multiply_mod(std::min(constant.v, constant.v - mint::mod()),
		blocked_ntt::detail::power(65536, mint::mod() - 2));
	sqrt(t, f, ct, g, scratch);
	append_scaled(g, out, 65536);
}
}

#pragma GCC pop_options
#endif
#endif
