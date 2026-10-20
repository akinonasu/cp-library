#include "math/fraction.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

int main() {
	std::mt19937_64 rng(20261020);
	std::vector<Fraction<>> values;
	for (int i = 0; i < 1000000; ++i) {
		auto n = std::int64_t(rng() % 1000000000000000000ULL) + 1, d = std::int64_t(rng() % 1000000000000000000ULL) + 1;
		values.emplace_back(i & 1 ? n : -n, d);
	}
	auto start = std::chrono::steady_clock::now();
	std::uint64_t checksum = 0;
	for (const auto &x : values) {
		auto y = x.inv();
		checksum = checksum * 1000003 + y.numerator();
		checksum = checksum * 1000003 + y.denominator();
	}
	double elapsed = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	std::cout << std::fixed << std::setprecision(6) << "inverse_ms=" << elapsed << " checksum=" << checksum << '\n';
}
