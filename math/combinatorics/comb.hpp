#ifndef COMB_998244353_HPP
#define COMB_998244353_HPP

#include <cassert>

#include "../mint.hpp"
#include "../../components/fps_factorials.hpp"

#ifndef COMB_MAX
#define COMB_MAX 500000 + 2022
#endif
constexpr int COMB_INF = static_cast<int>(COMB_MAX);
static_assert(COMB_INF >= 0 && COMB_INF < mint::mod());
inline constinit mint fact[COMB_INF + 1], finv[COMB_INF + 1];
inline void init_comb() {
	fps_detail::factorials(fact, finv, COMB_INF + 1);
}
namespace comb_detail {
struct Initializer {
	Initializer() {
		static const bool initialized = [] { init_comb(); return true; }();
		(void)initialized;
	}
};
[[maybe_unused]] static const Initializer initializer;
}
inline mint comb(int n, int k) {
	if (n < 0 || k < 0 || k > n) return 0;
	return fact[n] * finv[k] * finv[n - k];
}

#endif
