#include "math/floor_sum/generalized_floor_sum.hpp"
#include "math/mint.hpp"
#include "generalized_floor_sum_monoid.hpp"
#include "generalized_floor_sum_quadratic.hpp"
#include "generalized_floor_sum_base.hpp"
#include "raw.hpp"
#include "one.hpp"
#include "rawone.hpp"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;
using I = __int128_t;
struct Query { U n, m, a, b; };
std::uint64_t value(mint x) { return x.val(); }
std::uint64_t value(I x) { return std::uint64_t(x); }
std::uint64_t value(U x) { return x; }

template <int D, class T>
void run(const std::string &kind, const std::vector<Query> &queries, int trials, std::mt19937_64 &rng) {
	int variants = D <= 2 ? 7 : 4;
	std::vector<int> order(variants);
	std::iota(order.begin(), order.end(), 0);
	for (int trial = 0; trial < trials; ++trial) {
		std::shuffle(order.begin(), order.end(), rng);
		U expected = 0;
		bool first = true;
		for (int id : order) {
			U checksum = 0;
			auto start = std::chrono::steady_clock::now();
			for (auto q : queries) {
				generalized_floor_sum_detail::Table<D, T> result;
				if (id == 0) result = generalized_floor_sum_benchmark::Monoid<D, T>::run(q.n, q.m, q.a, q.b);
				if (id == 1) result = floor_base_detail::euclidean<D, T>(q.n, q.m, q.a, q.b);
				if (id == 2) result = generalized_floor_sum_moments<D, T>(q.n, q.m, q.a, q.b);
				if constexpr (D == 2) {
					if (id == 3) result = generalized_floor_sum_detail::quadratic_baseline<T>(q.n, q.m, q.a, q.b);
				}
				if constexpr (D == 1) {
					if (id == 3) {
						result = {};
						result[0][0] = T(q.n);
						result[1][0] = q.n ? generalized_floor_sum_detail::divide<2>(T(q.n) * T(q.n - 1)) : T(0);
						result[0][1] = generalized_floor_sum<0, 1, T>(q.n, q.m, q.a, q.b);
					}
				}
				if constexpr (D <= 2) {
					if (id == 4) result = floor_raw_moments<D, T>(q.n, q.m, q.a, q.b);
					if (id == 5) result = floor_one_moments<D, T>(q.n, q.m, q.a, q.b);
					if (id == 6) result = floor_rawone_moments<D, T>(q.n, q.m, q.a, q.b);
				} else if (id == 3) result = floor_one_moments<D, T>(q.n, q.m, q.a, q.b);

				for (int p = 0; p <= D; ++p)
					for (int j = 0; p + j <= D; ++j) checksum = checksum * 1000000007 + value(result[p][j]);
			}
			double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			if (first) expected = checksum, first = false;
			else if (expected != checksum) { std::cerr << "checksum mismatch " << kind << ' ' << D << ' ' << id << '\n'; std::abort(); }
			const char *name[] = {"monoid", "euclidean", "current", D <= 2 ? "baseline" : "one", "raw", "one", "rawone"};
			std::cout << kind << ',' << D << ',' << (std::is_same_v<T, I> ? "int128" : "mint") << ',' << name[id] << ',' << trial << ',' << queries.size() << ',' << ms << ',' << checksum << '\n';
		}
	}
}

template <int P, int Q>
void scalar(const std::string &kind, const std::vector<Query> &queries, int trials, std::mt19937_64 &rng) {
	std::array<int, 3> order{0, 1, 2};
	for (int trial = 0; trial < trials; ++trial) {
		std::shuffle(order.begin(), order.end(), rng);
		U expected = 0;
		bool first = true;
		for (int id : order) {
			U checksum = 0;
			auto start = std::chrono::steady_clock::now();
			for (auto q : queries) {
				mint result;
				if (id == 0) result = generalized_floor_sum_benchmark::RectangularMonoid<P, Q, mint>::run(q.n, q.m, q.a, q.b)[P][Q];
				if (id == 1) result = floor_base_detail::euclidean<P + Q, mint>(q.n, q.m, q.a, q.b)[P][Q];
				if (id == 2) result = generalized_floor_sum<P, Q, mint>(q.n, q.m, q.a, q.b);
				checksum = checksum * 1000000007 + value(result);
			}
			double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
			if (first) expected = checksum, first = false;
			else if (expected != checksum) std::abort();
			const char *name[] = {"rectangle", "euclidean", "current"};
			std::cout << kind << ',' << P << ':' << Q << ",scalar," << name[id] << ',' << trial << ',' << queries.size() << ',' << ms << ',' << checksum << '\n';
		}
	}
}

int main(int argc, char **argv) {
	int count = argc > 1 ? std::stoi(argv[1]) : 10000, trials = argc > 2 ? std::stoi(argv[2]) : 5;
	std::mt19937_64 rng(5264528);
	std::cout << "kind,degree,type,variant,trial,queries,ms,checksum\n";
	for (std::string kind : {"random", "fibonacci", "quotient", "small"}) {
		std::vector<Query> queries(count);
		for (auto &q : queries) {
			q = {rng() % 1000000000 + 1, rng() % 1000000000 + 1, 0, 0};
			q.a = rng() % q.m;
			q.b = rng() % q.m;
			if (kind == "fibonacci") q = {1000000000, 701408733, 433494437, rng() % 701408733};
			if (kind == "quotient") q = {rng() % 100000 + 1, rng() % 100000 + 1, rng() % 1000000000, rng() % 1000000000};
			if (kind == "small") q.n = rng() % 32;
		}
		run<1, I>(kind, queries, trials, rng);
		run<2, I>(kind, queries, trials, rng);
		run<2, mint>(kind, queries, trials, rng);
		queries.resize(std::max(1, count / 10));
		run<3, mint>(kind, queries, trials, rng);
		run<4, mint>(kind, queries, trials, rng);
		run<6, mint>(kind, queries, trials, rng);
		run<8, mint>(kind, queries, trials, rng);
		scalar<0, 2>(kind, queries, trials, rng);
		scalar<1, 1>(kind, queries, trials, rng);
		scalar<0, 4>(kind, queries, trials, rng);
		scalar<1, 3>(kind, queries, trials, rng);
		scalar<2, 2>(kind, queries, trials, rng);
		scalar<0, 8>(kind, queries, trials, rng);
		scalar<1, 7>(kind, queries, trials, rng);
		scalar<4, 4>(kind, queries, trials, rng);
	}
}
