#ifdef POWER_PROJECTION_REFERENCE
#include POWER_PROJECTION_REFERENCE
#else
#include "fps/power_projection.hpp"
#endif
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

__attribute__((target("avx2"))) unsigned int warm_cpu(unsigned int seed) {
	__m256i x = _mm256_set1_epi32(seed), one = _mm256_set1_epi32(1);
	for (int i = 0; i < 4096; ++i) x = _mm256_add_epi32(_mm256_mullo_epi32(x, x), one);
	return static_cast<unsigned int>(_mm256_extract_epi32(x, 0));
}

int main(int argc, char **argv) {
	if (argc != 4) return 1;
	std::string mode = argv[1], algorithm = argv[2];
	int n = std::stoi(argv[3]), count = mode == "short" ? 8 : n + (mode == "constant" ? 17 : 0);
	std::mt19937 random(718497);
	std::vector<unsigned int> input(n), weights(n), output(count);
	for (auto &x : input) x = random() % 998244353;
	for (auto &x : weights) x = random() % 998244353;
	input[0] = mode == "constant" ? 19 : 0;
	if (mode == "coefficient") std::fill(weights.begin(), weights.end(), 0), weights.back() = 1;
	if (mode == "monomial") std::fill(input.begin(), input.end(), 0), input[3] = 5;
	volatile auto warm = warm_cpu(random());
	(void)warm;
#ifdef POWER_PROJECTION_REFERENCE
	if (mode != "coefficient" || algorithm != "reference") return 1;
	auto start = std::chrono::steady_clock::now();
	int lg = poly::projection_log(n);
	std::size_t scratch = poly::projection_scratch(n);
	poly::Arena arena(poly::Transform::words(lg) + poly::Arena::footprint(n) + scratch);
	poly::Transform transform(arena, lg);
	auto result = arena.take(n);
	poly::power_projection(transform, input, result, arena.take(scratch));
	auto finish = std::chrono::steady_clock::now();
	std::copy(result.begin(), result.end(), output.begin());
#else
	fps f(n), w(n), result;
	for (int i = 0; i < n; ++i) f[i] = input[i], w[i] = weights[i];
	auto start = std::chrono::steady_clock::now();
	if (algorithm == "naive") {
		result.resize(count);
		fps power{1};
		for (int i = 0; i < count; ++i) {
			for (int j = 0; j < static_cast<int>(power.size()); ++j) result[i] += power[j] * w[j];
			if (i + 1 < count) {
				power = fps::multiply(power, f);
				if (static_cast<int>(power.size()) > n) power.resize(n);
			}
		}
	} else if (mode == "coefficient") result = power_projection(f, n - 1, count);
	else result = power_projection(f, w, count);
	auto finish = std::chrono::steady_clock::now();
	for (int i = 0; i < count; ++i) output[i] = result[i].val();
#endif
	unsigned long long checksum = 1469598103934665603ULL;
	for (auto x : output) checksum = (checksum ^ x) * 1099511628211ULL;
	double elapsed = std::chrono::duration<double, std::milli>(finish - start).count();
	std::cout << std::fixed << std::setprecision(6) << elapsed << ' ' << checksum << '\n';
}
