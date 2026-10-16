#include "compact.hpp"
#include "heap.hpp"
#include "aligned.hpp"
#include "plain_push.hpp"
#include "plain_query.hpp"
#include "prefetch.hpp"
#include "iterative.hpp"
#include "iterative_update.hpp"
#include "noinit.hpp"
#include "equal_first.hpp"
#include "branchless.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#if __has_include("reference.hpp")
#include "reference.hpp"
#endif

struct Query { int t, l, r; long long x; };
using Clock = std::chrono::steady_clock;
std::uint64_t answer;
template <class Tree>
void run(const char *name, const std::string &workload, const std::vector<long long> &a, const std::vector<Query> &queries, int trial) {
	auto start = Clock::now();
	Tree seg(a);
	auto built = Clock::now();
	std::uint64_t hash = 0;
	for (auto [t, l, r, x] : queries) {
		if (t == 0) seg.chmin(l, r, x);
		else if (t == 1) seg.chmax(l, r, x);
		else if (t == 2) seg.add(l, r, x);
		else if (t == 3) hash = hash * 1000000007 + seg.sum(l, r);
		else if (t == 4) seg.assign(l, r, x);
		else if (t == 5) hash = hash * 1000000007 + seg.min(l, r);
		else hash = hash * 1000000007 + seg.max(l, r);
	}
	auto end = Clock::now();
	answer ^= hash;
	std::cout << name << ',' << workload << ',' << a.size() << ',' << queries.size() << ',' << trial << ','
		<< std::chrono::duration<double, std::milli>(built - start).count() << ','
		<< std::chrono::duration<double, std::milli>(end - built).count() << ',' << hash << '\n';
}
int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 200000, q = argc > 2 ? std::atoi(argv[2]) : 200000, trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::cout << "variant,workload,n,q,trial,build_ms,query_ms,checksum\n";
	for (std::string workload : {"mixed", "short", "assign", "clamp", "uniform"}) {
		std::mt19937_64 rng(814239);
		std::vector<long long> a(n);
		for (auto &x : a) x = workload == "uniform" ? 0 : rng() % 1000000000;
		std::vector<Query> queries(q);
		for (auto &v : queries) {
			v.l = rng() % n;
			v.r = rng() % n;
			if (v.l > v.r) std::swap(v.l, v.r);
			++v.r;
			if (workload == "short") v.r = std::min(n, v.l + int(rng() % 64) + 1);
			v.t = rng() % 4;
			v.x = v.t == 2 ? static_cast<long long>(rng() % 2000001) - 1000000 : rng() % 1000000000;
			if (workload == "assign") v.t = rng() % 7;
			if (workload == "clamp") { v.t = rng() % 3; if (v.t == 2) v.t = 3; }
			if (workload == "uniform") { v.t = rng() % 4; if (v.t < 3) v.l = 0, v.r = n; }
		}
		for (int trial = 0; trial < trials; ++trial) {
			std::vector<int> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
			std::shuffle(order.begin(), order.end(), rng);
			for (int id : order) {
				if (id == 0) run<Beats_compact<>>("compact", workload, a, queries, trial);
				if (id == 1) run<Beats_heap<>>("heap", workload, a, queries, trial);
				if (id == 2) run<Beats_aligned<>>("aligned", workload, a, queries, trial);
				if (id == 3) run<Beats_plain_push<>>("plain_push", workload, a, queries, trial);
				if (id == 4) run<Beats_plain_query<>>("plain_query", workload, a, queries, trial);
				if (id == 5) run<Beats_prefetch<>>("prefetch", workload, a, queries, trial);
				if (id == 6) run<Beats_iterative<>>("iterative", workload, a, queries, trial);
				if (id == 7) run<Beats_iterative_update<>>("iterative_update", workload, a, queries, trial);
				if (id == 9) run<Beats_noinit<>>("noinit", workload, a, queries, trial);
				if (id == 10) run<Beats_equal_first<>>("equal_first", workload, a, queries, trial);
				if (id == 11) run<Beats_branchless<>>("branchless", workload, a, queries, trial);
#if __has_include("reference.hpp")
				if (id == 8 && workload != "assign") run<Beats_reference>("reference_204482", workload, a, queries, trial);
#endif
			}
		}
	}
	std::cerr << answer << '\n';
}
