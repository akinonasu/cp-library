#ifndef CP_COMPONENTS_FPS_COMPOSE_BLOCKED_HPP
#define CP_COMPONENTS_FPS_COMPOSE_BLOCKED_HPP

#include "fps_composition_common.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::composition_poly {
namespace detail {
inline size_t compose_length(size_t n) { return std::bit_ceil(std::max<size_t>(n, 256)); }

inline void compose_direct(ConstWords f, ConstWords g,
						   Words h) {
	const size_t n = h.size();
	std::fill(h.begin(), h.end(), 0);
	for (size_t i = std::min(f.size(), n); i-- > 0;) {
		for (size_t k = n; k-- > 1;) {
			std::uint64_t sum = 0;
			for (size_t j = 1; j <= k && j < g.size(); ++j) sum += std::uint64_t(g[j]) * h[k - j] % kP;
			h[k] = Word(sum % kP);
		}
		h[0] = f[i];
	}
}

inline void spread(Vec x, Vec (&out)[2]) {
	out[0] = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(x));
	out[1] = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(x, 1));
}

struct CompositionBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Tables* tables;
	const Word *f, *q;

	template <int kParity>
	[[gnu::always_inline]] static void sums(const Vec (&p)[4], const Vec (&wp)[4], const Vec (&c)[8], Vec (&sum)[4]) {
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) {
			const Vec first = product(p[0], c[2 * k + kParity]);
			Vec s = kParity ? minus(offset(), first) : first;
#pragma GCC unroll 3
			for (int i = 1; i < 4; ++i) {
				const Vec term = product(i <= k ? p[i] : wp[i], c[2 * ((k - i + 4) % 4) + kParity]);
				s = kParity ? minus(s, term) : plus(s, term);
			}
			sum[k] = s;
		}
	}

	[[gnu::always_inline]] static void leaf(const Vec (&p)[4], const Vec (&wp)[4], const Vec (&c)[8], Vec (&out)[8]) {
		Vec p2[4], wp2[4], c2[8], even[4], odd[4];
		odd_lanes(p, p2), odd_lanes(wp, wp2), odd_lanes(c, c2);
		sums<0>(p, wp, c, even);
		sums<0>(p2, wp2, c2, odd);
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) out[2 * k] = reduce_sums(even[k], odd[k]);
		sums<1>(p, wp, c, even);
		sums<1>(p2, wp2, c2, odd);
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) out[2 * k + 1] = reduce_sums(even[k], odd[k]);
	}

	[[gnu::flatten]] void products(size_t p, Vec (&r)[16]) const {
		Vec x[8], c[8], pa[4], pb[4], spa[4], spb[4], ra[8], rb[8];
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) x[i] = load(f + 8 * (p + i));
		transpose(x);
		const Factors s = entries(tables->roots, p);
#pragma GCC unroll 4
		for (int i = 0; i < 4; ++i) {
			const Vec t = times(x[i + 4], s);
			pa[i] = canonical(add(x[i], t)), pb[i] = canonical(diff(x[i], t));
			spa[i] = reduce(times(pa[i], s), kP), spb[i] = negate(reduce(times(pb[i], s), kP));
		}
		load_leaves(q + 16 * p, c);
		leaf(pa, spa, c, ra);
		load_leaves(q + 16 * p + 8, c);
		leaf(pb, spb, c, rb);
		transpose(ra), transpose(rb);
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) r[2 * i] = ra[i], r[2 * i + 1] = rb[i];
	}

	[[gnu::always_inline]] static void finish(const Tables& tables, const Vec (&r)[16], size_t g, Word* out) {
#pragma GCC unroll 4
		for (int i = 0; i < 4; ++i) {
			Vec f[4] = {r[4 * i], r[4 * i + 1], r[4 * i + 2], r[4 * i + 3]};
			inverse_h1(f, Group(tables.inverse_roots, g + i));
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(out + 32 * i + 8 * t, f[t]);
		}
	}

	void operator()(Word* out, size_t count, size_t first) const {
		for (size_t j = 0; j < count; j += 4, out += 128) {
			Vec r[16];
			products(2 * (first + j), r);
			finish(*tables, r, first + j, out);
		}
	}
};

