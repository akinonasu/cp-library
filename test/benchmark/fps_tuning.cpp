#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#include "fps/polynomial_gcd.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc < 3 || argc > 4) return 1;
	std::string op = argv[1];
	int n = std::atoi(argv[2]), repeat = argc == 4 ? std::atoi(argv[3]) : 1;
	std::mt19937 rng(20261018);
	fps f(n), g(n), result;
	std::vector<mint> points(n);
	for (int i = 0; i < n; ++i) { f[i] = rng(); g[i] = rng(); points[i] = i + 1; }
	if (n) f[0] = 1, g[0] = 1;
	if (op == "exp") f[0] = 0;
	if (op == "compose" || op == "power_projection") g[0] = 0;
	if (op == "compositional_inverse") f[0] = 0, f[1] = 1;
	if (op == "divmod") { g.resize(std::max(1, n / 2)); g.back() = 1; }
	if (op == "gcd" || op == "inv_mod") f.back() = 1, g.back() = 1;
	unsigned long long checksum = 0;
	double elapsed = 0;
	for (int i = 0; i < repeat; ++i) {
		auto start = std::chrono::steady_clock::now();
		if (op == "multiply") result = fps::multiply(f, g);
		else if (op == "square") result = fps::multiply(f, f);
		else if (op == "inv") result = f.inv();
		else if (op == "log") result = f.log();
		else if (op == "exp") result = f.exp();
		else if (op == "pow") result = f.pow(1000000000000000000LL);
		else if (op == "sqrt") result = f.sqrt();
		else if (op == "div_series") result = f.div_series(g);
		else if (op == "divmod") { auto p = f.divmod(g); result = std::move(p.first); result.insert(result.end(), p.second.begin(), p.second.end()); }
		else if (op == "compose") result = compose(f, g);
		else if (op == "compositional_inverse") result = compositional_inverse(f);
		else if (op == "power_projection") result = power_projection(g, f, n);
		else if (op == "bostan_mori") result = {bostan_mori(f, g, 1000000000000000000LL)};
		else if (op == "linear_recurrence") result = {linear_recurrence<mint>(f, g, 1000000000000000000LL)};
		else if (op == "multipoint_eval") result = multipoint_eval(f, points);
		else if (op == "interpolate") result = interpolate(points, g);
		else if (op == "taylor_shift") result = taylor_shift(f, mint(17));
		else if (op == "gcd") result = polynomial_gcd(f, g);
		else if (op == "inv_mod") { auto p = polynomial_inv_mod(g, f); result = p ? *p : fps{998244352}; }
		else return 1;
		elapsed += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
		checksum = checksum * 1000000007 + result.size();
		for (auto value : result) checksum = (checksum ^ value.val()) * 0x9e3779b97f4a7c15ULL;
	}
	std::printf("{\"ms\":%.9f,\"checksum\":\"%llu\",\"size\":%zu}\n", elapsed / repeat, checksum, result.size());
}
