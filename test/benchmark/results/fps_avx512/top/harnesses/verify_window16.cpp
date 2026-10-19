#include "components/fps_sqrt_blocked.hpp"
#include <sys/mman.h>
#include <unistd.h>
#include <new>
#include <cstdio>
#include <random>
namespace blocked_ntt = fps_detail::blocked_ntt;
using namespace fps_detail::blocked_poly;
using namespace fps_detail::blocked_poly::detail;
struct alignas(64) ReferenceWindow { std::uint32_t word[17]; };
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
[[gnu::always_inline]] inline Vec reference_product(const ReferenceWindow& window, const std::uint32_t* b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2, t3;
	asm("vpbroadcastd (%[b]), %[bi]\n\t"
		"vpmuludq 36(%[w]), %[bi], %[odd]\n\t"
		"vpmuludq 32(%[w]), %[bi], %[even]\n\t"
		"vpbroadcastd 4(%[b]), %[bi]\n\t"
		"vpmuludq 32(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t1]\n\t"
		"vpbroadcastd 8(%[b]), %[bi]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 24(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 12(%[b]), %[bi]\n\t"
		"vpmuludq 24(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpbroadcastd 16(%[b]), %[bi]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 16(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 20(%[b]), %[bi]\n\t"
		"vpmuludq 16(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpbroadcastd 24(%[b]), %[bi]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t2]\n\t"
		"vpmuludq 8(%[w]), %[bi], %[t3]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"
		"vpbroadcastd 28(%[b]), %[bi]\n\t"
		"vpmuludq 8(%[w]), %[bi], %[t0]\n\t"
		"vpmuludq 4(%[w]), %[bi], %[t1]\n\t"
		"vpaddq %[t2], %[odd], %[odd]\n\t"
		"vpaddq %[t3], %[even], %[even]\n\t"
		"vpaddq %[t0], %[odd], %[odd]\n\t"
		"vpaddq %[t1], %[even], %[even]\n\t"

		"vpmuludq %[ni], %[even], %[t0]\n\t"
		"vpmuludq %[ni], %[odd], %[t1]\n\t"
		"vpmuludq %[p], %[t0], %[t0]\n\t"
		"vpmuludq %[p], %[t1], %[t1]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpsrlq $32, %[even], %[even]\n\t"
		"vpblendd $0xaa, %[odd], %[even], %[even]\n\t"
		"vpsubd %[p2], %[even], %[t0]\n\t"
		"vpminud %[t0], %[even], %[even]"
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2),
		[t3] "=&x"(t3)
		: [w] "r"(window.word), [b] "r"(b), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)),
		"m"(*reinterpret_cast<const std::uint32_t(*)[8]>(b)));
	return even;
}

[[gnu::always_inline]] inline Vec reference_square(const ReferenceWindow& window) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2;
	asm("vpbroadcastd 32(%[w]), %[bi]\n\t"
		"vpmuludq 36(%[w]), %[bi], %[odd]\n\t"
		"vpmuludq 32(%[w]), %[bi], %[even]\n\t"
		"vpbroadcastd 36(%[w]), %[bi]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t0]\n\t"
		"vpbroadcastd 40(%[w]), %[bi]\n\t"
		"vpmuludq 28(%[w]), %[bi], %[t1]\n\t"
		"vpmuludq 24(%[w]), %[bi], %[t2]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpbroadcastd 44(%[w]), %[bi]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t0]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpaddq %[t2], %[even], %[even]\n\t"
		"vpbroadcastd 48(%[w]), %[bi]\n\t"
		"vpmuludq 20(%[w]), %[bi], %[t1]\n\t"
		"vpmuludq 16(%[w]), %[bi], %[t2]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpbroadcastd 52(%[w]), %[bi]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t0]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpaddq %[t2], %[even], %[even]\n\t"
		"vpbroadcastd 56(%[w]), %[bi]\n\t"
		"vpmuludq 12(%[w]), %[bi], %[t1]\n\t"
		"vpmuludq 8(%[w]), %[bi], %[t2]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpbroadcastd 60(%[w]), %[bi]\n\t"
		"vpmuludq 4(%[w]), %[bi], %[t0]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpaddq %[t2], %[even], %[even]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpaddq %[odd], %[odd], %[odd]\n\t"

		"vpmuludq %[ni], %[even], %[t0]\n\t"
		"vpmuludq %[ni], %[odd], %[t1]\n\t"
		"vpmuludq %[p], %[t0], %[t0]\n\t"
		"vpmuludq %[p], %[t1], %[t1]\n\t"
		"vpaddq %[t0], %[even], %[even]\n\t"
		"vpaddq %[t1], %[odd], %[odd]\n\t"
		"vpsrlq $32, %[even], %[even]\n\t"
		"vpblendd $0xaa, %[odd], %[even], %[even]\n\t"
		"vpsubd %[p2], %[even], %[t0]\n\t"
		"vpminud %[t0], %[even], %[even]"
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2)
		: [w] "r"(window.word), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)));
	return even;
}

static void require(bool ok) { if (!ok) std::abort(); }
static void check() {
	static_assert(sizeof(Window) == 64 && alignof(Window) == 64);
	const auto page = std::size_t(sysconf(_SC_PAGESIZE));
	void* mapping = mmap(nullptr, 4 * page, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	require(mapping != MAP_FAILED);
	require(mprotect(static_cast<char*>(mapping) + page, page, PROT_NONE) == 0);
	require(mprotect(static_cast<char*>(mapping) + 3 * page, page, PROT_NONE) == 0);
	auto* window = ::new(static_cast<char*>(mapping) + page - sizeof(Window)) Window;
	auto* b = reinterpret_cast<std::uint32_t*>(static_cast<char*>(mapping) + 3 * page) - 8;
	std::mt19937 rng(20261026);
	ReferenceWindow reference{};
	alignas(32) std::uint32_t x[8], y[8], a[8];
	for (unsigned trial = 0; trial < 200000; ++trial) {
		for (unsigned i = 0; i < 16; ++i) reference.word[i] = window->word[i] = trial == 0 ? kP - (i >= 8) : rng() % (kP + (i < 8));
		for (unsigned i = 0; i < 8; ++i) b[i] = trial == 0 ? kP - 1 : rng() % kP;
		store(x, reference_product(reference, b)); store(y, leaf_product(*window, b));
		for (unsigned i = 0; i < 8; ++i) require(x[i] == y[i]);
		for (unsigned i = 0; i < 8; ++i) a[i] = rng() % kP;
		fill_window(*window, load(a), Factor(rng() % kP));
		std::copy_n(window->word, 16, reference.word);
		store(x, reference_square(reference)); store(y, leaf_square(*window));
		for (unsigned i = 0; i < 8; ++i) require(x[i] == y[i]);
	}
	require(munmap(mapping, 4 * page) == 0);
}
#pragma GCC pop_options
int main() { check(); std::puts("200000 product/square leaf cases and exact PROT_NONE boundaries passed"); }
