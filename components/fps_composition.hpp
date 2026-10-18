#ifndef CP_COMPONENTS_FPS_COMPOSITION_HPP
#define CP_COMPONENTS_FPS_COMPOSITION_HPP

#include "fps_blocked.hpp"

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace composition_poly {
using blocked_poly::Arena;
using blocked_poly::Transform;
using blocked_poly::Half;
using blocked_poly::kModulus;
namespace kernels {
using Vec = __m256i;
using blocked_poly::kernels::kP;
using blocked_poly::kernels::k2P;
[[gnu::noinline]] inline void forward_columns(Vec* a, std::size_t h, std::size_t width, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	Vec* stop;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		"1:\n\t"
		"lea (%[a],%[width]), %[stop]\n\t"
		".p2align 5\n\t"
		"2:\n\t"
		"vmovdqa (%[a],%[h3]), %%ymm0\n\t"
		"vmovdqa 32(%[a],%[h3]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h]), %%ymm2\n\t"
		"vmovdqa 32(%[a],%[h]), %%ymm3\n\t"
		"vpmuludq (%[a],%[h3]), %%ymm11, %%ymm4\n\t"
		"vpmuludq 32(%[a],%[h3]), %%ymm11, %%ymm5\n\t"
		"vpmuludq 4(%[a],%[h3]), %%ymm11, %%ymm6\n\t"
		"vpmuludq 36(%[a],%[h3]), %%ymm11, %%ymm7\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm8\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm9\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpblendd $170, %%ymm6, %%ymm4, %%ymm6\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa (%[a]), %%ymm5\n\t"
		"vpmulld (%[w]), %%ymm0, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vmovdqa 32(%[a]), %%ymm4\n\t"
		"vpmulld (%[w]), %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpminud %%ymm10, %%ymm2, %%ymm10\n\t"
		"vpmuludq (%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm0\n\t"
		"vpminud %%ymm0, %%ymm3, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpmuludq 32(%[a],%[h],2), %%ymm11, %%ymm1\n\t"
		"vpmuludq 4(%[a],%[h],2), %%ymm11, %%ymm3\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpmulld (%[w]), %%ymm8, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpmuludq 36(%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm1\n\t"
		"vpaddd %%ymm6, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm3, %%ymm8, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm7, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm8, %%ymm7\n\t"
		"vpsrlq $32, %%ymm10, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm2, %%ymm9, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm1\n\t"
		"vpmulld 64(%[w]), %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm10\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm10, %%ymm1, %%ymm10\n\t"
		"vpsubd %%ymm9, %%ymm6, %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm1\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm3, %%ymm0, %%ymm4\n\t"
		"vpsubd %%ymm3, %%ymm0, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm1, %%ymm0\n\t"
		"vpsubd %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm5\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm5, %%ymm3, %%ymm5\n\t"
		"vpsubd %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpaddd %%ymm8, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm9, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm9, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm10, %%ymm7, %%ymm7\n\t"
		"vmovdqa %%ymm1, (%[a])\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpaddd %%ymm6, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vmovdqa %%ymm5, (%[a],%[h],2)\n\t"
		"vmovdqa %%ymm9, (%[a],%[h3])\n\t"
		"vmovdqa %%ymm7, 32(%[a])\n\t"
		"vmovdqa %%ymm10, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm3, 32(%[a],%[h],2)\n\t"
		"vmovdqa %%ymm6, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[stop], %[a]\n\t"
		"jne 2b\n\t"
		"add %[width], %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [stop] "=&r"(stop)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [width] "r"(32 * width), [y] "r"(y), [x] "r"(x), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_columns(Vec* a, std::size_t h, std::size_t width, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	Vec* stop;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		"1:\n\t"
		"lea (%[a],%[width]), %[stop]\n\t"
		".p2align 5\n\t"
		"2:\n\t"
		"vmovdqa (%[a]), %%ymm0\n\t"
		"vmovdqa 32(%[a]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm2\n\t"
		"vmovdqa 32(%[a],%[h],2), %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd (%[a],%[h]), %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd (%[a],%[h]), %%ymm4, %%ymm4\n\t"
		"vpaddd 32(%[a],%[h]), %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpsubd 32(%[a],%[h]), %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm4, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm10\n\t"
		"vpsubd (%[a],%[h3]), %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm4, %%ymm4\n\t"
		"vpsubd 32(%[a],%[h3]), %%ymm10, %%ymm10\n\t"
		"vpaddd (%[a],%[h3]), %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsrlq $32, %%ymm5, %%ymm0\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpaddd 32(%[a],%[h3]), %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm5, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm0, %%ymm9, %%ymm0\n\t"
		"vpsubd %%ymm8, %%ymm4, %%ymm8\n\t"
		"vpsrlq $32, %%ymm7, %%ymm4\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsrlq $32, %%ymm10, %%ymm5\n\t"
		"vpmuludq %%ymm13, %%ymm10, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpmulld 64(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpminud %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm1\n\t"
		"vpminud %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm3\n\t"
		"vpaddd %%ymm9, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm7, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm7\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm1\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm4, %%ymm8, %%ymm4\n\t"
		"vpaddd %%ymm5, %%ymm0, %%ymm5\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm7, %%ymm0\n\t"
		"vpmulld (%[w]), %%ymm7, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm1, %%ymm9, %%ymm1\n\t"
		"vpsrlq $32, %%ymm3, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm3, %%ymm0\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm1, (%[a],%[h],2)\n\t"
		"vpsrlq $32, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpmulld (%[w]), %%ymm3, %%ymm3\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm7, %%ymm8\n\t"
		"vpmuludq %%ymm11, %%ymm10, %%ymm7\n\t"
		"vpmulld (%[w]), %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm0\n\t"
		"vmovdqa %%ymm8, (%[a],%[h3])\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm8\n\t"
		"vpminud %%ymm0, %%ymm6, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm6\n\t"
		"vpminud %%ymm8, %%ymm4, %%ymm8\n\t"
		"vmovdqa %%ymm0, (%[a])\n\t"
		"vpblendd $170, %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm7\n\t"
		"vpminud %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm3, %%ymm9\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa %%ymm6, 32(%[a])\n\t"
		"vmovdqa %%ymm7, 32(%[a],%[h])\n\t"
		"vmovdqa %%ymm9, 32(%[a],%[h],2)\n\t"
		"vpsubd %%ymm1, %%ymm10, %%ymm1\n\t"
		"vmovdqa %%ymm1, 32(%[a],%[h3])\n\t"
		"add $64, %[a]\n\t"
		"cmp %[stop], %[a]\n\t"
		"jne 2b\n\t"
		"add %[width], %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a), [stop] "=&r"(stop)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [width] "r"(32 * width), [y] "r"(y), [x] "r"(x), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_even_columns(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a],%[h3]), %%ymm0\n\t"
		"vmovdqa 64(%[a],%[h3]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h]), %%ymm2\n\t"
		"vmovdqa 64(%[a],%[h]), %%ymm3\n\t"
		"vpmuludq (%[a],%[h3]), %%ymm11, %%ymm4\n\t"
		"vpmuludq 64(%[a],%[h3]), %%ymm11, %%ymm5\n\t"
		"vpmuludq 4(%[a],%[h3]), %%ymm11, %%ymm6\n\t"
		"vpmuludq 68(%[a],%[h3]), %%ymm11, %%ymm7\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm8\n\t"
		"vpsrlq $32, %%ymm4, %%ymm4\n\t"
		"vmovdqa 64(%[a],%[h],2), %%ymm9\n\t"
		"vpsrlq $32, %%ymm5, %%ymm5\n\t"
		"vpblendd $170, %%ymm6, %%ymm4, %%ymm6\n\t"
		"vpblendd $170, %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa (%[a]), %%ymm5\n\t"
		"vpmulld (%[w]), %%ymm0, %%ymm0\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vmovdqa 64(%[a]), %%ymm4\n\t"
		"vpmulld (%[w]), %%ymm1, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm10\n\t"
		"vpminud %%ymm10, %%ymm2, %%ymm10\n\t"
		"vpmuludq (%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpsubd %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm0\n\t"
		"vpminud %%ymm0, %%ymm3, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpmuludq 64(%[a],%[h],2), %%ymm11, %%ymm1\n\t"
		"vpmuludq 4(%[a],%[h],2), %%ymm11, %%ymm3\n\t"
		"vpsrlq $32, %%ymm2, %%ymm2\n\t"
		"vpmulld (%[w]), %%ymm8, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpmuludq 68(%[a],%[h],2), %%ymm11, %%ymm2\n\t"
		"vpmulld %%ymm15, %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm10, %%ymm1\n\t"
		"vpaddd %%ymm6, %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm2, %%ymm2\n\t"
		"vpsubd %%ymm3, %%ymm8, %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm7, %%ymm0, %%ymm0\n\t"
		"vpsubd %%ymm7, %%ymm8, %%ymm7\n\t"
		"vpsrlq $32, %%ymm10, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsubd %%ymm2, %%ymm9, %%ymm2\n\t"
		"vpsrlq $32, %%ymm6, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm9, %%ymm9\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm8, %%ymm1, %%ymm8\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm1\n\t"
		"vpmulld 64(%[w]), %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm10, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm10\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm12, %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm10, %%ymm1, %%ymm10\n\t"
		"vpsubd %%ymm9, %%ymm6, %%ymm9\n\t"
		"vpsrlq $32, %%ymm7, %%ymm6\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm1\n\t"
		"vpmuludq %%ymm13, %%ymm6, %%ymm6\n\t"
		"vpmulld %%ymm15, %%ymm10, %%ymm10\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm1, %%ymm1\n\t"
		"vpblendd $170, %%ymm6, %%ymm1, %%ymm6\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm0\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpmulld %%ymm15, %%ymm6, %%ymm6\n\t"
		"vpaddd %%ymm3, %%ymm0, %%ymm4\n\t"
		"vpsubd %%ymm3, %%ymm0, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm1, %%ymm0\n\t"
		"vpsubd %%ymm2, %%ymm1, %%ymm2\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm1\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm5\n\t"
		"vpminud %%ymm1, %%ymm4, %%ymm1\n\t"
		"vpminud %%ymm5, %%ymm3, %%ymm5\n\t"
		"vpsubd %%ymm6, %%ymm7, %%ymm6\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm3\n\t"
		"vpminud %%ymm7, %%ymm0, %%ymm7\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm5, %%ymm4\n\t"
		"vpminud %%ymm3, %%ymm2, %%ymm3\n\t"
		"vpaddd %%ymm8, %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpaddd %%ymm9, %%ymm5, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpsubd %%ymm9, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm10, %%ymm7, %%ymm7\n\t"
		"vmovdqa %%ymm1, (%[a])\n\t"
		"vpsubd %%ymm10, %%ymm0, %%ymm10\n\t"
		"vpaddd %%ymm6, %%ymm3, %%ymm3\n\t"
		"vpsubd %%ymm6, %%ymm2, %%ymm6\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vmovdqa %%ymm5, (%[a],%[h],2)\n\t"
		"vmovdqa %%ymm9, (%[a],%[h3])\n\t"
		"vmovdqa %%ymm7, 64(%[a])\n\t"
		"vmovdqa %%ymm10, 64(%[a],%[h])\n\t"
		"vmovdqa %%ymm3, 64(%[a],%[h],2)\n\t"
		"vmovdqa %%ymm6, 64(%[a],%[h3])\n\t"
		"add $128, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_even_columns(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	asm volatile(
		"vpbroadcastd %[P], %%ymm15\n\t"
		"vpbroadcastd %[P2], %%ymm14\n\t"
		"vbroadcastss (%[x]), %%ymm0\n\t"
		"vmovdqa %%ymm0, (%[w])\n\t"
		"vbroadcastss 32(%[x]), %%ymm11\n\t"
		"vbroadcastss (%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 32(%[w])\n\t"
		"vbroadcastss 32(%[y]), %%ymm12\n\t"
		"vbroadcastss 4(%[y]), %%ymm0\n\t"
		"vmovdqa %%ymm0, 64(%[w])\n\t"
		"vbroadcastss 36(%[y]), %%ymm13\n\t"
		".p2align 5\n\t"
		"1:\n\t"
		"vmovdqa (%[a]), %%ymm0\n\t"
		"vmovdqa 64(%[a]), %%ymm1\n\t"
		"vmovdqa (%[a],%[h],2), %%ymm2\n\t"
		"vmovdqa 64(%[a],%[h],2), %%ymm3\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm4\n\t"
		"vpaddd (%[a],%[h]), %%ymm0, %%ymm0\n\t"
		"vpaddd %%ymm14, %%ymm1, %%ymm5\n\t"
		"vpsubd (%[a],%[h]), %%ymm4, %%ymm4\n\t"
		"vpaddd 64(%[a],%[h]), %%ymm1, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm0, %%ymm6\n\t"
		"vpaddd %%ymm14, %%ymm2, %%ymm7\n\t"
		"vpsubd 64(%[a],%[h]), %%ymm5, %%ymm5\n\t"
		"vpsrlq $32, %%ymm4, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm4, %%ymm9\n\t"
		"vpaddd %%ymm14, %%ymm3, %%ymm10\n\t"
		"vpsubd (%[a],%[h3]), %%ymm7, %%ymm7\n\t"
		"vpmuludq %%ymm12, %%ymm8, %%ymm8\n\t"
		"vpmulld 32(%[w]), %%ymm4, %%ymm4\n\t"
		"vpsubd 64(%[a],%[h3]), %%ymm10, %%ymm10\n\t"
		"vpaddd (%[a],%[h3]), %%ymm2, %%ymm2\n\t"
		"vpminud %%ymm6, %%ymm0, %%ymm6\n\t"
		"vpsrlq $32, %%ymm5, %%ymm0\n\t"
		"vpmuludq %%ymm12, %%ymm0, %%ymm0\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpaddd 64(%[a],%[h3]), %%ymm3, %%ymm3\n\t"
		"vpblendd $170, %%ymm8, %%ymm9, %%ymm8\n\t"
		"vpmuludq %%ymm12, %%ymm5, %%ymm9\n\t"
		"vpmulld 32(%[w]), %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm0, %%ymm9, %%ymm0\n\t"
		"vpsubd %%ymm8, %%ymm4, %%ymm8\n\t"
		"vpsrlq $32, %%ymm7, %%ymm4\n\t"
		"vpmuludq %%ymm13, %%ymm7, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm4, %%ymm4\n\t"
		"vpmulld %%ymm15, %%ymm0, %%ymm0\n\t"
		"vpmulld 64(%[w]), %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm4, %%ymm9, %%ymm4\n\t"
		"vpsubd %%ymm0, %%ymm5, %%ymm0\n\t"
		"vpsrlq $32, %%ymm10, %%ymm5\n\t"
		"vpmuludq %%ymm13, %%ymm10, %%ymm9\n\t"
		"vpmuludq %%ymm13, %%ymm5, %%ymm5\n\t"
		"vpmulld %%ymm15, %%ymm4, %%ymm4\n\t"
		"vpmulld 64(%[w]), %%ymm10, %%ymm10\n\t"
		"vpsrlq $32, %%ymm9, %%ymm9\n\t"
		"vpblendd $170, %%ymm5, %%ymm9, %%ymm5\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm4\n\t"
		"vpsubd %%ymm14, %%ymm1, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm5, %%ymm5\n\t"
		"vpminud %%ymm7, %%ymm1, %%ymm7\n\t"
		"vpminud %%ymm9, %%ymm2, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm6, %%ymm1\n\t"
		"vpminud %%ymm2, %%ymm3, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm7, %%ymm3\n\t"
		"vpaddd %%ymm9, %%ymm6, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm1, %%ymm9\n\t"
		"vpsubd %%ymm2, %%ymm3, %%ymm3\n\t"
		"vpaddd %%ymm2, %%ymm7, %%ymm2\n\t"
		"vpaddd %%ymm14, %%ymm8, %%ymm7\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm5\n\t"
		"vpaddd %%ymm14, %%ymm0, %%ymm10\n\t"
		"vpsubd %%ymm4, %%ymm7, %%ymm7\n\t"
		"vpsrlq $32, %%ymm9, %%ymm1\n\t"
		"vpsubd %%ymm5, %%ymm10, %%ymm10\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpaddd %%ymm4, %%ymm8, %%ymm4\n\t"
		"vpaddd %%ymm5, %%ymm0, %%ymm5\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm8\n\t"
		"vpmulld (%[w]), %%ymm9, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm8, %%ymm8\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm1, %%ymm0, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm7, %%ymm0\n\t"
		"vpmulld (%[w]), %%ymm7, %%ymm7\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm8, %%ymm0, %%ymm8\n\t"
		"vpsubd %%ymm1, %%ymm9, %%ymm1\n\t"
		"vpsrlq $32, %%ymm3, %%ymm9\n\t"
		"vpmuludq %%ymm11, %%ymm3, %%ymm0\n\t"
		"vpmuludq %%ymm11, %%ymm9, %%ymm9\n\t"
		"vpmulld %%ymm15, %%ymm8, %%ymm8\n\t"
		"vmovdqa %%ymm1, (%[a],%[h],2)\n\t"
		"vpsrlq $32, %%ymm10, %%ymm1\n\t"
		"vpmuludq %%ymm11, %%ymm1, %%ymm1\n\t"
		"vpmulld (%[w]), %%ymm3, %%ymm3\n\t"
		"vpsrlq $32, %%ymm0, %%ymm0\n\t"
		"vpblendd $170, %%ymm9, %%ymm0, %%ymm9\n\t"
		"vpsubd %%ymm8, %%ymm7, %%ymm8\n\t"
		"vpmuludq %%ymm11, %%ymm10, %%ymm7\n\t"
		"vpmulld (%[w]), %%ymm10, %%ymm10\n\t"
		"vpmulld %%ymm15, %%ymm9, %%ymm9\n\t"
		"vpsubd %%ymm14, %%ymm6, %%ymm0\n\t"
		"vmovdqa %%ymm8, (%[a],%[h3])\n\t"
		"vpsubd %%ymm14, %%ymm4, %%ymm8\n\t"
		"vpminud %%ymm0, %%ymm6, %%ymm0\n\t"
		"vpsrlq $32, %%ymm7, %%ymm7\n\t"
		"vpsubd %%ymm14, %%ymm2, %%ymm6\n\t"
		"vpminud %%ymm8, %%ymm4, %%ymm8\n\t"
		"vmovdqa %%ymm0, (%[a])\n\t"
		"vpblendd $170, %%ymm1, %%ymm7, %%ymm1\n\t"
		"vpsubd %%ymm14, %%ymm5, %%ymm7\n\t"
		"vpminud %%ymm6, %%ymm2, %%ymm6\n\t"
		"vpsubd %%ymm9, %%ymm3, %%ymm9\n\t"
		"vmovdqa %%ymm8, (%[a],%[h])\n\t"
		"vpmulld %%ymm15, %%ymm1, %%ymm1\n\t"
		"vpminud %%ymm7, %%ymm5, %%ymm7\n\t"
		"vmovdqa %%ymm6, 64(%[a])\n\t"
		"vmovdqa %%ymm7, 64(%[a],%[h])\n\t"
		"vmovdqa %%ymm9, 64(%[a],%[h],2)\n\t"
		"vpsubd %%ymm1, %%ymm10, %%ymm1\n\t"
		"vmovdqa %%ymm1, 64(%[a],%[h3])\n\t"
		"add $128, %[a]\n\t"
		"cmp %[end], %[a]\n\t"
		"jne 1b\n\t"
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}
namespace detail {
using namespace blocked_poly::detail;
using blocked_poly::detail::times;
struct Factors {
	Vec w, q;
};

inline Vec times(Vec x, const Factors& w) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, w.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), _mm256_srli_epi64(w.q, 32));
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, w.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline constexpr auto kInverseScales = [] {
	std::array<std::array<std::uint32_t, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(std::uint32_t(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Factors entries(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return {load(e), load(e + 8)};
}

}
}
namespace composition_poly {

namespace detail {

inline constexpr std::size_t kComposeBase = 32;

inline std::size_t compose_length(std::size_t n) { return std::bit_ceil(std::max<std::size_t>(n, 256)); }

inline void compose_direct(std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
						   std::span<std::uint32_t> h) {
	const std::size_t n = h.size();
	std::fill(h.begin(), h.end(), 0);
	for (std::size_t i = std::min(f.size(), n); i-- > 0;) {
		for (std::size_t k = n; k-- > 1;) {
			std::uint64_t sum = 0;
			for (std::size_t j = 1; j <= k && j < g.size(); ++j) sum += std::uint64_t(g[j]) * h[k - j] % kP;
			h[k] = std::uint32_t(sum % kP);
		}
		h[0] = f[i];
	}
}

constexpr std::uint32_t minus_twice(std::uint32_t x) {
	const std::uint32_t y = x ? 2 * kP - 2 * x : 0;
	return y >= kP ? y - kP : y;
}

inline Vec negate(Vec x) {
	return _mm256_min_epu32(_mm256_sub_epi32(broadcast(kP), x), _mm256_sub_epi32(_mm256_setzero_si256(), x));
}

inline Vec even_lanes(Vec x, Vec y) {
	const Vec t = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), 0x88));
	return _mm256_permute4x64_epi64(t, 0xD8);
}

inline void spread(Vec x, Vec (&out)[2]) {
	out[0] = _mm256_cvtepu32_epi64(_mm256_castsi256_si128(x));
	out[1] = _mm256_cvtepu32_epi64(_mm256_extracti128_si256(x, 1));
}

inline Vec twice_mod(Vec x) { return reduce(add(x, x), kP); }

inline void transpose(Vec (&r)[8]) {
	Vec t[8], u[8];
#pragma GCC unroll 4
	for (int i = 0; i < 8; i += 2) t[i] = _mm256_unpacklo_epi32(r[i], r[i + 1]), t[i + 1] = _mm256_unpackhi_epi32(r[i], r[i + 1]);
#pragma GCC unroll 2
	for (int i = 0; i < 8; i += 4)
#pragma GCC unroll 2
		for (int j = 0; j < 2; ++j)
			u[i + 2 * j] = _mm256_unpacklo_epi64(t[i + j], t[i + j + 2]), u[i + 2 * j + 1] = _mm256_unpackhi_epi64(t[i + j], t[i + j + 2]);
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i)
		r[i] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x20), r[i + 4] = _mm256_permute2x128_si256(u[i], u[i + 4], 0x31);
}

