#include <atcoder/modint>
#include "fps/fps.hpp"
#ifdef BENCH_ACL_MODINT
using Series = FormalPowerSeries<atcoder::modint998244353>;
#else
using Series = fps;
#endif
using Scalar = Series::value_type;
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3 && argc != 4) return 1;
	const std::string operation = argv[1];
	const int n = std::atoi(argv[2]);
	const bool dump = argc == 4 && std::string(argv[3]) == "--dump";
	if (n < 2) return 1;
	std::mt19937 random(731);
	Series a(n), b(n), result, remainder;
	for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random();
	if (operation == "inv" || operation == "log") a[0] = 1;
	if (operation == "exp") a[0] = 0;
	if (operation == "sqrt") a[0] = Scalar(17) * 17;
	if (operation == "div_series") b[0] = 1;
	if (operation == "divmod") {
		b.resize(n / 2);
		b.back() = 1;
	}
	const bool linear = operation == "scale" || operation == "diff" || operation == "integral" || operation == "eval";
	const int repetitions = linear ? std::max(4, 131072 / n) : 1;
	auto start = std::chrono::steady_clock::now();
	for (int repeat = 0; repeat < repetitions; ++repeat) {
		if (operation == "multiply") result = a * b;
		else if (operation == "square") result = a * a;
		else if (operation == "inv") result = a.inv();
		else if (operation == "log") result = a.log();
		else if (operation == "exp") result = a.exp();
		else if (operation == "sqrt") result = a.sqrt();
		else if (operation == "pow") result = a.pow(123456789);
		else if (operation == "div_series") result = a.div_series(b);
		else if (operation == "divmod") {
			auto qr = a.divmod(b);
			result = std::move(qr.first);
			remainder = std::move(qr.second);
		} else if (operation == "scale") result = a * Scalar(123456789);
		else if (operation == "diff") result = a.diff();
		else if (operation == "integral") result = a.integral();
		else if (operation == "eval") result = Series{a.eval(Scalar(123456789))};
		else return 1;
		asm volatile("" : : "g"(result.data()) : "memory");
	}
	auto end = std::chrono::steady_clock::now();
	std::uint64_t checksum = result.size();
	for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	checksum = (checksum ^ remainder.size()) * 0x9e3779b97f4a7c15ULL;
	for (auto x : remainder) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	const double ms = std::chrono::duration<double, std::milli>(end - start).count();
	std::printf("%.9f %zu %zu %llu\n", ms / repetitions, result.size(), remainder.size(), static_cast<unsigned long long>(checksum));
	if (dump) {
		for (auto x : result) std::printf("%u ", unsigned(x.val()));
		std::putchar('\n');
		for (auto x : remainder) std::printf("%u ", unsigned(x.val()));
		std::putchar('\n');
	}
}
