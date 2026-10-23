#ifndef CP_COMPONENTS_FPS_INTERPOLATION_HPP
#define CP_COMPONENTS_FPS_INTERPOLATION_HPP

#include "fps_evaluation_common.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::evaluation_poly {
namespace detail {
template <size_t K>
[[gnu::always_inline]] inline Vec cross_sum(const Vec (&even)[K], const Vec (&odd)[K], size_t terms) {
	if constexpr (K == 1) {
		return montgomery_sums(even[0], odd[0], terms);
	} else {
		if (terms <= 17) return montgomery_sums(_mm256_add_epi64(even[0], even[1]), _mm256_add_epi64(odd[0], odd[1]), terms);
		return montgomery_sums(_mm256_add_epi64(fold(even[0]), fold(even[1])), _mm256_add_epi64(fold(odd[0]), fold(odd[1])), 0);
	}
}
[[gnu::noinline]] inline void cross_lanes(const Word* a, size_t n, const Word* b, size_t k,
										const Word* c, const Word* d, Word* out) {
	for (size_t t = 0; t < n + k; ++t) {
		Vec even[2] = {}, odd[2] = {};
		const size_t first[2] = {t > k ? t - k : 0, t > n ? t - n : 0}, last[2] = {std::min(t, n - 1), std::min(t, k - 1)};
		for (size_t i = first[0]; i <= last[0]; ++i) {
			const Vec x = load(a + 8 * i), y = load(d + 8 * (t - i));
			even[0] = _mm256_add_epi64(even[0], _mm256_mul_epu32(x, y));
			odd[0] = _mm256_add_epi64(odd[0], _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
		}
		for (size_t i = first[1]; i <= last[1]; ++i) {
			const Vec x = load(b + 8 * i), y = load(c + 8 * (t - i));
			even[1] = _mm256_add_epi64(even[1], _mm256_mul_epu32(x, y));
			odd[1] = _mm256_add_epi64(odd[1], _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(y, 32)));
		}
		store(out + 8 * t, cross_sum(even, odd, (last[0] + 1 - first[0]) + (last[1] + 1 - first[1])));
	}
}
template <size_t D, size_t B>
[[gnu::always_inline]] inline void cross_lanes(const Word* a, const Word* b, const Word* c,
											const Word* d, Word* out) {
	constexpr size_t kPad = B - 1, kSums = 2 * D > 17 ? 2 : 1;
	Vec x[D + 2 * kPad], xo[D + 2 * kPad], y[D + 2 * kPad], yo[D + 2 * kPad], co[D + 1], dq[D + 1];
	for (size_t i = 0; i < kPad; ++i)
		x[i] = xo[i] = y[i] = yo[i] = x[D + kPad + i] = xo[D + kPad + i] = y[D + kPad + i] = yo[D + kPad + i] = _mm256_setzero_si256();
#pragma GCC unroll 16
	for (size_t i = 0; i < D; ++i) {
		x[kPad + i] = load(a + 8 * i), xo[kPad + i] = _mm256_srli_epi64(x[kPad + i], 32);
		y[kPad + i] = load(b + 8 * i), yo[kPad + i] = _mm256_srli_epi64(y[kPad + i], 32);
	}
#pragma GCC unroll 17
	for (size_t j = 0; j <= D; ++j) co[j] = _mm256_srli_epi64(load(c + 8 * j), 32), dq[j] = _mm256_srli_epi64(load(d + 8 * j), 32);
	for (size_t t0 = 0; t0 < 2 * D; t0 += B) {
		Vec even[B][kSums] = {}, odd[B][kSums] = {};
		const size_t first = t0 + 1 > D ? t0 + 1 - D : 0, last = std::min(D, t0 + B - 1);
		for (size_t j = first; j <= last; ++j) {
			const Vec qd = load(d + 8 * j), qc = load(c + 8 * j);
#pragma GCC unroll 8
			for (size_t s = 0; s < B; ++s) {
				const size_t i = kPad + t0 + s - j;
				even[s][0] = _mm256_add_epi64(even[s][0], _mm256_mul_epu32(x[i], qd));
				odd[s][0] = _mm256_add_epi64(odd[s][0], _mm256_mul_epu32(xo[i], dq[j]));
				even[s][kSums - 1] = _mm256_add_epi64(even[s][kSums - 1], _mm256_mul_epu32(y[i], qc));
				odd[s][kSums - 1] = _mm256_add_epi64(odd[s][kSums - 1], _mm256_mul_epu32(yo[i], co[j]));
			}
		}
#pragma GCC unroll 8
		for (size_t s = 0; s < B; ++s) {
			const size_t t = t0 + s, terms = 2 * (std::min(t, D - 1) + 1 - (t > D ? t - D : 0));
			if (t < 2 * D) store(out + 8 * t, cross_sum(even[s], odd[s], terms));
		}
	}
}
template <size_t D>
[[gnu::always_inline]] inline void cross_lanes(const Word* a, const Word* b, const Word* c,
											const Word* d, Word* out) {
	static_assert(2 * D <= 17);
	Vec x[D], xo[D], y[D], yo[D], p[D + 1], po[D + 1], q[D + 1], qo[D + 1];
#pragma GCC unroll 8
	for (size_t i = 0; i < D; ++i) {
		x[i] = load(a + 8 * i), xo[i] = _mm256_srli_epi64(x[i], 32);
		y[i] = load(b + 8 * i), yo[i] = _mm256_srli_epi64(y[i], 32);
	}
#pragma GCC unroll 9
	for (size_t j = 0; j <= D; ++j) {
		p[j] = load(c + 8 * j), po[j] = _mm256_srli_epi64(p[j], 32);
		q[j] = load(d + 8 * j), qo[j] = _mm256_srli_epi64(q[j], 32);
	}
#pragma GCC unroll 16
	for (size_t t = 0; t < 2 * D; ++t) {
		const size_t first = t > D ? t - D : 0, last = std::min(t, D - 1);
		Vec even = _mm256_setzero_si256(), odd = _mm256_setzero_si256();
#pragma GCC unroll 8
		for (size_t i = first; i <= last; ++i) {
			even = _mm256_add_epi64(even, _mm256_add_epi64(_mm256_mul_epu32(x[i], q[t - i]), _mm256_mul_epu32(y[i], p[t - i])));
			odd = _mm256_add_epi64(odd, _mm256_add_epi64(_mm256_mul_epu32(xo[i], qo[t - i]), _mm256_mul_epu32(yo[i], po[t - i])));
		}
		store(out + 8 * t, montgomery_sums(even, odd, 2 * (last + 1 - first)));
	}
}
template <size_t D>
void cross_level(const Word* sums, const Word* products, Word* out, size_t nodes) {
	for (size_t i = 0; i < nodes; ++i) {
		const Word *r = sums + 8 * D * (2 * i), *q = products + 8 * (D + 1) * (2 * i);
		if constexpr (D <= 8) cross_lanes<D>(r, r + 8 * D, q, q + 8 * (D + 1), out + 16 * D * i);
		else cross_lanes<D, 2>(r, r + 8 * D, q, q + 8 * (D + 1), out + 16 * D * i);
	}
}
inline Vec inverse_montgomery(Vec x) {
	Vec result = broadcast(kR), power = x;
	for (Word e = kP - 2; e; e >>= 1) {
		if (e & 1) result = montgomery(result, power);
		power = montgomery(power, power);
	}
	return result;
}
class BaseInterpolation : private BaseTree {
public:
	BaseInterpolation(const PointSlots& slots, const Word* y, size_t m) : BaseTree(slots, storage_), y_(y), m_(m) {}
	const Word* run(size_t lo, size_t hi, const Word* w) {
		const size_t count = hi - lo;
		lo_ = lo;
		if (count == 1) {
			store(values_, load(w));
			weights(1);
			return values_;
		}
		if (std::has_single_bit(count)) return run_levels(count, w);
		free_ = memory_;
		build(1, lo, hi);
		descend(1, lo, hi, w);
		weights(count);
		ascend(1, lo, hi, sums_);
		return sums_;
	}
private:
	const Word* run_levels(size_t count, const Word* w) {
		const Word* products[6];
		Word* at = build_levels(lo_, count, products);
		Word* const buffers[2] = {at, at + 8 * count};
		descend_levels(lo_, count, w, products, buffers);
		weights(count);
		const Word* sums = values_;
		for (size_t d = 1, j = 0; d < count; d *= 2, ++j) {
			Word* const out = 2 * d == count ? sums_ : buffers[j & 1];
			with_degree(d, [&]<size_t D>() { cross_level<D>(sums, products[j], out, count / (2 * D)); });
			sums = out;
		}
		return sums_;
	}
	void ascend(size_t v, size_t lo, size_t hi, Word* out) {
		if (hi - lo == 1) return store(out, load(values_ + 8 * (lo - lo_)));
		const size_t mid = lo + (hi - lo) / 2;
		Word* const left = free_;
		Word* const right = left + 8 * (mid - lo);
		free_ = right + 8 * (hi - mid);
		ascend(2 * v, lo, mid, left);
		ascend(2 * v + 1, mid, hi, right);
		cross_lanes(left, mid - lo, right, hi - mid, product_[2 * v], product_[2 * v + 1], out);
		free_ = left;
	}
	void weights(size_t count) {
		constexpr size_t kChains = 4;
		const Vec iota = _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7), one = broadcast(kR);
		const size_t groups = (count + kChains - 1) / kChains;
		Vec prefix[kMaxSlots];
		for (size_t g = 0; g < groups; ++g) {
#pragma GCC unroll 4
			for (size_t c = 0; c < kChains; ++c) {
				const size_t k = kChains * g + c, first = 8 * (lo_ + k);
				Vec x = one;
				if (k < count) {
					x = load(values_ + 8 * k);
					if (first + 8 > m_) {
						x = _mm256_blendv_epi8(one, x, _mm256_cmpgt_epi32(broadcast(Word(m_ - first)), iota));
						store(values_ + 8 * k, x);
					}
				}
				prefix[k] = g ? montgomery(prefix[k - kChains], x) : x;
			}
		}
		const Vec* const last = prefix + kChains * (groups - 1);
		const Vec p01 = montgomery(last[0], last[1]), p23 = montgomery(last[2], last[3]);
		const Vec inverse = inverse_montgomery(montgomery(p01, p23));
		const Vec i01 = montgomery(inverse, p23), i23 = montgomery(inverse, p01);
		Vec e[kChains] = {montgomery(i01, last[1]), montgomery(i01, last[0]), montgomery(i23, last[3]), montgomery(i23, last[2])};
		for (size_t g = groups; g-- > 0;) {
#pragma GCC unroll 4
			for (size_t c = 0; c < kChains; ++c) {
				const size_t k = kChains * g + c;
				if (k >= count) continue;
				const Vec x = load(values_ + 8 * k), reciprocal = g ? montgomery(e[c], prefix[k - kChains]) : e[c];
				e[c] = montgomery(e[c], x);
				store(values_ + 8 * k, canonical(montgomery(reciprocal, load(y_ + 8 * (lo_ + k)))));
			}
		}
	}
	static constexpr size_t kMaxSlots = LaneLayout::kBase;
	alignas(32) Word storage_[8 * kMaxSlots];
	alignas(32) Word sums_[8 * kMaxSlots];
	const Word* y_;
	size_t m_;
};
}
class InterpolationTree : public PointTree {
public:
	InterpolationTree(Arena& arena, ConstSpan points) : PointTree(arena, points), m_(points.size()) {}
	static size_t scratch_words(size_t m) {
		const size_t m8 = (m + 7) / 8 * 8, lane_length = LaneLayout::length(Word(m8 / 8));
		const size_t stride = std::max<size_t>(8, lane_length), top_length = StandardLayout::length(Word(m8));
		return descend_words(m) + 10 * Arena::footprint(8 * stride) + 6 * Arena::footprint(top_length) + 4096;
	}
	void interpolate(ConstSpan w, ConstSpan y, Span r,
					Span scratch) {
		using namespace detail;
		Stack stack(scratch);
		const size_t lane_length = LaneLayout::length(Word(count_)), stride = std::max<size_t>(8, lane_length);
		const LaneState root = lane_state(w, kR, stack);
		BaseInterpolation base(slots_, y.data(), m_);
		Word* const sums = stack.take(8 * stride);
		if (count_ <= LaneLayout::kBase) {
			std::copy_n(base.run(0, count_, root.state + 8 * (lane_length - count_)), 8 * count_, sums);
		} else {
			lanes(0, count_, root.state, root.u, stack, base, sums, lane_length);
			t_.inverse({sums, 8 * lane_length}, {sums, 8 * lane_length});
		}
		Word* const columns = root.state;
		for (size_t j = 0; j < count_; j += 8) {
			Vec v[8];
			for (size_t i = 0; i < 8; ++i) v[i] = load(sums + 8 * (j + i));
			transpose8(v);
			for (size_t l = 0; l < 8; ++l) store(columns + l * stride + j, v[l]);
		}
		const size_t length = StandardLayout::length(Word(m8_));
		top(0, 8, columns, stride, stack, r.data(), length);
		t_.inverse(r.first(length), r.first(length));
	}
private:
	void lanes(size_t lo, size_t hi, const Word* s, Word u, detail::Stack& stack,
			detail::BaseInterpolation& base, Word* out, size_t out_length) {
		using namespace detail;
		const LaneTree& tree = *lanes_;
		const size_t length = LaneLayout::length(Word(tree.degree(lo, hi))), words = 8 * length;
		const size_t mid = tree.split(lo, hi), mark = stack.mark();
		const Word* const stored[2] = {lane_store_.pair(mid), lane_store_.pair(mid) + Arena::footprint(words)};
		Word* const sums[2] = {out_length >= 2 * length ? out + words : out, stack.take(words)};
		const size_t sides[2][2] = {{lo, mid}, {mid, hi}};
		for (int side = 0; side < 2; ++side) {
			const size_t xlo = sides[side][0], xhi = sides[side][1], degree = tree.degree(xlo, xhi);
			const size_t inner = 8 * LaneLayout::length(Word(degree)), inner_mark = stack.mark();
			Word* const p = stack.take(words);
			TreeTransform::pointwise_products(s, stored[1 - side], {p, words});
			if (xhi - xlo == 1 || degree <= LaneLayout::kBase) {
				coefficients(p, words, u);
				const Word* const sum = base.run(xlo, xhi, p + words - 8 * degree);
				t_.forward({sum, 8 * degree}, 0, {sums[side], words});
			} else if (2 * inner == words) {
				halve(p, words);
				lanes(xlo, xhi, p, half(u), stack, base, sums[side], length);
			} else {
				coefficients(p, words, u);
				Word* const q = stack.take(inner);
				t_.forward({p + words - inner, inner}, 0, {q, inner});
				lanes(xlo, xhi, q, 1, stack, base, sums[side], length);
			}
			stack.release(inner_mark);
		}
		TreeTransform::pointwise_product_sums(sums[0], stored[1], sums[1], stored[0], {out, words});
		double_to(out, words, out_length / length, stack);
		stack.release(mark);
	}
	void top(size_t lo, size_t hi, const Word* columns, size_t stride, detail::Stack& stack,
			Word* out, size_t out_length) {
		const TopTree& tree = *top_;
		if (hi - lo == 1) return t_.forward({columns + lo * stride, count_}, 0, {out, out_length});
		const size_t length = StandardLayout::length(tree.degree(lo, hi)), mid = tree.split(lo, hi), mark = stack.mark();
		const Word* const stored[2] = {top_store_.pair(mid), top_store_.pair(mid) + Arena::footprint(length)};
		Word* const sums[2] = {out_length >= 2 * length ? out + length : out, stack.take(length)};
		top(lo, mid, columns, stride, stack, sums[0], length);
		top(mid, hi, columns, stride, stack, sums[1], length);
		t_.leaf_product_sums(sums[0], stored[1], sums[1], stored[0], {out, length});
		double_to(out, length, out_length / length, stack);
		stack.release(mark);
	}
	void double_to(Word* out, size_t words, size_t factor, detail::Stack& stack) const {
		if (factor == 1) return;
		Word* const c = stack.take(words);
		t_.inverse({out, words}, {c, words});
		for (size_t n = words; n < factor * words; n *= 2) t_.forward_upper({c, words}, 0, {out + n, n});
	}
	size_t m_;
};
inline size_t interpolate_words(size_t m) {
	const size_t m8 = (m + 7) / 8 * 8;
	const detail::Division sizes(m, m8);
	return PointTree::words(m) + Transform::words(sizes.log()) + Arena::footprint(std::max(m + 1, m8)) +
		Arena::footprint(StandardLayout::length(Word(m8))) +
		Arena::footprint(std::max(sizes.scratch_words(), InterpolationTree::scratch_words(m)));
}
inline void interpolate(Arena& arena, ConstSpan points, ConstSpan values,
						Span c) {
	const size_t m = points.size();
	InterpolationTree tree(arena, points);
	const size_t m8 = tree.size(), length = StandardLayout::length(Word(m8));
	const Span y = arena.take(std::max(m + 1, m8));
	std::reverse_copy(tree.product().begin(), tree.product().begin() + std::ptrdiff_t(m + 1), y.begin());
	derivative(y.first(m + 1), y.first(m));
	const detail::Division sizes(m, m8);
	const Transform t(arena, sizes.log());
	const Span window = arena.take(length);
	const Span scratch = arena.take(std::max(sizes.scratch_words(), InterpolationTree::scratch_words(m)));
	detail::divide(t, sizes, y.first(m), tree.product(), tree.product_transform(), window.first(m8), scratch);
	std::copy(values.begin(), values.end(), y.begin());
	y[m] = 0;
	tree.interpolate(window.first(m8), y.first(m8), window, scratch);
	std::reverse_copy(window.begin(), window.begin() + std::ptrdiff_t(m), c.begin());
}
inline std::vector<mint> native_interpolate(std::span<const mint> points, std::span<const mint> values) {
	const size_t n = points.size();
	Arena arena(interpolate_words(n) + 3 * Arena::footprint(n));
	auto x = arena.take(n), y = arena.take(n), c = arena.take(n);
	import_words(points, x);
	import_montgomery(values, y);
	interpolate(arena, x, y, c);
	return export_words(c);
}
}

#pragma GCC pop_options
#endif
#endif
