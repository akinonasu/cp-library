#if defined(CP_FPS_HEADERS_USER_MACROS)
#define rep(...) cp_fps_rep_marker
#define per(...) cp_fps_per_marker
#define vec cp_fps_vec_marker
#define sz(...) cp_fps_sz_marker
#define all(...) cp_fps_all_marker
#define uint cp_fps_uint_marker
#endif

#if defined(CP_FPS_HEADERS_IO_FIRST)
#include "components/fast_io.hpp"
#endif

#if defined(CP_FPS_HEADERS_REVERSE)
#include "fps/taylor_shift.hpp"
#include "fps/power_projection.hpp"
#include "fps/polynomial_gcd.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/interpolate.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/compose.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/fps.hpp"
#else
#include "fps/fps.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/polynomial_gcd.hpp"
#include "fps/power_projection.hpp"
#include "fps/taylor_shift.hpp"
#endif
#include "fps/fps.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/polynomial_gcd.hpp"
#include "fps/power_projection.hpp"
#include "fps/taylor_shift.hpp"
#include "components/fast_io.hpp"

#if defined(CP_FPS_HEADERS_USER_MACROS)
#ifndef rep
#error "rep was removed"
#endif
#ifndef per
#error "per was removed"
#endif
#ifndef vec
#error "vec was removed"
#endif
#ifndef sz
#error "sz was removed"
#endif
#ifndef all
#error "all was removed"
#endif
#ifndef uint
#error "uint was removed"
#endif
#undef rep
#undef per
#undef vec
#undef sz
#undef all
#undef uint
#elif defined(TEMPLATE_HPP) || defined(rep) || defined(per) || defined(vec) || defined(all) || defined(sz) || defined(uint)
#error "FPS headers introduced template macros"
#endif

#if defined(CP_FPS_HEADERS_ACL)
#include <atcoder/modint>
#endif
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <string_view>
#include <type_traits>

void require(bool ok) {
	if (!ok) std::abort();
}

template<class T>
void check() {
	using F = FormalPowerSeries<T>;
	const F a{1, 2, 3}, b{1, 2, 1}, inner{0, 1, 1};
	F padded = b;
	padded.resize(129);
	auto inverse = padded.inv();
	auto one = (padded * inverse).pre(129);
	require(one[0] == T(1));
	for (int i = 1; i < 129; ++i) require(one[i] == T(0));
	require(padded.log().exp() == padded);
	require((padded.sqrt() * padded.sqrt()).pre(129) == padded);
	require(padded.pow(3) == (padded * padded * padded).pre(129));
	require(padded.div_series(padded) == one);
	require((padded / padded) == one);
	F compound = padded;
	compound /= padded;
	require(compound == one);
	require(padded.diff().integral() + T(1) == padded);
	require((padded << 3) >> 3 == padded);
	require(padded.eval(T(2)) == T(9));
	F modified = padded;
	modified.onemul(3, T(7));
	modified.onediv(3, T(7));
	require(modified == padded);
	require((a + T(4) - T(4)) == a && a * T(3) / T(3) == a);
	require(-(-a) == a && a + a - a == a);
	F base(std::vector<T>{1, 2, 3});
	base = std::vector<T>{1, 2};
	base = {1, 2, 3};
	require(base == a && F::multiply(a, a) == a * a);
	auto [quotient, remainder] = (a * b + F{7}).divmod(b);
	require(quotient == a && remainder == F{7});
	require((a * b + F{7}) % b == remainder);
	base = a * b + F{7};
	base %= b;
	require(base == remainder);
	require(compose(a, inner, 5) == F({1, 2, 5, 6, 3}));
	F identity(129);
	identity[1] = 1;
	auto reverse = compositional_inverse(inner, 129);
	require(compose(inner, reverse, 129) == identity);
	require(compose(reverse, inner, 129) == identity);
	const std::vector<T> xs{0, 1, 2};
	const std::vector<T> ys{1, 6, 17};
	require(multipoint_eval(a, xs) == ys && interpolate(xs, ys) == a);
	require(taylor_shift(a, T(1)) == F({6, 8, 3}));
	require(power_projection(inner, 2, 4) == F({0, 1, 1, 0}));
	require(power_projection(inner, F{0, 0, 1}, 4) == F({0, 1, 1, 0}));
	F common{1, 1}, left = common * F{2, 1}, right = common * F{3, 1};
	require(polynomial_gcd(left, right) == common);
	auto [gcd, x, y] = polynomial_extended_gcd(left, right);
	require(gcd == common);
	auto bezout = left * x + right * y;
	while (!bezout.empty() && bezout.back() == T(0)) bezout.pop_back();
	require(bezout == gcd);
	auto modular_inverse = polynomial_inv_mod(F{0, 1}, F{1, 1, 1});
	require(modular_inverse && *modular_inverse == F({T(-1), T(-1)}));
	T fibonacci = T(12586269025LL);
	require(bostan_mori(F{0, 1}, F{1, -1, -1}, 50) == fibonacci);
	require(linear_recurrence(std::vector<T>{0, 1}, std::vector<T>{1, 1}, 50) == fibonacci);
	require(compose(F{}, F{}, 0).empty());
	require(compositional_inverse(F{}, 0).empty());
	require(multipoint_eval(F{}, std::vector<T>{1, 2}) == std::vector<T>({0, 0}));
}

int main() {
	static_assert(std::is_same_v<fps, FormalPowerSeries<fps_mint>>);
	check<fps_mint>();
#if defined(CP_FPS_HEADERS_ACL)
	check<atcoder::modint998244353>();
#endif
	FastInput input(std::string_view("3 1 2 3"));
	int n = 0;
	require(input.read(n) && n == 3);
	fps f(n);
	for (auto &coefficient : f) {
		unsigned value = 0;
		require(input.read(value));
		coefficient = fps_mint::raw(value);
	}
	FILE *file = std::tmpfile();
	require(file != nullptr);
	{
		FastOutput output(file);
		output.println(f.eval(2).val());
		require(output.flush());
	}
	std::rewind(file);
	char bytes[4]{};
	require(std::fread(bytes, 1, 3, file) == 3 && std::string_view(bytes, 3) == "17\n");
	std::fclose(file);
}
