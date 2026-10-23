#include "fps/fps_light.hpp"
#include "fps/fps_light_algorithms.hpp"
#include "fps/compose_light.hpp"
#include "fps/compositional_inverse_light.hpp"
#include "fps/polynomial_gcd_light.hpp"
#include "fps/power_projection_light.hpp"
#include "fps/rational_fps_coefficients_light.hpp"
#include "components/fps_light_ntt.hpp"
#include "components/fps_light_product_tree.hpp"
#include "components/fast_io.hpp"
#include <cstdlib>

void require(bool ok) {
	if (!ok) std::abort();
}

int main() {
	namespace A = light_fps_algorithms;
	using F = light_fps;
	const F f{1, 2, 1}, g{0, 1, 1};
	F padded = f;
	padded.resize(64);
	F one = (padded * padded.inv()).pre(64);
	one.shrink();
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
	require(A::taylor_shift(f, mint(1)) == F({4, 4, 1}));
	std::vector<mint> xs{0, 1, 2}, ys{1, 4, 9};
	require(A::multipoint_eval(f, xs) == ys);
	require(A::interpolate(xs, ys) == f);
	require(A::bostan_mori(F{1}, F{1, -1}, 100) == mint(1));
	require(A::rational_fps_coefficients(F{1}, F{1, -1, -1}, 10, 15) == F({89, 144, 233, 377, 610}));
	require(A::linear_recurrence(std::vector<mint>{0, 1}, std::vector<mint>{1, 1}, 10) == mint(55));
	F inverse = A::compositional_inverse(g, 33);
	F identity = A::compose(g, inverse, 33);
	identity.shrink();
	require(identity == F({0, 1}));
	require(A::power_projection(g, 2) == F({0, 1, 1}));
	require(A::power_projection(g, F{1, 2, 3}) == F({1, 5, 3}));
	require(A::polynomial_gcd(f, F{1, 1}) == F({1, 1}));
	auto extended = A::polynomial_extended_gcd(f, F{1, 1});
	F bezout = extended.x * f + extended.y * F{1, 1};
	bezout.shrink();
	require(bezout == extended.gcd);
	const F modulus{1, 0, 1};
	auto modular_inverse = A::polynomial_inv_mod(f, modulus);
	require(modular_inverse && (f * *modular_inverse).divmod(modulus).second == F{1});
	FastInput input(std::string_view("123"));
	int value;
	require(input.read(value) && value == 123);
	FastOutput output;
	output.flush();
}
