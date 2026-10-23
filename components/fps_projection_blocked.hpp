#ifndef CP_COMPONENTS_FPS_PROJECTION_BLOCKED_HPP
#define CP_COMPONENTS_FPS_PROJECTION_BLOCKED_HPP

#include "fps_composition_common.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::composition_poly {
namespace detail {
inline size_t projection_length(size_t n) { return std::bit_ceil(std::max<size_t>(n, 512)); }

inline void projection_direct(ConstWords g, Words a) {
	const size_t n = a.size();
	Word power[kComposeBase] = {1}, next[kComposeBase];
	for (size_t k = 0; k < n; ++k) {
		a[k] = power[n - 1];
		for (size_t i = 0; i < n; ++i) {
			std::uint64_t c = 0;
			for (size_t j = 1; j <= i && j < g.size(); ++j) c += std::uint64_t(g[j]) * power[i - j] % kP;
			next[i] = Word(c % kP);
		}
		std::copy_n(next, n, power);
	}
}

inline Vec odd_lanes(Vec x, Vec y) {
	const Vec t = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), 0xDD));
	return _mm256_permute4x64_epi64(t, 0xD8);
}

template <bool kAdd, class Op>
void even_terms(ConstWords b, size_t lo, size_t hi, std::ptrdiff_t offset,
				Words out, const Op& op) {
	const auto scalar = [&op](Word x) {
		return Word(_mm_cvtsi128_si32(_mm256_castsi256_si128(op(_mm256_set1_epi32(int(x))))));
	};
	const auto put = [&out](size_t j, Word x) {
		if constexpr (kAdd) out[j] = out[j] + x >= kP ? out[j] + x - kP : out[j] + x;
		else out[j] = x;
	};
	const auto at = [offset](size_t j) { return std::ptrdiff_t(2 * j) + offset; };
	const auto first = [&](size_t i) {
		const std::ptrdiff_t d = std::ptrdiff_t(i) - offset;
		return std::min(out.size(), d <= 0 ? size_t(0) : size_t(d + 1) / 2);
	};
	const size_t begin = first(lo), end = std::max(begin, first(hi));
	if constexpr (!kAdd) {
		std::fill(out.begin(), out.begin() + std::ptrdiff_t(begin), 0);
		std::fill(out.begin() + std::ptrdiff_t(end), out.end(), 0);
	}
	size_t j = begin;
	for (; j + 8 <= end && size_t(at(j)) + 16 <= b.size(); j += 8) {
		const Word* from = b.data() + at(j);
		Vec x = op(even_lanes(load_unaligned(from), load_unaligned(from + 8)));
		if constexpr (kAdd) x = reduce(add(load_unaligned(out.data() + j), x), kP);
		store_unaligned(out.data() + j, x);
	}
	for (; j < end; ++j) put(j, scalar(b[size_t(at(j))]));
}

inline void add_into(ConstWords x, ConstWords y, Words out) {
	for (size_t i = 0; i < out.size(); i += 8) store(out.data() + i, reduce(add(load(x.data() + i), load(y.data() + i)), kP));
}

struct ProjectionBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Tables* tables;
	const Word* p;
	Word *v, *w;

	void operator()(Word* a, size_t count, size_t first) const {
		ForwardBottom{tables->roots}(a, count, first);
		for (size_t i = 0; i < count; i += 4) pairs(a + 32 * i, 2 * (first + i));
	}

	[[gnu::flatten]] void pairs(const Word* q, size_t k) const {
		const PairWeights weights(*tables, k);
		Vec va[4], vb[4], wa[4], wb[4];
		leaves<false>(q, p + 16 * k, weights.s, va, wa);
		leaves<true>(q + 8, p + 16 * k + 8, weights.s, vb, wb);
		combine_pairs(va, vb, weights.inverse, v + 8 * k);
		combine_pairs(wa, wb, weights.inverse, w + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] static void leaves(const Word* q, const Word* p, const Factors& s,
													   Vec (&v)[4], Vec (&w)[4]) {
		Vec c[8], a[8], tc[8];
		load_leaves(q, c), load_leaves(p, a);
		scaled_leaves<kNegative>(c, s, 2, tc);
		graeffe_leaf(c, tc, v);
		product_leaf<1>(a, c, tc, w);
	}
};

