#ifndef FPS_SPARSE_TERMS_HPP
#define FPS_SPARSE_TERMS_HPP

#include "../math/mint.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <utility>
#include <vector>

namespace fps_detail {
struct SparseTerms {
	using Series = std::vector<mint>;
	using Terms = std::vector<std::pair<int, mint>>;
	enum class Kind { inverse, logarithm, exponential, power };
	static Terms collect(const mint *f, int size, int n, int limit = 12) {
		Terms terms;
		for (int i = 1; i < std::min(size, n) && int(terms.size()) <= limit; ++i)
			if (f[i] != mint(0)) terms.emplace_back(i, f[i]);
		return terms;
	}
};
}
#endif