struct CompositionSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	CompositionBottom terms[2];

	void operator()(Word* out, size_t count, size_t first) const {
		for (size_t j = 0; j < count; j += 4, out += 128) {
			Vec r[16], r2[16];
			terms[0].products(2 * (first + j), r);
			terms[1].products(2 * (first + j), r2);
#pragma GCC unroll 16
			for (int i = 0; i < 16; ++i) r[i] = low(add(r[i], r2[i]));
			CompositionBottom::finish(*terms[0].tables, r, first + j, out);
		}
	}
};

struct Levels {
	size_t m;
	int lg;
	Tables tables;
	Words transforms;

	Words level(int s) const { return transforms.subspan(size_t(s) * Arena::footprint(4 * m), 4 * m); }

	Words first_transform(size_t i) const { return level(1).subspan(i * Arena::footprint(m), m); }

	Words column_transform(size_t k) const {
		return level(lg - 3).subspan((k - 1) * Arena::footprint(m / 4), m / 4);
	}

	static size_t scratch(size_t m) {
		return Arena::footprint(Tables::words(m)) + Carve::words(size_t(std::countr_zero(m)) * Arena::footprint(4 * m));
	}

	Levels(size_t size, Words scratch, const Transform* shared = nullptr)
		: m(size), lg(std::countr_zero(size)), tables(scratch.first(Tables::words(size)), size, shared) {
		Carve carve(scratch.subspan(Arena::footprint(Tables::words(size))));
		transforms = carve.take(size_t(lg) * Arena::footprint(4 * m));
	}
};

inline void first_level(const Transform& t, ConstWords g, Levels& levels) {
	const size_t m = levels.m, h = m / 2, quarter = m / 4;
	const Tables& tables = levels.tables;
	const ConstWords v = levels.level(3).first(h);
	const Words t1 = levels.first_transform(0), t2 = levels.first_transform(1);
	const size_t even = std::max<size_t>(std::min(h, (g.size() + 1) / 2), 1);
	size_t j = 0;
	for (; j + 8 <= even && 2 * j + 16 <= g.size(); j += 8) {
		const Vec x = even_lanes(_mm256_loadu_si256(reinterpret_cast<const Vec*>(g.data() + 2 * j)),
								 _mm256_loadu_si256(reinterpret_cast<const Vec*>(g.data() + 2 * j + 8)));
		store(t1.data() + j, negate(twice_mod(x)));
	}
	for (; j < even; ++j) t1[j] = minus_twice(g[2 * j]);
	t1[0] = 0;
	std::fill(t1.begin() + std::ptrdiff_t(even), t1.begin() + std::ptrdiff_t(h), 0);
	const Words q = levels.level(2);
	for (size_t i = 0; i < quarter; i += 8)
		store(q.data() + h + i, twice_mod(even_lanes(load(t1.data() + 2 * i), load(t1.data() + 2 * i + 8))));
	t.forward(t1.first(h), 0, t1);
	t.forward(v, 0, t2);
	Word* out[3];
	for (size_t i = 0; i < 3; ++i) out[i] = levels.level(3).data() + Arena::footprint(m) + i * Arena::footprint(h);
	const FirstLevelProducts<false> products{&tables, t1.data(), t2.data(), nullptr, {out[0], out[1], out[2]}};
	for (size_t k = 0; k < m / 16; k += 8) products.pairs(k);
	for (Word* x : out)
		inverse_with(Words(x, h), tables, InverseBottom{tables.inverse_roots, x}, graeffe_scale(levels.lg - 1),
					 Half::kLower);
	q[0] = 1;
	std::fill(q.begin() + 1, q.begin() + std::ptrdiff_t(quarter), 0);
	for (size_t i = 0; i < quarter; i += 8) {
		const Vec v2 = twice_mod(even_lanes(load(v.data() + 2 * i), load(v.data() + 2 * i + 8)));
		store(q.data() + 2 * h + i, reduce(add(v2, load(out[0] + i)), kP));
		store(q.data() + 3 * h + i, twice_mod(load(out[2] + i)));
	}
	std::copy_n(out[1], quarter, q.begin() + std::ptrdiff_t(4 * h));
}