inline void inverse_lower(const Word* a, size_t h, const Group& w, Word* out) {
	for (size_t i = 0; i < 8 * h; i += 8) {
		const Vec f0 = load(a + i), f1 = load(a + i + 8 * h), f2 = load(a + i + 16 * h), f3 = load(a + i + 24 * h);
		store(out + i, low(add(low(add(f0, f1)), low(add(f2, f3)))));
		store(out + i + 8 * h, low(add(times(diff(f0, f1), w.y), times(diff(f2, f3), w.z))));
	}
}

inline void forward_lower(const Word* in, size_t h, const Group& w, Word* a) {
	for (size_t i = 0; i < 8 * h; i += 8) {
		const Vec u = low(load(in + i)), b = load(in + i + 8 * h), yb = times(b, w.y), zb = times(b, w.z);
		store(a + i, add(u, yb)), store(a + i + 8 * h, diff(u, yb));
		store(a + i + 16 * h, add(u, zb)), store(a + i + 24 * h, diff(u, zb));
	}
}

template <class Bottom>
class Doubling {
public:

	Doubling(Words a, size_t block, const Tables& tables, const Bottom& bottom)
		: a_(a.data()), half_(a.size() / 2), block_(block), b_(size_t(std::countr_zero(block)) - 1),
		  aligned_(std::countr_zero(block) % 2 == 0), tables_(tables), bottom_(bottom),
		  inverse_(tables.roots, tables.inverse_roots, InverseBottom{tables.inverse_roots, a.data()}),
		  forward_(tables.roots, tables.inverse_roots, bottom) {}

	void run(Word y_scale, Word fix) const {
		Word* const copy = a_ + half_;
		const size_t nv = half_ / 8, chunk = std::max<size_t>(block_, 16);
		for (size_t c = 0; c < nv; c += chunk) {
			for (size_t j = c / block_; j < (c + chunk) / block_; ++j) {
				inverse_x(a_ + 8 * block_ * j, j, copy + 8 * block_ * j);
				forward_x(copy + 8 * block_ * j, j, a_ + 8 * block_ * j);
			}
			if (block_ < 16) bottom_(a_ + 8 * c, chunk / 4, c / 4);
		}
		inverse_y(copy, y_scale);
		if (fix) {
			for (size_t i = 0; i < 4 * block_; i += 8) store(copy + i, negate(load(copy + i)));
			copy[0] = fix >= kP - copy[0] ? fix - (kP - copy[0]) : fix + copy[0];
		}
		forward_upper();
	}

private:

	void inverse_x(Word* a, size_t j, Word* out) const {
		if (block_ == 4) {
			inverse_.visit(a, 4, j);
			std::copy_n(a, 16, out);
		} else if (aligned_) {
			const size_t h = block_ / 4;
			for (size_t t = 0; t < 4; ++t) inverse_.visit(a + 8 * t * h, h, 4 * j + t);
			inverse_lower(a, h, Group(tables_.inverse_roots, j), out);
		} else {
			const size_t h = block_ / 2;
			inverse_.visit(a, h, 2 * j);
			inverse_.visit(a + 8 * h, h, 2 * j + 1);
			for (size_t i = 0; i < 8 * h; i += 8) store(out + i, low(add(load(a + i), load(a + i + 8 * h))));
		}
	}

	void forward_x(const Word* in, size_t j, Word* a) const {
		if (block_ == 4) {
			std::copy_n(in, 16, a);
			std::fill_n(a + 16, 16, 0);
		} else if (aligned_) {
			const size_t h = block_ / 4;
			forward_lower(in, h, Group(tables_.roots, j), a);
			if (h == 4) return bottom_(a, 4, 4 * j);
			for (size_t t = 0; t < 4; ++t) forward_.visit(a + 8 * t * h, h, 4 * j + t);
		} else {
			const size_t h = block_ / 2;
			for (size_t i = 0; i < 8 * h; i += 8) {
				const Vec x = load(in + i);
				store(a + i, x), store(a + i + 8 * h, x);
			}
			if (h == 4) return;
			forward_.visit(a, h, 2 * j);
			forward_.visit(a + 8 * h, h, 2 * j + 1);
		}
	}

