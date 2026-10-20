#include "math/number_theory/prime_sieve.hpp"
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

std::vector<std::size_t> baseline(std::size_t n) {
	std::vector<std::size_t> result;
	if (n <= 2) return result;
	result.push_back(2);
	std::vector<std::uint8_t> composite(n / 2);
	for (std::size_t p = 3; p < n; p += 2) {
		if (composite[p / 2]) continue;
		result.push_back(p);
		if (p > (n - 1) / p) continue;
		for (std::size_t j = p * p / 2; j < composite.size(); j += p) composite[j] = 1;
	}
	return result;
}

int main(int argc, char **argv) {
	std::size_t n = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 1000000;
	int rounds = argc > 2 ? std::atoi(argv[2]) : 7;
	int repeats = argc > 3 ? std::atoi(argv[3]) : 1;
	std::vector<std::size_t> (*implementations[])(std::size_t) = {baseline, prime_sieve};
	if (argc > 4) {
		int kind = std::atoi(argv[4]);
		if (kind < 0 || kind > 1) return 2;
		auto primes = implementations[kind](n);
		std::cout << n << ',' << kind << ',' << primes.size() << ',' << (primes.empty() ? 0 : primes.back()) << '\n';
		return 0;
	}
	std::uint64_t checksums[2]{};
	std::vector<double> times[2];
	for (int round = -1; round < rounds; ++round) {
		for (int order = 0; order < 2; ++order) {
			int kind = (order + (round & 1)) & 1;
			std::uint64_t checksum = 0;
			auto start = std::chrono::steady_clock::now();
			for (int rep = 0; rep < repeats; ++rep) {
				auto primes = implementations[kind](n);
				checksum += primes.size();
				for (std::size_t i = 0; i < primes.size(); i += 1024) checksum = checksum * 1000000007 + primes[i];
			}
			auto end = std::chrono::steady_clock::now();
			checksums[kind] = checksum;
			if (round >= 0) times[kind].push_back(std::chrono::duration<double, std::milli>(end - start).count() / repeats);
		}
	}
	for (auto &t : times) std::sort(t.begin(), t.end());
	std::cout << n << ',' << std::fixed << std::setprecision(6) << times[0][rounds / 2] << ',' << times[1][rounds / 2] << ',' << checksums[0] << ',' << checksums[1] << '\n';
	return checksums[0] != checksums[1];
}