inline Vec product(Vec x, Vec y) { return _mm256_mul_epu32(x, y); }
inline Vec twice(Vec x) { return _mm256_add_epi64(x, x); }
inline Vec plus(Vec x, Vec y) { return _mm256_add_epi64(x, y); }
inline Vec minus(Vec x, Vec y) { return _mm256_sub_epi64(x, y); }

inline Vec offset() { return _mm256_set1_epi64x(static_cast<long long>(4 * std::uint64_t(kP) * kP)); }

template <std::size_t N>
[[gnu::always_inline]] inline void odd_lanes(const Vec (&x)[N], Vec (&out)[N]) {
#pragma GCC unroll 8
	for (std::size_t i = 0; i < N; ++i) out[i] = _mm256_srli_epi64(x[i], 32);
}

inline Vec reduce_sums(Vec even, Vec odd) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

[[gnu::always_inline]] inline void graeffe_sums(const Vec (&c)[8], const Vec (&tc)[8], Vec (&sum)[4]) {
	const auto x = [&c](int i, int j) { return product(c[i], c[j]); };
	const auto t = [&c, &tc](int i, int j) { return product(c[i], tc[j]); };
	sum[0] = minus(minus(plus(plus(x(0, 0), twice(t(2, 6))), plus(t(4, 4), offset())), twice(t(1, 7))), twice(t(3, 5)));
	sum[1] = minus(minus(plus(plus(twice(x(0, 2)), twice(t(4, 6))), offset()), x(1, 1)), plus(twice(t(3, 7)), t(5, 5)));
	sum[2] = minus(minus(plus(plus(twice(x(0, 4)), x(2, 2)), plus(t(6, 6), offset())), twice(x(1, 3))), twice(t(5, 7)));
	sum[3] = minus(minus(plus(plus(twice(x(0, 6)), twice(x(2, 4))), offset()), twice(x(1, 5))), plus(x(3, 3), t(7, 7)));
}

