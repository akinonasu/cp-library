#include "fps/fps.hpp"
#include <cstdio>
#include <random>
#include <utility>

using namespace fps_detail;
using namespace fps_detail::multiply_detail;

static std::mt19937 generator(731);
static std::vector<Mint> input(std::size_t n) {
	std::vector<Mint> a(n);
	for (auto& x : a) x.v = generator() % (2 * Mint::mod());
	return a;
}
static void top_case(std::size_t len, std::size_t n) {
	auto a = input(n), before = a;
	const std::size_t words = 2 * (len + 16) + ntt::detail::table_words(std::bit_width(len) - 1);
	auto storage = allocate_fps_words(words);
	auto* old = storage.get();
	auto* result = old + len + 16;
	auto* roots = result + len + 16;
	std::fill(old, old + words, 0xdeadbeefu);
	std::fill(roots, old + words, 0);
	ntt::detail::build_table(roots, len / 16, ntt::detail::kRoots[0]);
	copy_input(a, old, len);
	forward_radix8(reinterpret_cast<Vec*>(old), len / 64, roots);
	forward_radix8_input(reinterpret_cast<Vec*>(result), len / 64, roots, a);
	for (std::size_t j = 0; j < len; ++j) if (old[j] % Mint::mod() != result[j] % Mint::mod()) {
		std::fprintf(stderr, "top %zu %zu at %zu: %u %u\n", len, n, j, old[j], result[j]);
		std::abort();
	}
	for (std::size_t j = 0; j < n; ++j) if (a[j].v != before[j].v) std::abort();
}
static void convolution_case(std::size_t n, std::size_t m) {
	auto a = input(n), b = input(m), old_a = a, old_b = b;
	auto c = multiply_detail::multiply(a, b);
	auto expected = run<ntt::Convolution>(a, b);
	if (c != expected) {std::fprintf(stderr, "convolution %zu %zu\n", n, m);std::abort();}
	for (std::size_t i = 0; i < n; ++i) if (a[i].v != old_a[i].v) std::abort();
	for (std::size_t i = 0; i < m; ++i) if (b[i].v != old_b[i].v) std::abort();
	if (n < 600 && m < 600) {
		std::vector<Mint> naive(n + m - 1);
		for (std::size_t i = 0; i < n; ++i) for (std::size_t j = 0; j < m; ++j) naive[i + j] += a[i] * b[j];
		if (c != naive) std::abort();
	}
}
int main() {
	for (std::size_t n = 0; n <= 512; ++n) top_case(1024, n);
	for (std::size_t len : {std::size_t(4096), std::size_t(16384), std::size_t(1048576)}) {
		const std::size_t quarter = len / 8;
		for (std::size_t s = 0; s < 4; ++s) for (std::size_t k = 0; k < 16; ++k)
			top_case(len, s * quarter + k);
		top_case(len, len / 2);
		top_case(len, len / 2 - 1);
	}
	for (const auto& [n,m] : {std::pair{3,512},{33,480},{127,511},{128,512},{129,512},{255,512},{256,512},{257,512},{383,512},{384,512},{385,512},{511,511},{512,512},{513,513},{2048,2048},{1999,2048},{8191,8192},{32769,32769},{500000,500000},{524288,524288}}) {
		convolution_case(n,m);
		convolution_case(m,n);
	}
	puts("fused input top + convolution + input preservation passed");
}
