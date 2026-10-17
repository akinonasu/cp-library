#include "math/mod_pow.hpp"
#include "math/mint.hpp"
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>

std::uint32_t binary(std::uint32_t a, std::uint64_t e, std::uint32_t p) {
	std::uint32_t r = 1;
	for (; e; e >>= 1, a = std::uint64_t(a) * a % p)
		if (e & 1) r = std::uint64_t(r) * a % p;
	return r;
}
std::uint32_t euclid(std::uint32_t a, std::uint32_t p) {
	std::int64_t b = p, x = 1, y = 0;
	while (b) {
		std::int64_t q = a / b, next = a - q * b;
		a = b, b = next;
		next = x - q * y, x = y, y = next;
	}
	return x < 0 ? x + p : x;
}

int main(int argc, char** argv) {
	if (argc != 5) return 1;
	std::string variant = argv[1], workload = argv[2];
	std::uint32_t p = std::strtoul(argv[3], nullptr, 10);
	int count = std::atoi(argv[4]);
	std::vector<std::uint32_t> bases(count);
	std::vector<std::uint64_t> exponents(count);
	std::mt19937_64 rng(731);
	for (int i = 0; i < count; ++i) {
		bases[i] = 1 + rng() % (p - 1);
		exponents[i] = workload == "pow64" ? rng() : rng() % (p - 1);
	}
	std::unique_ptr<ModPow> power;
	std::unique_ptr<ModInv> inverse;
	mod_fast_detail::Reducer reducer(p);
	mod_fast_detail::Reducer order(p - 1);
	auto first = std::chrono::steady_clock::now();
	if (variant == "library") {
		if (workload == "inv") inverse = std::make_unique<ModInv>(p);
		else power = std::make_unique<ModPow>(p);
	}
	auto start = std::chrono::steady_clock::now();
	std::uint64_t hash = 0;
	for (int i = 0; i < count; ++i) {
		std::uint32_t result;
		if (variant == "library") result = workload == "inv" ? inverse->inv(bases[i]) : power->pow(bases[i], exponents[i]);
		else if (variant == "euclid") result = euclid(bases[i], p);
		else if (variant == "mint") {
			auto a = mint::raw(bases[i]);
			result = workload == "inv" ? a.inv().val() : a.pow(exponents[i]).val();
		} else if (variant == "barrett") {
			result = reducer.pow(bases[i], workload == "inv" ? p - 2 : order.reduce(exponents[i]));
		} else result = binary(bases[i], workload == "inv" ? p - 2 : exponents[i], p);
		hash = (hash ^ result) * 0x9e3779b97f4a7c15ULL;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::setprecision(12) << std::chrono::duration<double>(start - first).count() << '\n'
		<< std::chrono::duration<double>(end - start).count() << '\n' << hash << '\n';
}