[[gnu::always_inline]] inline void graeffe_leaf(const Vec (&c)[8], const Vec (&tc)[8], Vec (&out)[4]) {
	Vec even[4], odd[4], c2[8], tc2[8];
	graeffe_sums(c, tc, even);
	odd_lanes(c, c2), odd_lanes(tc, tc2);
	graeffe_sums(c2, tc2, odd);
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) out[k] = low(reduce_sums(even[k], odd[k]));
}

[[gnu::always_inline]] inline void load_leaves(const std::uint32_t* from, Vec (&c)[8]) {
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) c[i] = load(from + 16 * i);
	transpose(c);
}

template <bool kNegative>
[[gnu::always_inline]] inline void scaled_leaves(const Vec (&c)[8], const Factors& s, int first, Vec (&tc)[8]) {
#pragma GCC unroll 8
	for (int j = 0; j < 8; ++j) {
		const Vec x = reduce(times(c[j], s), kP);
		tc[j] = j < first ? _mm256_setzero_si256() : kNegative ? negate(x) : x;
	}
}

inline void combine_pairs(const Vec (&x)[4], const Vec (&y)[4], const Factors& inverse, std::uint32_t* out) {
	Vec r[8];
#pragma GCC unroll 4
	for (int i = 0; i < 4; ++i) r[i] = low(add(x[i], y[i])), r[i + 4] = times(diff(x[i], y[i]), inverse);
	transpose(r);
#pragma GCC unroll 8
	for (int i = 0; i < 8; ++i) store(out + 8 * i, r[i]);
}

struct Tables {
	std::uint32_t *roots, *inverse_roots;

	static std::size_t words(std::size_t m) { return 2 * Arena::footprint(blocked_ntt::detail::table_words(std::countr_zero(m) + 2)); }

	Tables(std::span<std::uint32_t> memory, std::size_t m) {
		roots = memory.data(), inverse_roots = roots + Arena::footprint(blocked_ntt::detail::table_words(std::countr_zero(m) + 2));
		std::fill(memory.begin(), memory.end(), 0u);
		blocked_ntt::detail::build_table(roots, m / 4, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots, m / 4, blocked_ntt::detail::kRoots[1]);
	}
};

struct PairWeights {
	Factors s, inverse;
	PairWeights(const Tables& tables, std::size_t k) : s(entries(tables.roots, k)), inverse(entries(tables.inverse_roots, k)) {}
};

template <int kParity>
[[gnu::always_inline]] inline void product_sums(const Vec (&a)[8], const Vec (&c)[8], const Vec (&tc)[8],
												Vec (&sum)[4]) {
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) {
		Vec s = offset();
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) {
			const int j = 2 * k + kParity - i, l = j + 8;
			if (j >= 0) s = j % 2 ? minus(s, product(a[i], c[j])) : plus(s, product(a[i], c[j]));
			if (l < 8) s = l % 2 ? minus(s, product(a[i], tc[l])) : plus(s, product(a[i], tc[l]));
		}
		sum[k] = s;
	}
}

template <int kParity>
[[gnu::always_inline]] inline void product_leaf(const Vec (&a)[8], const Vec (&c)[8], const Vec (&tc)[8],
												Vec (&out)[4]) {
	Vec even[4], odd[4], a2[8], c2[8], tc2[8];
	product_sums<kParity>(a, c, tc, even);
	odd_lanes(a, a2), odd_lanes(c, c2), odd_lanes(tc, tc2);
	product_sums<kParity>(a2, c2, tc2, odd);
#pragma GCC unroll 4
	for (int k = 0; k < 4; ++k) out[k] = low(reduce_sums(even[k], odd[k]));
}

template <bool kNumerator>
struct FirstLevelProducts {
	static constexpr int kOutputs = kNumerator ? 5 : 3;
	const Tables* tables;
	const std::uint32_t *q1, *q2, *p1;
	std::uint32_t* out[kOutputs];

	[[gnu::flatten]] void pairs(std::size_t k) const {
		const PairWeights weights(*tables, k);
		Vec a[kOutputs][4], b[kOutputs][4];
		leaves<false>(16 * k, weights.s, a);
		leaves<true>(16 * k + 8, weights.s, b);
#pragma GCC unroll 5
		for (int i = 0; i < kOutputs; ++i) combine_pairs(a[i], b[i], weights.inverse, out[i] + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] void leaves(std::size_t at, const Factors& s, Vec (&r)[kOutputs][4]) const {
		Vec c1[8], c2[8], a[8], t1[8], t2[8];
		load_leaves(q1 + at, c1), load_leaves(q2 + at, c2);
		if constexpr (kNumerator) load_leaves(p1 + at, a);
		scaled_leaves<kNegative>(c1, s, kNumerator ? 2 : 4, t1);
		scaled_leaves<kNegative>(c2, s, 1, t2);
		graeffe_leaf(c1, t1, r[0]);
		graeffe_leaf(c2, t2, r[1]);
		product_leaf<0>(c1, c2, t2, r[2]);
		if constexpr (kNumerator) {
			product_leaf<1>(a, c1, t1, r[3]);
			product_leaf<1>(a, c2, t2, r[4]);
		}
	}
};

inline std::uint32_t graeffe_scale(int lg) {
	return blocked_ntt::detail::multiply_mod(kInverseScales[0][lg], std::uint32_t((std::uint64_t(1) << 31) % kP));
}

struct LevelBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Tables* tables;
	std::uint32_t* graeffe;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		ForwardBottom{tables->roots}(a, count, first);
		if (graeffe)
			for (std::size_t i = 0; i < count; i += 4) pairs(a + 32 * i, 2 * (first + i));
	}

	[[gnu::flatten]] void pairs(const std::uint32_t* q, std::size_t k) const {
		const PairWeights weights(*tables, k);
		Vec va[4], vb[4];
		leaves<false>(q, weights.s, va);
		leaves<true>(q + 8, weights.s, vb);
		combine_pairs(va, vb, weights.inverse, graeffe + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] static void leaves(const std::uint32_t* q, const Factors& s, Vec (&v)[4]) {
		Vec c[8], tc[8];
		load_leaves(q, c);
		scaled_leaves<kNegative>(c, s, 4, tc);
		graeffe_leaf(c, tc, v);
	}
};

struct CompositionBottom {
	static constexpr bool kForward = false, kInverse = true;
	const Tables* tables;
	const std::uint32_t *f, *q;

	template <int kParity>
	[[gnu::always_inline]] static void sums(const Vec (&p)[4], const Vec (&wp)[4], const Vec (&c)[8], Vec (&sum)[4]) {
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) {
			const Vec first = product(p[0], c[2 * k + kParity]);
			Vec s = kParity ? minus(offset(), first) : first;
#pragma GCC unroll 3
			for (int i = 1; i < 4; ++i) {
				const Vec term = product(i <= k ? p[i] : wp[i], c[2 * ((k - i + 4) % 4) + kParity]);
				s = kParity ? minus(s, term) : plus(s, term);
			}
			sum[k] = s;
		}
	}

	[[gnu::always_inline]] static void leaf(const Vec (&p)[4], const Vec (&wp)[4], const Vec (&c)[8], Vec (&out)[8]) {
		Vec p2[4], wp2[4], c2[8], even[4], odd[4];
		odd_lanes(p, p2), odd_lanes(wp, wp2), odd_lanes(c, c2);
		sums<0>(p, wp, c, even);
		sums<0>(p2, wp2, c2, odd);
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) out[2 * k] = reduce_sums(even[k], odd[k]);
		sums<1>(p, wp, c, even);
		sums<1>(p2, wp2, c2, odd);
#pragma GCC unroll 4
		for (int k = 0; k < 4; ++k) out[2 * k + 1] = reduce_sums(even[k], odd[k]);
	}

	[[gnu::flatten]] void products(std::size_t p, Vec (&r)[16]) const {
		Vec x[8], c[8], pa[4], pb[4], spa[4], spb[4], ra[8], rb[8];
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) x[i] = load(f + 8 * (p + i));
		transpose(x);
		const Factors s = entries(tables->roots, p);
#pragma GCC unroll 4
		for (int i = 0; i < 4; ++i) {
			const Vec t = times(x[i + 4], s);
			pa[i] = canonical(add(x[i], t)), pb[i] = canonical(diff(x[i], t));
			spa[i] = reduce(times(pa[i], s), kP), spb[i] = negate(reduce(times(pb[i], s), kP));
		}
		load_leaves(q + 16 * p, c);
		leaf(pa, spa, c, ra);
		load_leaves(q + 16 * p + 8, c);
		leaf(pb, spb, c, rb);
		transpose(ra), transpose(rb);
#pragma GCC unroll 8
		for (int i = 0; i < 8; ++i) r[2 * i] = ra[i], r[2 * i + 1] = rb[i];
	}

	[[gnu::always_inline]] static void finish(const Tables& tables, const Vec (&r)[16], std::size_t g, std::uint32_t* out) {
#pragma GCC unroll 4
		for (int i = 0; i < 4; ++i) {
			Vec f[4] = {r[4 * i], r[4 * i + 1], r[4 * i + 2], r[4 * i + 3]};
			inverse_h1(f, Group(tables.inverse_roots, g + i));
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(out + 32 * i + 8 * t, f[t]);
		}
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		for (std::size_t j = 0; j < count; j += 4, out += 128) {
			Vec r[16];
			products(2 * (first + j), r);
			finish(*tables, r, first + j, out);
		}
	}
};

struct CompositionSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	CompositionBottom terms[2];

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		for (std::size_t j = 0; j < count; j += 4, out += 128) {
			Vec r[16], r2[16];
			terms[0].products(2 * (first + j), r);
			terms[1].products(2 * (first + j), r2);
#pragma GCC unroll 16
			for (int i = 0; i < 16; ++i) r[i] = low(add(r[i], r2[i]));
			CompositionBottom::finish(*terms[0].tables, r, first + j, out);
		}
	}
};

