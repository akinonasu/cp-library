#ifndef FPS_SPARSE_SCALAR_HPP
#define FPS_SPARSE_SCALAR_HPP

#include "fps_sparse_core.hpp"

namespace fps_detail {
struct ScalarSparseSeries : SparseBase<ScalarSparseSeries> {
	template <bool exponential = false>
	static void geometric(Series &out, mint initial, mint ratio, const mint *inv = nullptr) {
		for (int i = 0; i < int(out.size()); ++i) {
			out[i] = initial;
			initial *= ratio;
			if constexpr (exponential) if (i + 1 < int(out.size())) initial *= inv[i + 1];
		}
	}
	static void scale(Series &out, const mint *inv) {
		for (int i = 1; i < int(out.size()); ++i) out[i] *= inv[i];
	}
	template <Kind kind>
	static Series solve(const Terms &terms, int n, mint initial, mint exponent, const mint *inv) {
		Series out(n);
		out[0] = initial;
		for (int i = 1; i < n; ++i) {
			mint sum = 0;
			for (auto [j, x] : terms) {
				if (j > i) break;
				if constexpr (kind == Kind::power) sum += ((exponent + mint(1)) * mint(j) - mint(i)) * x * out[i - j];
				if constexpr (kind == Kind::exponential) sum += mint(j) * x * out[i - j];
				if constexpr (kind == Kind::inverse || kind == Kind::logarithm) sum -= x * out[i - j];
				if constexpr (kind == Kind::logarithm) if (j == i) sum += mint(j) * x;
			}
			if constexpr (kind == Kind::power || kind == Kind::exponential) sum *= inv[i];
			out[i] = sum;
		}
		if constexpr (kind == Kind::logarithm) scale(out, inv);
		return out;
	}
};
}
#endif
