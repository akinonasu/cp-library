#ifndef CP_COMPONENTS_FPS_EXP_BLOCKED_HPP
#define CP_COMPONENTS_FPS_EXP_BLOCKED_HPP

#include "fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")
namespace fps_detail {
namespace blocked_poly {
namespace detail {
struct Factors {
	Vec w, q;
};

inline Vec times(Vec x, const Factors& w) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, w.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(w.q, 32));
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline Factors entries(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return {load(e), load(e + 8)};
}


}
}
namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kExpBase = 64;

inline void exp_direct(std::span<const std::uint32_t> q, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	std::uint32_t inv[kExpBase] = {0, 1};
	for (std::size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / std::uint32_t(i), inv[kP % i]);
	g[0] = 1;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (std::size_t k = 1; k <= i; ++k) sum += std::uint64_t(q[k]) * g[i - k] % kP;
		g[i] = multiply_mod(std::uint32_t(sum % kP), inv[i]);
	}
}

inline void multiply_by_index(std::span<const std::uint32_t> a, std::size_t first, std::span<std::uint32_t> out) {
	const std::size_t n = out.size(), full = n / 8 * 8;
	Indices index(first);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(out.data() + i, canonical(montgomery(load_unaligned(a.data() + i), index.value())));
	for (std::size_t i = full; i < n; ++i) out[i] = blocked_ntt::detail::multiply_mod(std::uint32_t((first + i) % kP), a[i]);
}

inline std::size_t exp_length(std::size_t n) {
	return std::size_t(1) << std::max(Transform::kMinLog + 1, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)));
}

}

inline int exp_log(std::size_t n) { return std::countr_zero(detail::exp_length(n)) - 1; }

namespace detail {

struct QTransforms {
	std::span<const std::uint32_t> low, block2;
};
struct NoQTransforms {};

template <class Bottom>
void product_to(const Transform& t, std::span<std::uint32_t> out, const Source& in, const Bottom& bottom,
				std::uint32_t scale, Half output, std::span<std::uint32_t> to) {
	const std::size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	const Recursion recursion(t.roots(), t.inverse_roots(), bottom);
	const Factor s(scale);
	const auto put = [to, &s](std::size_t j, Vec x) __attribute__((target("avx2"))) {
		const Vec y = reduce(times(x, s), kP);
		if (8 * j + 8 <= to.size()) return store_unaligned(to.data() + 8 * j, y);
		if (8 * j >= to.size()) return;
		const Vec lanes = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7);
		const Vec mask = _mm256_cmpgt_epi32(broadcast(std::uint32_t(to.size() - 8 * j)), lanes);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(to.data() + 8 * j), mask, y);
	};
	const bool lower = output == Half::kLower;
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward_top4(in, v, h, t.roots());
		for (std::size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
		const Factor z(t.inverse_roots()[1], t.inverse_roots()[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec ab = low(add(v[j], v[j + h])), cd = low(add(v[j + 2 * h], v[j + 3 * h]));
			const Vec amb = low(diff(v[j], v[j + h])), cmd = times(diff(v[j + 2 * h], v[j + 3 * h]), z);
			put(j, lower ? add(ab, cd) : diff(ab, cd));
			put(j + h, lower ? add(amb, cmd) : diff(amb, cmd));
		}
	} else {
		const std::size_t h = nv / 2;
		radix2_halves(recursion, in, out.data(), nv, t.roots());
		for (std::size_t j = 0; j < h; ++j) put(j, lower ? add(v[j], v[j + h]) : diff(v[j], v[j + h]));
	}
}

