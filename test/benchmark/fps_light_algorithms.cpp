#ifdef LIGHT_FPS
#include "fps/fps_light_algorithms.hpp"
using Series = light_fps;
namespace algorithms = light_fps_algorithms;
#else
#include "fps/taylor_shift.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/linear_recurrence.hpp"
using Series = fps;
#endif
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	const std::string operation = argv[1];
	const int n = std::atoi(argv[2]);
	if (n < 2) return 1;
	std::mt19937 random(731);
	Series a(n), b(n);
	std::vector<mint> xs(n), result;
	for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random(), xs[i] = i;
	b.back() = 1;
	auto start = std::chrono::steady_clock::now();
#ifdef LIGHT_FPS
	if (operation == "taylor_shift") result = algorithms::taylor_shift(std::move(a), mint(17));
	else if (operation == "multipoint_eval") result = algorithms::multipoint_eval(a, xs);
	else if (operation == "interpolate") result = algorithms::interpolate(xs, b);
	else if (operation == "linear_recurrence") result = {algorithms::linear_recurrence<mint>(a, b, 1000000000000000000LL)};
#else
	if (operation == "taylor_shift") result = taylor_shift(std::move(a), mint(17));
	else if (operation == "multipoint_eval") result = multipoint_eval(a, xs);
	else if (operation == "interpolate") result = interpolate<mint>(xs, b);
	else if (operation == "linear_recurrence") result = {linear_recurrence<mint>(a, b, 1000000000000000000LL)};
#endif
	else return 1;
	auto end = std::chrono::steady_clock::now();
	std::uint64_t checksum = result.size();
	for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	const double ms = std::chrono::duration<double, std::milli>(end - start).count();
	std::printf("%.9f %zu %llu\n", ms, result.size(), static_cast<unsigned long long>(checksum));
}
