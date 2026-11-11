#ifndef TEST_FPS_PUBLIC_API_HPP
#define TEST_FPS_PUBLIC_API_HPP

#include "components/fast_io.hpp"
#include <cstdlib>

namespace fps_public_api_test {
inline void require(bool ok) {
	if (!ok) std::abort();
}

inline void check() {
	using F = fps;
	auto trim = [](F &f) { while (!f.empty() && f.back() == mint(0)) f.pop_back(); };
	const F f{1, 2, 1}, g{0, 1, 1};
	F padded = f;
	padded.resize(64);
	F one = (padded * padded.inv()).pre(64);
	trim(one);
	require(one == F{1});
	require(padded.log().exp() == padded);
	require((padded * padded).sqrt(64) == padded);
	require(f.pow(2) == F({1, 4, 6}));
	require((f * g).divmod(g).first == f);
	require(f.div_series(F{1}, 3) == f);
	require(f.diff().integral() == F({0, 2, 1}));
	require(f.eval(2) == mint(9));
	F shifted = f;
	shifted.onemul(1, 2);
	shifted.onediv(1, 2);
	require(shifted == f);
	require(taylor_shift(f, mint(1)) == F({4, 4, 1}));
	std::vector<mint> xs{0, 1, 2}, ys{1, 4, 9};
	require(multipoint_eval(f, xs) == ys);
	require(interpolate(xs, ys) == f);
	require(bostan_mori(F{1}, F{1, -1}, 100) == mint(1));
	require(rational_fps_coefficients(F{1}, F{1, -1, -1}, 10, 15) == F({89, 144, 233, 377, 610}));
	require(linear_recurrence(std::vector<mint>{0, 1}, std::vector<mint>{1, 1}, 10) == mint(55));
	F inverse = compositional_inverse(g, 33);
	F identity = compose(g, inverse, 33);
	trim(identity);
	require(identity == F({0, 1}));
	require(power_projection(g, 2) == F({0, 1, 1}));
	require(power_projection(g, F{1, 2, 3}) == F({1, 5, 3}));
	require(polynomial_gcd(f, F{1, 1}) == F({1, 1}));
	PolynomialExtendedGCD<mint> extended = polynomial_extended_gcd(f, F{1, 1});
	F bezout = extended.x * f + extended.y * F{1, 1};
	trim(bezout);
	require(bezout == extended.gcd);
	const F modulus{1, 0, 1};
	auto modular_inverse = polynomial_inv_mod(f, modulus);
	require(modular_inverse && (f * *modular_inverse).divmod(modulus).second == F{1});
	FastInput input(std::string_view("123"));
	int value;
	require(input.read(value) && value == 123);
	FastOutput output;
	output.flush();
}
}

#endif