template <class Given>
[[gnu::always_inline]] inline void exp_last_step(const Transform& t, std::span<std::uint32_t> lo, std::span<std::uint32_t> g,
												 std::size_t m, bool first, std::span<std::uint32_t> gt,
												 std::span<const std::uint32_t> ht, std::span<std::uint32_t> w,
												 std::span<std::uint32_t> work, const Given& given) {
	const std::size_t n = g.size(), half = m / 2, rest = n - m, count = std::min(rest, half);
	const bool upper = rest > half;
	const std::span<std::uint32_t> g0t = gt.first(m), glt = gt.subspan(m, m), own = w.first(m), r = w.subspan(m, m);
	std::span<const std::uint32_t> qt = own, q2t = lo;
	bool given_low = false, given_block2 = false;
	if constexpr (std::is_same_v<Given, QTransforms>) {
		given_low = !given.low.empty(), given_block2 = !given.block2.empty();
		if (given_low) qt = given.low;
		if (given_block2) q2t = given.block2;
	}
	const auto q = [g, n](std::size_t i) __attribute__((target("avx2"))) {
		if (i + 8 <= n) return load_unaligned(g.data() + i);
		alignas(32) std::uint32_t x[8] = {};
		std::copy(g.begin() + std::ptrdiff_t(i), g.end(), x);
		return load(x);
	};
	if (first) t.forward(g.first(half), 0, g0t);
	t.forward(g.first(m), 0, glt);
	if (given_low) {
		t.inverse_product(qt, glt, r, upper ? Half::kBoth : Half::kLower);
	} else if (upper) {
		t.forward(lo, 0, own);
		t.inverse_product(own, glt, r);
	} else {
		t.cyclic_product(lo, 0, r, glt, Half::kLower);
	}

	t.forward(r.first(half), 0, work);
	t.inverse_product(ht, work, work, Half::kLower);
	detail::divide_by_index(m, r.first(count), [&](std::size_t i) __attribute__((target("avx2"))) {
		const Vec x = reduce(add(load(work.data() + i), q(m + i)), kP);
		return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
	});
	if (upper && !given_block2) t.forward(g.subspan(m, half), 0, lo);
	t.forward(r.first(count), 0, work);
	const std::uint32_t scale = kInverseScales[1][std::countr_zero(m)];
	const Source none(nullptr, 0, 0);
	product_to(t, work, none, InverseProductBottom{{t.roots(), t.inverse_roots(), work.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m, count));
	if (!upper) return;

	t.forward(g.subspan(m, half), 0, work);
	const Transform::Pair pairs[] = {{work, qt}, {q2t, glt}};
	t.inverse_product_sum(pairs, work, Half::kUpper);
	Indices index(half);
	for (std::size_t i = half; i < m; i += 8, index.next()) {
		const Vec xg = montgomery(load_unaligned(g.data() + i), index.value());
		const Vec sum = add(load(r.data() + i), load(work.data() + i));
		store(r.data() + i, canonical(_mm256_sub_epi32(add(sum, broadcast(2 * kP)), xg)));
	}
	t.forward(r.subspan(half, half), 0, own);
	t.inverse_product(ht, own, own, Half::kLower);
	detail::divide_by_index(m + half, r.first(rest - half),
							[&](std::size_t i) __attribute__((target("avx2"))) { return add(load(own.data() + i), q(m + half + i)); });
	t.forward(r.first(rest - half), 0, own);
	product_to(t, own, none, InverseProductBottom{{t.roots(), t.inverse_roots(), own.data()}, g0t.data()}, scale,
			   Half::kLower, g.subspan(m + half, rest - half));
}

template <class Given = NoQTransforms>
[[gnu::always_inline]] inline void exp_newton(const Transform& t, std::span<std::uint32_t> lo, std::span<std::uint32_t> g,
											  std::span<std::uint32_t> scratch, const Given& given = {}) {
	const std::size_t n = g.size(), len = 2 * lo.size();
	const auto take = [&scratch](std::size_t words) __attribute__((target("avx2"))) {
		const std::span<std::uint32_t> s = scratch.first(words);
		scratch = scratch.subspan(Arena::footprint(words));
		return s;
	};
	const std::span<std::uint32_t> h = take(len / 2), gt = take(len), ht = take(len / 2), w = take(len);
	std::size_t m = kExpBase;
	inverse_direct(g, h.first(m / 2));
	t.forward(h.first(m / 2), 0, ht.first(m));
	for (; 2 * m < n; m *= 2) {
		const std::size_t half = m / 2;
		const std::span<std::uint32_t> g_low = gt.first(m), h_low = ht.first(m), w_low = w.first(m);
		t.forward(g.first(m), 0, g_low);

		t.inverse_product(g_low, h_low, w_low, Half::kUpper);
		const std::uint32_t negated = blocked_ntt::detail::multiply_mod(kInverseScales[1][std::countr_zero(m)], kP - 1);
		product_to(t, w_low, Source(w_low.data() + half, half, half), ProductBottom{t.roots(), t.inverse_roots(), h_low.data()},
				   negated, Half::kUpper, h.subspan(half, half));

		t.forward_product(lo.first(m), 0, w_low, g_low);
		t.inverse(w_low, gt.subspan(m, m));

		t.forward(h.first(m), 0, ht.first(2 * m));
		t.forward_upper(gt.subspan(m, m), 0, w.subspan(m, m));
		t.inverse_product(w.first(2 * m), ht.first(2 * m), w.first(2 * m), Half::kLower);

		detail::divide_by_index(m, w.subspan(m, m), [&](std::size_t i) __attribute__((target("avx2"))) {
			const Vec x = reduce(add(load(w.data() + i), load(lo.data() + m + i)), kP);
			return add(x, _mm256_sub_epi32(broadcast(kP), load(lo.data() + i)));
		});
		t.forward_upper(g.first(m), 0, gt.subspan(m, m));
		product_to(t, w.first(2 * m), Source(w.data() + m, m, m), ProductBottom{t.roots(), t.inverse_roots(), gt.data()},
				   kInverseScales[1][std::countr_zero(2 * m)], Half::kUpper, g.subspan(m, m));
	}
	exp_last_step(t, lo, g, m, m == kExpBase, gt, ht, w, h, given);
}

inline std::size_t exp_newton_scratch(std::size_t n) {
	const std::size_t len = exp_length(n);
	return 2 * Arena::footprint(len) + 2 * Arena::footprint(len / 2);
}

}

