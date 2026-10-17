#include "fps/polynomial_gcd.hpp"
#include "fps/multipoint_eval.hpp"
#include "fps/interpolate.hpp"
#include "fps/taylor_shift.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char** argv) {
	if (argc != 4) return 1;
	std::string operation = argv[1];
	int n = std::atoi(argv[2]), repeat = std::atoi(argv[3]);
	if (n < 2 || repeat <= 0) return 1;
	std::mt19937 random(294384);
	fps a(n), b(n - 1), result;
	std::vector<mint> xs(n);
	for (auto& x : a) x = random();
	for (auto& x : b) x = random();
	for (int i = 0; i < n; ++i) xs[i] = i + 1;
	a.back() = b.back() = 1;
	unsigned long long checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int k = 0; k < repeat; ++k) {
		if (operation == "gcd") result = polynomial_gcd(a, b);
		else if (operation == "extended") {
			auto value = polynomial_extended_gcd(a, b);
			result = std::move(value.gcd);
			result.insert(result.end(), value.x.begin(), value.x.end());
			result.insert(result.end(), value.y.begin(), value.y.end());
		}
		else if (operation == "inverse") result = *polynomial_inv_mod(b, a);
		else if (operation == "taylor") result = taylor_shift(a, mint(17));
		else if (operation == "eval") result = multipoint_eval(a, xs);
		else if (operation == "interpolate") result = interpolate(xs, a);
		else return 1;
		for (auto x : result) checksum = (checksum ^ x.val()) * 0x9e3779b97f4a7c15ULL;
	}
	auto stop = std::chrono::steady_clock::now();
	std::printf("%.9f %llu\n", std::chrono::duration<double>(stop - start).count() / repeat, checksum);
}
