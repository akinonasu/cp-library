#include "math/combinatorics/binomial_prefix_sum_incremental.hpp"
#include ".verify-helper/cache/arc190-b/baseline.hpp"
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/resource.h>

template<class Sum> void run(int n, const std::string& shape, bool shared) {
	using Clock = std::chrono::steady_clock;
	auto start = Clock::now();
	Sum first(n);
	Sum second = shared ? first : Sum(n);
	auto ready = Clock::now();
	mint checksum(0);
	if (shape == "rows") {
		for (int i = 0; i <= n; ++i) checksum += first.query(i, n / 3) + second.query(i, 2 * n / 3);
		for (int i = n; i >= 0; --i) checksum += first.query(i, n / 3) + second.query(i, 2 * n / 3);
	} else if (shape == "diagonal") {
		for (int i = 0; i <= n; ++i) checksum += first.query(i, i - n / 4) + second.query(i, i - n / 3);
		for (int i = n; i >= 0; --i) checksum += first.query(i, i - n / 4) + second.query(i, i - n / 3);
	} else if (shape == "columns") {
		for (int i = 0; i <= n + 1; ++i) checksum += first.query(n, i) + second.query(n, n + 1 - i);
	} else if (shape == "walk") {
		unsigned seed = 20261020;
		int row = n / 2, k = n / 4;
		for (int i = 0; i < 4 * n; ++i) {
			seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
			row = std::clamp(row + int(seed % 3) - 1, 0, n);
			k = std::clamp(k + int(seed / 3 % 3) - 1, 0, row + 1);
			checksum += first.query(row, k);
		}
	} else if (shape == "jumps") {
		unsigned seed = 20261020;
		for (int i = 0; i < 2000; ++i) {
			seed ^= seed << 13, seed ^= seed >> 17, seed ^= seed << 5;
			int row = seed % (n + 1);
			checksum += first.query(row, seed / 3 % (row + 2));
		}
	} else if (shape == "arc190") {
		int a = n / 3, b = 3 * n / 5;
		for (int k = 1; k <= n; ++k) checksum += first.query(n - k, b + 1) - second.query(n - k, b - k + 1);
		for (int k = n; k > 0; --k) checksum += first.query(n - k, a + 1) - second.query(n - k, a - k + 1);
		for (int k = 1; k <= n; ++k) checksum += first.query(n - k, a + 1) - second.query(n - k, a - k + 1);
		for (int k = n; k > 0; --k) checksum += first.query(n - k, b + 1) - second.query(n - k, b - k + 1);
	}
	auto finished = Clock::now();
	rusage usage{};
	getrusage(RUSAGE_SELF, &usage);
	std::printf("%.6f %.6f %u %ld\n", std::chrono::duration<double, std::milli>(ready - start).count(),
		std::chrono::duration<double, std::milli>(finished - ready).count(), checksum.val(), usage.ru_maxrss);
}

int main(int argc, char** argv) {
	if (argc != 4) return 1;
	int n = std::atoi(argv[3]);
	std::string mode = argv[1], shape = argv[2];
	if (mode == "baseline") run<BaselineIncrementalBinomialPrefixSum<>>(n, shape, false);
	else run<IncrementalBinomialPrefixSum<>>(n, shape, mode == "shared");
}