inline void first_level_transposed(const Transform& t, const Levels& levels, ConstWords in,
								   Words out) {
	const size_t m = levels.m, h = m / 2, quarter = m / 4;
	const Tables& tables = levels.tables;
	const auto row = [&](size_t k) { return in.subspan(2 * m + k * h, quarter); };
	Words tp[3];
	for (size_t k = 0; k < 3; ++k) {
		tp[k] = out.subspan(k * Arena::footprint(h), h);
		t.forward(row(k), 0, tp[k]);
	}
	const Word *q1 = levels.first_transform(0).data(), *q2 = levels.first_transform(1).data();
	const auto term = [&](size_t k, const Word* q) { return CompositionBottom{&tables, tp[k].data(), q}; };
	for (size_t r = 2; r < 4; ++r) {
		const Words result = out.subspan(r * m, m);
		inverse_with(result, tables, CompositionSumBottom{{term(r - 1, q1), term(r - 2, q2)}}, kInverseScales[1][levels.lg],
					 Half::kLower);
		const ConstWords add_row = row(r);
		for (size_t i = 0; i < quarter; i += 8) {
			Vec x[2];
			spread(load(add_row.data() + i), x);
			store(result.data() + 2 * i, reduce(add(load(result.data() + 2 * i), x[0]), kP));
			store(result.data() + 2 * i + 8, reduce(add(load(result.data() + 2 * i + 8), x[1]), kP));
		}
	}
}

