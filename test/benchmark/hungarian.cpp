#include "classic.hpp"
#include "dense.hpp"
#include "greedy.hpp"
#include "column.hpp"
#include "no_transfer.hpp"
#include "reduced.hpp"
#include "ascending.hpp"
#include "queue.hpp"
#include "current.hpp"
#include "baseline.hpp"
#include "narrow.hpp"
#include "narrow_order.hpp"
#include "free_only.hpp"
#include "rect_scan.hpp"
#include "queue_half.hpp"
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

using Clock = std::chrono::steady_clock;
std::uint64_t checksum;
template <class Solver>
void run(const char *name, const std::string &kind, int n, int m, const std::vector<int> &a, int trial) {
	auto start = Clock::now();
	Solver res(n, m, a.data());
	auto end = Clock::now();
	long long total = 0;
	std::vector<int> used(m);
	for (int i = 0; i < n; ++i) {
		int j = res.row_to_col[i];
		if (j < 0 || j >= m || used[j]++ || res.col_to_row[j] != i) std::abort();
		total += a[std::size_t(i) * m + j];
	}
	if (total != res.cost) std::abort();
	checksum ^= std::uint64_t(res.cost);
	std::cout << name << ',' << kind << ',' << n << ',' << m << ',' << trial << ','
		<< std::chrono::duration<double, std::milli>(end - start).count() << ',' << res.cost << '\n';
}
int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 512, m = argc > 2 ? std::atoi(argv[2]) : n, trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::cout << "variant,kind,n,m,trial,ms,cost\n";
	for (std::string kind : {"random", "signed", "ties", "constant", "monge", "anti_monge", "correlated", "blocks"}) {
		std::mt19937 rng(928133);
		std::vector<int> a(std::size_t(n) * m);
		std::vector<int> row(n), col(m);
		for (auto &x : row) x = int(rng() % 100000001) - 50000000;
		for (auto &x : col) x = int(rng() % 100000001) - 50000000;
		for (int i = 0; i < n; ++i) for (int j = 0; j < m; ++j) {
			int x = rng() % 1000000001;
			if (kind == "signed") x -= 500000000;
			if (kind == "ties") x = rng() % 8;
			if (kind == "constant") x = -17;
			if (kind == "monge") x = (i - j) * (i - j);
			if (kind == "anti_monge") x = -(i - j) * (i - j);
			if (kind == "correlated") x = row[i] + col[j] + int(rng() % 1000);
			if (kind == "blocks") x = i / 32 == j / 32 ? rng() % 2 : 1000;
			a[std::size_t(i) * m + j] = x;
		}
		for (int trial = 0; trial < trials; ++trial) {
			std::vector<int> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
			std::shuffle(order.begin(), order.end(), rng);
			for (int id : order) {
				if (id == 0) run<Hungarian_classic<>>("classic", kind, n, m, a, trial);
				if (id == 1) run<Hungarian_dense<>>("dense", kind, n, m, a, trial);
				if (id == 2) run<Hungarian_greedy<>>("greedy", kind, n, m, a, trial);
				if (id == 3) run<Hungarian_column<>>("column", kind, n, m, a, trial);
				if (id == 4) run<Hungarian_no_transfer<>>("no_transfer", kind, n, m, a, trial);
				if (id == 5) run<Hungarian_reduced<>>("reduced", kind, n, m, a, trial);
				if (id == 6) run<Hungarian_ascending<>>("ascending", kind, n, m, a, trial);
				if (id == 7) run<Hungarian_queue<>>("queue", kind, n, m, a, trial);
				if (id == 8) run<Hungarian_current<>>("current", kind, n, m, a, trial);
				if (id == 10) run<Hungarian_baseline<>>("baseline", kind, n, m, a, trial);
				if (id == 11) run<Hungarian_narrow<>>("narrow", kind, n, m, a, trial);
				if (id == 12) run<Hungarian_narrow_order<>>("narrow_order", kind, n, m, a, trial);
				if (id == 13) run<Hungarian_free_only<>>("free_only", kind, n, m, a, trial);
				if (id == 14) run<Hungarian_rect_scan<>>("rect_scan", kind, n, m, a, trial);
				if (id == 15) run<Hungarian_queue_half<>>("queue_half", kind, n, m, a, trial);
#if __has_include("reference.hpp")
				if (id == 9 && n == m) run<Hungarian_reference>("reference_410724", kind, n, m, a, trial);
#endif
			}
		}
	}
	std::cerr << checksum << '\n';
}
