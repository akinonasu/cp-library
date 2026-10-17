#include "components/bitwise_zeta.hpp"
#include "components/walsh_hadamard.hpp"
#include "math/mint.hpp"
#include "bitwise_transforms_baseline.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

std::uint64_t checksum = 0;
template <class T>
std::uint64_t value(T x) {
	if constexpr (std::is_same_v<T, mint>) return x.val();
	else if constexpr (std::is_same_v<T, double>) return std::bit_cast<std::uint64_t>(x);
	else return std::uint64_t(x);
}

template <class T, class F, class G>
void compare(const char *type, const char *operation, int exponent, int samples, F before, G after) {
	std::size_t n = std::size_t(1) << exponent;
	std::mt19937 rng(20261017);
	std::vector<T> input(n), a(n), b(n);
	for (auto &x : input) x = T(rng() % 1001);
	a = input;
	b = input;
	before(a);
	after(b);
	if (a != b) std::abort();
	int repetitions = std::max(1, int(262144 / n));
	std::vector<double> times[2];
	for (int sample = 0; sample < samples; ++sample)
		for (int turn = 0; turn < 2; ++turn) {
			int variant = (sample + turn) % 2;
			double total = 0;
			for (int repetition = 0; repetition < repetitions; ++repetition) {
				a = input;
				auto begin = std::chrono::steady_clock::now();
				if (variant) after(a);
				else before(a);
				auto end = std::chrono::steady_clock::now();
				total += std::chrono::duration<double, std::milli>(end - begin).count();
				checksum += value(a[(sample + repetition) % n]);
			}
			times[variant].push_back(total / repetitions);
		}
	for (auto &t : times) std::sort(t.begin(), t.end());
	double old_time = times[0][samples / 2], new_time = times[1][samples / 2];
	std::cout << type << ',' << operation << ',' << exponent << ',' << old_time << ',' << new_time << ',' << old_time / new_time << '\n';
}

template <class T>
void run(const char *type, int exponent, int samples) {
	compare<T>(type, "walsh", exponent, samples,
		[](auto &a) { bitwise_transforms_baseline::walsh_hadamard(a); }, [](auto &a) { walsh_hadamard(a); });
	compare<T>(type, "subset_zeta", exponent, samples,
		[](auto &a) { bitwise_transforms_baseline::bitwise_zeta(a); }, [](auto &a) { bitwise_zeta(a); });
	compare<T>(type, "subset_mobius", exponent, samples,
		[](auto &a) { bitwise_transforms_baseline::bitwise_zeta<true>(a); }, [](auto &a) { bitwise_zeta<true>(a); });
	compare<T>(type, "superset_zeta", exponent, samples,
		[](auto &a) { bitwise_transforms_baseline::bitwise_zeta<false, true>(a); }, [](auto &a) { bitwise_zeta<false, true>(a); });
	compare<T>(type, "superset_mobius", exponent, samples,
		[](auto &a) { bitwise_transforms_baseline::bitwise_zeta<true, true>(a); }, [](auto &a) { bitwise_zeta<true, true>(a); });
}

int main(int argc, char **argv) {
	int exponent = argc > 1 ? std::atoi(argv[1]) : 18;
	int samples = argc > 2 ? std::atoi(argv[2]) : 11;
	if (exponent < 0 || exponent > 25 || samples <= 0) return 1;
	std::cout << std::fixed << std::setprecision(6);
	std::cout << "type,operation,log2_n,baseline_ms,optimized_ms,speedup\n";
	run<mint>("mint", exponent, samples);
	run<std::uint64_t>("uint64", exponent, samples);
	run<double>("double", exponent, samples);
	std::cerr << "checksum=" << checksum << '\n';
}