inline void third_last_level(const Transform& t, const Levels& levels, const Words (&c)[4]) {
	const size_t m = levels.m, quarter = m / 4, eighth = m / 8;
	const ConstWords q = levels.level(levels.lg - 3);
	const Words temporary = levels.level(levels.lg - 1);
	Words coefficients[8], tc[8];
	for (size_t k = 1; k < 8; ++k) {
		coefficients[k] = temporary.subspan((k - 1) * Arena::footprint(eighth + 1), eighth + 1);
		tc[k] = levels.column_transform(k);
	}
	for (size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (size_t i = 0; i < 8; ++i) x[i] = load(q.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 7
		for (size_t k = 1; k < 8; ++k) store(coefficients[k].data() + r, k % 2 ? negate(x[k]) : x[k]);
	}
	for (size_t k = 1; k < 8; ++k) {
		const Word x = q[16 * eighth + k];
		coefficients[k][eighth] = k % 2 && x ? kP - x : x;
	}
	for (size_t k = 1; k < 8; ++k) t.forward(coefficients[k], 0, tc[k]);
	Words twice[3], square[4];
	const Words sums = temporary.subspan(7 * Arena::footprint(eighth + 1));
	for (size_t i = 0; i < 3; ++i) twice[i] = sums.subspan(i * Arena::footprint(quarter), quarter);
	for (size_t k = 1; k < 4; ++k) square[k] = sums.subspan((k + 2) * Arena::footprint(quarter), quarter);
	multiply_constant(tc[5], 2, twice[0]), multiply_constant(tc[3], kP - 2, twice[1]), multiply_constant(tc[4], kP - 2, twice[2]);
	const Transform::Pair first[1] = {{tc[1], tc[1]}}, second[2] = {{tc[2], tc[2]}, {tc[1], twice[1]}},
						  third[3] = {{tc[3], tc[3]}, {tc[1], twice[0]}, {tc[2], twice[2]}};
	t.inverse_product_sum(first, square[1]);
	t.inverse_product_sum(second, square[2]);
	t.inverse_product_sum(third, square[3]);
	const Word weight[4] = {0, kP - 2, 2, kP - 2};
	for (size_t k = 1; k < 4; ++k) {
		const ConstWords linear = coefficients[2 * k];
		const Factor w(weight[k]);
		for (size_t i = 0; i < quarter; i += 8) {
			const Vec x = load(square[k].data() + i);
			store(c[k].data() + i, i < eighth ? canonical(add(x, reduce(times(load(linear.data() + i), w), kP))) : x);
		}
		const std::uint64_t last = c[k][eighth] + std::uint64_t(weight[k]) * linear[eighth];
		c[k][eighth] = Word(last % kP);
		c[k][quarter] = c[k][0];
		c[k][0] = 0;
		std::fill(c[k].begin() + std::ptrdiff_t(quarter + 1), c[k].end(), 0);
	}
}

inline void third_last_level_transposed(const Transform& t, const Levels& levels, const ConstWords (&p)[4],
										Words work, Words out) {
	const size_t m = levels.m, quarter = m / 4, eighth = m / 8;
	Words tp[4], r[8];
	for (size_t j = 0; j < 4; ++j) {
		tp[j] = work.subspan(j * Arena::footprint(quarter), quarter);
		t.forward(p[j], 0, tp[j]);
	}
	for (size_t c = 1; c < 8; ++c) {
		r[c] = work.subspan((c + 3) * Arena::footprint(quarter), quarter);
		Transform::Pair pairs[4];
		size_t count = 0;
		for (size_t j = 0; j < 4 && 2 * j < c; ++j) pairs[count++] = {tp[j], levels.column_transform(c - 2 * j)};
		if (count < 4) t.inverse_product_sum(std::span(pairs, count), r[c], Half::kUpper);
		else inverse_product_sum<4>(levels.tables, pairs, r[c], Half::kUpper);
	}
	for (size_t i = eighth; i < quarter; i += 8) {
		Vec x[8];
		x[0] = load(p[0].data() + i);
#pragma GCC unroll 7
		for (size_t c = 1; c < 8; ++c) x[c] = c % 2 ? load(r[c].data() + i) : canonical(add(load(r[c].data() + i), load(p[c / 2].data() + i)));
		transpose(x);
#pragma GCC unroll 8
		for (size_t j = 0; j < 8; ++j) store(out.data() + 2 * m + 16 * (i - eighth + j), x[j]);
	}
}

inline void build_levels(const Transform& t, ConstWords g, Levels& levels) {
	const size_t m = levels.m;
	const Words v = levels.level(3).first(m), q0 = levels.level(0).first(2 * m);
	std::fill(q0.begin(), q0.end(), 0);
	std::copy(g.begin() + std::ptrdiff_t(std::min<size_t>(g.size(), 1)), g.begin() + std::ptrdiff_t(std::min(g.size(), m)),
			  q0.begin() + 1);
	const Tables& tables = levels.tables;
	forward_with(q0, m, tables, LevelBottom{&tables, v.data()});
	inverse_with(v, tables, InverseBottom{tables.inverse_roots, v.data()}, graeffe_scale(levels.lg), Half::kBoth);
	first_level(t, g, levels);
	for (int s = 2; s + 3 < levels.lg; ++s) {
		const size_t stride = 2 * (m >> s), pad = size_t(levels.lg - s - 3);
		const Words next = levels.level(s + 1).first(2 * m);
		forward_pruned(levels.level(s), ((size_t(1) << s) + 1) * stride, pad, tables, LevelBottom{&tables, next.data()});
		inverse_pruned(next, pad - 1, tables, InverseBottom{tables.inverse_roots, next.data()}, graeffe_scale(levels.lg + 1));
		next_level(levels.level(s + 1), stride / 2, size_t(2) << s);
	}
	Words c[4];
	for (size_t k = 1; k < 4; ++k) c[k] = levels.level(levels.lg - 2).subspan((k - 1) * Arena::footprint(m / 2), m / 2);
	third_last_level(t, levels, c);
	const Words q = levels.level(levels.lg - 1).first(m);
	std::fill(q.begin(), q.end(), 0);
	for (size_t i = 0; i < m / 4; i += 8) store(q.data() + i, negate(twice_mod(load(c[2].data() + i))));
	q[m / 4] = minus_twice(c[2][m / 4]);
	const Words square = levels.level(levels.lg - 1).subspan(Arena::footprint(m), m / 2);
	for (size_t k = 1; k < 4; ++k) t.forward(c[k]);
	t.inverse_product(c[1], c[1], square);
	for (size_t i = 0; i < m / 2; i += 8) store(q.data() + i, reduce(add(load(q.data() + i), load(square.data() + i)), kP));
	const Word top = q[m / 2] + q[0];
	q[m / 2] = top >= kP ? top - kP : top;
	q[0] = 0;
	t.forward(q);
}


}

