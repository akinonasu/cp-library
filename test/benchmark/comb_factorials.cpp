#ifndef COMB_HEADER
#define COMB_HEADER "math/combinatorics/comb.hpp"
#endif
#ifndef PREFIX_HEADER
#define PREFIX_HEADER "components/binomial_prefix_core.hpp"
#endif
#include COMB_HEADER
#include PREFIX_HEADER
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string_view>

#ifndef COMB_GENERATE
#define COMB_GENERATE() fps_detail::factorials(fact, finv, COMB_INF + 1)
#endif

int main(int argc, char** argv) {
	const bool prefix = argc > 1 && std::string_view(argv[1]) == "prefix";
	const int repeats = std::max(1, std::min(20000, 10000000 / (COMB_INF + 1)));
	binomial_prefix_detail::Table<mint> table;
	auto generate = [&] { if (prefix) table.build(COMB_INF); else COMB_GENERATE(); };
	generate();
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < repeats; ++i) {
		generate();
#if defined(__GNUC__) || defined(__clang__)
		asm volatile("" ::: "memory");
#endif
	}
	auto end = std::chrono::steady_clock::now();
	const double ms = std::chrono::duration<double, std::milli>(end - start).count() / repeats;
	const mint* a = prefix ? table.factorial.data() : fact;
	const mint* b = prefix ? table.inverse_factorial.data() : finv;
	std::uint64_t checksum = 1;
	for (int i = 0; i <= COMB_INF; ++i) {
		checksum = (checksum ^ a[i].val()) * 1099511628211ULL;
		checksum = (checksum ^ b[i].val()) * 1099511628211ULL;
		if (prefix) checksum = (checksum ^ table.power2[i].val()) * 1099511628211ULL;
	}
	std::cout << std::setprecision(17) << ms << ' ' << checksum << ' ' << repeats << '\n';
}
