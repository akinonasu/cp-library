#ifndef CP_FPS_MULTIPOINT_EVAL_HPP
#define CP_FPS_MULTIPOINT_EVAL_HPP

#include "fps.hpp"
#include "../components/fps_product_tree.hpp"
#include "../components/fps_direct_eval.hpp"
#include "../components/fps_multipoint.hpp"

#if CP_GCC_X86
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

#ifndef CP_FPS_EVAL_API
#define CP_FPS_EVAL_API 1
template<class T>
std::vector<T> multipoint_eval(const FormalPowerSeries<T> &f, const std::vector<T> &xs) {
	using W = FormalPowerSeries<mint>;
	using base = std::vector<T>;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, mint>) return O::export_work(W(multipoint_eval<mint>(O::import_work(f), O::import_work(xs))));
	if (xs.empty()) return {};
	if (f.size() <= 32 || xs.size() <= 32) {
#if CP_X86
		if constexpr (std::is_same_v<T, mint>) {
			if (((xs.size() >= 8 && f.size() >= 8 && (xs.size() >= 32 || f.size() >= 32)) ||
				 (xs.size() >= 4096 && f.size() >= 4)) && fps_detail::NTT::vectorized()) {
				base r(xs.size());
				fps_detail::evaluate_direct(O::words(r), O::words(f), f.size(), O::words(xs), xs.size());
				return r;
			}
		}
#endif
		base r;
		r.reserve(xs.size());
		for (T x : xs) r.push_back(f.eval(x));
		return r;
	}
#if CP_X86
	if constexpr (std::is_same_v<T, mint>) {
		if (f.size() * xs.size() <= (std::size_t(1) << 14) && fps_detail::NTT::vectorized()) {
			base result(xs.size());
			fps_detail::evaluate_direct(O::words(result), O::words(f), f.size(), O::words(xs), xs.size());
			return result;
		}
	}
#endif
#if CP_GCC_LINUX64
	if constexpr (std::is_same_v<T, mint>) {
		if (std::max(f.size(), xs.size()) <= (1 << 23) && fps_detail::NTT::vectorized()) {
			if (xs.size() <= 512 && f.size() > 4 * xs.size()) {
				int size = 1;
				const auto tree = fps_detail::ProductTree<T>::build(xs, size, 8, false);
				return fps_detail::evaluation_poly::native_evaluate(f.divmod(tree[1]).second, xs);
			}
			return fps_detail::evaluation_poly::native_evaluate(f, xs);
		}
	}
#endif

	return fps_detail::ProductTree<T>(xs).evaluate(f);
}

#endif

#if CP_GCC_X86
#pragma GCC pop_options
#endif

#endif