inline int compose_log(size_t n) {
	if (n <= detail::kComposeBase) return Transform::kMinLog;
	return std::countr_zero(detail::compose_length(n)) + 1;
}

inline size_t compose_scratch(size_t n) {
	if (n <= detail::kComposeBase) return 0;
	return detail::Levels::scratch(detail::compose_length(n));
}

inline Word outer_word(Word x) { return x; }
inline Word outer_word(const mint& x) { return std::min(x.v, x.v - mint::mod()); }

template <class Coefficient>
void compose_imported(const Transform& t, std::span<const Coefficient> f, ConstWords g,
					Words h, Words scratch, const Transform* shared = nullptr) {
	static_assert(std::is_same_v<Coefficient, Word> || std::is_same_v<Coefficient, mint>);
	const size_t n = h.size();
	if (f.empty() || g.size() <= 1) {
		std::fill(h.begin(), h.end(), 0u);
		if (n && !f.empty()) h[0] = outer_word(f[0]);
		return;
	}
	if (n <= detail::kComposeBase) {
		if constexpr (std::is_same_v<Coefficient, Word>) return detail::compose_direct(f, g, h);
		Word a[detail::kComposeBase];
		const size_t size = std::min(f.size(), n);
		for (size_t i = 0; i < size; ++i) a[i] = outer_word(f[i]);
		return detail::compose_direct(ConstWords(a, size), g, h);
	}
	detail::Levels levels(detail::compose_length(n), scratch, shared);
	detail::build_levels(t, g, levels);
	const size_t m = levels.m;
	const int lg = levels.lg;

	const Words last = levels.level(lg - 1);
	const Words p = last.subspan(Arena::footprint(m), m), pq = last.subspan(2 * Arena::footprint(m), m);
	{
		const size_t size = std::min(f.size(), m);
		std::fill(p.begin(), p.end() - std::ptrdiff_t(size), 0);
		Word* const to = p.data() + (m - size);
		const detail::Vec reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
		size_t j = 0;
		for (; j + 8 <= size; j += 8) {
			static_assert(sizeof(Coefficient) == sizeof(Word));
			detail::Vec x;
			std::memcpy(&x, f.data() + size - 8 - j, sizeof(x));
			if constexpr (std::is_same_v<Coefficient, mint>) x = detail::reduce(x, detail::kP);
			_mm256_storeu_si256(reinterpret_cast<detail::Vec*>(to + j), _mm256_permutevar8x32_epi32(x, reverse));
		}
		for (; j < size; ++j) to[j] = outer_word(f[size - 1 - j]);
	}
	t.cyclic_product(p, 0, pq, last.first(m), Half::kUpper);

	{
		const size_t h = m / 2, quarter = m / 4;
		const Words p0 = p.subspan(h), p1 = pq.subspan(h), t0 = last.first(h), t1 = last.subspan(Arena::footprint(h), h);
		const Words c = levels.level(lg - 2);
		const auto column = [&](size_t k) { return c.subspan((k - 1) * Arena::footprint(h), h); };
		const Words r1 = p.first(h), r2 = pq.first(h), r3 = last.subspan(3 * Arena::footprint(m), h);
		t.forward(p0, 0, t0);
		t.forward(p1, 0, t1);
		t.inverse_product(t0, column(1), r1, Half::kUpper);
		t.inverse_product(t0, column(2), r2, Half::kUpper);
		const Transform::Pair pairs[2] = {{t1, column(1)}, {t0, column(3)}};
		t.inverse_product_sum(pairs, r3, Half::kUpper);
		for (size_t i = quarter; i < h; i += 8)
			detail::store(r2.data() + i, detail::reduce(detail::add(detail::load(p1.data() + i), detail::load(r2.data() + i)), kModulus));
		const ConstWords columns[4] = {p0.subspan(quarter), r1.subspan(quarter), r2.subspan(quarter),
														   r3.subspan(quarter)};
		detail::third_last_level_transposed(t, levels, columns, levels.level(lg - 2), levels.level(lg - 3));
	}

	Words in = levels.level(lg - 3);
	const Word scale = detail::kInverseScales[1][lg + 2];
	for (int s = lg - 4; s >= 2; --s) {
		const Words x = in.subspan(2 * m), out = levels.level(s == lg - 4 ? lg - 2 : s + 1);
		const size_t pad = size_t(lg - s - 3);
		detail::forward_pruned(x, 2 * m, pad - 1, levels.tables, detail::ForwardBottom{levels.tables.roots});
		detail::inverse_pruned(out, pad, levels.tables, detail::CompositionBottom{&levels.tables, x.data(), levels.level(s).data()},
							   scale, Half::kUpper);
		in = out;
	}
	detail::first_level_transposed(t, levels, in, levels.level(2));
	in = levels.level(2);
	const Words out = levels.level(1);

	const Words p0 = in.subspan(2 * m, m), r = out.first(2 * m);
	t.forward(p0.first(m / 2), 0, p0);
	detail::inverse_with(r, levels.tables, detail::CompositionBottom{&levels.tables, p0.data(), levels.level(0).data()},
						 detail::kInverseScales[1][levels.lg + 1], Half::kLower);
	size_t k = 0;
	for (; k + 16 <= n; k += 16) {
		detail::Vec a[2];
		detail::spread(detail::load(in.data() + 3 * m + k / 2), a);
		for (size_t t = 0; t < 2; ++t)
			_mm256_storeu_si256(reinterpret_cast<detail::Vec*>(h.data() + k + 8 * t), detail::difference(a[t], detail::load(r.data() + k + 8 * t)));
	}
	for (; k < n; ++k) {
		const Word a = k % 2 ? 0 : in[3 * m + k / 2];
		h[k] = a >= r[k] ? a - r[k] : a + kModulus - r[k];
	}
}

inline void compose(const Transform& t, ConstWords f, ConstWords g,
		Words h, Words scratch) {
	compose_imported(t, f, g, h, scratch);
}

inline void native_compose(const mint *f, size_t nf, const mint *g, size_t ng,
						   std::vector<mint> &out, size_t n) {
	nf = std::min(nf, n);
	ng = std::min(ng, n);
	const int lg = compose_log(n) + (n > detail::kComposeBase);
	Arena arena(Transform::words(lg) + Arena::footprint(nf) + Arena::footprint(ng) +
				Arena::footprint(n) + Arena::footprint(compose_scratch(n)));
	const Transform t(arena, lg);
	auto a = arena.take(nf), b = arena.take(ng), c = arena.take(n);
	for (size_t i = 0; i < nf; ++i) a[i] = std::min(f[i].v, f[i].v - mint::mod());
	for (size_t i = 0; i < ng; ++i) b[i] = coefficient_value(g[i]);
	compose_imported(t, ConstWords(a), b, c, arena.take(compose_scratch(n)), &t);
	append_fps_words<true>(out, c.data(), c.size());
}

}
#pragma GCC pop_options
#endif
#endif