template <class Bottom>
void forward_with(std::span<std::uint32_t> a, std::size_t size, const Tables& tables, const Bottom& bottom) {
	const Recursion recursion(tables.roots, tables.inverse_roots, bottom);
	const Source in(a.data(), size, 0);
	const std::size_t nv = a.size() / 8;
	auto* v = reinterpret_cast<Vec*>(a.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		forward_top4(in, v, h, tables.roots);
		for (std::size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
	} else {
		const std::size_t h = nv / 2;
		forward_top2(in, v, h);
		recursion.visit(a.data(), h, 0);
		recursion.visit(a.data() + 8 * h, h, 1);
	}
}

template <class Bottom>
void inverse_with(std::span<std::uint32_t> a, const Tables& tables, const Bottom& bottom, std::uint32_t scale, Half output) {
	const Recursion recursion(tables.roots, tables.inverse_roots, bottom);
	const std::size_t nv = a.size() / 8;
	auto* v = reinterpret_cast<Vec*>(a.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		for (std::size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
		inverse_top4(v, h, output, tables.inverse_roots, Factor(scale));
	} else {
		const std::size_t h = nv / 2;
		recursion.visit(a.data(), h, 0);
		recursion.visit(a.data() + 8 * h, h, 1);
		inverse_top2(v, h, output, scale);
	}
}

inline void forward_columns(std::uint32_t* a, std::size_t h, std::size_t b, const std::uint32_t* roots, std::size_t k) {
	Vec* const v = reinterpret_cast<Vec*>(a);
	if (b == 0) return kernels::forward_even_columns(v, h, roots + slot(k), roots + slot(2 * k));
	kernels::forward_columns(v, h, std::size_t(1) << b, roots + slot(k), roots + slot(2 * k));
}

inline void inverse_columns(std::uint32_t* a, std::size_t h, std::size_t b, const std::uint32_t* inverse_roots,
							std::size_t k) {
	Vec* const v = reinterpret_cast<Vec*>(a);
	if (b == 0) return kernels::inverse_even_columns(v, h, inverse_roots + slot(k), inverse_roots + slot(2 * k));
	kernels::inverse_columns(v, h, std::size_t(1) << b, inverse_roots + slot(k), inverse_roots + slot(2 * k));
}

inline void forward_half(std::uint32_t* a, std::size_t h, bool high, const Group& w) {
	for (std::uint32_t* x = a; x < a + 8 * h; x += 8) {
		const Vec u = low(load(x));
		if (high) {
			const Vec b = load(x + 8 * h), yb = times(b, w.y), zb = times(b, w.z);
			store(x, add(u, yb)), store(x + 8 * h, diff(u, yb));
			store(x + 16 * h, add(u, zb)), store(x + 24 * h, diff(u, zb));
		} else {
			const Vec xc = times(load(x + 16 * h), w.x), sum = low(add(u, xc)), difference = low_difference(u, xc);
			store(x, sum), store(x + 8 * h, sum), store(x + 16 * h, difference), store(x + 24 * h, difference);
		}
	}
}

inline void inverse_half(std::uint32_t* a, std::size_t h, bool high, const Group& w) {
	for (std::uint32_t* x = a; x < a + 8 * h; x += 8) {
		const Vec f0 = load(x), f1 = load(x + 8 * h), f2 = load(x + 16 * h), f3 = load(x + 24 * h);
		const Vec ab = low(add(f0, f1)), cd = low(add(f2, f3));
		store(x, low(add(ab, cd)));
		if (high) store(x + 8 * h, low(add(times(diff(f0, f1), w.y), times(diff(f2, f3), w.z))));
		else store(x + 16 * h, times(diff(ab, cd), w.x));
	}
}

template <class Bottom>
class Pruned {
public:
	Pruned(const Tables& tables, const Bottom& bottom, std::size_t b)
		: recursion_(tables.roots, tables.inverse_roots, bottom), tables_(tables), b_(b) {}

	void forward(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		const std::size_t h = nv / 4, low = std::size_t(std::countr_zero(h));
		if (nv <= kForwardFull || low + 1 < b_) return recursion_.visit(a, nv, k);
		if (low > b_) {
			forward_columns(a, h, b_, tables_.roots, k);
			for (std::size_t t = 0; t < 4; ++t) forward(a + 8 * t * h, h, 4 * k + t);
			return;
		}
		forward_half(a, h, low + 1 == b_, Group(tables_.roots, k));
		for (std::size_t t = 0; t < 4; ++t) recursion_.visit(a + 8 * t * h, h, 4 * k + t);
	}

	void inverse(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		const std::size_t h = nv / 4, low = std::size_t(std::countr_zero(h));
		if (nv <= kInverseFull || low + 1 < b_) return recursion_.visit(a, nv, k);
		if (low > b_) {
			for (std::size_t t = 0; t < 4; ++t) inverse(a + 8 * t * h, h, 4 * k + t);
			return inverse_columns(a, h, b_, tables_.inverse_roots, k);
		}
		for (std::size_t t = 0; t < 4; ++t) recursion_.visit(a + 8 * t * h, h, 4 * k + t);
		inverse_half(a, h, low + 1 == b_, Group(tables_.inverse_roots, k));
	}

private:
	static constexpr std::size_t kForwardFull = 64, kInverseFull = 16;

	Recursion<Bottom> recursion_;
	const Tables& tables_;
	std::size_t b_;
};

template <class Bottom>
void forward_pruned(std::span<std::uint32_t> a, std::size_t size, std::size_t b, const Tables& tables,
					const Bottom& bottom) {
	const Pruned<Bottom> pruned(tables, bottom, b);
	const Source in(a.data(), size, 0);
	const std::size_t nv = a.size() / 8, width = std::size_t(1) << b;
	auto* v = reinterpret_cast<Vec*>(a.data());
	const Vec p = broadcast(kP), zero = _mm256_setzero_si256();
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		const Factor z(tables.roots[1], tables.roots[9]);
		for (std::size_t c = 0; c < h; c += 2 * width) {
			for (std::size_t j = c; j < c + width; ++j) {
				const Vec x0 = in(j), x1 = in(j + h), x2 = in(j + 2 * h), x3 = in(j + 3 * h);
				const Vec ac = add(x0, x2), amc = _mm256_sub_epi32(add(x0, p), x2);
				const Vec bd = add(x1, x3), zbmd = times(_mm256_sub_epi32(add(x1, p), x3), z);
				v[j] = add(ac, bd), v[j + h] = diff(ac, bd);
				v[j + 2 * h] = add(amc, zbmd), v[j + 3 * h] = diff(amc, zbmd);
			}
			for (std::size_t j = c + width; j < c + 2 * width; ++j)
				v[j] = v[j + h] = v[j + 2 * h] = v[j + 3 * h] = zero;
		}
		for (std::size_t t = 0; t < 4; ++t) pruned.forward(a.data() + 8 * t * h, h, t);
	} else {
		const std::size_t h = nv / 2;
		for (std::size_t c = 0; c < h; c += 2 * width) {
			for (std::size_t j = c; j < c + width; ++j) {
				const Vec x0 = in(j), x1 = in(j + h);
				v[j] = add(x0, x1), v[j + h] = _mm256_sub_epi32(add(x0, p), x1);
			}
			for (std::size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = zero;
		}
		pruned.forward(a.data(), h, 0);
		pruned.forward(a.data() + 8 * h, h, 1);
	}
}

template <class Bottom>
void inverse_pruned(std::span<std::uint32_t> a, std::size_t b, const Tables& tables, const Bottom& bottom,
					std::uint32_t scale, Half output = Half::kBoth) {
	const Pruned<Bottom> pruned(tables, bottom, b);
	const std::size_t nv = a.size() / 8, width = std::size_t(1) << b;
	auto* v = reinterpret_cast<Vec*>(a.data());
	const Factor s(scale);
	const auto scaled = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	const bool lower = output != Half::kUpper, upper = output != Half::kLower;
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		for (std::size_t t = 0; t < 4; ++t) pruned.inverse(a.data() + 8 * t * h, h, t);
		const Factor z(tables.inverse_roots[1], tables.inverse_roots[9]);
		for (std::size_t c = 0; c < h; c += 2 * width)
			for (std::size_t j = c; j < c + width; ++j) {
				const Vec x0 = v[j], x1 = v[j + h], x2 = v[j + 2 * h], x3 = v[j + 3 * h];
				const Vec ab = low(add(x0, x1)), cd = low(add(x2, x3));
				const Vec amb = low(diff(x0, x1)), cmd = times(diff(x2, x3), z);
				if (lower) v[j] = scaled(add(ab, cd)), v[j + h] = scaled(add(amb, cmd));
				if (upper) v[j + 2 * h] = scaled(diff(ab, cd)), v[j + 3 * h] = scaled(diff(amb, cmd));
			}
	} else {
		const std::size_t h = nv / 2;
		pruned.inverse(a.data(), h, 0);
		pruned.inverse(a.data() + 8 * h, h, 1);
		for (std::size_t c = 0; c < h; c += 2 * width)
			for (std::size_t j = c; j < c + width; ++j) {
				const Vec x0 = v[j], x1 = v[j + h];
				if (lower) v[j] = scaled(add(x0, x1));
				if (upper) v[j + h] = scaled(diff(x0, x1));
			}
	}
}

class Carve {
public:
	explicit Carve(std::span<std::uint32_t> scratch) : rest_(scratch) {}

	static constexpr std::size_t words(std::size_t n) { return Arena::footprint(n) + 8; }

	std::span<std::uint32_t> take(std::size_t n) {
		const std::size_t skip = (0 - reinterpret_cast<std::uintptr_t>(rest_.data()) / 4) % 16;
		if (skip + Arena::footprint(n) > rest_.size()) std::abort();
		const std::span<std::uint32_t> s = rest_.subspan(skip, n);
		rest_ = rest_.subspan(skip + Arena::footprint(n));
		return s;
	}

private:
	std::span<std::uint32_t> rest_;
};

struct Levels {
	std::size_t m;
	int lg;
	Tables tables;
	std::span<std::uint32_t> transforms;

	std::span<std::uint32_t> level(int s) const { return transforms.subspan(std::size_t(s) * Arena::footprint(4 * m), 4 * m); }
	
	std::span<std::uint32_t> first_transform(std::size_t i) const { return level(1).subspan(i * Arena::footprint(m), m); }
	
	std::span<std::uint32_t> column_transform(std::size_t k) const {
		return level(lg - 3).subspan((k - 1) * Arena::footprint(m / 4), m / 4);
	}

	static std::size_t scratch(std::size_t m) {
		return Arena::footprint(Tables::words(m)) + Carve::words(std::size_t(std::countr_zero(m)) * Arena::footprint(4 * m));
	}

	Levels(std::size_t size, std::span<std::uint32_t> scratch)
		: m(size), lg(std::countr_zero(size)), tables(scratch.first(Tables::words(size)), size) {
		Carve carve(scratch.subspan(Arena::footprint(Tables::words(size))));
		transforms = carve.take(std::size_t(lg) * Arena::footprint(4 * m));
	}
};

inline void next_level(std::span<std::uint32_t> a, std::size_t stride, std::size_t rows) {
	const std::size_t half = stride / 2, top = rows * stride;
	std::copy(a.begin(), a.begin() + std::ptrdiff_t(half), a.begin() + std::ptrdiff_t(top));
	a[top] = a[top] ? a[top] - 1 : kP - 1;
	a[0] = 1;
	std::fill(a.begin() + 1, a.begin() + std::ptrdiff_t(half), 0);
}

inline void first_level(const Transform& t, std::span<const std::uint32_t> g, Levels& levels) {
	const std::size_t m = levels.m, h = m / 2, quarter = m / 4;
	const Tables& tables = levels.tables;
	const std::span<const std::uint32_t> v = levels.level(3).first(h);
	const std::span<std::uint32_t> t1 = levels.first_transform(0), t2 = levels.first_transform(1);
	const std::size_t even = std::max<std::size_t>(std::min(h, (g.size() + 1) / 2), 1);
	std::size_t j = 0;
	for (; j + 8 <= even && 2 * j + 16 <= g.size(); j += 8) {
		const Vec x = even_lanes(_mm256_loadu_si256(reinterpret_cast<const Vec*>(g.data() + 2 * j)),
								 _mm256_loadu_si256(reinterpret_cast<const Vec*>(g.data() + 2 * j + 8)));
		store(t1.data() + j, negate(twice_mod(x)));
	}
	for (; j < even; ++j) t1[j] = minus_twice(g[2 * j]);
	t1[0] = 0;
	std::fill(t1.begin() + std::ptrdiff_t(even), t1.begin() + std::ptrdiff_t(h), 0);
	const std::span<std::uint32_t> q = levels.level(2);
	for (std::size_t i = 0; i < quarter; i += 8)
		store(q.data() + h + i, twice_mod(even_lanes(load(t1.data() + 2 * i), load(t1.data() + 2 * i + 8))));
	t.forward(t1.first(h), 0, t1);
	t.forward(v, 0, t2);
	std::uint32_t* out[3];
	for (std::size_t i = 0; i < 3; ++i) out[i] = levels.level(3).data() + Arena::footprint(m) + i * Arena::footprint(h);
	const FirstLevelProducts<false> products{&tables, t1.data(), t2.data(), nullptr, {out[0], out[1], out[2]}};
	for (std::size_t k = 0; k < m / 16; k += 8) products.pairs(k);
	for (std::uint32_t* x : out)
		inverse_with(std::span<std::uint32_t>(x, h), tables, InverseBottom{tables.inverse_roots, x}, graeffe_scale(levels.lg - 1),
					 Half::kLower);
	q[0] = 1;
	std::fill(q.begin() + 1, q.begin() + std::ptrdiff_t(quarter), 0);
	for (std::size_t i = 0; i < quarter; i += 8) {
		const Vec v2 = twice_mod(even_lanes(load(v.data() + 2 * i), load(v.data() + 2 * i + 8)));
		store(q.data() + 2 * h + i, reduce(add(v2, load(out[0] + i)), kP));
		store(q.data() + 3 * h + i, twice_mod(load(out[2] + i)));
	}
	std::copy_n(out[1], quarter, q.begin() + std::ptrdiff_t(4 * h));
}

inline void first_level_transposed(const Transform& t, const Levels& levels, std::span<const std::uint32_t> in,
								   std::span<std::uint32_t> out) {
	const std::size_t m = levels.m, h = m / 2, quarter = m / 4;
	const Tables& tables = levels.tables;
	const auto row = [&](std::size_t k) { return in.subspan(2 * m + k * h, quarter); };
	std::span<std::uint32_t> tp[3];
	for (std::size_t k = 0; k < 3; ++k) {
		tp[k] = out.subspan(k * Arena::footprint(h), h);
		t.forward(row(k), 0, tp[k]);
	}
	const std::uint32_t *q1 = levels.first_transform(0).data(), *q2 = levels.first_transform(1).data();
	const auto term = [&](std::size_t k, const std::uint32_t* q) { return CompositionBottom{&tables, tp[k].data(), q}; };
	for (std::size_t r = 2; r < 4; ++r) {
		const std::span<std::uint32_t> result = out.subspan(r * m, m);
		inverse_with(result, tables, CompositionSumBottom{{term(r - 1, q1), term(r - 2, q2)}}, kInverseScales[1][levels.lg],
					 Half::kLower);
		const std::span<const std::uint32_t> add_row = row(r);
		for (std::size_t i = 0; i < quarter; i += 8) {
			Vec x[2];
			spread(load(add_row.data() + i), x);
			store(result.data() + 2 * i, reduce(add(load(result.data() + 2 * i), x[0]), kP));
			store(result.data() + 2 * i + 8, reduce(add(load(result.data() + 2 * i + 8), x[1]), kP));
		}
	}
}

template <std::size_t K>
void inverse_product_sum(const Tables& tables, const Transform::Pair* pairs, std::span<std::uint32_t> out, Half output) {
	InverseProductSumBottom<K> bottom{};
	for (std::size_t k = 0; k < K; ++k) bottom.terms[k] = {{tables.roots, tables.inverse_roots, pairs[k].b.data()}, pairs[k].a.data()};
	inverse_with(out, tables, bottom, kInverseScales[1][std::countr_zero(out.size())], output);
}

inline void multiply_constant(std::span<const std::uint32_t> a, std::uint32_t c, std::span<std::uint32_t> t) {
	const Factor f(c);
	for (std::size_t i = 0; i < a.size(); i += 8) store(t.data() + i, reduce(times(load(a.data() + i), f), kP));
}

inline void third_last_level(const Transform& t, const Levels& levels, const std::span<std::uint32_t> (&c)[4]) {
	const std::size_t m = levels.m, quarter = m / 4, eighth = m / 8;
	const std::span<const std::uint32_t> q = levels.level(levels.lg - 3);
	const std::span<std::uint32_t> temporary = levels.level(levels.lg - 1);
	std::span<std::uint32_t> coefficients[8], tc[8];
	for (std::size_t k = 1; k < 8; ++k) {
		coefficients[k] = temporary.subspan((k - 1) * Arena::footprint(eighth + 1), eighth + 1);
		tc[k] = levels.column_transform(k);
	}
	for (std::size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i) x[i] = load(q.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 7
		for (std::size_t k = 1; k < 8; ++k) store(coefficients[k].data() + r, k % 2 ? negate(x[k]) : x[k]);
	}
	for (std::size_t k = 1; k < 8; ++k) {
		const std::uint32_t x = q[16 * eighth + k];
		coefficients[k][eighth] = k % 2 && x ? kP - x : x;
	}
	for (std::size_t k = 1; k < 8; ++k) t.forward(coefficients[k], 0, tc[k]);
	std::span<std::uint32_t> twice[3], square[4];
	const std::span<std::uint32_t> sums = temporary.subspan(7 * Arena::footprint(eighth + 1));
	for (std::size_t i = 0; i < 3; ++i) twice[i] = sums.subspan(i * Arena::footprint(quarter), quarter);
	for (std::size_t k = 1; k < 4; ++k) square[k] = sums.subspan((k + 2) * Arena::footprint(quarter), quarter);
	multiply_constant(tc[5], 2, twice[0]), multiply_constant(tc[3], kP - 2, twice[1]), multiply_constant(tc[4], kP - 2, twice[2]);
	const Transform::Pair first[1] = {{tc[1], tc[1]}}, second[2] = {{tc[2], tc[2]}, {tc[1], twice[1]}},
						  third[3] = {{tc[3], tc[3]}, {tc[1], twice[0]}, {tc[2], twice[2]}};
	t.inverse_product_sum(first, square[1]);
	t.inverse_product_sum(second, square[2]);
	t.inverse_product_sum(third, square[3]);
	const std::uint32_t weight[4] = {0, kP - 2, 2, kP - 2};
	for (std::size_t k = 1; k < 4; ++k) {
		const std::span<const std::uint32_t> linear = coefficients[2 * k];
		const Factor w(weight[k]);
		for (std::size_t i = 0; i < quarter; i += 8) {
			const Vec x = load(square[k].data() + i);
			store(c[k].data() + i, i < eighth ? canonical(add(x, reduce(times(load(linear.data() + i), w), kP))) : x);
		}
		const std::uint64_t last = c[k][eighth] + std::uint64_t(weight[k]) * linear[eighth];
		c[k][eighth] = std::uint32_t(last % kP);
		c[k][quarter] = c[k][0];
		c[k][0] = 0;
		std::fill(c[k].begin() + std::ptrdiff_t(quarter + 1), c[k].end(), 0);
	}
}

inline void third_last_level_transposed(const Transform& t, const Levels& levels, const std::span<const std::uint32_t> (&p)[4],
										std::span<std::uint32_t> work, std::span<std::uint32_t> out) {
	const std::size_t m = levels.m, quarter = m / 4, eighth = m / 8;
	std::span<std::uint32_t> tp[4], r[8];
	for (std::size_t j = 0; j < 4; ++j) {
		tp[j] = work.subspan(j * Arena::footprint(quarter), quarter);
		t.forward(p[j], 0, tp[j]);
	}
	for (std::size_t c = 1; c < 8; ++c) {
		r[c] = work.subspan((c + 3) * Arena::footprint(quarter), quarter);
		Transform::Pair pairs[4];
		std::size_t count = 0;
		for (std::size_t j = 0; j < 4 && 2 * j < c; ++j) pairs[count++] = {tp[j], levels.column_transform(c - 2 * j)};
		if (count < 4) t.inverse_product_sum(std::span(pairs, count), r[c], Half::kUpper);
		else inverse_product_sum<4>(levels.tables, pairs, r[c], Half::kUpper);
	}
	for (std::size_t i = eighth; i < quarter; i += 8) {
		Vec x[8];
		x[0] = load(p[0].data() + i);
#pragma GCC unroll 7
		for (std::size_t c = 1; c < 8; ++c) x[c] = c % 2 ? load(r[c].data() + i) : canonical(add(load(r[c].data() + i), load(p[c / 2].data() + i)));
		transpose(x);
#pragma GCC unroll 8
		for (std::size_t j = 0; j < 8; ++j) store(out.data() + 2 * m + 16 * (i - eighth + j), x[j]);
	}
}

inline void build_levels(const Transform& t, std::span<const std::uint32_t> g, Levels& levels) {
	const std::size_t m = levels.m;
	const std::span<std::uint32_t> v = levels.level(3).first(m), q0 = levels.level(0).first(2 * m);
	std::fill(q0.begin(), q0.end(), 0);
	std::copy(g.begin() + std::ptrdiff_t(std::min<std::size_t>(g.size(), 1)), g.begin() + std::ptrdiff_t(std::min(g.size(), m)),
			  q0.begin() + 1);
	const Tables& tables = levels.tables;
	forward_with(q0, m, tables, LevelBottom{&tables, v.data()});
	inverse_with(v, tables, InverseBottom{tables.inverse_roots, v.data()}, graeffe_scale(levels.lg), Half::kBoth);
	first_level(t, g, levels);
	for (int s = 2; s + 3 < levels.lg; ++s) {
		const std::size_t stride = 2 * (m >> s), pad = std::size_t(levels.lg - s - 3);
		const std::span<std::uint32_t> next = levels.level(s + 1).first(2 * m);
		forward_pruned(levels.level(s), ((std::size_t(1) << s) + 1) * stride, pad, tables, LevelBottom{&tables, next.data()});
		inverse_pruned(next, pad - 1, tables, InverseBottom{tables.inverse_roots, next.data()}, graeffe_scale(levels.lg + 1));
		next_level(levels.level(s + 1), stride / 2, std::size_t(2) << s);
	}
	std::span<std::uint32_t> c[4];
	for (std::size_t k = 1; k < 4; ++k) c[k] = levels.level(levels.lg - 2).subspan((k - 1) * Arena::footprint(m / 2), m / 2);
	third_last_level(t, levels, c);
	const std::span<std::uint32_t> q = levels.level(levels.lg - 1).first(m);
	std::fill(q.begin(), q.end(), 0);
	for (std::size_t i = 0; i < m / 4; i += 8) store(q.data() + i, negate(twice_mod(load(c[2].data() + i))));
	q[m / 4] = minus_twice(c[2][m / 4]);
	const std::span<std::uint32_t> square = levels.level(levels.lg - 1).subspan(Arena::footprint(m), m / 2);
	for (std::size_t k = 1; k < 4; ++k) t.forward(c[k]);
	t.inverse_product(c[1], c[1], square);
	for (std::size_t i = 0; i < m / 2; i += 8) store(q.data() + i, reduce(add(load(q.data() + i), load(square.data() + i)), kP));
	const std::uint32_t top = q[m / 2] + q[0];
	q[m / 2] = top >= kP ? top - kP : top;
	q[0] = 0;
	t.forward(q);
}

}