	void above(Word* a, size_t nv, size_t k) const {
		if (nv == block_) return;
		if (nv == 2 * block_) {
			const Factor x = entry(tables_.inverse_roots, k);
			for (size_t i = 0; i < 4 * block_; i += 8) {
				const Vec u = load(a + i), v = load(a + i + 8 * block_);
				store(a + i, low(add(u, v))), store(a + i + 8 * block_, times(diff(u, v), x));
			}
			return;
		}
		const size_t h = nv / 4;
		for (size_t t = 0; t < 4; ++t) above(a + 8 * t * h, h, 4 * k + t);
		inverse_columns(a, h, b_, tables_.inverse_roots, k);
	}

	void inverse_y(Word* a, Word scale) const {
		const size_t nv = half_ / 8, width = size_t(1) << b_;
		const bool zero = b_ <= 5;
		Vec* const v = reinterpret_cast<Vec*>(a);
		const Factor s(scale);
		const auto scaled = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
		const Vec z0 = _mm256_setzero_si256();
		if (std::countr_zero(nv) % 2 == 0) {
			const size_t h = nv / 4;
			for (size_t t = 0; t < 4; ++t) above(a + 8 * t * h, h, t);
			const Factor z(tables_.inverse_roots[1], tables_.inverse_roots[9]);
			for (size_t c = 0; c < h; c += 2 * width) {
				for (size_t j = c; j < c + width; ++j) {
					const Vec x0 = v[j], x1 = v[j + h], x2 = v[j + 2 * h], x3 = v[j + 3 * h];
					const Vec ab = low(add(x0, x1)), cd = low(add(x2, x3));
					const Vec amb = low(diff(x0, x1)), cmd = times(diff(x2, x3), z);
					v[j] = scaled(add(ab, cd)), v[j + h] = scaled(add(amb, cmd));
					v[j + 2 * h] = scaled(diff(ab, cd)), v[j + 3 * h] = scaled(diff(amb, cmd));
				}
				if (zero)
					for (size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = v[j + 2 * h] = v[j + 3 * h] = z0;
			}
		} else {
			const size_t h = nv / 2;
			above(a, h, 0);
			above(a + 8 * h, h, 1);
			for (size_t c = 0; c < h; c += 2 * width) {
				for (size_t j = c; j < c + width; ++j) {
					const Vec x0 = v[j], x1 = v[j + h];
					v[j] = scaled(add(x0, x1)), v[j + h] = scaled(diff(x0, x1));
				}
				if (zero)
					for (size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = z0;
			}
		}
	}

	void forward_upper() const {
		const size_t nv = half_ / 4, width = size_t(1) << b_;
		const Pruned<Bottom> pruned(tables_, bottom_, b_);
		if (std::countr_zero(nv) % 2 == 0) {
			const size_t h = nv / 4;
			Vec* const v = reinterpret_cast<Vec*>(a_);
			const Factor z(tables_.roots[1], tables_.roots[9]);
			for (size_t c = 0; c < h; c += 2 * width)
				for (size_t j = c; j < c + width; ++j) {
					const Vec x = v[2 * h + j], y = times(v[3 * h + j], z);
					v[2 * h + j] = add(x, y), v[3 * h + j] = diff(x, y);
				}
			pruned.forward(a_ + 16 * h, h, 2);
			pruned.forward(a_ + 24 * h, h, 3);
		} else {
			pruned.forward(a_ + 4 * nv, nv / 2, 1);
		}
	}

	Word* a_;
	size_t half_, block_, b_;
	bool aligned_;
	const Tables& tables_;
	Bottom bottom_;
	Recursion<InverseBottom> inverse_;
	Recursion<Bottom> forward_;
};

inline void projection_first_levels(const Transform& t, const Tables& tables, ConstWords g,
									size_t n, Words q, Words p) {
	const size_t m = q.size() / 4, h = m / 2, quarter = m / 4, e = m - n, size = std::min(n, g.size());
	const auto identity = [&](Vec x) __attribute__((target("avx2"))) { return x; };
	const auto negated = [&](Vec x) __attribute__((target("avx2"))) { return negate(x); };
	const auto doubled = [&](Vec x) __attribute__((target("avx2"))) { return twice_mod(x); };
	const auto minus_twice = [&](Vec x) __attribute__((target("avx2"))) { return negate(twice_mod(x)); };
	const auto row = [h, quarter](Words a, size_t k) { return a.subspan(k * h, quarter); };

	if (size > 0) {
		q[0] = 0;
		std::copy(g.begin() + 1, g.begin() + std::ptrdiff_t(size), q.begin() + 1);
	}
	forward_with(q.first(2 * m), size, tables, LevelBottom{&tables, q.data()});
	inverse_with(q.first(m), tables, InverseBottom{tables.inverse_roots, q.data()}, graeffe_scale(std::countr_zero(m)),
				 Half::kBoth);
	const Words q2 = p.subspan(7 * h, h), q1 = q.subspan(7 * h, h), p1 = q.subspan(4 * h, h);
	std::copy_n(q.begin(), h, q2.begin());
	even_terms<false>(g, 1, size, 0, q1, minus_twice);
	if (e % 2) even_terms<false>(g, 1, size, 1 - std::ptrdiff_t(e), p1, negated);
	else even_terms<false>(g, 1, size, 1 - std::ptrdiff_t(e), p1, identity);

	const Words tq2 = q.first(m), tq1 = q.subspan(m, m), tp1 = p.first(m);
	t.forward(tq2.first(h), 0, tq2);
	t.forward(q1, 0, tq1);
	t.forward(p1, 0, tp1);
	Word* const g1 = p.data() + 4 * h;
	Word* const g2 = p.data() + 5 * h;
	Word* const e12 = p.data() + 6 * h;
	Word* const o1 = q.data() + 5 * h;
	Word* const o2 = q.data() + 6 * h;
	const FirstLevelProducts<true> products{&tables, tq1.data(), tq2.data(), tp1.data(), {g1, g2, e12, o1, o2}};
	for (size_t k = 0; k < m / 16; k += 8) products.pairs(k);
	const Word scale = graeffe_scale(std::countr_zero(h));
	for (Word* out : products.out)
		inverse_with(Words(out, h), tables, InverseBottom{tables.inverse_roots, out}, scale, Half::kLower);

	std::fill(p.begin(), p.begin() + std::ptrdiff_t(quarter), 0);
	if (e % 2 && (e - 1) / 2 % 2) p[(e - 3) / 4] = 1;
	even_terms<false>(p1, 0, h, 1, row(p, 1), identity);
	std::copy_n(o1, quarter, p.begin() + std::ptrdiff_t(2 * h));
	std::copy_n(o2, quarter, p.begin() + std::ptrdiff_t(3 * h));
	if (e % 2) {
		const size_t d = (e - 1) / 2;
		const std::ptrdiff_t offset = 1 - std::ptrdiff_t(d);
		const ConstWords b1 = q1, b2 = q2;
		if (d % 2) {
			even_terms<true>(b1, 0, h, offset, row(p, 1), identity);
			even_terms<true>(b2, 0, h, offset, row(p, 2), identity);
		} else {
			even_terms<true>(b1, 0, h, offset, row(p, 1), negated);
			even_terms<true>(b2, 0, h, offset, row(p, 2), negated);
		}
	}

	std::fill(q.begin(), q.begin() + std::ptrdiff_t(quarter), 0);
	q[0] = 1;
	even_terms<false>(q1, 0, h, 0, row(q, 1), doubled);
	even_terms<false>(q2, 0, h, 0, row(q, 2), doubled);
	add_into(row(q, 2), ConstWords(g1, quarter), row(q, 2));
	for (size_t i = 0; i < quarter; i += 8) store(q.data() + 3 * h + i, twice_mod(load(e12 + i)));
	std::copy_n(g2, quarter, q.begin() + std::ptrdiff_t(4 * h));
}

inline void projection_weighted_first(const Transform& t, const Tables& tables,
		ConstWords g, ConstWords weights,
		Words q, Words p, Words work) {
	const size_t m = q.size() / 4, h = m / 2, quarter = m / 4, n = weights.size();
	projection_first_levels(t, tables, g, n, q, p);
	auto x = work.first(2 * m), y = work.subspan(2 * m, 2 * m), z = work.subspan(4 * m, 2 * m);
	std::fill(x.begin(), x.end(), 0u);
	std::fill(y.begin(), y.end(), 0u);
	for (size_t i = 0; i < n; ++i) x[m - 1 - i] = weights[i];
	for (size_t i = 1; i < std::min(g.size(), n); ++i) y[i] = i % 2 && g[i] ? kP - g[i] : g[i];
	t.forward(y);
	t.cyclic_product(x.first(m), 0, z, y, Half::kLower);
	auto p0 = work.first(h), p1 = work.subspan(h, h);
	for (size_t i = 0; i < h; ++i) {
		const auto value = z[2 * i + 1];
		p1[i] = value ? kP - value : 0;
		p0[i] = 2 * i + 1 < m && m - 2 - 2 * i < n ? weights[m - 2 - 2 * i] : 0;
	}
	auto tp0 = work.subspan(m, m), tp1 = work.subspan(2 * m, m);
	auto tq1 = work.subspan(3 * m, m), tq2 = work.subspan(4 * m, m), product = work.subspan(5 * m, m);
	for (size_t i = 0; i < h; ++i) {
		const auto a = q[7 * h + i], b = p[7 * h + i];
		tq1[i] = i % 2 && a ? kP - a : a;
		tq2[i] = i % 2 && b ? kP - b : b;
	}
	t.forward(p0, 0, tp0);
	t.forward(p1, 0, tp1);
	t.forward(tq1.first(h), 0, tq1);
	t.forward(tq2.first(h), 0, tq2);
	for (size_t i = 0; i < quarter; ++i) p[i] = p0[2 * i + 1];
	t.inverse_product(tp0, tq1, product, Half::kLower);
	for (size_t i = 0; i < quarter; ++i) {
		const auto value = product[2 * i + 1] + p1[2 * i + 1];
		p[h + i] = value >= kP ? value - kP : value;
	}
	const Transform::Pair pairs[2] = {{tp0, tq2}, {tp1, tq1}};
	t.inverse_product_sum(pairs, product, Half::kLower);
	for (size_t i = 0; i < quarter; ++i) p[2 * h + i] = product[2 * i + 1];
	t.inverse_product(tp1, tq2, product, Half::kLower);
	for (size_t i = 0; i < quarter; ++i) p[3 * h + i] = product[2 * i + 1];
}

inline size_t projection_work(size_t m) {
	const size_t eighth = m / 8;
	return std::max(7 * Carve::words(eighth + 1) + 8 * Carve::words(eighth), Carve::words(m) + 2 * Carve::words(m / 2));
}

inline void projection_last_levels(const Transform& t, const Tables& tables, Words q,
								   Words p, Words w, Words a) {
	const size_t m = q.size() / 4, eighth = m / 8, quarter = m / 4, h = m / 2;
	Words c[8], pc[8];
	{
		Carve carve(w);
		for (size_t k = 1; k < 8; ++k) c[k] = carve.take(eighth + 1);
		for (size_t k = 0; k < 8; ++k) pc[k] = carve.take(eighth);
	}
	for (size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (size_t i = 0; i < 8; ++i) x[i] = load(q.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 7
		for (size_t k = 1; k < 8; ++k) store(c[k].data() + r, k % 2 ? negate(x[k]) : x[k]);
	}
	for (size_t k = 1; k < 8; ++k) {
		const Word x = q[16 * eighth + k];
		c[k][eighth] = k % 2 && x ? kP - x : x;
	}
	for (size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (size_t i = 0; i < 8; ++i) x[i] = load(p.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 8
		for (size_t k = 0; k < 8; ++k) store(pc[k].data() + r, x[k]);
	}

	Carve carve_q(q), carve_p(p);
	Words tc[8], tp[7], twice[3], cn[4], pn[4];
	for (size_t k = 1; k < 8; ++k) {
		tc[k] = carve_q.take(quarter);
		t.forward(c[k], 0, tc[k]);
	}
	for (size_t k = 0; k < 7; ++k) {
		tp[k] = carve_p.take(quarter);
		t.forward(pc[k], 0, tp[k]);
	}
	for (auto& s : twice) s = carve_q.take(quarter);
	multiply_constant(tc[5], 2, twice[0]), multiply_constant(tc[3], kP - 2, twice[1]), multiply_constant(tc[4], kP - 2, twice[2]);
	for (size_t k = 1; k < 4; ++k) cn[k] = carve_q.take(quarter + 1);
	for (auto& s : pn) s = carve_p.take(quarter);
	{
		const Transform::Pair first[1] = {{tc[1], tc[1]}}, second[2] = {{tc[2], tc[2]}, {tc[1], twice[1]}},
							  third[3] = {{tc[3], tc[3]}, {tc[1], twice[0]}, {tc[2], twice[2]}};
		t.inverse_product_sum(first, cn[1].first(quarter));
		t.inverse_product_sum(second, cn[2].first(quarter));
		t.inverse_product_sum(third, cn[3].first(quarter));
	}
	const Word weight[4] = {0, kP - 2, 2, kP - 2};
	for (size_t k = 1; k < 4; ++k) {
		const ConstWords linear = c[2 * k];
		const Factor f(weight[k]);
		for (size_t i = 0; i < eighth; i += 8)
			store(cn[k].data() + i, canonical(add(load(cn[k].data() + i), reduce(times(load(linear.data() + i), f), kP))));
		const std::uint64_t last = cn[k][eighth] + std::uint64_t(weight[k]) * linear[eighth];
		cn[k][eighth] = Word(last % kP);
		cn[k][quarter] = cn[k][0];
		cn[k][0] = 0;
	}
	{
		Transform::Pair pairs[7];
		for (size_t j = 0; j < 4; ++j) {
			for (size_t b = 1; b <= 2 * j + 1; ++b) pairs[b - 1] = {tp[2 * j + 1 - b], tc[b]};
			const std::span<const Transform::Pair> sum(pairs, 2 * j + 1);
			if (j < 2) t.inverse_product_sum(sum, pn[j]);
			else if (j == 2) inverse_product_sum<5>(tables, pairs, pn[j], Half::kBoth);
			else inverse_product_sum<7>(tables, pairs, pn[j], Half::kBoth);
			add_into(pn[j].first(eighth), pc[2 * j + 1], pn[j].first(eighth));
		}
	}

	Carve next_q(q), next_p(p), next_w(w);
	Words tcn[4], tpn[3];
	for (size_t k = 1; k < 4; ++k) {
		tcn[k] = next_q.take(h);
		t.forward(cn[k], 0, tcn[k]);
	}
	for (size_t j = 0; j < 3; ++j) {
		tpn[j] = next_p.take(h);
		t.forward(pn[j], 0, tpn[j]);
	}
	const Words square = next_q.take(h), negative_q = next_w.take(m);
	t.inverse_product(tcn[1], tcn[1], square);
	for (size_t i = 0; i < h; i += 8) {
		const Vec x = i < quarter ? negate(twice_mod(load(cn[2].data() + i))) : _mm256_setzero_si256();
		store(negative_q.data() + i, reduce(add(x, load(square.data() + i)), kP));
	}
	{
		const Word x = negative_q[quarter] + minus_twice(cn[2][quarter]);
		negative_q[quarter] = x >= kP ? x - kP : x;
		negative_q[h] = negative_q[0];
		negative_q[0] = 0;
		std::fill(negative_q.begin() + std::ptrdiff_t(h + 1), negative_q.end(), 0);
	}
	const Words r1 = next_w.take(h), r3 = next_w.take(h);
	t.inverse_product(tpn[0], tcn[1], r1);
	add_into(r1.first(quarter), pn[1], r1.first(quarter));
	const Transform::Pair pairs[3] = {{tpn[2], tcn[1]}, {tpn[1], tcn[2]}, {tpn[0], tcn[3]}};
	t.inverse_product_sum(pairs, r3);
	add_into(r3.first(quarter), pn[3], r3.first(quarter));
	t.forward(negative_q);
	const Words product = next_q.take(m);
	t.cyclic_product(r1, 0, product, negative_q);
	size_t k = 0;
	for (; k + 8 <= std::min(a.size(), h); k += 8)
		store_unaligned(a.data() + k, reduce(add(load(r3.data() + k), load(product.data() + k)), kP));
	for (; k < a.size(); ++k) {
		const Word x = (k < h ? r3[k] : 0) + product[k];
		a[k] = x >= kP ? x - kP : x;
	}
}


}

inline int projection_log(size_t n) {
	if (n <= detail::kComposeBase) return Transform::kMinLog;
	return std::countr_zero(detail::projection_length(n));
}

inline size_t projection_scratch(size_t n) {
	using detail::Carve;
	if (n <= detail::kComposeBase) return 0;
	const size_t m = detail::projection_length(n);
	return Arena::footprint(detail::Tables::words(m)) + 2 * Carve::words(4 * m) + Carve::words(detail::projection_work(m));
}

namespace detail {
template<bool Weighted>
inline void project(const Transform& t, ConstWords g, ConstWords weights,
		Words a, Words scratch) {
	const size_t n = a.size();
	if constexpr (Weighted) {
		assert(n > kComposeBase && weights.size() == n);
	} else if (n <= kComposeBase) return projection_direct(g, a);
	const size_t m = projection_length(n);
	const int lg = std::countr_zero(m);
	const Tables tables(scratch.first(Tables::words(m)), m);
	Carve carve(scratch.subspan(Arena::footprint(Tables::words(m))));
	const Words q = carve.take(4 * m), p = carve.take(4 * m), w = carve.take(projection_work(m));
	if constexpr (Weighted) projection_weighted_first(t, tables, g, weights, q, p, scratch.last(6 * m));
	else projection_first_levels(t, tables, g, n, q, p);
	const ProjectionBottom bottom{&tables, p.data(), q.data(), p.data()};
	const Word half_scale = blocked_ntt::detail::power(kP / 2 + 1, 31);
	Word c = 1;
	for (int s = 2; s + 3 < lg; ++s) {
		const size_t stride = 2 * (m >> s), rows = size_t(1) << s;
		const size_t pad = size_t(std::countr_zero(stride / 16));
		if (s == 2) {
			forward_pruned(p, rows * stride, pad, tables, ForwardBottom{tables.roots});
			forward_pruned(q, (rows + 1) * stride, pad, tables, bottom);
		}
		if (s + 4 < lg) {
			const size_t block = stride / 16;
			const Word next =
				blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(c, c), half_scale), Word(block));
			const Word y_scale = kInverseScales[0][s + 4];
			Doubling<ForwardBottom>(p, block, tables, ForwardBottom{tables.roots}).run(y_scale, 0);
			Doubling<ProjectionBottom>(q, block, tables, bottom).run(y_scale, next + next >= kP ? next + next - kP : next + next);
			c = next;
		} else {
			const Word inverse_c = blocked_ntt::detail::power(c, kP - 2);
			const Word scale = blocked_ntt::detail::multiply_mod(graeffe_scale(lg + 1), blocked_ntt::detail::multiply_mod(inverse_c, inverse_c));
			inverse_pruned(q.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, q.data()}, scale);
			inverse_pruned(p.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, p.data()}, scale);
			next_level(q, stride / 2, 2 * rows);
		}
	}
	projection_last_levels(t, tables, q, p, w, a);
}

}