inline std::size_t exp_scratch(std::size_t n) {
	return Arena::footprint(detail::exp_length(n) / 2) + detail::exp_newton_scratch(n);
}

inline void exp(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size(), m = exp_length(n) / 2, size = std::min(f.size(), n), below = std::min(size, m);
	const std::span<std::uint32_t> lo = scratch.first(m);
	multiply_by_index(f.first(below), 0, lo.first(below));
	std::fill(lo.begin() + std::ptrdiff_t(below), lo.end(), 0);
	if (size > m) multiply_by_index(f.subspan(m, size - m), m, g.subspan(m, size - m));
	if (n > std::max(size, m)) std::fill(g.begin() + std::ptrdiff_t(std::max(size, m)), g.end(), 0);
	exp_direct(lo, g.first(std::min(n, kExpBase)));
	if (n > kExpBase) exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
}

}

namespace blocked_poly {

namespace detail {

template <bool kSum>
void times_x(const std::uint32_t* roots, const std::uint32_t* a, const std::uint32_t* b, std::uint32_t e,
			 std::uint32_t alpha, std::uint32_t beta, std::span<std::uint32_t> out) {
	const std::size_t leaves = out.size() / 8;
	const Factor scale(e);
	const Vec rotate = _mm256_setr_epi32(7, 0, 1, 2, 3, 4, 5, 6), odd = _mm256_setr_epi32(0, -1, 0, -1, 0, -1, 0, -1);
	const std::uint32_t constants[2] = {(alpha + beta) % kP, (alpha + kP - beta) % kP};
	for (std::size_t p = 0; p < leaves; p += 8) {
		const bool upper = p >= leaves / 2;

		const Factors r = entries(roots, p / 16 * 8);
		const Vec pick = p % 16 ? _mm256_setr_epi32(4, 4, 5, 5, 6, 6, 7, 7) : _mm256_setr_epi32(0, 0, 1, 1, 2, 2, 3, 3);
		const Vec rw = _mm256_permutevar8x32_epi32(r.w, pick), rq = _mm256_permutevar8x32_epi32(r.q, pick);
		const Factors w{_mm256_blend_epi32(rw, _mm256_sub_epi32(broadcast(kP), rw), 0xAA), _mm256_xor_si256(rq, odd)};
		Vec y[8];
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i) {
			Vec c = load(a + 8 * (p + i));
			if constexpr (kSum) c = upper ? difference(c, load(b + 8 * (p + i))) : reduce(add(c, load(b + 8 * (p + i))), kP);
			y[i] = reduce(times(c, scale), kP);
		}

		const Vec t01 = _mm256_unpackhi_epi32(y[0], y[1]), t23 = _mm256_unpackhi_epi32(y[2], y[3]);
		const Vec t45 = _mm256_unpackhi_epi32(y[4], y[5]), t67 = _mm256_unpackhi_epi32(y[6], y[7]);
		const Vec u03 = _mm256_unpackhi_epi64(t01, t23), u47 = _mm256_unpackhi_epi64(t45, t67);
		const Vec z = canonical(add(times(_mm256_permute2x128_si256(u03, u47, 0x31), w), broadcast(constants[upper])));
		const Vec zh = _mm256_permute2x128_si256(z, z, 0x11);
		const Vec lane0[8] = {z,  _mm256_srli_si256(z, 4),  _mm256_srli_si256(z, 8),  _mm256_srli_si256(z, 12),
							  zh, _mm256_srli_si256(zh, 4), _mm256_srli_si256(zh, 8), _mm256_srli_si256(zh, 12)};
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i)
			store(out.data() + 8 * (p + i), _mm256_blend_epi32(_mm256_permutevar8x32_epi32(y[i], rotate), lane0[i], 1));
	}
}

}

inline int power_log(std::size_t n) { return std::max(detail::log_derivative_log(n), exp_log(n)); }

inline std::size_t power_scratch(std::size_t n) {
	return 3 * Arena::footprint(detail::exp_length(n) / 2) +
		   std::max(detail::log_derivative_scratch(n), detail::exp_newton_scratch(n));
}