inline int compose_log(std::size_t n) {
	if (n <= detail::kComposeBase) return Transform::kMinLog;
	return std::countr_zero(detail::compose_length(n)) + 1;
}

inline std::size_t compose_scratch(std::size_t n) {
	if (n <= detail::kComposeBase) return 0;
	return detail::Levels::scratch(detail::compose_length(n));
}

inline void compose(const Transform& t, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
					std::span<std::uint32_t> h, std::span<std::uint32_t> scratch) {
	const std::size_t n = h.size();
	if (f.empty() || g.size() <= 1) {
		std::fill(h.begin(), h.end(), 0u);
		if (n && !f.empty()) h[0] = f[0];
		return;
	}
	if (n <= detail::kComposeBase) return detail::compose_direct(f, g, h);
	detail::Levels levels(detail::compose_length(n), scratch);
	detail::build_levels(t, g, levels);
	const std::size_t m = levels.m;
	const int lg = levels.lg;

	const std::span<std::uint32_t> last = levels.level(lg - 1);
	const std::span<std::uint32_t> p = last.subspan(Arena::footprint(m), m), pq = last.subspan(2 * Arena::footprint(m), m);
	{
		const std::size_t size = std::min(f.size(), m);
		std::fill(p.begin(), p.end() - std::ptrdiff_t(size), 0);
		std::uint32_t* const to = p.data() + (m - size);
		const detail::Vec reverse = _mm256_setr_epi32(7, 6, 5, 4, 3, 2, 1, 0);
		std::size_t j = 0;
		for (; j + 8 <= size; j += 8) {
			const detail::Vec x = _mm256_loadu_si256(reinterpret_cast<const detail::Vec*>(f.data() + size - 8 - j));
			_mm256_storeu_si256(reinterpret_cast<detail::Vec*>(to + j), _mm256_permutevar8x32_epi32(x, reverse));
		}
		for (; j < size; ++j) to[j] = f[size - 1 - j];
	}
	t.cyclic_product(p, 0, pq, last.first(m), Half::kUpper);

	{
		const std::size_t h = m / 2, quarter = m / 4;
		const std::span<std::uint32_t> p0 = p.subspan(h), p1 = pq.subspan(h), t0 = last.first(h), t1 = last.subspan(Arena::footprint(h), h);
		const std::span<std::uint32_t> c = levels.level(lg - 2);
		const auto column = [&](std::size_t k) { return c.subspan((k - 1) * Arena::footprint(h), h); };
		const std::span<std::uint32_t> r1 = p.first(h), r2 = pq.first(h), r3 = last.subspan(3 * Arena::footprint(m), h);
		t.forward(p0, 0, t0);
		t.forward(p1, 0, t1);
		t.inverse_product(t0, column(1), r1, Half::kUpper);
		t.inverse_product(t0, column(2), r2, Half::kUpper);
		const Transform::Pair pairs[2] = {{t1, column(1)}, {t0, column(3)}};
		t.inverse_product_sum(pairs, r3, Half::kUpper);
		for (std::size_t i = quarter; i < h; i += 8)
			detail::store(r2.data() + i, detail::reduce(detail::add(detail::load(p1.data() + i), detail::load(r2.data() + i)), kModulus));
		const std::span<const std::uint32_t> columns[4] = {p0.subspan(quarter), r1.subspan(quarter), r2.subspan(quarter),
														   r3.subspan(quarter)};
		detail::third_last_level_transposed(t, levels, columns, levels.level(lg - 2), levels.level(lg - 3));
	}
	
	std::span<std::uint32_t> in = levels.level(lg - 3);
	const std::uint32_t scale = detail::kInverseScales[1][lg + 2];
	for (int s = lg - 4; s >= 2; --s) {
		const std::span<std::uint32_t> x = in.subspan(2 * m), out = levels.level(s == lg - 4 ? lg - 2 : s + 1);
		const std::size_t pad = std::size_t(lg - s - 3);
		detail::forward_pruned(x, 2 * m, pad - 1, levels.tables, detail::ForwardBottom{levels.tables.roots});
		detail::inverse_pruned(out, pad, levels.tables, detail::CompositionBottom{&levels.tables, x.data(), levels.level(s).data()},
							   scale, Half::kUpper);
		in = out;
	}
	detail::first_level_transposed(t, levels, in, levels.level(2));
	in = levels.level(2);
	const std::span<std::uint32_t> out = levels.level(1);
	
	const std::span<std::uint32_t> p0 = in.subspan(2 * m, m), r = out.first(2 * m);
	t.forward(p0.first(m / 2), 0, p0);
	detail::inverse_with(r, levels.tables, detail::CompositionBottom{&levels.tables, p0.data(), levels.level(0).data()},
						 detail::kInverseScales[1][levels.lg + 1], Half::kLower);
	std::size_t k = 0;
	for (; k + 16 <= n; k += 16) {
		detail::Vec a[2];
		detail::spread(detail::load(in.data() + 3 * m + k / 2), a);
		for (std::size_t t = 0; t < 2; ++t)
			_mm256_storeu_si256(reinterpret_cast<detail::Vec*>(h.data() + k + 8 * t), detail::difference(a[t], detail::load(r.data() + k + 8 * t)));
	}
	for (; k < n; ++k) {
		const std::uint32_t a = k % 2 ? 0 : in[3 * m + k / 2];
		h[k] = a >= r[k] ? a - r[k] : a + kModulus - r[k];
	}
}

}
namespace composition_poly {

namespace detail {

inline std::size_t projection_length(std::size_t n) { return std::bit_ceil(std::max<std::size_t>(n, 512)); }

inline void projection_direct(std::span<const std::uint32_t> g, std::span<std::uint32_t> a) {
	const std::size_t n = a.size();
	std::uint32_t power[kComposeBase] = {1}, next[kComposeBase];
	for (std::size_t k = 0; k < n; ++k) {
		a[k] = power[n - 1];
		for (std::size_t i = 0; i < n; ++i) {
			std::uint64_t c = 0;
			for (std::size_t j = 1; j <= i && j < g.size(); ++j) c += std::uint64_t(g[j]) * power[i - j] % kP;
			next[i] = std::uint32_t(c % kP);
		}
		std::copy_n(next, n, power);
	}
}

inline Vec odd_lanes(Vec x, Vec y) {
	const Vec t = _mm256_castps_si256(_mm256_shuffle_ps(_mm256_castsi256_ps(x), _mm256_castsi256_ps(y), 0xDD));
	return _mm256_permute4x64_epi64(t, 0xD8);
}

template <bool kAdd, class Op>
void even_terms(std::span<const std::uint32_t> b, std::size_t lo, std::size_t hi, std::ptrdiff_t offset,
				std::span<std::uint32_t> out, const Op& op) {
	const auto scalar = [&op](std::uint32_t x) {
		return std::uint32_t(_mm_cvtsi128_si32(_mm256_castsi256_si128(op(_mm256_set1_epi32(int(x))))));
	};
	const auto put = [&out](std::size_t j, std::uint32_t x) {
		if constexpr (kAdd) out[j] = out[j] + x >= kP ? out[j] + x - kP : out[j] + x;
		else out[j] = x;
	};
	const auto at = [offset](std::size_t j) { return std::ptrdiff_t(2 * j) + offset; };
	const auto first = [&](std::size_t i) {
		const std::ptrdiff_t d = std::ptrdiff_t(i) - offset;
		return std::min(out.size(), d <= 0 ? std::size_t(0) : std::size_t(d + 1) / 2);
	};
	const std::size_t begin = first(lo), end = std::max(begin, first(hi));
	if constexpr (!kAdd) {
		std::fill(out.begin(), out.begin() + std::ptrdiff_t(begin), 0);
		std::fill(out.begin() + std::ptrdiff_t(end), out.end(), 0);
	}
	std::size_t j = begin;
	for (; j + 8 <= end && std::size_t(at(j)) + 16 <= b.size(); j += 8) {
		const std::uint32_t* from = b.data() + at(j);
		Vec x = op(even_lanes(load_unaligned(from), load_unaligned(from + 8)));
		if constexpr (kAdd) x = reduce(add(load_unaligned(out.data() + j), x), kP);
		store_unaligned(out.data() + j, x);
	}
	for (; j < end; ++j) put(j, scalar(b[std::size_t(at(j))]));
}

inline void add_into(std::span<const std::uint32_t> x, std::span<const std::uint32_t> y, std::span<std::uint32_t> out) {
	for (std::size_t i = 0; i < out.size(); i += 8) store(out.data() + i, reduce(add(load(x.data() + i), load(y.data() + i)), kP));
}

struct ProjectionBottom {
	static constexpr bool kForward = true, kInverse = false;
	const Tables* tables;
	const std::uint32_t* p;
	std::uint32_t *v, *w;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		ForwardBottom{tables->roots}(a, count, first);
		for (std::size_t i = 0; i < count; i += 4) pairs(a + 32 * i, 2 * (first + i));
	}

