#include "math/number_theory/factorize.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

int main(int argc, char **argv) {
	if (argc != 3) return 2;
	std::string mode = argv[1], kind = argv[2];
	std::mt19937_64 rng(20261020);
	std::vector<std::uint64_t> values(kind == "small" ? 10000 : 2000);
	for (auto &n : values) n = rng() % (kind == "small" ? 1000000ULL : 4294967295ULL) + 1;
	std::vector<std::vector<std::pair<std::uint64_t, int>>> factors;
	if (mode == "reused" || mode == "reused_unsorted")
		for (auto n : values) factors.push_back(factorize_count(n));
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	for (std::size_t i = 0; i < values.size(); ++i) {
		std::vector<std::uint64_t> ds;
		if (mode == "trial") {
			for (std::uint64_t d = 1; d <= values[i] / d; ++d) if (values[i] % d == 0) {
				ds.push_back(d);
				if (d != values[i] / d) ds.push_back(values[i] / d);
			}
			std::sort(ds.begin(), ds.end());
		} else if (mode == "reused" || mode == "reused_unsorted") ds = divisors(factors[i], mode == "reused");
		else ds = divisors(values[i], mode == "sorted");
		std::uint64_t sum = ds.size();
		for (auto d : ds) sum += d;
		checksum = checksum * 1000000007 + sum;
	}
	auto elapsed = std::chrono::steady_clock::now() - begin;
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << ' ' << checksum << '\n';
}
