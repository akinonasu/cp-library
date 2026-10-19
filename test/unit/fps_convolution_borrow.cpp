#include "components/fps_convolution.hpp"
#include <cstdlib>
#include <random>
#include <new>
#include <utility>
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <sys/mman.h>
#include <unistd.h>

static void require(bool ok) { if (!ok) std::abort(); }

static void protect(const std::vector<fps_mint>& a, int mode) {
	const auto page = std::uintptr_t(sysconf(_SC_PAGESIZE));
	const auto first = reinterpret_cast<std::uintptr_t>(a.data());
	const auto last = first + a.size() * sizeof(fps_mint);
	const auto lo = (first + page - 1) & ~(page - 1), hi = last & ~(page - 1);
	if (lo < hi) require(mprotect(reinterpret_cast<void*>(lo), hi - lo, mode) == 0);
}

static void check(const std::vector<fps_mint>& a, const std::vector<fps_mint>& b) {
	using namespace fps_detail::multiply_detail;
	const auto saved_a = a, saved_b = b;
	protect(a, PROT_READ);
	protect(b, PROT_READ);
	const auto expected = run<ntt::Convolution>(a, b);
	const auto actual = multiply(a, b);
	const auto generic = run_borrowed_as<ntt::Convolution>(a, b);
	require(generic == expected);
	for (const auto x : generic) require(x.v < fps_mint::mod());
	require(actual == expected && actual.size() == a.size() + b.size() - 1);
	for (const auto x : actual) require(x.v < fps_mint::mod());
	for (std::size_t i = 0; i < a.size(); ++i) require(a[i].v == saved_a[i].v);
	for (std::size_t i = 0; i < b.size(); ++i) require(b[i].v == saved_b[i].v);
	protect(a, PROT_READ | PROT_WRITE);
	protect(b, PROT_READ | PROT_WRITE);
}

template <class Plan>
static void check_guarded(std::mt19937& rng, std::size_t n, std::size_t m, bool half_line) {
	using namespace fps_detail::multiply_detail;
	const auto page = std::size_t(sysconf(_SC_PAGESIZE));
	const std::size_t len = std::max<std::size_t>(64, std::bit_ceil(n + m - 1));
	const std::size_t extra = half_line ? 8 : 0, bytes = (len + 16 + extra) * sizeof(fps_mint);
	const std::size_t readable = (bytes + page - 1) / page * page;
	void* region = mmap(nullptr, readable + 2 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	require(region != MAP_FAILED);
	auto* const end = static_cast<char*>(region) + page + readable;
	auto* const to = ::new (static_cast<void*>(end - bytes)) fps_mint[len + 16 + extra];
	require(reinterpret_cast<std::uintptr_t>(to) % 64 == (half_line ? 32u : 0u));
	for (std::size_t i = len + 16; i < len + 16 + extra; ++i) to[i] = fps_mint::word(0xdeadbeefu);
	require(mprotect(region, page, PROT_NONE) == 0 && mprotect(end, page, PROT_NONE) == 0);
	std::vector<fps_mint> a(n), b(m);
	for (auto& x : a) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
	for (auto& x : b) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
	const auto saved_a = a, saved_b = b;
	protect(a, PROT_READ);
	protect(b, PROT_READ);
	const auto expected = run<Plan>(a, b);
	{
		Plan plan(n, m, reinterpret_cast<std::uint32_t*>(to));
		require(plan.a() == reinterpret_cast<std::uint32_t*>(to));
		copy_input_mint(a, to);
		copy_input(b, plan.b(), len);
		require(plan.multiply() == reinterpret_cast<std::uint32_t*>(to));
		for (std::size_t i = 0; i < expected.size(); ++i) require(to[i] == expected[i] && to[i].v < fps_mint::mod());
		for (std::size_t i = len; i < len + 16; ++i) require(to[i].v == 0);
		for (std::size_t i = len + 16; i < len + 16 + extra; ++i) require(to[i].v == 0xdeadbeefu);
	}
	require(to[0] == expected[0]);
	for (std::size_t i = 0; i < a.size(); ++i) require(a[i].v == saved_a[i].v);
	for (std::size_t i = 0; i < b.size(); ++i) require(b[i].v == saved_b[i].v);
	protect(a, PROT_READ | PROT_WRITE);
	protect(b, PROT_READ | PROT_WRITE);
	require(munmap(region, readable + 2 * page) == 0);
}

#endif

int main() {
#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
	if (!__builtin_cpu_supports("avx2")) return 0;
	std::mt19937 rng(733);
	for (const auto& [n, m] : {std::pair{257, 257}, {257, 511}, {511, 512}, {512, 512}, {4097, 4097}, {131072, 131072}, {524288, 1000}}) {
		std::vector<fps_mint> a(n), b(m);
		for (auto& x : a) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
		for (auto& x : b) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
		check(a, b);
		check(a, a);
		for (auto& x : b) x = fps_mint::word(fps_mint::mod());
		check(a, b);
	}
	for (const auto& [n, m] : {std::pair{1, 1}, {32, 33}, {33, 33}, {33, 95}, {64, 64}, {64, 65}, {65, 64}, {127, 129}, {128, 128}, {128, 129}, {257, 100}, {257, 255}, {511, 512}, {512, 513}, {4096, 4096}, {4097, 2048}, {8193, 1000}, {65536, 65536}, {262144, 262144}, {300000, 220000}}) {
		std::vector<fps_mint> a(n), b(m);
		for (auto& x : a) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
		for (auto& x : b) x = fps_mint::word(fps_mint::mod() + rng() % fps_mint::mod());
		check(a, b);
		check(b, a);
		check(a, a);
		for (const bool half_line : {false, true}) {
			check_guarded<fps_detail::multiply_detail::ntt::Convolution>(rng, n, m, half_line);
			check_guarded<fps_detail::multiply_detail::ntt::Convolution>(rng, m, n, half_line);
		}
	}
	for (const std::size_t n : {257u, 511u, 512u, 4097u})
		for (const bool half_line : {false, true}) check_guarded<fps_detail::multiply_detail::Product>(rng, n, n, half_line);
#endif
}
