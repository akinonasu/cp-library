#include "math/floor_sum/generalized_floor_sum.hpp"
#include "math/mint.hpp"
#include "baseline.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

struct Query { std::uint64_t n, m, a, b; };

template <int D>
void run(const std::string &mode) {
	std::mt19937_64 rng(20261020);
	std::vector<Query> queries(D == 4 ? 10000 : D == 8 ? 2000 : 100);
	for (auto &q : queries) q = {rng() % 1000000001, rng() % 1000000000 + 1, rng() % 1000000001, rng() % 1000000001};
	std::uint64_t checksum = 0;
	auto call = [&](Query q) {
		if (mode == "base") return baseline_generalized_floor_sum_moments<D, mint>(q.n, q.m, q.a, q.b);
		if (mode == "signed") return generalized_floor_sum_moments<D, mint>(q.n, q.m, std::int64_t(q.a), std::int64_t(q.b));
		return generalized_floor_sum_moments<D, mint>(q.n, q.m, q.a, q.b);
	};
	call(queries.front());
	auto begin = std::chrono::steady_clock::now();
	for (auto q : queries) {
		auto result = call(q);
		for (int p = 0; p <= D; ++p)
			for (int r = 0; p + r <= D; ++r) checksum = checksum * 1000000007 + result[p][r].val();
	}
	auto elapsed = std::chrono::steady_clock::now() - begin;
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 3) return 2;
	int d = std::atoi(argv[2]);
	if (d == 4) run<4>(argv[1]);
	else if (d == 8) run<8>(argv[1]);
	else if (d == 20) run<20>(argv[1]);
	else return 2;
}
