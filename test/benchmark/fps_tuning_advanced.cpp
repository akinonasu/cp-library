#include "fps/compose.hpp"
#include "fps/compositional_inverse.hpp"
#include "fps/power_projection.hpp"
#include "fps/bostan_mori.hpp"
#include "fps/linear_recurrence.hpp"
#ifdef FPS_ADVANCED_BEFORE
#include FPS_ADVANCED_BEFORE
#endif
#ifdef FPS_ADVANCED_BASELINE
#include FPS_ADVANCED_BASELINE
#endif
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#if defined(__x86_64__) || defined(__i386__)
__attribute__((target("avx2"))) unsigned int warm_cpu(unsigned int seed) {
	__m256i x = _mm256_set1_epi32(seed), one = _mm256_set1_epi32(1);
	for (int i = 0; i < 4096; ++i) x = _mm256_add_epi32(_mm256_mullo_epi32(x, x), one);
	return static_cast<unsigned int>(_mm256_extract_epi32(x, 0));
}
#endif

int main(int argc, char **argv) {
	if (argc != 4) return 1;
	std::string operation = argv[1], variant = argv[2];
	int n = std::stoi(argv[3]);
	std::mt19937 random(731);
	fps f(n), g(n), answer;
	for (int i = 0; i < n; ++i) f[i] = random(), g[i] = random();
	if (operation == "inverse" || operation.starts_with("compose") || operation.starts_with("projection")) g[0] = 0;
	else g[0] = 1;
	if (n > 1 && g[1] == mint(0)) g[1] = 1;
	if (operation.ends_with("_stride"))
		for (int i = 1; i < n; ++i) if (i % 4) g[i] = 0;
	if (operation == "compose_valuation") std::fill(g.begin(), g.begin() + n / 8, mint(0));
	if (operation == "recurrence_tail") std::fill(g.begin() + 8, g.end(), mint(0));
#if defined(__x86_64__) || defined(__i386__)
	if (__builtin_cpu_supports("avx2")) {
		volatile auto warm = warm_cpu(random());
		(void)warm;
	}
#endif
	auto begin = std::chrono::steady_clock::now();
	if (operation == "bostan") {
#ifdef FPS_ADVANCED_BEFORE
		if (variant == "before") answer = {bostan_mori_before(f, g, 1000000000000000000LL)};
		else
#endif
		answer = {bostan_mori(f, g, 1000000000000000000LL)};
	} else if (operation.starts_with("compose")) {
#ifdef FPS_ADVANCED_BASELINE
		if (variant == "before") answer = compose_before(f, g);
		else
#endif
		answer = compose(f, g);
	}
	else if (operation == "inverse") {
#ifdef FPS_ADVANCED_BASELINE
		if (variant == "before") answer = compositional_inverse_before(g);
		else
#endif
		answer = compositional_inverse(g);
	}
	else if (operation == "projection") answer = power_projection(g, n - 1);
	else if (operation == "projection_weighted" || operation == "projection_stride") {
#ifdef FPS_ADVANCED_BASELINE
		if (variant == "before") answer = power_projection_before(g, f);
		else
#endif
		answer = power_projection(g, f);
	} else if (operation.starts_with("recurrence")) {
#ifdef FPS_ADVANCED_BASELINE
		if (variant == "before") answer = {linear_recurrence_before(f, g, 1000000000000000000LL)};
		else
#endif
		answer = {linear_recurrence(f, g, 1000000000000000000LL)};
	}
	else if (operation == "short") answer = power_projection(f, g, 8);
	else return 2;
	auto end = std::chrono::steady_clock::now();
	unsigned long long checksum = 1469598103934665603ULL;
	for (auto x : answer) checksum = (checksum ^ x.val()) * 1099511628211ULL;
	std::cout << std::fixed << std::setprecision(6)
		<< std::chrono::duration<double, std::milli>(end - begin).count() << ' ' << checksum << '\n';
}
