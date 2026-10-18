#include "fps/fps.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>

int main(int argc, char **argv) {
	const int n = argc > 2 ? std::atoi(argv[2]) : 500000;
	const std::string op = argc > 1 ? argv[1] : "inv";
	std::mt19937 rng(20261019);
	fps a(n);
	for (auto &x : a) x = rng();
	a[0] = 1;
	const auto start = std::chrono::steady_clock::now();
	const fps r = op == "inv" ? a.inv(n) : a.sqrt(n);
	const auto end = std::chrono::steady_clock::now();
	unsigned long long hash = 0;
	for (auto x : r) hash = hash * 0x9e3779b97f4a7c15ULL + x.val();
	std::printf("%.9f %llu\n", std::chrono::duration<double, std::milli>(end - start).count(), hash);
}