	[[gnu::flatten]] void pairs(const std::uint32_t* q, std::size_t k) const {
		const PairWeights weights(*tables, k);
		Vec va[4], vb[4], wa[4], wb[4];
		leaves<false>(q, p + 16 * k, weights.s, va, wa);
		leaves<true>(q + 8, p + 16 * k + 8, weights.s, vb, wb);
		combine_pairs(va, vb, weights.inverse, v + 8 * k);
		combine_pairs(wa, wb, weights.inverse, w + 8 * k);
	}

	template <bool kNegative>
	[[gnu::noinline, gnu::flatten]] static void leaves(const std::uint32_t* q, const std::uint32_t* p, const Factors& s,
													   Vec (&v)[4], Vec (&w)[4]) {
		Vec c[8], a[8], tc[8];
		load_leaves(q, c), load_leaves(p, a);
		scaled_leaves<kNegative>(c, s, 2, tc);
		graeffe_leaf(c, tc, v);
		product_leaf<1>(a, c, tc, w);
	}
};

inline void inverse_lower(const std::uint32_t* a, std::size_t h, const Group& w, std::uint32_t* out) {
	for (std::size_t i = 0; i < 8 * h; i += 8) {
		const Vec f0 = load(a + i), f1 = load(a + i + 8 * h), f2 = load(a + i + 16 * h), f3 = load(a + i + 24 * h);
		store(out + i, low(add(low(add(f0, f1)), low(add(f2, f3)))));
		store(out + i + 8 * h, low(add(times(diff(f0, f1), w.y), times(diff(f2, f3), w.z))));
	}
}

inline void forward_lower(const std::uint32_t* in, std::size_t h, const Group& w, std::uint32_t* a) {
	for (std::size_t i = 0; i < 8 * h; i += 8) {
		const Vec u = low(load(in + i)), b = load(in + i + 8 * h), yb = times(b, w.y), zb = times(b, w.z);
		store(a + i, add(u, yb)), store(a + i + 8 * h, diff(u, yb));
		store(a + i + 16 * h, add(u, zb)), store(a + i + 24 * h, diff(u, zb));
	}
}

template <class Bottom>
class Doubling {
public:
	
	Doubling(std::span<std::uint32_t> a, std::size_t block, const Tables& tables, const Bottom& bottom)
		: a_(a.data()), half_(a.size() / 2), block_(block), b_(std::size_t(std::countr_zero(block)) - 1),
		  aligned_(std::countr_zero(block) % 2 == 0), tables_(tables), bottom_(bottom),
		  inverse_(tables.roots, tables.inverse_roots, InverseBottom{tables.inverse_roots, a.data()}),
		  forward_(tables.roots, tables.inverse_roots, bottom) {}

	void run(std::uint32_t y_scale, std::uint32_t fix) const {
		std::uint32_t* const copy = a_ + half_;
		const std::size_t nv = half_ / 8, chunk = std::max<std::size_t>(block_, 16);
		for (std::size_t c = 0; c < nv; c += chunk) {
			for (std::size_t j = c / block_; j < (c + chunk) / block_; ++j) {
				inverse_x(a_ + 8 * block_ * j, j, copy + 8 * block_ * j);
				forward_x(copy + 8 * block_ * j, j, a_ + 8 * block_ * j);
			}
			if (block_ < 16) bottom_(a_ + 8 * c, chunk / 4, c / 4);
		}
		inverse_y(copy, y_scale);
		if (fix) {
			for (std::size_t i = 0; i < 4 * block_; i += 8) store(copy + i, negate(load(copy + i)));
			copy[0] = fix >= kP - copy[0] ? fix - (kP - copy[0]) : fix + copy[0];
		}
		forward_upper();
	}

private:
	
	void inverse_x(std::uint32_t* a, std::size_t j, std::uint32_t* out) const {
		if (block_ == 4) {
			inverse_.visit(a, 4, j);
			std::copy_n(a, 16, out);
		} else if (aligned_) {
			const std::size_t h = block_ / 4;
			for (std::size_t t = 0; t < 4; ++t) inverse_.visit(a + 8 * t * h, h, 4 * j + t);
			inverse_lower(a, h, Group(tables_.inverse_roots, j), out);
		} else {
			const std::size_t h = block_ / 2;
			inverse_.visit(a, h, 2 * j);
			inverse_.visit(a + 8 * h, h, 2 * j + 1);
			for (std::size_t i = 0; i < 8 * h; i += 8) store(out + i, low(add(load(a + i), load(a + i + 8 * h))));
		}
	}

	void forward_x(const std::uint32_t* in, std::size_t j, std::uint32_t* a) const {
		if (block_ == 4) {
			std::copy_n(in, 16, a);
			std::fill_n(a + 16, 16, 0);
		} else if (aligned_) {
			const std::size_t h = block_ / 4;
			forward_lower(in, h, Group(tables_.roots, j), a);
			if (h == 4) return bottom_(a, 4, 4 * j);
			for (std::size_t t = 0; t < 4; ++t) forward_.visit(a + 8 * t * h, h, 4 * j + t);
		} else {
			const std::size_t h = block_ / 2;
			for (std::size_t i = 0; i < 8 * h; i += 8) {
				const Vec x = load(in + i);
				store(a + i, x), store(a + i + 8 * h, x);
			}
			if (h == 4) return;
			forward_.visit(a, h, 2 * j);
			forward_.visit(a + 8 * h, h, 2 * j + 1);
		}
	}

	void above(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if (nv == block_) return;
		if (nv == 2 * block_) {
			const Factor x = entry(tables_.inverse_roots, k);
			for (std::size_t i = 0; i < 4 * block_; i += 8) {
				const Vec u = load(a + i), v = load(a + i + 8 * block_);
				store(a + i, low(add(u, v))), store(a + i + 8 * block_, times(diff(u, v), x));
			}
			return;
		}
		const std::size_t h = nv / 4;
		for (std::size_t t = 0; t < 4; ++t) above(a + 8 * t * h, h, 4 * k + t);
		inverse_columns(a, h, b_, tables_.inverse_roots, k);
	}

