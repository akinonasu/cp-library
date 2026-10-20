#include "sequence/lis.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

template<bool Strict, bool Append, bool Restore>
std::vector<int> baseline(const std::vector<int> &a) {
	std::vector<int> tails, lengths(Restore ? a.size() : 0);
	if constexpr (Restore) tails.reserve(a.size());
	for (int i = 0; i < int(a.size()); ++i) {
		int p = tails.size();
		bool append = false;
		if constexpr (Append) append = tails.empty() || (Strict ? tails.back() < a[i] : tails.back() <= a[i]);
		if (!append) {
			if constexpr (Strict) p = std::lower_bound(tails.begin(), tails.end(), a[i]) - tails.begin();
			else p = std::upper_bound(tails.begin(), tails.end(), a[i]) - tails.begin();
		}
		if (p == int(tails.size())) tails.push_back(a[i]);
		else tails[p] = a[i];
		if constexpr (Restore) lengths[i] = p + 1;
	}
	int need = tails.size();
	if constexpr (!Restore) return {need};
	std::vector<int> indices(need);
	for (int i = a.size(); need && i--;)
		if (lengths[i] == need) indices[--need] = i;
	return indices;
}

template<bool Strict, bool Restore>
[[gnu::noinline]] std::vector<int> solve(const std::vector<int> &a, const std::string &variant) {
	if (variant == "standard") return baseline<Strict, false, Restore>(a);
	if (variant == "append") return baseline<Strict, true, Restore>(a);
	if constexpr (Restore) return lis<Strict>(a);
	else return {lis_length<Strict>(a)};
}

int main(int argc, char **argv) {
	if (argc != 6) return 2;
	std::string variant = argv[1], operation = argv[2], kind = argv[3];
	int n = std::atoi(argv[4]), repeats = std::atoi(argv[5]);
	if (n <= 0 || repeats <= 0) return 2;
	std::mt19937 rng(20261019);
	std::vector<int> a(n);
	for (auto &x : a) x = int(rng() & 0x7fffffff);
	if (kind == "increasing") std::iota(a.begin(), a.end(), 0);
	if (kind == "decreasing") for (int i = 0; i < n; ++i) a[i] = n - i;
	if (kind == "equal") std::fill(a.begin(), a.end(), 7);
	if (kind == "few") for (auto &x : a) x %= 16;
	if (kind == "alternating") for (int i = 0; i < n; ++i) a[i] = i & 1 ? i : n - i;
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int rep = 0; rep < repeats; ++rep) {
		std::vector<int> result;
		if (operation == "restore") result = solve<true, true>(a, variant);
		else if (operation == "weak") result = solve<false, false>(a, variant);
		else result = solve<true, false>(a, variant);
		for (int value : result) checksum = checksum * 1000000007 + value;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << ' ' << checksum << '\n';
}
