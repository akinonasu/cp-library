#ifndef CP_COMPONENTS_FPS_MULTIPOINT_HPP
#define CP_COMPONENTS_FPS_MULTIPOINT_HPP

#include "fps_evaluation_common.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail::evaluation_poly {
namespace detail {
class BaseDescent : private BaseTree {
public:
	BaseDescent(const PointSlots& slots, Word* values) : BaseTree(slots, values) {}
	void run(size_t lo, size_t hi, const Word* w) {
		const size_t count = hi - lo;
		if (count == 1) return store(values_ + 8 * lo, load(w));
		if (std::has_single_bit(count)) {
			const Word* products[6];
			Word* at = build_levels(lo, count, products);
			Word* const windows[2] = {at, at + 8 * count};
			return descend_levels(lo, count, w, products, windows);
		}
		free_ = memory_;
		build(1, lo, hi);
		descend(1, lo, hi, w);
	}
};
}
class EvaluationTree : public PointTree {
public:
	using PointTree::PointTree;
	void descend(ConstSpan w, Span values, Word c, Span scratch) {
		using namespace detail;
		Stack stack(scratch);
		const size_t lane_length = LaneLayout::length(Word(count_));
		const LaneState root = lane_state(w, c, stack);
		BaseDescent base(slots_, values.data());
		if (count_ <= LaneLayout::kBase) return base.run(0, count_, root.state + 8 * (lane_length - count_));
		descend_lanes(0, count_, root.state, root.u, stack, base);
	}
private:
	void descend_lanes(size_t lo, size_t hi, const Word* s, Word u, detail::Stack& stack,
					detail::BaseDescent& base) {
		descend_tree<LaneLayout>(*lanes_, lane_store_, lo, hi, s, u, stack,
			[&](size_t xlo, size_t xhi, size_t degree, Word* p, size_t words, Word scale) {
				if (xhi - xlo != 1 && degree > LaneLayout::kBase) return false;
				coefficients(p, words, scale);
				base.run(xlo, xhi, p + words - 8 * degree);
				return true;
			});
	}
};
namespace detail {
inline constexpr size_t kDirectEvaluation = size_t(1) << 14;
inline void evaluate_direct(ConstSpan f, ConstSpan points, Span values) {
	constexpr size_t kChains = 4;
	const Factor to_r(kR);
	for (size_t i = 0; i < points.size(); i += 8 * kChains) {
		alignas(32) Word x[8 * kChains] = {}, v[8 * kChains];
		const size_t here = std::min<size_t>(8 * kChains, points.size() - i);
		std::copy_n(points.begin() + std::ptrdiff_t(i), here, x);
		Vec xm[kChains], acc[kChains];
		for (size_t c = 0; c < kChains; ++c) xm[c] = reduce(times(load(x + 8 * c), to_r), kP), acc[c] = _mm256_setzero_si256();
		for (size_t j = f.size(); j-- > 0;) {
			const Vec fj = broadcast(f[j]);
			for (size_t c = 0; c < kChains; ++c) acc[c] = reduce(add(montgomery(acc[c], xm[c]), fj), 2 * kP);
		}
		for (size_t c = 0; c < kChains; ++c) store(v + 8 * c, reduce(acc[c], kP));
		std::copy_n(v, here, values.begin() + std::ptrdiff_t(i));
	}
}
inline size_t evaluate_tree_words(size_t n, size_t m) {
	const size_t m8 = (m + 7) / 8 * 8;
	const Division sizes(n, m8);
	return PointTree::words(m) + Transform::words(sizes.log()) + Arena::footprint(sizes.big) +
		Arena::footprint(std::max(sizes.scratch_words(), PointTree::descend_words(m))) + Arena::footprint(m8);
}
inline void evaluate_tree(Arena& arena, ConstSpan f, ConstSpan points,
						Span values) {
	const size_t m = points.size();
	EvaluationTree tree(arena, points);
	const size_t m8 = tree.size();
	const Division sizes(f.size(), m8);
	const Transform t(arena, sizes.log());
	const Span d = arena.take(sizes.big);
	const Span scratch = arena.take(std::max(sizes.scratch_words(), PointTree::descend_words(m)));
	divide(t, sizes, f, tree.product(), tree.product_transform(), d, scratch);
	const bool direct = m8 == m && reinterpret_cast<std::uintptr_t>(values.data()) % 32 == 0;
	const Span all = direct ? values : arena.take(m8);
	tree.descend(d.subspan(sizes.big - m8, m8), all, kR, scratch);
	if (!direct) std::copy_n(all.begin(), m, values.begin());
}
}
inline size_t evaluate_words(size_t n, size_t m) {
	return n * m <= detail::kDirectEvaluation ? 0 : detail::evaluate_tree_words(n, m);
}
inline void evaluate(Arena& arena, ConstSpan f, ConstSpan points,
					Span values) {
	if (f.size() * points.size() <= detail::kDirectEvaluation) return detail::evaluate_direct(f, points, values);
	detail::evaluate_tree(arena, f, points, values);
}
inline std::vector<mint> native_evaluate(std::span<const mint> f, std::span<const mint> points) {
	Arena arena(evaluate_words(f.size(), points.size()) + Arena::footprint(f.size()) + 2 * Arena::footprint(points.size()));
	auto a = arena.take(f.size()), x = arena.take(points.size()), y = arena.take(points.size());
	import_montgomery(f, a);
	import_words(points, x);
	evaluate(arena, a, x, y);
	return export_words(y);
}

}

#pragma GCC pop_options
#endif
#endif