	void inverse_y(std::uint32_t* a, std::uint32_t scale) const {
		const std::size_t nv = half_ / 8, width = std::size_t(1) << b_;
		const bool zero = b_ <= 5;
		Vec* const v = reinterpret_cast<Vec*>(a);
		const Factor s(scale);
		const auto scaled = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
		const Vec z0 = _mm256_setzero_si256();
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			for (std::size_t t = 0; t < 4; ++t) above(a + 8 * t * h, h, t);
			const Factor z(tables_.inverse_roots[1], tables_.inverse_roots[9]);
			for (std::size_t c = 0; c < h; c += 2 * width) {
				for (std::size_t j = c; j < c + width; ++j) {
					const Vec x0 = v[j], x1 = v[j + h], x2 = v[j + 2 * h], x3 = v[j + 3 * h];
					const Vec ab = low(add(x0, x1)), cd = low(add(x2, x3));
					const Vec amb = low(diff(x0, x1)), cmd = times(diff(x2, x3), z);
					v[j] = scaled(add(ab, cd)), v[j + h] = scaled(add(amb, cmd));
					v[j + 2 * h] = scaled(diff(ab, cd)), v[j + 3 * h] = scaled(diff(amb, cmd));
				}
				if (zero)
					for (std::size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = v[j + 2 * h] = v[j + 3 * h] = z0;
			}
		} else {
			const std::size_t h = nv / 2;
			above(a, h, 0);
			above(a + 8 * h, h, 1);
			for (std::size_t c = 0; c < h; c += 2 * width) {
				for (std::size_t j = c; j < c + width; ++j) {
					const Vec x0 = v[j], x1 = v[j + h];
					v[j] = scaled(add(x0, x1)), v[j + h] = scaled(diff(x0, x1));
				}
				if (zero)
					for (std::size_t j = c + width; j < c + 2 * width; ++j) v[j] = v[j + h] = z0;
			}
		}
	}

	void forward_upper() const {
		const std::size_t nv = half_ / 4, width = std::size_t(1) << b_;
		const Pruned<Bottom> pruned(tables_, bottom_, b_);
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			Vec* const v = reinterpret_cast<Vec*>(a_);
			const Factor z(tables_.roots[1], tables_.roots[9]);
			for (std::size_t c = 0; c < h; c += 2 * width)
				for (std::size_t j = c; j < c + width; ++j) {
					const Vec x = v[2 * h + j], y = times(v[3 * h + j], z);
					v[2 * h + j] = add(x, y), v[3 * h + j] = diff(x, y);
				}
			pruned.forward(a_ + 16 * h, h, 2);
			pruned.forward(a_ + 24 * h, h, 3);
		} else {
			pruned.forward(a_ + 4 * nv, nv / 2, 1);
		}
	}

	std::uint32_t* a_;
	std::size_t half_, block_, b_;
	bool aligned_;
	const Tables& tables_;
	Bottom bottom_;
	Recursion<InverseBottom> inverse_;
	Recursion<Bottom> forward_;
};

inline void projection_first_levels(const Transform& t, const Tables& tables, std::span<const std::uint32_t> g,
									std::size_t n, std::span<std::uint32_t> q, std::span<std::uint32_t> p) {
	const std::size_t m = q.size() / 4, h = m / 2, quarter = m / 4, e = m - n, size = std::min(n, g.size());
	const auto identity = [&](Vec x) __attribute__((target("avx2"))) { return x; };
	const auto negated = [&](Vec x) __attribute__((target("avx2"))) { return negate(x); };
	const auto doubled = [&](Vec x) __attribute__((target("avx2"))) { return twice_mod(x); };
	const auto minus_twice = [&](Vec x) __attribute__((target("avx2"))) { return negate(twice_mod(x)); };
	const auto row = [h, quarter](std::span<std::uint32_t> a, std::size_t k) { return a.subspan(k * h, quarter); };

	if (size > 0) {
		q[0] = 0;
		std::copy(g.begin() + 1, g.begin() + std::ptrdiff_t(size), q.begin() + 1);
	}
	forward_with(q.first(2 * m), size, tables, LevelBottom{&tables, q.data()});
	inverse_with(q.first(m), tables, InverseBottom{tables.inverse_roots, q.data()}, graeffe_scale(std::countr_zero(m)),
				 Half::kBoth);
	const std::span<std::uint32_t> q2 = p.subspan(7 * h, h), q1 = q.subspan(7 * h, h), p1 = q.subspan(4 * h, h);
	std::copy_n(q.begin(), h, q2.begin());
	even_terms<false>(g, 1, size, 0, q1, minus_twice);
	if (e % 2) even_terms<false>(g, 1, size, 1 - std::ptrdiff_t(e), p1, negated);
	else even_terms<false>(g, 1, size, 1 - std::ptrdiff_t(e), p1, identity);

	const std::span<std::uint32_t> tq2 = q.first(m), tq1 = q.subspan(m, m), tp1 = p.first(m);
	t.forward(tq2.first(h), 0, tq2);
	t.forward(q1, 0, tq1);
	t.forward(p1, 0, tp1);
	std::uint32_t* const g1 = p.data() + 4 * h;
	std::uint32_t* const g2 = p.data() + 5 * h;
	std::uint32_t* const e12 = p.data() + 6 * h;
	std::uint32_t* const o1 = q.data() + 5 * h;
	std::uint32_t* const o2 = q.data() + 6 * h;
	const FirstLevelProducts<true> products{&tables, tq1.data(), tq2.data(), tp1.data(), {g1, g2, e12, o1, o2}};
	for (std::size_t k = 0; k < m / 16; k += 8) products.pairs(k);
	const std::uint32_t scale = graeffe_scale(std::countr_zero(h));
	for (std::uint32_t* out : products.out)
		inverse_with(std::span<std::uint32_t>(out, h), tables, InverseBottom{tables.inverse_roots, out}, scale, Half::kLower);

	std::fill(p.begin(), p.begin() + std::ptrdiff_t(quarter), 0);
	if (e % 2 && (e - 1) / 2 % 2) p[(e - 3) / 4] = 1;
	even_terms<false>(p1, 0, h, 1, row(p, 1), identity);
	std::copy_n(o1, quarter, p.begin() + std::ptrdiff_t(2 * h));
	std::copy_n(o2, quarter, p.begin() + std::ptrdiff_t(3 * h));
	if (e % 2) {
		const std::size_t d = (e - 1) / 2;
		const std::ptrdiff_t offset = 1 - std::ptrdiff_t(d);
		const std::span<const std::uint32_t> b1 = q1, b2 = q2;
		if (d % 2) {
			even_terms<true>(b1, 0, h, offset, row(p, 1), identity);
			even_terms<true>(b2, 0, h, offset, row(p, 2), identity);
		} else {
			even_terms<true>(b1, 0, h, offset, row(p, 1), negated);
			even_terms<true>(b2, 0, h, offset, row(p, 2), negated);
		}
	}

	std::fill(q.begin(), q.begin() + std::ptrdiff_t(quarter), 0);
	q[0] = 1;
	even_terms<false>(q1, 0, h, 0, row(q, 1), doubled);
	even_terms<false>(q2, 0, h, 0, row(q, 2), doubled);
	add_into(row(q, 2), std::span<const std::uint32_t>(g1, quarter), row(q, 2));
	for (std::size_t i = 0; i < quarter; i += 8) store(q.data() + 3 * h + i, twice_mod(load(e12 + i)));
	std::copy_n(g2, quarter, q.begin() + std::ptrdiff_t(4 * h));
}

inline void projection_weighted_first(const Transform& t, const Tables& tables,
		std::span<const std::uint32_t> g, std::span<const std::uint32_t> weights,
		std::span<std::uint32_t> q, std::span<std::uint32_t> p, std::span<std::uint32_t> work) {
	const std::size_t m = q.size() / 4, h = m / 2, quarter = m / 4, n = weights.size();
	projection_first_levels(t, tables, g, n, q, p);
	auto x = work.first(2 * m), y = work.subspan(2 * m, 2 * m), z = work.subspan(4 * m, 2 * m);
	std::fill(x.begin(), x.end(), 0u);
	std::fill(y.begin(), y.end(), 0u);
	for (std::size_t i = 0; i < n; ++i) x[m - 1 - i] = weights[i];
	for (std::size_t i = 1; i < std::min(g.size(), n); ++i) y[i] = i % 2 && g[i] ? kP - g[i] : g[i];
	t.forward(y);
	t.cyclic_product(x.first(m), 0, z, y, Half::kLower);
	auto p0 = work.first(h), p1 = work.subspan(h, h);
	for (std::size_t i = 0; i < h; ++i) {
		const auto value = z[2 * i + 1];
		p1[i] = value ? kP - value : 0;
		p0[i] = 2 * i + 1 < m && m - 2 - 2 * i < n ? weights[m - 2 - 2 * i] : 0;
	}
	auto tp0 = work.subspan(m, m), tp1 = work.subspan(2 * m, m);
	auto tq1 = work.subspan(3 * m, m), tq2 = work.subspan(4 * m, m), product = work.subspan(5 * m, m);
	for (std::size_t i = 0; i < h; ++i) {
		const auto a = q[7 * h + i], b = p[7 * h + i];
		tq1[i] = i % 2 && a ? kP - a : a;
		tq2[i] = i % 2 && b ? kP - b : b;
	}
	t.forward(p0, 0, tp0);
	t.forward(p1, 0, tp1);
	t.forward(tq1.first(h), 0, tq1);
	t.forward(tq2.first(h), 0, tq2);
	for (std::size_t i = 0; i < quarter; ++i) p[i] = p0[2 * i + 1];
	t.inverse_product(tp0, tq1, product, Half::kLower);
	for (std::size_t i = 0; i < quarter; ++i) {
		const auto value = product[2 * i + 1] + p1[2 * i + 1];
		p[h + i] = value >= kP ? value - kP : value;
	}
	const Transform::Pair pairs[2] = {{tp0, tq2}, {tp1, tq1}};
	t.inverse_product_sum(pairs, product, Half::kLower);
	for (std::size_t i = 0; i < quarter; ++i) p[2 * h + i] = product[2 * i + 1];
	t.inverse_product(tp1, tq2, product, Half::kLower);
	for (std::size_t i = 0; i < quarter; ++i) p[3 * h + i] = product[2 * i + 1];
}

inline std::size_t projection_work(std::size_t m) {
	const std::size_t eighth = m / 8;
	return std::max(7 * Carve::words(eighth + 1) + 8 * Carve::words(eighth), Carve::words(m) + 2 * Carve::words(m / 2));
}

