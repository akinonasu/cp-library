#include "components/ntt_mod.hpp"
#include "math/prime_sieve.hpp"
#include "convolution/convolution_mod.hpp"
#include "components/quad_bit_vector.hpp"
#include "data_structure/wavelet_matrix.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

int main(int argc, char **argv) {
	std::string kind = argc > 1 ? argv[1] : "ntt";
	int n = argc > 2 ? std::atoi(argv[2]) : 262144;
	int repetitions = argc > 3 ? std::atoi(argv[3]) : 4;
	std::mt19937 rng(20261016);
	std::vector<std::uint32_t> a(kind == "sieve" ? 0 : n), b = a;
	for (auto &x : a) x = rng() % 1000000007;
	for (auto &x : b) x = rng() % 1000000007;
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	for (int trial = 0; trial < repetitions; ++trial) {
		if (kind == "sieve") {
			auto primes = prime_sieve(n);
			for (auto p : primes) checksum = checksum * 1234567891 + p;
		} else if (kind == "quad") {
			QuadBitVector bits(n);
			bits.build(a, trial % 31);
			for (const auto &block : bits.blocks) checksum = checksum * 1234567891 + block.low + block.high;
		} else if (kind == "wavelet") {
			WaveletMatrix<std::uint32_t> matrix(a);
			checksum = checksum * 1234567891 + matrix.kth_smallest(0, n, n / 2);
		} else if (kind == "crt") {
			auto c = convolution_mod(a, b, 1000000007);
			for (auto x : c) checksum = checksum * 1234567891 + x;
		} else {
			auto c = a;
			ntt_mod<2013265921, 31>(c);
			ntt_mod<2013265921, 31>(c, true);
			for (auto x : c) checksum = checksum * 1234567891 + x;
		}
	}
	double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - begin).count();
	std::cout << kind << ',' << n << ',' << repetitions << ',' << ms << ',' << checksum << '\n';
}
