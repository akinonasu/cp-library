#ifdef FPS_SPLIT_BASELINE
#include FPS_SPLIT_BASELINE
#else
#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#endif
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	std::string operation = argv[1];
	int n = std::atoi(argv[2]);
	std::mt19937 random(731);
	fps f(n), g(n), result;
	std::vector<mint> xs(n);
	for (int i = 0; i < n; ++i) { f[i] = random(); g[i] = random(); xs[i] = i + 1; }
	if (operation == "compose") g[0] = 0;
	if (operation == "compositional_inverse") { f[0] = 0; f[1] = 1; }
	if (operation == "bostan_mori") g[0] = 1;
	if (operation == "inv") f[0] = 1;
	if (operation == "exp") f[0] = 0;
	auto start = std::chrono::steady_clock::now();
#ifdef FPS_SPLIT_BASELINE
	if (operation == "bostan_mori") result = {fps::bostan_mori(f, g, 1000000000000000000LL)};
	else if (operation == "linear_recurrence") result = {fps::linear_recurrence(f, g, 1000000000000000000LL)};
	else if (operation == "compose") result = f.compose(g);
	else if (operation == "compositional_inverse") result = f.compositional_inverse();
	else if (operation == "multipoint_eval") result = f.multipoint_eval(xs);
	else if (operation == "interpolate") result = fps::interpolate(xs, g);
	else if (operation == "taylor_shift") result = f.shift(17);
#else
	if (operation == "bostan_mori") result = {bostan_mori(f, g, 1000000000000000000LL)};
	else if (operation == "linear_recurrence") result = {linear_recurrence<mint>(f, g, 1000000000000000000LL)};
	else if (operation == "compose") result = compose(f, g);
	else if (operation == "compositional_inverse") result = compositional_inverse(f);
	else if (operation == "multipoint_eval") result = multipoint_eval(f, xs);
	else if (operation == "interpolate") result = interpolate(xs, g);
	else if (operation == "taylor_shift") result = taylor_shift(std::move(f), 17);
#endif
	else if (operation == "inv") result = f.inv();
	else if (operation == "exp") result = f.exp();
	else return 1;
	auto stop = std::chrono::steady_clock::now();
	unsigned long long checksum = 0;
	for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	std::printf("%.9f\n%llu\n", std::chrono::duration<double>(stop - start).count(), checksum);
}
