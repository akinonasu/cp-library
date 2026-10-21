#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include "fps/polynomial_gcd.hpp"
#include <array>
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
	fps a(n), b(n);
	for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random();
	if (operation.starts_with("compose") || operation == "reversion" || operation.starts_with("projection")) b[0] = 0;
	if (operation == "reversion") b[1] = 1;
	if (operation == "compose_stride") for (int i = 1; i < n; ++i) if (i % 4) b[i] = 0;
	if (operation == "compose_valuation") std::fill(b.begin(), b.begin() + n / 8, mint(0));
	if (operation == "gcd" || operation == "extended_gcd" || operation == "inv_mod") a.back() = b.back() = 1;
	std::array<fps, 3> answer;
	auto start = std::chrono::steady_clock::now();
	if (operation.starts_with("compose")) answer[0] = compose(a, b);
	else if (operation == "reversion") answer[0] = compositional_inverse(b);
	else if (operation == "projection") answer[0] = power_projection(b, n - 1);
	else if (operation == "projection_weighted") answer[0] = power_projection(b, a);
	else if (operation == "projection_short") answer[0] = power_projection(b, a, 8);
	else if (operation == "gcd") answer[0] = polynomial_gcd(std::move(a), std::move(b));
	else if (operation == "extended_gcd") {
		auto result = polynomial_extended_gcd(std::move(a), std::move(b));
		answer = {std::move(result.gcd), std::move(result.x), std::move(result.y)};
	} else if (operation == "inv_mod") {
		auto result = polynomial_inv_mod(std::move(a), std::move(b));
		if (!result) return 2;
		answer[0] = std::move(*result);
	} else return 1;
	auto finish = std::chrono::steady_clock::now();
	std::uint64_t checksum = 1469598103934665603ULL;
	for (const auto &polynomial : answer) {
		checksum = (checksum ^ polynomial.size()) * 1099511628211ULL;
		for (mint value : polynomial) checksum = (checksum ^ value.val()) * 1099511628211ULL;
	}
	std::printf("%.9f %zu %zu %zu %llu\n", std::chrono::duration<double, std::milli>(finish - start).count(),
		answer[0].size(), answer[1].size(), answer[2].size(), static_cast<unsigned long long>(checksum));
}