inline void projection_last_levels(const Transform& t, const Tables& tables, std::span<std::uint32_t> q,
								   std::span<std::uint32_t> p, std::span<std::uint32_t> w, std::span<std::uint32_t> a) {
	const std::size_t m = q.size() / 4, eighth = m / 8, quarter = m / 4, h = m / 2;
	std::span<std::uint32_t> c[8], pc[8];
	{
		Carve carve(w);
		for (std::size_t k = 1; k < 8; ++k) c[k] = carve.take(eighth + 1);
		for (std::size_t k = 0; k < 8; ++k) pc[k] = carve.take(eighth);
	}
	for (std::size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i) x[i] = load(q.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 7
		for (std::size_t k = 1; k < 8; ++k) store(c[k].data() + r, k % 2 ? negate(x[k]) : x[k]);
	}
	for (std::size_t k = 1; k < 8; ++k) {
		const std::uint32_t x = q[16 * eighth + k];
		c[k][eighth] = k % 2 && x ? kP - x : x;
	}
	for (std::size_t r = 0; r < eighth; r += 8) {
		Vec x[8];
#pragma GCC unroll 8
		for (std::size_t i = 0; i < 8; ++i) x[i] = load(p.data() + 16 * (r + i));
		transpose(x);
#pragma GCC unroll 8
		for (std::size_t k = 0; k < 8; ++k) store(pc[k].data() + r, x[k]);
	}

	Carve carve_q(q), carve_p(p);
	std::span<std::uint32_t> tc[8], tp[7], twice[3], cn[4], pn[4];
	for (std::size_t k = 1; k < 8; ++k) {
		tc[k] = carve_q.take(quarter);
		t.forward(c[k], 0, tc[k]);
	}
	for (std::size_t k = 0; k < 7; ++k) {
		tp[k] = carve_p.take(quarter);
		t.forward(pc[k], 0, tp[k]);
	}
	for (auto& s : twice) s = carve_q.take(quarter);
	multiply_constant(tc[5], 2, twice[0]), multiply_constant(tc[3], kP - 2, twice[1]), multiply_constant(tc[4], kP - 2, twice[2]);
	for (std::size_t k = 1; k < 4; ++k) cn[k] = carve_q.take(quarter + 1);
	for (auto& s : pn) s = carve_p.take(quarter);
	{
		const Transform::Pair first[1] = {{tc[1], tc[1]}}, second[2] = {{tc[2], tc[2]}, {tc[1], twice[1]}},
							  third[3] = {{tc[3], tc[3]}, {tc[1], twice[0]}, {tc[2], twice[2]}};
		t.inverse_product_sum(first, cn[1].first(quarter));
		t.inverse_product_sum(second, cn[2].first(quarter));
		t.inverse_product_sum(third, cn[3].first(quarter));
	}
	const std::uint32_t weight[4] = {0, kP - 2, 2, kP - 2};
	for (std::size_t k = 1; k < 4; ++k) {
		const std::span<const std::uint32_t> linear = c[2 * k];
		const Factor f(weight[k]);
		for (std::size_t i = 0; i < eighth; i += 8)
			store(cn[k].data() + i, canonical(add(load(cn[k].data() + i), reduce(times(load(linear.data() + i), f), kP))));
		const std::uint64_t last = cn[k][eighth] + std::uint64_t(weight[k]) * linear[eighth];
		cn[k][eighth] = std::uint32_t(last % kP);
		cn[k][quarter] = cn[k][0];
		cn[k][0] = 0;
	}
	{
		Transform::Pair pairs[7];
		for (std::size_t j = 0; j < 4; ++j) {
			for (std::size_t b = 1; b <= 2 * j + 1; ++b) pairs[b - 1] = {tp[2 * j + 1 - b], tc[b]};
			const std::span<const Transform::Pair> sum(pairs, 2 * j + 1);
			if (j < 2) t.inverse_product_sum(sum, pn[j]);
			else if (j == 2) inverse_product_sum<5>(tables, pairs, pn[j], Half::kBoth);
			else inverse_product_sum<7>(tables, pairs, pn[j], Half::kBoth);
			add_into(pn[j].first(eighth), pc[2 * j + 1], pn[j].first(eighth));
		}
	}

	Carve next_q(q), next_p(p), next_w(w);
	std::span<std::uint32_t> tcn[4], tpn[3];
	for (std::size_t k = 1; k < 4; ++k) {
		tcn[k] = next_q.take(h);
		t.forward(cn[k], 0, tcn[k]);
	}
	for (std::size_t j = 0; j < 3; ++j) {
		tpn[j] = next_p.take(h);
		t.forward(pn[j], 0, tpn[j]);
	}
	const std::span<std::uint32_t> square = next_q.take(h), negative_q = next_w.take(m);
	t.inverse_product(tcn[1], tcn[1], square);
	for (std::size_t i = 0; i < h; i += 8) {
		const Vec x = i < quarter ? negate(twice_mod(load(cn[2].data() + i))) : _mm256_setzero_si256();
		store(negative_q.data() + i, reduce(add(x, load(square.data() + i)), kP));
	}
	{
		const std::uint32_t x = negative_q[quarter] + minus_twice(cn[2][quarter]);
		negative_q[quarter] = x >= kP ? x - kP : x;
		negative_q[h] = negative_q[0];
		negative_q[0] = 0;
		std::fill(negative_q.begin() + std::ptrdiff_t(h + 1), negative_q.end(), 0);
	}
	const std::span<std::uint32_t> r1 = next_w.take(h), r3 = next_w.take(h);
	t.inverse_product(tpn[0], tcn[1], r1);
	add_into(r1.first(quarter), pn[1], r1.first(quarter));
	const Transform::Pair pairs[3] = {{tpn[2], tcn[1]}, {tpn[1], tcn[2]}, {tpn[0], tcn[3]}};
	t.inverse_product_sum(pairs, r3);
	add_into(r3.first(quarter), pn[3], r3.first(quarter));
	t.forward(negative_q);
	const std::span<std::uint32_t> product = next_q.take(m);
	t.cyclic_product(r1, 0, product, negative_q);
	std::size_t k = 0;
	for (; k + 8 <= std::min(a.size(), h); k += 8)
		store_unaligned(a.data() + k, reduce(add(load(r3.data() + k), load(product.data() + k)), kP));
	for (; k < a.size(); ++k) {
		const std::uint32_t x = (k < h ? r3[k] : 0) + product[k];
		a[k] = x >= kP ? x - kP : x;
	}
}

}

inline int projection_log(std::size_t n) {
	if (n <= detail::kComposeBase) return Transform::kMinLog;
	return std::countr_zero(detail::projection_length(n));
}

inline std::size_t projection_scratch(std::size_t n) {
	using detail::Carve;
	if (n <= detail::kComposeBase) return 0;
	const std::size_t m = detail::projection_length(n);
	return Arena::footprint(detail::Tables::words(m)) + 2 * Carve::words(4 * m) + Carve::words(detail::projection_work(m));
}

inline void power_projection(const Transform& t, std::span<const std::uint32_t> g, std::span<std::uint32_t> a,
							 std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = a.size();
	if (n <= kComposeBase) return projection_direct(g, a);
	const std::size_t m = projection_length(n);
	const int lg = std::countr_zero(m);
	const Tables tables(scratch.first(Tables::words(m)), m);
	Carve carve(scratch.subspan(Arena::footprint(Tables::words(m))));
	const std::span<std::uint32_t> q = carve.take(4 * m), p = carve.take(4 * m), w = carve.take(projection_work(m));
	projection_first_levels(t, tables, g, n, q, p);
	const ProjectionBottom bottom{&tables, p.data(), q.data(), p.data()};
	const std::uint32_t half_scale = blocked_ntt::detail::power(kP / 2 + 1, 31);
	std::uint32_t c = 1;
	for (int s = 2; s + 3 < lg; ++s) {
		const std::size_t stride = 2 * (m >> s), rows = std::size_t(1) << s;
		const std::size_t pad = std::size_t(std::countr_zero(stride / 16));
		if (s == 2) {
			forward_pruned(p, rows * stride, pad, tables, ForwardBottom{tables.roots});
			forward_pruned(q, (rows + 1) * stride, pad, tables, bottom);
		}
		if (s + 4 < lg) {
			const std::size_t block = stride / 16;
			const std::uint32_t next =
				blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(c, c), half_scale), std::uint32_t(block));
			const std::uint32_t y_scale = kInverseScales[0][s + 4];
			Doubling<ForwardBottom>(p, block, tables, ForwardBottom{tables.roots}).run(y_scale, 0);
			Doubling<ProjectionBottom>(q, block, tables, bottom).run(y_scale, next + next >= kP ? next + next - kP : next + next);
			c = next;
		} else {
			const std::uint32_t inverse_c = blocked_ntt::detail::power(c, kP - 2);
			const std::uint32_t scale = blocked_ntt::detail::multiply_mod(graeffe_scale(lg + 1), blocked_ntt::detail::multiply_mod(inverse_c, inverse_c));
			inverse_pruned(q.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, q.data()}, scale);
			inverse_pruned(p.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, p.data()}, scale);
			next_level(q, stride / 2, 2 * rows);
		}
	}
	projection_last_levels(t, tables, q, p, w, a);
}

inline void weighted_projection(const Transform& t, std::span<const std::uint32_t> g, std::span<const std::uint32_t> weights, std::span<std::uint32_t> a,
							 std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = a.size();
	assert(n > kComposeBase && weights.size() == n);
	const std::size_t m = projection_length(n);
	const int lg = std::countr_zero(m);
	const Tables tables(scratch.first(Tables::words(m)), m);
	Carve carve(scratch.subspan(Arena::footprint(Tables::words(m))));
	const std::span<std::uint32_t> q = carve.take(4 * m), p = carve.take(4 * m), w = carve.take(projection_work(m));
	projection_weighted_first(t, tables, g, weights, q, p, scratch.last(6 * m));
	const ProjectionBottom bottom{&tables, p.data(), q.data(), p.data()};
	const std::uint32_t half_scale = blocked_ntt::detail::power(kP / 2 + 1, 31);
	std::uint32_t c = 1;
	for (int s = 2; s + 3 < lg; ++s) {
		const std::size_t stride = 2 * (m >> s), rows = std::size_t(1) << s;
		const std::size_t pad = std::size_t(std::countr_zero(stride / 16));
		if (s == 2) {
			forward_pruned(p, rows * stride, pad, tables, ForwardBottom{tables.roots});
			forward_pruned(q, (rows + 1) * stride, pad, tables, bottom);
		}
		if (s + 4 < lg) {
			const std::size_t block = stride / 16;
			const std::uint32_t next =
				blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(blocked_ntt::detail::multiply_mod(c, c), half_scale), std::uint32_t(block));
			const std::uint32_t y_scale = kInverseScales[0][s + 4];
			Doubling<ForwardBottom>(p, block, tables, ForwardBottom{tables.roots}).run(y_scale, 0);
			Doubling<ProjectionBottom>(q, block, tables, bottom).run(y_scale, next + next >= kP ? next + next - kP : next + next);
			c = next;
		} else {
			const std::uint32_t inverse_c = blocked_ntt::detail::power(c, kP - 2);
			const std::uint32_t scale = blocked_ntt::detail::multiply_mod(graeffe_scale(lg + 1), blocked_ntt::detail::multiply_mod(inverse_c, inverse_c));
			inverse_pruned(q.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, q.data()}, scale);
			inverse_pruned(p.first(2 * m), pad - 1, tables, InverseBottom{tables.inverse_roots, p.data()}, scale);
			next_level(q, stride / 2, 2 * rows);
		}
	}
	projection_last_levels(t, tables, q, p, w, a);
}

}

namespace composition_poly {
inline unsigned coefficient_value(const Mint &x) {
	const unsigned value = x.v;
	const unsigned reduced = (static_cast<unsigned long long>(unsigned(value * 998244351u)) * 998244353u + value) >> 32;
	return std::min(reduced, reduced - 998244353u);
}
inline void append_word(std::vector<Mint> &out, unsigned value) {
	out.emplace_back();
	out.back().v = value;
}
inline void native_compose(const Mint *f, std::size_t nf, const Mint *g, std::size_t ng,
						   std::vector<Mint> &out, std::size_t n) {
	nf = std::min(nf, n);
	ng = std::min(ng, n);
	const int lg = compose_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(nf) + Arena::footprint(ng) +
				Arena::footprint(n) + Arena::footprint(compose_scratch(n)));
	const Transform t(arena, lg);
	auto a = arena.take(nf), b = arena.take(ng), c = arena.take(n);
	for (std::size_t i = 0; i < nf; ++i) a[i] = std::min(f[i].v, f[i].v - Mint::mod());
	for (std::size_t i = 0; i < ng; ++i) b[i] = coefficient_value(g[i]);
	compose(t, a, b, c, arena.take(compose_scratch(n)));
	append_fps_words<true>(out, c.data(), c.size());
}
inline void native_weighted_projection(const Mint *g, std::size_t ng, const Mint *weights,
		std::vector<Mint> &out, std::size_t n, std::size_t count) {
	ng = std::min(ng, n);
	const int lg = projection_log(n) + 1;
	const std::size_t scratch_words = projection_scratch(n) + Arena::footprint(6 * detail::projection_length(n));
	Arena arena(Transform::words(lg) + Arena::footprint(ng) + 2 * Arena::footprint(n) + Arena::footprint(scratch_words));
	const Transform t(arena, lg);
	auto a = arena.take(ng), b = arena.take(n), w = arena.take(n);
	for (std::size_t i = 0; i < ng; ++i) a[i] = i ? coefficient_value(g[i]) : 0;
	for (std::size_t i = 0; i < n; ++i) w[i] = std::min(weights[i].v, weights[i].v - Mint::mod());
	weighted_projection(t, a, w, b, arena.take(scratch_words));
	append_fps_words<true>(out, b.data(), count);
}
inline void native_projection(const Mint *g, std::size_t ng, std::vector<Mint> &out, std::size_t n) {
	ng = std::min(ng, n);
	const int lg = projection_log(n);
	Arena arena(Transform::words(lg) + Arena::footprint(ng) + Arena::footprint(n) +
				Arena::footprint(projection_scratch(n)));
	const Transform t(arena, lg);
	auto a = arena.take(ng), b = arena.take(n);
	for (std::size_t i = 0; i < ng; ++i) a[i] = coefficient_value(g[i]);
	power_projection(t, a, b, arena.take(projection_scratch(n)));
	append_fps_words<false>(out, b.data(), b.size());
}
}

}
#pragma GCC pop_options
#endif
#endif
