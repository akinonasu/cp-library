#ifndef DYNAMIC_WAVELET_COUNT_HEADER
#define DYNAMIC_WAVELET_COUNT_HEADER "data_structure/dynamic_wavelet_matrix.hpp"
#endif
#include DYNAMIC_WAVELET_COUNT_HEADER
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using T = std::int64_t;

struct Operation {
	int kind, l, r;
	T value;
};

template <int Bits>
void run(const std::string &mode, const std::string &method, int n, int q) {
	std::mt19937_64 rng(18106471);
	std::vector<T> initial(n);
	auto value = [&]() -> T {
		if (mode == "uniform64") return 7;
		if (mode == "duplicates64") return T(rng() & 15);
		if (mode == "full64" || mode == "missing64") return T(rng());
		return T(rng() & ((1ULL << 20) - 1));
	};
	for (auto &x : initial) x = value();
	std::vector<T> current = initial;
	std::vector<Operation> operations;
	operations.reserve(q);
	for (int i = 0; i < q; ++i) {
		if (mode == "mixed20" && i % 4 == 0) {
			int kind = current.empty() ? 1 : int(rng() % 3);
			int p = int(rng() % (current.size() + (kind == 1)));
			T x = value();
			operations.push_back({kind, p, 0, x});
			if (kind == 0) current[p] = x;
			else if (kind == 1) current.insert(current.begin() + p, x);
			else current.erase(current.begin() + p);
			continue;
		}
		int l = int(rng() % (current.size() + 1)), r = int(rng() % (current.size() + 1));
		if (l > r) std::swap(l, r);
		if (mode == "short20") r = std::min(r, l + 16);
		if (mode == "single20") r = std::min(int(current.size()), l + 1);
		T x = mode == "uniform64" ? 7 + T(i & 1) : value();
		if ((i & 1) && l < r && mode != "missing64" && mode != "uniform64") x = current[l + rng() % (r - l)];
		operations.push_back({3, l, r, x});
	}
	DynamicWaveletMatrix<T, Bits> wm(initial);
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (const auto &op : operations) {
		if (op.kind == 0) wm.set(op.l, op.value);
		else if (op.kind == 1) wm.insert(op.l, op.value);
		else if (op.kind == 2) wm.erase(op.l);
		else {
			int answer;
			if (method == "range_freq") {
				if (op.value == std::numeric_limits<T>::max())
					answer = op.r - op.l - wm.range_freq(op.l, op.r, op.value);
				else answer = wm.range_freq(op.l, op.r, op.value, op.value + 1);
			} else answer = wm.count(op.l, op.r, op.value);
			checksum = checksum * 1000000007 + unsigned(answer);
		}
	}
	double milliseconds = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	if (mode == "mixed20")
		for (int i = 0; i < int(current.size()); ++i) if (wm[i] != current[i]) std::abort();
	std::cout << std::setprecision(12) << milliseconds << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string mode = argv[1], method = argv[2];
	int n = std::atoi(argv[3]), q = std::atoi(argv[4]);
	if (n < 1 || q < 0) return 1;
	if (mode == "full64" || mode == "missing64" || mode == "duplicates64" || mode == "uniform64" || mode == "narrow64") run<64>(mode, method, n, q);
	else run<21>(mode, method, n, q);
}