inline void power_projection(const Transform& t, ConstWords g, Words a,
		Words scratch) {
	detail::project<false>(t, g, {}, a, scratch);
}

inline void weighted_projection(const Transform& t, ConstWords g,
		ConstWords weights, Words a, Words scratch) {
	detail::project<true>(t, g, weights, a, scratch);
}

inline void native_weighted_projection(const mint *g, size_t ng, const mint *weights,
		std::vector<mint> &out, size_t n, size_t count) {
	ng = std::min(ng, n);
	const int lg = projection_log(n) + 1;
	const size_t scratch_words = projection_scratch(n) + Arena::footprint(6 * detail::projection_length(n));
	Arena arena(Transform::words(lg) + Arena::footprint(ng) + 2 * Arena::footprint(n) + Arena::footprint(scratch_words));
	const Transform t(arena, lg);
	auto a = arena.take(ng), b = arena.take(n), w = arena.take(n);
	for (size_t i = 0; i < ng; ++i) a[i] = i ? coefficient_value(g[i]) : 0;
	for (size_t i = 0; i < n; ++i) w[i] = std::min(weights[i].v, weights[i].v - mint::mod());
	weighted_projection(t, a, w, b, arena.take(scratch_words));
	append_fps_words<true>(out, b.data(), count);
}
inline void native_projection(const mint *g, size_t ng, std::vector<mint> &out, size_t n) {
	ng = std::min(ng, n);
	const int lg = projection_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(ng) + Arena::footprint(n) +
				Arena::footprint(projection_scratch(n)));
	const Transform t(arena, lg);
	auto a = arena.take(ng), b = arena.take(n);
	for (size_t i = 0; i < ng; ++i) a[i] = coefficient_value(g[i]);
	power_projection(t, a, b, arena.take(projection_scratch(n)));
	append_fps_words<false>(out, b.data(), b.size());
}

}
#pragma GCC pop_options
#endif
#endif
