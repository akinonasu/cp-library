#include "components/fps_factorials.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string_view>
#include <vector>

[[gnu::noinline]] void naive(mint *fact, mint *finv, int n) {
	fact[0] = mint(1);
	for (int i = 1; i < n; ++i) fact[i] = fact[i - 1] * mint(i);
	finv[n - 1] = fact[n - 1].inv();
	for (int i = n - 1; i > 0; --i) finv[i - 1] = finv[i] * mint(i);
}

[[gnu::noinline]] void scalar(mint *fact, mint *finv, int n) {
	using N = fps_detail::NTT;
	auto *a = reinterpret_cast<N::U *>(fact), *b = reinterpret_cast<N::U *>(finv);
	N::U value = N::one;
	for (int i = 0; i < n; ++i) a[i] = value, value = N::mul(value, N::in(i + 1));
	value = mint::word(a[n - 1]).inv().v;
	for (int i = n - 1; i >= 0; --i) b[i] = value, value = N::mul(value, N::in(i));
}

[[gnu::noinline]] void simd(mint *fact, mint *finv, int n) {
	fps_detail::factorials(fact, finv, n);
}

int main(int argc, char **argv) {
	if (argc != 4) return 1;
	int n = std::atoi(argv[1]) + 1, repeats = std::atoi(argv[3]);
	if (n <= 0 || n > 10000001 || repeats <= 0) return 1;
	std::string_view method(argv[2]);
	auto generate = method == "naive" ? naive : method == "scalar" ? scalar : simd;
#if defined(__x86_64__) || defined(__i386__)
	if (!fps_detail::NTT::vectorized()) return 2;
#else
	return 2;
#endif
	std::vector<mint> fact(n), finv(n);
	auto first_start = std::chrono::steady_clock::now();
	generate(fact.data(), finv.data(), n);
	auto start = std::chrono::steady_clock::now();
	for (int r = 0; r < repeats; ++r) {
		generate(fact.data(), finv.data(), n);
		asm volatile("" ::: "memory");
	}
	auto end = std::chrono::steady_clock::now();
	std::uint64_t expected = 1, checksum = 1469598103934665603ULL;
	for (int i = 0; i < n; ++i) {
		if (i) expected = expected * i % mint::mod();
		if (fact[i].val() != expected) return 3;
		checksum = (checksum ^ expected) * 1099511628211ULL;
	}
	std::uint64_t inverse = 1, value = expected;
	for (unsigned e = mint::mod() - 2; e; e >>= 1, value = value * value % mint::mod())
		if (e & 1) inverse = inverse * value % mint::mod();
	for (int i = n - 1; i >= 0; --i) {
		if (finv[i].val() != inverse) return 4;
		checksum = (checksum ^ inverse) * 1099511628211ULL;
		inverse = inverse * i % mint::mod();
	}
	double first_ms = std::chrono::duration<double, std::milli>(start - first_start).count();
	double repeated_ms = std::chrono::duration<double, std::milli>(end - start).count() / repeats;
	std::cout << std::setprecision(17) << first_ms << ' ' << repeated_ms << ' ' << checksum << '\n';
}
