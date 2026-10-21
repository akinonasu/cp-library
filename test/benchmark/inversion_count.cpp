#include "sequence/inversion_count.hpp"
#include "components/fenwick_sum.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

std::int64_t fenwick_count(const std::vector<int> &a) {
	auto sorted = a;
	std::sort(sorted.begin(), sorted.end());
	sorted.erase(std::unique(sorted.begin(), sorted.end()), sorted.end());
	FenwickSum<int> counts(sorted.size());
	std::int64_t result = 0;
	for (int i = 0; i < int(a.size()); ++i) {
		int rank = std::lower_bound(sorted.begin(), sorted.end(), a[i]) - sorted.begin();
		result += i - counts.prefix(rank + 1);
		counts.add(rank, 1);
	}
	return result;
}

template<class F>
double measure(F function, std::int64_t expected) {
	auto start = std::chrono::steady_clock::now();
	auto result = function();
	auto end = std::chrono::steady_clock::now();
	if (result != expected) std::abort();
	return std::chrono::duration<double, std::milli>(end - start).count();
}

int main() {
	std::mt19937 random(20261021);
	const int n = 500000;
	std::cout << "n,shape,fenwick_ms,merge_ms\n";
	for (std::string shape : {"random", "increasing", "decreasing", "equal", "few"}) {
		std::vector<int> a(n);
		std::iota(a.begin(), a.end(), 0);
		if (shape == "random") std::shuffle(a.begin(), a.end(), random);
		if (shape == "decreasing") std::reverse(a.begin(), a.end());
		if (shape == "equal") std::fill(a.begin(), a.end(), 7);
		if (shape == "few") for (int &value : a) value = random() % 16;
		const auto expected = fenwick_count(a);
		if (inversion_count(a) != expected) std::abort();
		std::vector<double> times[2];
		for (int round = 0; round < 7; ++round) {
			int first = random() & 1;
			for (int j = 0; j < 2; ++j) {
				int variant = first ^ j;
				times[variant].push_back(measure([&] { return variant ? inversion_count(a) : fenwick_count(a); }, expected));
			}
		}
		for (auto &values : times) std::sort(values.begin(), values.end());
		std::cout << n << ',' << shape << ',' << times[0][3] << ',' << times[1][3] << '\n';
	}
}