inline void power(const Transform& t, std::span<const std::uint32_t> f, std::uint32_t e, std::uint32_t c,
				  std::span<std::uint32_t> g, std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size(), m = exp_length(n) / 2, k = m / 2, stride = Arena::footprint(m);
	if (f.empty() || f[0] == 0) std::abort();
	const std::span<std::uint32_t> lo = scratch.first(m), rest = scratch.subspan(3 * stride);
	const auto q = [&](std::size_t i) __attribute__((target("avx2"))) { return i < m ? lo.data() + i : g.data() + i; };
	const Factor factor(e);

	const bool reuse = m >= 128 && log_block(n) == k && log_blocks(n) >= 3;
	const bool block2 = reuse && log_blocks(n) == 4 && n - m > k;
	const std::span<std::uint32_t> qt = scratch.subspan(stride, reuse ? m : 0);
	const std::span<std::uint32_t> q2t = scratch.subspan(2 * stride, block2 ? m : 0);
	const std::uint32_t* d0t = nullptr;
	lo[0] = 0;
	if (n >= 2)
		log_derivative(
			t, f, n, rest,
			[&](std::size_t first, std::span<const std::uint32_t> d) __attribute__((target("avx2"))) {
				std::size_t i = 0;
				for (; i + 8 <= d.size(); i += 8) {
					const std::size_t at = first + 1 + i;
					const Vec x = reduce(times(load(d.data() + i), factor), kP);
					if (at + 8 <= m || at >= m) {
						store_unaligned(q(at), x);
					} else {
						alignas(32) std::uint32_t words[8];
						store(words, x);
						for (std::size_t j = 0; j < 8; ++j) *q(at + j) = words[j];
					}
				}
				for (; i < d.size(); ++i) *q(first + 1 + i) = blocked_ntt::detail::multiply_mod(d[i], e);
			},
			[&](std::size_t j, std::span<const std::uint32_t> dt) __attribute__((target("avx2"))) {
				if (j == 0) d0t = dt.data();
				if (j == 1 && reuse) times_x<true>(t.roots(), d0t, dt.data(), e, kP - *q(m), 0, qt);
				if (j == 2 && block2) times_x<false>(t.roots(), dt.data(), nullptr, e, *q(m), kP - *q(m + k), q2t);
			});
	const std::span<std::uint32_t> start = g.first(std::min(n, kExpBase));
	exp_direct(lo, start);
	for (std::uint32_t& x : start) x = blocked_ntt::detail::multiply_mod(x, c);
	if (n > kExpBase) exp_newton(t, lo, g, rest, QTransforms{qt, q2t});
}

}
namespace blocked_poly {
inline void native_exp(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool differentiated = false) {
	const std::size_t m = detail::exp_length(n) / 2, size = std::min(input, n), below = std::min(size, m);
	const int lg = exp_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(exp_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(exp_scratch(n));
	auto lo = scratch.first(m);
	const detail::Factor undo(blocked_ntt::detail::power(detail::kR, detail::kP - 2));
	const auto import = [&](std::size_t first, std::size_t count, std::uint32_t *to) __attribute__((target("avx2"))) {
		std::size_t i = 0;
		detail::Indices index(first);
		for (; i + 8 <= count; i += 8, index.next()) {
			alignas(32) std::uint32_t words[8];
			for (int j = 0; j < 8; ++j) words[j] = in[first + i + j].v;
			auto v = detail::reduce(detail::times(detail::load(words), undo), detail::kP);
			if (!differentiated) v = detail::canonical(detail::montgomery(v, index.value()));
			detail::store_unaligned(to + i, v);
		}
		for (; i < count; ++i) to[i] = differentiated ? in[first + i].val() : blocked_ntt::detail::multiply_mod(in[first + i].val(), first + i);
	};
	import(0, below, lo.data());
	std::fill(lo.begin() + below, lo.end(), 0u);
	if (size > m) import(m, size - m, g.data() + m);
	if (n > std::max(size, m)) std::fill(g.begin() + std::max(size, m), g.end(), 0u);
	detail::exp_direct(lo, g.first(std::min(n, detail::kExpBase)));
	for (auto &x : g.first(std::min(n, detail::kExpBase))) x = blocked_ntt::detail::multiply_mod(x, detail::kR);
	if (n > detail::kExpBase) detail::exp_newton(t, lo, g, scratch.subspan(Arena::footprint(m)));
	append_fps_words<true>(out, g.data(), n);
}

inline void native_power(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, std::uint32_t e, Mint c) {
	const int lg = power_log(n);
	const std::size_t size = std::min(input, n);
	Arena arena(Transform::words(lg) + Arena::footprint(n) + Arena::footprint(power_scratch(n)));
	const Transform t(arena, lg);
	auto g = arena.take(n), scratch = arena.take(power_scratch(n));
	for (std::size_t i = 0; i < size; ++i) {
		const auto x = in[i].v;
		g[i] = std::min(x, x - Mint::mod());
	}
	power(t, g.first(size), e, std::min(c.v, c.v - Mint::mod()), g, scratch);
	append_fps_words<true>(out, g.data(), n);
}
}
}
#pragma GCC pop_options
#endif
#endif
