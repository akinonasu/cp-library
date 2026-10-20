#include "string/lcs.hpp"
#include "string/edit_distance.hpp"
#include "string/run_enumerate.hpp"
#include "math/number_theory/factorize.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;

[[gnu::noinline]] U strings(const std::string &operation, const std::string &a, const std::string &b) {
	if (operation == "lcs") return lcs_length(a, b);
	if (operation == "restore") {
		auto value = lcs(a, b);
		U result = value.size();
		for (unsigned char c : value) result = result * 31 + c;
		return result;
	}
	if (operation == "edit") return edit_distance(a, b);
	U result = 0;
	for (auto [p, l, r] : run_enumerate(a)) result = result * 1000000007 + U(p) * 1000003 + U(l) * 1009 + r;
	return result;
}

[[gnu::noinline]] U numbers(const std::string &operation, const std::vector<U> &values) {
	U result = 0;
	for (U n : values) {
		if (operation == "prime") result = result * 31 + is_prime(n);
		else for (U p : factorize(n ? n : 1)) result = result * 1000000007 + p;
	}
	return result;
}

int main(int argc, char **argv) {
	if (argc != 5) return 2;
	std::string operation = argv[1], kind = argv[2];
	int size = std::atoi(argv[3]), repeats = std::atoi(argv[4]);
	if (size <= 0 || repeats <= 0) return 2;
	std::mt19937_64 rng(20261019);
	std::string a, b;
	std::vector<U> values;
	if (operation == "prime" || operation == "factor") {
		for (int i = 0; i < size; ++i) {
			U n = rng();
			if (kind.find("32") != std::string::npos) n = std::uint32_t(n);
			if (kind.find("prime") == 0) {
				n |= 1;
				while (!is_prime(n)) n += 2;
			}
			if (kind == "semiprime" || kind == "hard") {
				U base = kind == "hard" ? 3000000000ULL : 100000;
				U width = kind == "hard" ? 1000000000 : 100000;
				U p = (rng() % width + base) | 1, q = (rng() % width + base) | 1;
				while (!is_prime(p)) p += 2;
				while (!is_prime(q)) q += 2;
				n = p * q;
			}
			values.push_back(n);
		}
	} else {
		a.resize(size);
		for (char &c : a) c = char('a' + rng() % 26);
		b = a;
		if (kind == "random") for (char &c : b) c = char('a' + rng() % 26);
		if (kind == "near") b[size / 2] = char(b[size / 2] ^ 0x40);
		if (kind == "prefix") for (int i = std::max(0, size - 128); i < size; ++i) b[i] = char('a' + rng() % 26);
		if (kind == "suffix") for (int i = 0; i < std::min(128, size); ++i) b[i] = char('a' + rng() % 26);
		if (kind == "periodic") for (int i = 0; i < size; ++i) a[i] = "abcab"[i % 5];
	}
	U expected = operation == "prime" || operation == "factor" ? numbers(operation, values) : strings(operation, a, b);
	U checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int repeat = 0; repeat < repeats; ++repeat)
		checksum += operation == "prime" || operation == "factor" ? numbers(operation, values) : strings(operation, a, b);
	auto end = std::chrono::steady_clock::now();
	if (checksum != expected * U(repeats)) return 1;
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << ' ' << checksum << '\n';
}
