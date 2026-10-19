#ifndef CP_COMPONENTS_FPS_BLOCKED_HPP
#define CP_COMPONENTS_FPS_BLOCKED_HPP
#include "fps_asm.hpp"
#include "fps_workspace.hpp"
#include "fps_output.hpp"
#include "../math/mint.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#include <span>
#include <cstring>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#include <immintrin.h>
#pragma GCC push_options
_Pragma("GCC target(\"avx2\")")
#pragma GCC optimize("O3")

namespace fps_detail {
namespace blocked_ntt::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP, kNI = 998244351;

inline void forward(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
R(%[P],z) R(%[P2],r) D((%[x]),d) m(d,G)\
D(32(%[x]),w)\
D((%[y]),d)\
m(d,L)\
D(32(%[y]),E)\
D(4(%[y]),d)\
m(d,M)\
D(36(%[y]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(H,d)\
m(J,c)\
m(N,k)\
m(Q,g)\
q(H,w,u)\
q(J,w,s)\
q(4(%[a],%[h3]),w,e)\
q(36(%[a],%[h3]),w,i)\
m(I,l)\
v(t,u,u)\
m(K,n)\
v(t,s,s)\
B(A,e,u,e)\
B(A,i,s,i)\
m((%[a]),s)\
p(G,d,d)\
p(z,e,e)\
m(O,u)\
p(G,c,c)\
p(z,i,i)\
j(r,k,f)\
C(f,k,f)\
q(I,w,k)\
j(e,d,e)\
j(r,g,d)\
C(d,g,d)\
j(i,c,i)\
q(K,w,c)\
q(4(%[a],%[h],2),w,g)\
v(t,k,k)\
p(G,l,l)\
p(G,n,n)\
v(t,c,c)\
B(A,g,k,g)\
q(36(%[a],%[h],2),w,k)\
p(z,g,g)\
B(A,k,c,k)\
o(r,f,c)\
o(e,f,f)\
j(e,c,e)\
p(z,k,k)\
j(g,l,g)\
o(r,d,l)\
o(i,d,d)\
j(i,l,i)\
v(t,f,l)\
q(E,f,c)\
q(E,l,l)\
p(L,f,f)\
j(k,n,k)\
v(t,e,n)\
q(F,n,n)\
v(t,c,c)\
B(A,l,c,l)\
q(F,e,c)\
p(M,e,e)\
p(z,l,l)\
v(t,c,c)\
B(A,n,c,n)\
j(l,f,l)\
v(t,d,f)\
q(E,d,c)\
q(E,f,f)\
p(z,n,n)\
p(L,d,d)\
v(t,c,c)\
B(A,f,c,f)\
j(n,e,n)\
v(t,i,e)\
q(F,i,c)\
q(F,e,e)\
p(z,f,f)\
p(M,i,i)\
v(t,c,c)\
B(A,e,c,e)\
j(f,d,f)\
j(r,s,d)\
j(r,u,c)\
C(d,s,d)\
C(c,u,c)\
p(z,e,e)\
o(g,d,u)\
j(g,d,g)\
o(k,c,d)\
j(k,c,k)\
j(r,u,c)\
o(r,g,s)\
C(c,u,c)\
C(s,g,s)\
j(e,i,e)\
j(r,d,i)\
o(r,k,g)\
C(i,d,i)\
o(r,c,d)\
o(r,s,u)\
C(g,k,g)\
o(l,c,c)\
j(l,d,l)\
o(n,s,s)\
o(r,i,d)\
o(r,g,k)\
j(n,u,n)\
o(f,i,i)\
m(c,(%[a]))\
j(f,d,f)\
o(e,g,g)\
j(e,k,e)\
m(l,N)\
m(s,I)\
m(n,H)\
m(i,O)\
m(f,Q)\
m(g,K)\
m(e,J)\
S($64,%[a])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm1,%%ymm0,%%ymm6,%%ymm10,%%ymm3,%%ymm7,CP_FPS_ASM_VPSUBD,%%ymm2,%%ymm8,CP_FPS_ASM_VMOVDQA,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,%%ymm5,$32,%%ymm4,CP_FPS_ASM_VPSRLQ,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h],2),32(%[a],%[h3]),32(%[a],%[h],2),32(%[w]),64(%[w]),(%[a],%[h]),32(%[a]),32(%[a],%[h]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) \
Q(%[P],w) Q(%[P2],k) C((%[x]),c) r(c,F)\
C(32(%[x]),v)\
C((%[y]),c)\
r(c,K)\
C(32(%[y]),D)\
C(4(%[y]),c)\
r(c,L)\
C(36(%[y]),E)\
".p2align 5\n\t" \
"1:\n\t" \
r((%[a]),c)\
r(N,f)\
r(M,s)\
r(O,t)\
n(k,c,i)\
n(H,c,c)\
n(k,f,j)\
h(H,i,i)\
n(J,f,f)\
h(k,c,u)\
n(k,s,e)\
h(J,j,j)\
q(l,i,g)\
p(D,i,d)\
n(k,t,m)\
h(G,e,e)\
p(D,g,g)\
o(K,i,i)\
h(I,m,m)\
n(G,s,s)\
B(u,c,u)\
q(l,j,c)\
p(D,c,c)\
q(l,d,d)\
n(I,t,t)\
A(z,g,d,g)\
p(D,j,d)\
o(K,j,j)\
o(w,g,g)\
q(l,d,d)\
A(z,c,d,c)\
h(g,i,g)\
q(l,e,i)\
p(E,e,d)\
p(E,i,i)\
o(w,c,c)\
o(L,e,e)\
q(l,d,d)\
A(z,i,d,i)\
h(c,j,c)\
q(l,m,j)\
p(E,m,d)\
p(E,j,j)\
o(w,i,i)\
o(L,m,m)\
q(l,d,d)\
A(z,j,d,j)\
h(i,e,i)\
h(k,f,e)\
h(k,s,d)\
o(w,j,j)\
B(e,f,e)\
B(d,s,d)\
h(k,t,s)\
n(k,u,f)\
B(s,t,s)\
n(k,e,t)\
n(d,u,u)\
h(d,f,d)\
h(s,t,t)\
n(s,e,s)\
n(k,g,e)\
h(j,m,j)\
n(k,c,m)\
h(i,e,e)\
q(l,d,f)\
h(j,m,m)\
p(v,f,f)\
n(i,g,i)\
n(j,c,j)\
p(v,d,c)\
q(l,e,g)\
o(F,d,d)\
p(v,g,g)\
q(l,c,c)\
A(z,f,c,f)\
p(v,e,c)\
o(F,e,e)\
o(w,f,f)\
q(l,c,c)\
A(z,g,c,g)\
h(f,d,f)\
q(l,t,d)\
p(v,t,c)\
p(v,d,d)\
o(w,g,g)\
r(f,M)\
q(l,m,f)\
p(v,f,f)\
o(F,t,t)\
q(l,c,c)\
A(z,d,c,d)\
h(g,e,g)\
p(v,m,e)\
o(F,m,m)\
o(w,d,d)\
h(k,u,c)\
r(g,G)\
h(k,i,g)\
B(c,u,c)\
q(l,e,e)\
h(k,s,u)\
B(g,i,g)\
r(c,(%[a]))\
A(z,f,e,f)\
h(k,j,e)\
B(u,s,u)\
h(d,t,d)\
r(g,H)\
o(w,f,f)\
B(e,j,e)\
r(u,N)\
r(e,J)\
r(d,O)\
h(f,m,f)\
r(f,I)\
R($64,%[a])\
S(%[end],%[a])\
T(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm9,%%ymm7,%%ymm1,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm4,%%ymm5,%%ymm14,$32,%%ymm10,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm2,%%ymm3,%%ymm6,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h]),32(%[a],%[h3]),32(%[a],%[h]),32(%[w]),64(%[w]),(%[a],%[h],2),32(%[a]),32(%[a],%[h],2),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void forward_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) \
K(%[P],A) K(%[P2],d) I(4(%[y]),z) I(36(%[y]),t)\
".p2align 5\n\t" \
"1:\n\t" \
j(C,o)\
j(G,g)\
j(B,p)\
j(F,l)\
j((%[a]),i)\
j(D,m)\
j(E,q)\
j(H,r)\
c(d,o,h)\
c(d,g,f)\
n(h,o,h)\
c(d,p,o)\
n(f,g,f)\
k(d,h,g)\
c(d,l,s)\
n(o,p,o)\
k(d,f,p)\
n(s,l,s)\
c(o,g,g)\
c(d,i,l)\
k(o,h,o)\
c(s,p,p)\
x(u,g,h)\
w(t,g,e)\
n(l,i,l)\
x(u,p,i)\
w(t,h,h)\
v(z,g,g)\
k(s,f,s)\
w(t,p,f)\
w(t,i,i)\
x(u,e,e)\
v(z,p,p)\
J($170,h,e,h)\
c(d,m,e)\
x(u,f,f)\
n(e,m,e)\
v(A,h,h)\
c(d,q,m)\
n(m,q,m)\
J($170,i,f,i)\
c(d,r,f)\
k(e,l,q)\
n(f,r,f)\
c(e,l,e)\
v(A,i,i)\
c(d,q,l)\
k(f,m,r)\
c(f,m,f)\
k(d,e,m)\
n(l,q,l)\
c(d,r,q)\
n(m,e,m)\
c(d,o,e)\
c(h,g,h)\
k(d,f,g)\
n(q,r,q)\
c(d,s,r)\
n(e,o,e)\
n(g,f,g)\
k(d,l,f)\
k(d,m,o)\
n(r,s,r)\
c(i,p,i)\
k(d,q,p)\
k(d,g,s)\
k(e,l,l)\
c(e,f,e)\
k(h,m,m)\
c(h,o,h)\
k(r,q,q)\
j(l,(%[a]))\
c(r,p,r)\
k(i,g,g)\
c(i,s,i)\
j(e,C)\
j(m,D)\
j(h,B)\
j(q,E)\
j(r,G)\
j(g,H)\
j(i,F)\
L($64,%[a])\
M(%[end],%[a])\
N(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm11,%%ymm9,%%ymm1,%%ymm8,%%ymm4,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,%%ymm3,%%ymm5,CP_FPS_ASM_VPMINUD,%%ymm0,%%ymm2,%%ymm6,%%ymm7,%%ymm10,%%ymm13,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm12,%%ymm15,(%[a],%[h3]),(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h3]),32(%[a],%[h]),32(%[a],%[h],2),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void inverse_identity(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N) \
K(%[P],A) K(%[P2],f) I(4(%[y]),z) I(36(%[y]),t)\
".p2align 5\n\t" \
"1:\n\t" \
p(F,c)\
p(H,r)\
p((%[a]),n)\
p(G,j)\
h(f,c,o)\
h(B,c,c)\
h(f,r,i)\
e(B,o,o)\
h(D,r,r)\
e(f,c,s)\
e(D,i,i)\
x(u,o,d)\
w(t,o,g)\
v(z,o,o)\
x(u,i,k)\
w(t,i,m)\
w(t,d,d)\
h(C,n,q)\
w(t,k,k)\
v(z,i,i)\
e(C,n,n)\
l(s,c,s)\
x(u,g,g)\
h(E,j,c)\
e(E,j,j)\
x(u,m,m)\
J($170,d,g,d)\
e(f,q,g)\
J($170,k,m,k)\
v(A,d,d)\
l(g,q,g)\
h(f,n,q)\
v(A,k,k)\
e(f,c,m)\
l(q,n,q)\
h(s,g,n)\
l(m,c,m)\
e(s,g,s)\
e(f,r,g)\
h(f,j,c)\
e(d,o,d)\
l(g,r,g)\
l(c,j,c)\
e(f,n,j)\
e(k,i,k)\
h(d,q,i)\
e(d,q,d)\
h(g,m,q)\
h(k,c,r)\
e(g,m,g)\
e(k,c,k)\
e(f,i,c)\
h(f,s,m)\
h(f,d,o)\
l(j,n,j)\
l(c,i,c)\
e(f,q,i)\
e(f,r,n)\
l(m,s,m)\
l(o,d,o)\
p(j,(%[a]))\
h(f,g,j)\
h(f,k,d)\
l(i,q,i)\
l(n,r,n)\
p(c,C)\
l(j,g,j)\
l(d,k,d)\
p(m,F)\
p(o,B)\
p(i,G)\
p(n,E)\
p(j,H)\
p(d,D)\
L($64,%[a])\
M(%[end],%[a])\
N(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm7,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm8,CP_FPS_ASM_VPADDD,%%ymm5,%%ymm3,%%ymm9,CP_FPS_ASM_VPMINUD,%%ymm10,%%ymm2,%%ymm4,CP_FPS_ASM_VMOVDQA,%%ymm11,%%ymm1,%%ymm6,%%ymm13,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm12,%%ymm15,(%[a],%[h3]),(%[a],%[h]),32(%[a],%[h3]),32(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h],2),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void scale_radix2(Vec* a, std::size_t h, const std::uint32_t* y) {
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,z,A,B,C,D,E,F,G,H,I,J,K) \
H(%[P],m) H(%[P2],C) G(4(%[y]),s) G(36(%[y]),l)\
".p2align 5\n\t" \
"1:\n\t" \
r((%[a]),c)\
r(F,d)\
z(C,c,e)\
z(D,c,c)\
z(C,d,f)\
k(D,e,e)\
z(E,d,d)\
q(n,c,h)\
k(E,f,f)\
p(l,c,v)\
q(n,e,i)\
p(l,e,w)\
q(n,d,j)\
p(l,d,x)\
q(n,f,g)\
p(l,f,u)\
p(l,h,h)\
p(l,i,i)\
p(l,j,j)\
p(l,g,g)\
q(n,v,v)\
q(n,w,w)\
q(n,x,x)\
q(n,u,u)\
o(s,c,c)\
o(s,e,e)\
A(t,h,v,h)\
A(t,i,w,i)\
o(s,d,d)\
o(s,f,f)\
A(t,j,x,j)\
A(t,g,u,g)\
o(m,h,h)\
o(m,i,i)\
o(m,j,j)\
o(m,g,g)\
k(h,c,h)\
k(i,e,i)\
k(j,d,j)\
k(g,f,g)\
k(m,h,f)\
k(m,i,d)\
k(m,j,e)\
k(m,g,c)\
B(f,h,f)\
B(d,i,d)\
B(e,j,e)\
B(c,g,c)\
r(f,(%[a]))\
r(d,D)\
r(e,F)\
r(c,E)\
I($64,%[a])\
J(%[end],%[a])\
K(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm1,%%ymm2,%%ymm3,%%ymm10,%%ymm4,%%ymm6,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm13,%%ymm15,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm12,$170,%%ymm11,%%ymm5,%%ymm7,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm14,(%[a],%[h]),32(%[a],%[h]),32(%[a]),CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_ADD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}
namespace blocked_ntt {

inline constexpr std::uint32_t kModulus = kernels::kP;
inline constexpr int kMaxLog = 25;

namespace detail {

using kernels::Vec;
using kernels::kP;

constexpr std::uint32_t multiply_mod(std::uint32_t x, std::uint32_t y) {
	return std::uint32_t(std::uint64_t(x) * y % kP);
}

constexpr std::uint32_t power(std::uint32_t x, std::uint32_t e) {
	std::uint32_t r = 1;
	for (; e; e >>= 1, x = multiply_mod(x, x))
		if (e & 1) r = multiply_mod(r, x);
	return r;
}

constexpr std::uint32_t quotient(std::uint32_t w) { return std::uint32_t((std::uint64_t(w) << 32) / kP); }

inline constexpr std::uint32_t kR = std::uint32_t((std::uint64_t(1) << 32) % kP);

inline constexpr int kRootCount = kMaxLog - 4;
inline constexpr auto kRoots = [] {
	std::array<std::array<std::uint32_t, kRootCount>, 2> t{};
	for (int j = 0; j < kRootCount; ++j) {
		t[0][j] = power(3, (kP - 1) >> (j + 2));
		t[1][j] = power(t[0][j], kP - 2);
	}
	return t;
}();

inline Vec broadcast(std::uint32_t x) { return _mm256_set1_epi32(int(x)); }
inline Vec add(Vec x, Vec y) { return _mm256_add_epi32(x, y); }
inline Vec diff(Vec x, Vec y) { return _mm256_sub_epi32(_mm256_add_epi32(x, broadcast(2 * kP)), y); }

inline Vec reduce(Vec x, std::uint32_t bound) { return _mm256_min_epu32(x, _mm256_sub_epi32(x, broadcast(bound))); }

struct Factor {
	Vec w, q;
	explicit Factor(std::uint32_t value) : w(broadcast(value)), q(broadcast(quotient(value))) {}
	Factor(std::uint32_t value, std::uint32_t value_quotient) : w(broadcast(value)), q(broadcast(value_quotient)) {}
};

inline Vec multiply(Vec x, const Factor& f) {
	const Vec even = _mm256_srli_epi64(_mm256_mul_epu32(x, f.q), 32);
	const Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), f.q);
	const Vec q = _mm256_blend_epi32(even, odd, 0xAA);
	return _mm256_sub_epi32(_mm256_mullo_epi32(x, f.w), _mm256_mullo_epi32(q, broadcast(kP)));
}

inline std::size_t slot(std::size_t k) { return (k >> 3 << 4) | (k & 7); }

inline std::size_t table_words(int lg) { return (std::size_t(1) << lg) / 8 + 16; }

inline void build_table(std::uint32_t* t, std::size_t count, const std::array<std::uint32_t, kRootCount>& roots) {
	t[slot(0)] = 1;
	t[slot(0) + 8] = quotient(1);
	for (int level = 0; level < kRootCount; ++level) {
		const std::size_t h = std::size_t(1) << level;
		if (h >= count) break;
		const std::uint32_t root = roots[level];
		if (h < 8) {
			for (std::size_t j = 0; j < h; ++j) {
				const std::uint32_t v = multiply_mod(t[slot(j)], root);
				t[slot(h + j)] = v;
				t[slot(h + j) + 8] = quotient(v);
			}
			continue;
		}

		const Factor by(root), to_r(kR);
		for (std::size_t j = 0; j < h; j += 8) {
			const Vec v = reduce(multiply(_mm256_load_si256(reinterpret_cast<const Vec*>(t + slot(j))), by), kP);
			const Vec vr = reduce(multiply(v, to_r), kP);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j)), v);
			_mm256_store_si256(reinterpret_cast<Vec*>(t + slot(h + j) + 8), _mm256_mullo_epi32(vr, broadcast(kernels::kNI)));
		}
	}
}

inline void inverse_radix4(Vec* f, std::size_t h, const std::uint32_t* inverse_roots, const Factor& s) {
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(multiply(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = reduce(add(p0, p1), 2 * kP), cd = reduce(add(p2, p3), 2 * kP);
		const Vec amb = reduce(diff(p0, p1), 2 * kP), cmd = multiply(diff(p2, p3), z);
		f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
	}
}

}
}
namespace blocked_poly::kernels {

using Vec = __m256i;

inline constexpr std::uint32_t kP = 998244353, k2P = 2 * kP;

[[gnu::noinline]] inline void forward_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4) \
_0(%[P],w) _0(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
_4(H,E)\
Z($-8,E)\
Y(H,E)\
G((%[table],%[tx],4),E)\
G((%[k],%[k]),F)\
Z($-8,F)\
G((%[ty],%[k],2),F)\
G((%[table],%[ty],4),F)\
D(32(%[tx]),p)\
D((%[tx]),A)\
s(R,d)\
s((%[a]),u)\
o(M,p,e)\
o(388(%[a]),p,g)\
n(M,A,c)\
i(q,d,f)\
o(I,p,C)\
z(f,d,f)\
o(260(%[a]),p,d)\
n(I,A,v)\
i(q,u,B)\
t(r,e,e)\
z(B,u,B)\
y(x,g,e,g)\
m(q,f,e)\
t(r,C,C)\
D(32(%[ty]),u)\
n(w,g,g)\
y(x,d,C,d)\
n(w,d,d)\
D((%[ty]),C)\
i(g,c,g)\
m(g,f,f)\
i(g,e,g)\
i(d,v,d)\
D(36(%[ty]),v)\
t(r,f,e)\
t(r,g,c)\
m(d,B,j)\
i(d,B,d)\
D(4(%[ty]),B)\
i(q,j,h)\
m(q,d,l)\
z(h,j,h)\
z(l,d,l)\
o(u,f,d)\
o(u,e,e)\
m(q,h,j)\
n(C,f,f)\
t(r,d,d)\
y(x,e,d,e)\
m(q,l,d)\
o(v,c,c)\
n(w,e,e)\
i(e,f,e)\
s(V,f)\
m(e,h,h)\
i(e,j,e)\
o(v,g,j)\
n(B,g,g)\
s(h,(%[a]))\
s(S,h)\
s(e,R)\
o(J,p,e)\
t(r,j,j)\
y(x,c,j,c)\
o(292(%[a]),p,j)\
t(r,e,e)\
n(w,c,c)\
y(x,j,e,j)\
o(N,p,e)\
n(w,j,j)\
i(c,g,c)\
o(420(%[a]),p,g)\
m(c,l,l)\
i(c,d,c)\
n(N,A,d)\
t(r,e,e)\
s(l,I)\
i(q,h,l)\
y(x,g,e,g)\
z(l,h,l)\
s(c,M)\
n(J,A,c)\
i(q,f,h)\
n(w,g,g)\
m(q,l,e)\
z(h,f,h)\
i(j,c,j)\
i(g,d,g)\
m(j,h,d)\
i(j,h,j)\
m(g,l,l)\
i(g,e,g)\
i(q,d,e)\
m(q,j,h)\
t(r,l,c)\
o(u,l,f)\
n(C,l,l)\
z(e,d,e)\
t(r,g,d)\
o(u,c,c)\
z(h,j,h)\
m(q,e,j)\
o(v,d,d)\
t(r,f,f)\
y(x,c,f,c)\
o(v,g,f)\
n(B,g,g)\
n(w,c,c)\
t(r,f,f)\
y(x,d,f,d)\
i(c,l,c)\
s(W,l)\
m(q,h,f)\
n(w,d,d)\
m(c,e,e)\
i(c,j,c)\
s(T,j)\
s(e,V)\
o(K,p,e)\
s(c,S)\
o(324(%[a]),p,c)\
i(d,g,d)\
o(O,p,g)\
t(r,e,e)\
m(d,h,h)\
i(d,f,d)\
o(452(%[a]),p,f)\
y(x,c,e,c)\
s(h,J)\
n(O,A,h)\
n(K,A,e)\
t(r,g,g)\
n(w,c,c)\
s(d,N)\
y(x,f,g,f)\
i(q,l,g)\
n(w,f,f)\
i(q,j,d)\
z(g,l,g)\
z(d,j,d)\
i(c,e,c)\
m(q,d,e)\
m(c,g,j)\
i(c,g,c)\
i(f,h,f)\
i(q,j,h)\
m(q,c,g)\
m(f,d,d)\
i(f,e,f)\
z(h,j,h)\
z(g,c,g)\
t(r,d,c)\
o(u,d,j)\
t(r,f,e)\
o(v,f,l)\
o(u,c,c)\
o(v,e,e)\
n(C,d,d)\
n(B,f,f)\
t(r,j,j)\
t(r,l,l)\
y(x,c,j,c)\
y(x,e,l,e)\
m(q,h,l)\
m(q,g,j)\
n(w,c,c)\
n(w,e,e)\
i(c,d,c)\
i(e,f,e)\
s(U,f)\
s(X,d)\
m(c,h,h)\
i(c,l,c)\
m(e,g,g)\
i(e,j,e)\
o(Q,p,j)\
o(L,p,l)\
s(h,W)\
o(484(%[a]),p,h)\
o(356(%[a]),p,p)\
s(c,T)\
n(Q,A,c)\
n(L,A,A)\
s(g,K)\
t(r,j,j)\
t(r,l,l)\
s(e,O)\
y(x,h,j,h)\
y(x,p,l,p)\
n(w,h,h)\
n(w,p,p)\
i(q,f,l)\
z(l,f,l)\
i(q,d,f)\
i(h,c,h)\
m(q,l,c)\
z(f,d,f)\
i(p,A,p)\
m(h,l,l)\
i(h,c,h)\
m(p,f,c)\
i(p,f,p)\
t(r,l,f)\
o(u,l,A)\
t(r,h,d)\
o(v,h,j)\
o(u,f,u)\
o(v,d,v)\
i(q,c,d)\
m(q,p,f)\
n(C,l,C)\
n(B,h,B)\
z(d,c,d)\
z(f,p,f)\
t(r,A,A)\
t(r,j,j)\
m(q,d,p)\
m(q,f,c)\
y(x,u,A,u)\
y(x,v,j,v)\
n(w,u,u)\
n(w,v,v)\
i(u,C,u)\
i(v,B,v)\
m(u,d,d)\
i(u,p,u)\
m(v,f,f)\
i(v,c,v)\
s(d,X)\
s(u,U)\
s(f,L)\
s(v,Q)\
Y($512,%[a])\
_2(H)\
_1(%[end],%[a])\
_3(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm2,%%ymm4,%%ymm7,%%ymm5,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm11,%%ymm13,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm0,%%ymm14,$32,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPSRLQ,%%ymm3,%%ymm9,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm1,%%ymm10,%%ymm8,CP_FPS_ASM_VBROADCASTSS,%[tx],%[ty],CP_FPS_ASM_LEA,%[k],256(%[a]),288(%[a]),320(%[a]),352(%[a]),384(%[a]),416(%[a]),448(%[a]),480(%[a]),128(%[a]),160(%[a]),192(%[a]),224(%[a]),32(%[a]),64(%[a]),96(%[a]),CP_FPS_ASM_ADD,CP_FPS_ASM_AND,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_INC,CP_FPS_ASM_JNE,CP_FPS_ASM_MOV)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_h4(Vec* a, std::size_t count, const std::uint32_t* table, std::size_t k) {
	Vec* const end = a + 16 * count;
	const std::uint32_t *tx, *ty;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4) \
_0(%[P],x) _0(%[P2],n) ".p2align 5\n\t" "1:\n\t" \
_4(H,E)\
Z($-8,E)\
Y(H,E)\
G((%[table],%[tx],4),E)\
G((%[k],%[k]),F)\
Z($-8,F)\
G((%[ty],%[k],2),F)\
G((%[table],%[ty],4),F)\
t((%[a]),l)\
t(R,m)\
D(32(%[ty]),v)\
D((%[ty]),C)\
p(n,l,w)\
p(I,l,l)\
i(I,w,w)\
p(n,m,g)\
i(n,l,c)\
p(M,m,m)\
D(36(%[ty]),u)\
s(o,w,d)\
r(v,w,e)\
i(M,g,g)\
q(C,w,w)\
D(4(%[ty]),B)\
r(v,d,d)\
s(o,g,f)\
A(c,l,c)\
i(n,m,l)\
A(l,m,l)\
s(o,e,e)\
p(l,c,m)\
z(y,d,e,d)\
i(n,m,e)\
q(x,d,d)\
p(n,c,c)\
A(e,m,e)\
D(32(%[tx]),m)\
i(l,c,l)\
t(e,(%[a]))\
s(o,l,e)\
r(u,g,c)\
r(u,f,f)\
i(d,w,d)\
q(B,g,g)\
D((%[tx]),w)\
s(o,c,c)\
z(y,f,c,f)\
p(n,d,c)\
q(x,f,f)\
r(m,l,h)\
r(m,e,e)\
i(f,g,f)\
i(f,c,c)\
s(o,h,h)\
q(w,l,l)\
p(f,d,f)\
s(o,c,d)\
r(m,c,g)\
z(y,e,h,e)\
q(w,c,c)\
t(T,h)\
r(m,d,d)\
q(x,e,e)\
i(n,f,j)\
A(j,f,j)\
t(S,f)\
s(o,g,g)\
t(j,I)\
z(y,d,g,d)\
i(e,l,e)\
q(x,d,d)\
t(e,R)\
i(d,c,d)\
p(n,h,c)\
p(J,h,h)\
t(d,M)\
i(J,c,c)\
p(n,f,d)\
i(n,h,e)\
p(N,f,f)\
s(o,c,l)\
r(v,c,g)\
i(N,d,d)\
q(C,c,c)\
r(v,l,l)\
s(o,d,j)\
A(e,h,e)\
i(n,f,h)\
r(u,j,j)\
A(h,f,h)\
r(u,d,f)\
s(o,g,g)\
q(B,d,d)\
z(y,l,g,l)\
p(n,e,g)\
p(h,e,e)\
s(o,f,f)\
q(x,l,l)\
i(h,g,h)\
i(n,e,g)\
z(y,j,f,j)\
A(g,e,g)\
s(o,h,e)\
r(m,h,f)\
q(x,j,j)\
r(m,e,e)\
t(g,T)\
i(l,c,l)\
q(w,h,h)\
p(n,l,c)\
s(o,f,f)\
i(j,d,j)\
z(y,e,f,e)\
i(j,c,c)\
q(x,e,e)\
p(j,l,j)\
t(W,l)\
s(o,c,f)\
r(m,c,d)\
q(w,c,c)\
i(n,j,g)\
r(m,f,f)\
A(g,j,g)\
t(U,j)\
i(e,h,e)\
t(g,J)\
s(o,d,d)\
t(e,S)\
z(y,f,d,f)\
q(x,f,f)\
p(n,l,d)\
p(K,l,l)\
i(f,c,f)\
i(K,d,d)\
p(n,j,c)\
i(n,l,e)\
p(O,j,j)\
t(f,N)\
s(o,d,f)\
r(v,d,g)\
i(O,c,c)\
q(C,d,d)\
r(v,f,f)\
s(o,c,h)\
A(e,l,e)\
i(n,j,l)\
r(u,h,h)\
A(l,j,l)\
r(u,c,j)\
s(o,g,g)\
q(B,c,c)\
z(y,f,g,f)\
p(n,e,g)\
p(l,e,e)\
s(o,j,j)\
q(x,f,f)\
i(l,g,l)\
i(n,e,g)\
z(y,h,j,h)\
A(g,e,g)\
s(o,l,e)\
r(m,l,j)\
q(x,h,h)\
r(m,e,e)\
t(g,W)\
i(f,d,f)\
q(w,l,l)\
p(n,f,d)\
s(o,j,j)\
i(h,c,h)\
z(y,e,j,e)\
i(h,d,d)\
q(x,e,e)\
p(h,f,h)\
t(X,f)\
s(o,d,j)\
r(m,d,c)\
q(w,d,d)\
i(n,h,g)\
r(m,j,j)\
A(g,h,g)\
t(V,h)\
i(e,l,e)\
t(g,K)\
s(o,c,c)\
t(e,U)\
z(y,j,c,j)\
q(x,j,j)\
p(n,f,c)\
p(L,f,f)\
i(j,d,j)\
i(L,c,c)\
p(n,h,d)\
i(n,f,e)\
p(Q,h,h)\
t(j,O)\
s(o,c,j)\
r(v,c,g)\
i(Q,d,d)\
q(C,c,C)\
r(v,j,v)\
s(o,d,j)\
r(u,d,c)\
A(e,f,e)\
r(u,j,u)\
q(B,d,B)\
i(n,h,d)\
s(o,g,g)\
A(d,h,d)\
z(y,v,g,v)\
s(o,c,c)\
p(n,e,g)\
p(d,e,e)\
q(x,v,v)\
z(y,u,c,u)\
i(d,g,d)\
i(n,e,g)\
q(x,u,u)\
s(o,d,c)\
r(m,d,h)\
A(g,e,g)\
r(m,c,c)\
q(w,d,d)\
t(g,X)\
i(v,C,v)\
i(u,B,u)\
p(n,v,B)\
s(o,h,h)\
i(u,B,B)\
z(y,c,h,c)\
p(u,v,u)\
s(o,B,v)\
r(m,B,h)\
q(x,c,c)\
i(n,u,C)\
r(m,v,m)\
q(w,B,w)\
A(C,u,C)\
t(C,L)\
s(o,h,h)\
i(c,d,c)\
z(y,m,h,m)\
t(c,V)\
q(x,m,m)\
i(m,w,m)\
t(m,Q)\
Y($512,%[a])\
_2(H)\
_1(%[end],%[a])\
_3(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm8,%%ymm9,%%ymm11,%%ymm5,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm13,%%ymm0,%%ymm1,%%ymm14,$32,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,%%ymm7,%%ymm2,%%ymm4,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,%%ymm10,%%ymm3,CP_FPS_ASM_VBROADCASTSS,%[tx],%[ty],CP_FPS_ASM_LEA,%[k],128(%[a]),160(%[a]),192(%[a]),224(%[a]),384(%[a]),416(%[a]),448(%[a]),480(%[a]),256(%[a]),288(%[a]),32(%[a]),320(%[a]),352(%[a]),64(%[a]),96(%[a]),CP_FPS_ASM_ADD,CP_FPS_ASM_AND,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_INC,CP_FPS_ASM_JNE,CP_FPS_ASM_MOV)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [k] "+r"(k), [tx] "=&r"(tx), [ty] "=&r"(ty)
		: [end] "r"(end), [table] "r"(table), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y) {
	Vec* const end = a + 4 * count;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) \
_12(%[P],t) _12(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
A(32(%[x]),i)\
A((%[x]),d)\
B(_1,n)\
A(32(%[y]),j)\
A(36(%[y]),h)\
B((%[a]),o)\
A((%[y]),r)\
w(T,i,f)\
w(100(%[a]),i,e)\
v(T,d,s)\
c(q,n,g)\
w(N,i,k)\
l(g,n,g)\
w(68(%[a]),i,i)\
A(4(%[y]),n)\
C(z,f,f)\
u(q,g,m)\
v(N,d,d)\
c(q,o,p)\
E(D,e,f,e)\
C(z,k,k)\
l(p,o,p)\
v(t,e,e)\
E(D,i,k,i)\
v(t,i,i)\
c(e,s,e)\
u(e,g,g)\
c(e,m,e)\
c(i,d,i)\
C(z,g,d)\
w(j,g,m)\
C(z,e,s)\
w(h,e,k)\
w(j,d,j)\
w(h,s,h)\
u(i,p,s)\
c(i,p,i)\
v(r,g,r)\
v(n,e,n)\
c(q,s,e)\
u(q,i,g)\
B(Y,p)\
C(z,m,m)\
C(z,k,k)\
l(e,s,e)\
l(g,i,g)\
A(36(%[x]),i)\
B(X,s)\
E(D,j,m,j)\
E(D,h,k,h)\
u(q,e,k)\
u(q,g,m)\
A(4(%[x]),d)\
v(t,j,j)\
v(t,h,h)\
c(j,r,j)\
c(h,n,h)\
u(j,e,e)\
c(j,k,j)\
u(h,g,g)\
c(h,m,h)\
c(q,p,m)\
c(q,e,k)\
c(q,j,n)\
c(q,g,r)\
w(G,i,o)\
w(228(%[a]),i,f)\
l(m,p,m)\
c(q,s,p)\
l(p,s,p)\
l(k,e,k)\
l(n,j,n)\
l(r,g,r)\
A(40(%[y]),g)\
v(G,d,j)\
u(q,m,e)\
c(q,h,s)\
C(z,o,o)\
v(F,d,d)\
l(s,h,s)\
w(F,i,h)\
E(D,f,o,f)\
w(196(%[a]),i,i)\
A(44(%[y]),o)\
v(t,f,f)\
C(z,h,h)\
E(D,i,h,i)\
A(8(%[y]),h)\
c(f,j,f)\
v(t,i,i)\
A(12(%[y]),j)\
u(f,m,m)\
c(f,e,f)\
c(t,k,e)\
l(e,k,e)\
C(z,m,k)\
w(g,k,k)\
c(i,d,i)\
B(e,(%[a]))\
w(g,m,g)\
C(z,f,e)\
w(o,e,e)\
w(o,f,o)\
u(i,p,d)\
c(i,p,i)\
c(q,d,p)\
C(z,g,g)\
l(p,d,p)\
u(q,i,d)\
C(z,o,o)\
E(D,k,g,k)\
v(h,m,h)\
l(d,i,d)\
E(D,e,o,e)\
v(t,k,k)\
v(j,f,j)\
u(q,p,f)\
B(Z,o)\
v(t,e,e)\
u(q,d,i)\
c(t,n,m)\
c(t,r,g)\
l(m,n,m)\
l(g,r,g)\
A(40(%[x]),r)\
B(_0,n)\
c(k,h,k)\
B(m,_1)\
A(8(%[x]),m)\
c(t,s,h)\
c(e,j,e)\
u(k,p,p)\
c(k,f,k)\
l(h,s,h)\
B(g,N)\
u(e,d,d)\
c(e,i,e)\
c(q,p,i)\
c(q,k,g)\
B(h,T)\
c(q,d,h)\
c(q,e,s)\
l(i,p,i)\
l(g,k,g)\
l(h,d,h)\
l(s,e,s)\
c(t,i,e)\
c(t,g,d)\
A(48(%[y]),k)\
c(t,h,p)\
c(t,s,f)\
l(e,i,e)\
l(d,g,d)\
A(16(%[y]),g)\
c(q,o,i)\
l(p,h,p)\
l(f,s,f)\
B(e,X)\
w(I,r,e)\
w(356(%[a]),r,s)\
c(q,n,h)\
l(i,o,i)\
B(d,Y)\
A(52(%[y]),d)\
v(I,m,o)\
l(h,n,h)\
B(p,F)\
w(H,r,p)\
w(324(%[a]),r,r)\
u(q,h,n)\
B(f,G)\
C(z,e,e)\
A(20(%[y]),f)\
v(H,m,m)\
E(D,s,e,s)\
v(t,s,s)\
C(z,p,p)\
E(D,r,p,r)\
v(t,r,r)\
c(s,o,s)\
u(s,h,h)\
c(s,n,s)\
C(z,h,n)\
w(k,h,o)\
C(z,s,p)\
w(d,s,e)\
w(k,n,k)\
w(d,p,d)\
c(r,m,r)\
u(r,i,m)\
c(r,i,r)\
v(g,h,g)\
v(f,s,f)\
B(_2,s)\
C(z,o,o)\
C(z,e,e)\
c(q,m,h)\
u(q,r,i)\
A(44(%[x]),p)\
B(_3,n)\
E(D,k,o,k)\
E(D,d,e,d)\
l(h,m,h)\
l(i,r,i)\
A(12(%[x]),r)\
v(t,k,k)\
v(t,d,d)\
u(q,h,m)\
u(q,i,e)\
c(k,g,k)\
c(d,f,d)\
u(k,h,h)\
c(k,m,k)\
u(d,i,i)\
c(d,e,d)\
c(q,s,e)\
c(q,h,m)\
c(q,k,f)\
c(q,i,g)\
w(K,p,o)\
w(484(%[a]),p,j)\
l(e,s,e)\
c(q,d,s)\
l(m,h,m)\
l(f,k,f)\
l(g,i,g)\
l(s,d,s)\
A(56(%[y]),d)\
v(K,r,i)\
c(q,n,k)\
c(t,m,h)\
C(z,o,o)\
l(k,n,k)\
v(J,r,r)\
l(h,m,h)\
A(24(%[y]),m)\
E(D,j,o,j)\
B(h,Z)\
A(60(%[y]),h)\
w(J,p,o)\
u(q,k,n)\
v(t,j,j)\
w(452(%[a]),p,p)\
C(z,o,o)\
c(j,i,j)\
E(D,p,o,p)\
A(28(%[y]),o)\
c(t,f,i)\
u(j,k,k)\
c(j,n,j)\
v(t,p,p)\
l(i,f,i)\
C(z,k,f)\
w(d,k,n)\
B(i,_0)\
w(d,f,d)\
v(m,k,m)\
C(z,j,k)\
c(t,g,f)\
w(h,k,k)\
c(p,r,p)\
l(f,g,f)\
w(h,j,h)\
C(z,n,n)\
u(p,e,g)\
c(p,e,p)\
c(t,s,e)\
B(f,H)\
E(D,d,n,d)\
c(q,g,n)\
u(q,p,f)\
l(e,s,e)\
B(_5,s)\
C(z,h,h)\
v(t,d,d)\
l(n,g,n)\
l(f,p,f)\
B(e,I)\
A(48(%[x]),e)\
B(_4,p)\
A(16(%[x]),g)\
E(D,k,h,k)\
v(o,j,o)\
u(q,n,j)\
u(q,f,h)\
v(t,k,k)\
c(d,m,d)\
u(d,n,n)\
c(d,j,d)\
c(q,n,j)\
c(q,d,m)\
c(k,o,k)\
l(j,n,j)\
l(m,d,m)\
u(k,f,f)\
c(k,h,k)\
c(t,j,h)\
c(t,m,d)\
c(q,s,n)\
c(q,f,o)\
c(q,k,r)\
l(h,j,h)\
w(M,e,j)\
w(612(%[a]),e,i)\
l(n,s,n)\
c(q,p,s)\
B(h,_2)\
l(s,p,s)\
l(o,f,o)\
l(r,k,r)\
l(d,m,d)\
A(96(%[y]),m)\
A(64(%[y]),k)\
A(100(%[y]),f)\
v(M,g,p)\
u(q,n,h)\
B(d,_3)\
C(z,j,j)\
v(L,g,g)\
w(L,e,d)\
E(D,i,j,i)\
w(580(%[a]),e,e)\
A(68(%[y]),j)\
v(t,i,i)\
C(z,d,d)\
E(D,e,d,e)\
c(t,o,d)\
c(i,p,i)\
v(t,e,e)\
l(d,o,d)\
c(t,r,o)\
u(i,n,n)\
c(i,h,i)\
l(o,r,o)\
B(d,J)\
C(z,n,d)\
w(m,n,r)\
C(z,i,h)\
w(f,i,p)\
B(o,K)\
w(m,d,m)\
w(f,h,f)\
c(e,g,e)\
u(e,s,g)\
c(e,s,e)\
v(k,n,k)\
v(j,i,j)\
B(_6,i)\
C(z,r,r)\
C(z,p,p)\
c(q,g,n)\
u(q,e,s)\
A(52(%[x]),h)\
B(_7,d)\
E(D,m,r,m)\
E(D,f,p,f)\
l(n,g,n)\
l(s,e,s)\
A(20(%[x]),e)\
v(t,m,m)\
v(t,f,f)\
u(q,n,g)\
u(q,s,p)\
c(m,k,m)\
c(f,j,f)\
u(m,n,n)\
c(m,g,m)\
u(f,s,s)\
c(f,p,f)\
c(q,i,p)\
c(q,n,g)\
c(q,m,j)\
c(q,s,k)\
w(Q,h,r)\
w(740(%[a]),h,o)\
l(p,i,p)\
c(q,f,i)\
l(g,n,g)\
l(j,m,j)\
l(k,s,k)\
l(i,f,i)\
A(104(%[y]),f)\
v(Q,e,s)\
c(q,d,m)\
c(t,g,n)\
C(z,r,r)\
l(m,d,m)\
v(O,e,e)\
l(n,g,n)\
A(72(%[y]),g)\
E(D,o,r,o)\
B(n,_4)\
A(108(%[y]),n)\
w(O,h,r)\
u(q,m,d)\
v(t,o,o)\
w(708(%[a]),h,h)\
C(z,r,r)\
c(o,s,o)\
E(D,h,r,h)\
A(76(%[y]),r)\
c(t,j,s)\
u(o,m,m)\
c(o,d,o)\
v(t,h,h)\
l(s,j,s)\
C(z,m,j)\
w(f,m,d)\
B(s,_5)\
w(f,j,f)\
v(g,m,g)\
C(z,o,m)\
c(t,k,j)\
w(n,m,m)\
c(h,e,h)\
l(j,k,j)\
w(n,o,n)\
C(z,d,d)\
u(h,p,k)\
c(h,p,h)\
c(t,i,p)\
B(j,L)\
E(D,f,d,f)\
c(q,k,d)\
u(q,h,j)\
l(p,i,p)\
B(_9,i)\
C(z,n,n)\
v(t,f,f)\
l(d,k,d)\
l(j,h,j)\
B(p,M)\
A(56(%[x]),p)\
B(_8,h)\
A(24(%[x]),k)\
E(D,m,n,m)\
v(r,o,r)\
u(q,d,o)\
u(q,j,n)\
v(t,m,m)\
c(f,g,f)\
u(f,d,d)\
c(f,o,f)\
c(q,d,o)\
c(q,f,g)\
c(m,r,m)\
l(o,d,o)\
l(g,f,g)\
u(m,j,j)\
c(m,n,m)\
c(t,o,n)\
c(t,g,f)\
c(q,i,d)\
c(q,j,r)\
c(q,m,e)\
l(n,o,n)\
w(S,p,o)\
w(868(%[a]),p,s)\
l(d,i,d)\
c(q,h,i)\
B(n,_6)\
l(i,h,i)\
l(r,j,r)\
l(e,m,e)\
l(f,g,f)\
A(112(%[y]),g)\
A(80(%[y]),m)\
A(116(%[y]),j)\
v(S,k,h)\
u(q,d,n)\
B(f,_7)\
C(z,o,o)\
v(R,k,k)\
w(R,p,f)\
E(D,s,o,s)\
w(836(%[a]),p,p)\
A(84(%[y]),o)\
v(t,s,s)\
C(z,f,f)\
E(D,p,f,p)\
c(t,r,f)\
c(s,h,s)\
v(t,p,p)\
l(f,r,f)\
c(t,e,r)\
u(s,d,d)\
c(s,n,s)\
l(r,e,r)\
B(f,O)\
C(z,d,f)\
w(g,d,e)\
C(z,s,n)\
w(j,s,h)\
B(r,Q)\
w(g,f,g)\
w(j,n,j)\
c(p,k,p)\
u(p,i,k)\
c(p,i,p)\
v(m,d,m)\
v(o,s,o)\
B(_10,s)\
C(z,e,e)\
C(z,h,h)\
c(q,k,d)\
u(q,p,i)\
A(60(%[x]),n)\
B(_11,f)\
E(D,g,e,g)\
E(D,j,h,j)\
l(d,k,d)\
l(i,p,i)\
A(28(%[x]),p)\
v(t,g,g)\
v(t,j,j)\
u(q,d,k)\
u(q,i,h)\
c(g,m,g)\
c(j,o,j)\
u(g,d,d)\
c(g,k,g)\
u(j,i,i)\
c(j,h,j)\
c(q,s,h)\
c(q,d,k)\
c(q,g,o)\
c(q,i,m)\
w(V,n,e)\
w(996(%[a]),n,r)\
l(h,s,h)\
c(q,j,s)\
l(k,d,k)\
l(o,g,o)\
l(m,i,m)\
l(s,j,s)\
A(120(%[y]),j)\
v(V,p,i)\
c(q,f,g)\
c(t,k,d)\
C(z,e,e)\
l(g,f,g)\
v(U,p,p)\
l(d,k,d)\
A(88(%[y]),k)\
E(D,r,e,r)\
B(d,_8)\
A(124(%[y]),d)\
w(U,n,e)\
u(q,g,f)\
v(t,r,r)\
w(964(%[a]),n,n)\
C(z,e,e)\
c(r,i,r)\
E(D,n,e,n)\
A(92(%[y]),e)\
c(t,o,i)\
u(r,g,g)\
c(r,f,r)\
v(t,n,n)\
l(i,o,i)\
C(z,g,o)\
w(j,g,f)\
B(i,_9)\
w(j,o,j)\
v(k,g,k)\
C(z,r,g)\
c(t,m,o)\
w(d,g,g)\
c(n,p,n)\
l(o,m,o)\
w(d,r,d)\
C(z,f,f)\
u(n,h,m)\
c(n,h,n)\
c(t,s,h)\
B(o,R)\
E(D,j,f,j)\
c(q,m,f)\
u(q,n,o)\
l(h,s,h)\
C(z,d,d)\
v(t,j,j)\
l(f,m,f)\
l(o,n,o)\
B(h,S)\
E(D,g,d,g)\
v(e,r,e)\
u(q,f,r)\
u(q,o,d)\
v(t,g,g)\
c(j,k,j)\
u(j,f,f)\
c(j,r,j)\
c(g,e,g)\
c(q,f,e)\
c(q,j,r)\
u(g,o,o)\
c(g,d,g)\
l(e,f,e)\
l(r,j,r)\
c(q,o,j)\
c(q,g,f)\
c(t,e,d)\
c(t,r,k)\
l(j,o,j)\
l(f,g,f)\
l(d,e,d)\
l(k,r,k)\
c(t,j,r)\
c(t,f,e)\
B(d,_10)\
l(r,j,r)\
l(e,f,e)\
B(k,_11)\
B(r,U)\
B(e,V)\
W($1024,%[a])\
W($64,%[x])\
W($128,%[y])\
_13(%[end],%[a])\
_14(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm1,%%ymm8,%%ymm7,%%ymm10,%%ymm4,%%ymm0,%%ymm3,%%ymm11,CP_FPS_ASM_VPMINUD,%%ymm12,%%ymm2,%%ymm5,%%ymm13,%%ymm14,%%ymm6,%%ymm9,%%ymm15,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,$32,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPSRLQ,$170,CP_FPS_ASM_VPBLENDD,192(%[a]),224(%[a]),320(%[a]),352(%[a]),448(%[a]),480(%[a]),576(%[a]),608(%[a]),64(%[a]),704(%[a]),736(%[a]),832(%[a]),864(%[a]),96(%[a]),960(%[a]),992(%[a]),CP_FPS_ASM_ADD,128(%[a]),160(%[a]),256(%[a]),288(%[a]),32(%[a]),384(%[a]),416(%[a]),512(%[a]),544(%[a]),640(%[a]),672(%[a]),768(%[a]),800(%[a]),896(%[a]),928(%[a]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void inverse_bottom(Vec* a, std::size_t count, const std::uint32_t* x, const std::uint32_t* y, const Vec* from) {
	Vec* const end = a + 4 * count;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z) \
X(%[P],B) X(%[P2],q) ".p2align 5\n\t" "1:\n\t" \
A((%[from]),p)\
A(64(%[from]),i)\
z(32(%[y]),c)\
z(36(%[y]),k)\
z((%[y]),w)\
z(4(%[y]),f)\
z(32(%[x]),n)\
s(q,p,g)\
s(J,p,p)\
e(J,g,g)\
s(q,i,j)\
e(q,p,o)\
s(V,i,i)\
v(r,g,m)\
u(c,g,h)\
e(V,j,j)\
E(o,p,o)\
z((%[x]),p)\
u(c,m,c)\
v(r,j,m)\
u(k,j,d)\
e(q,i,l)\
u(k,m,k)\
t(w,g,w)\
E(l,i,l)\
s(q,o,i)\
A(128(%[from]),g)\
v(r,h,h)\
t(f,j,f)\
e(l,i,i)\
s(l,o,l)\
A(192(%[from]),o)\
D(C,c,h,c)\
v(r,d,d)\
v(r,i,h)\
u(n,i,j)\
t(B,c,c)\
D(C,k,d,k)\
u(n,h,h)\
z(40(%[y]),d)\
e(q,l,m)\
t(B,k,k)\
E(m,l,m)\
z(8(%[y]),l)\
v(r,j,j)\
A(m,(%[a]))\
e(c,w,c)\
D(C,h,j,h)\
t(p,i,i)\
z(44(%[y]),j)\
e(k,f,k)\
s(q,c,f)\
t(B,h,h)\
z(12(%[y]),w)\
e(k,f,f)\
s(k,c,k)\
s(q,g,c)\
s(G,g,g)\
v(r,f,m)\
t(p,f,p)\
e(G,c,c)\
u(n,f,f)\
u(n,m,n)\
e(h,i,h)\
s(q,o,i)\
v(r,c,m)\
s(H,o,o)\
A(h,64(%[a]))\
e(H,i,i)\
u(d,m,m)\
t(l,c,l)\
z(36(%[x]),h)\
u(d,c,d)\
v(r,f,f)\
v(r,i,c)\
D(C,n,f,n)\
e(q,g,f)\
u(j,c,c)\
E(f,g,f)\
t(B,n,n)\
e(q,o,g)\
u(j,i,j)\
v(r,d,d)\
t(w,i,w)\
E(g,o,g)\
z(4(%[x]),o)\
D(C,m,d,m)\
s(q,f,d)\
s(g,f,f)\
e(q,k,i)\
t(B,m,m)\
e(g,d,g)\
E(i,k,i)\
e(n,p,n)\
A(256(%[from]),p)\
A(320(%[from]),k)\
v(r,j,j)\
v(r,g,d)\
A(i,32(%[a]))\
D(C,c,j,c)\
u(h,d,d)\
A(n,96(%[a]))\
u(h,g,n)\
e(q,f,j)\
t(B,c,c)\
e(m,l,m)\
E(j,f,j)\
z(48(%[y]),f)\
z(16(%[y]),l)\
s(q,m,i)\
A(j,128(%[a]))\
v(r,n,n)\
e(c,w,c)\
D(C,d,n,d)\
t(o,g,g)\
z(52(%[y]),n)\
e(c,i,i)\
t(B,d,d)\
s(c,m,c)\
s(q,p,m)\
z(20(%[y]),w)\
s(I,p,p)\
v(r,i,j)\
t(o,i,o)\
e(I,m,m)\
u(h,i,i)\
u(h,j,h)\
s(q,k,j)\
e(K,j,j)\
s(K,k,k)\
e(d,g,d)\
v(r,m,g)\
u(f,g,g)\
A(d,192(%[a]))\
u(f,m,f)\
v(r,j,d)\
t(l,m,l)\
v(r,i,i)\
z(40(%[x]),m)\
D(C,h,i,h)\
e(q,p,i)\
u(n,d,d)\
v(r,f,f)\
E(i,p,i)\
t(B,h,h)\
z(8(%[x]),p)\
u(n,j,n)\
D(C,g,f,g)\
e(q,k,f)\
t(B,g,g)\
t(w,j,w)\
E(f,k,f)\
s(q,i,k)\
e(f,k,k)\
s(f,i,f)\
e(h,o,h)\
A(384(%[from]),o)\
e(q,c,i)\
v(r,n,n)\
E(i,c,i)\
A(h,224(%[a]))\
A(448(%[from]),h)\
v(r,k,c)\
e(q,f,j)\
D(C,d,n,d)\
e(g,l,g)\
E(j,f,j)\
A(i,160(%[a]))\
z(56(%[y]),i)\
z(24(%[y]),f)\
t(B,d,d)\
s(q,g,l)\
u(m,k,n)\
A(j,256(%[a]))\
u(m,c,c)\
t(p,k,k)\
e(d,w,d)\
v(r,n,n)\
z(60(%[y]),w)\
e(d,l,l)\
D(C,c,n,c)\
s(d,g,d)\
s(q,o,g)\
z(28(%[y]),n)\
s(L,o,o)\
v(r,l,j)\
t(B,c,c)\
e(L,g,g)\
u(m,j,j)\
t(p,l,p)\
u(m,l,m)\
s(q,h,l)\
s(M,h,h)\
e(M,l,l)\
t(f,g,f)\
e(c,k,c)\
v(r,g,k)\
u(i,g,g)\
u(i,k,i)\
A(c,320(%[a]))\
v(r,l,c)\
z(44(%[x]),k)\
v(r,m,m)\
u(w,c,c)\
D(C,j,m,j)\
u(w,l,w)\
e(q,o,m)\
v(r,g,g)\
E(m,o,m)\
t(B,j,j)\
e(q,h,o)\
D(C,i,g,i)\
t(n,l,n)\
E(o,h,o)\
z(12(%[x]),h)\
s(q,m,l)\
v(r,w,w)\
t(B,i,i)\
e(o,l,l)\
s(o,m,o)\
A(512(%[from]),m)\
D(C,c,w,c)\
v(r,l,w)\
e(q,d,g)\
e(j,p,j)\
t(B,c,c)\
E(g,d,g)\
A(j,352(%[a]))\
A(576(%[from]),j)\
e(q,o,d)\
e(i,f,i)\
u(k,w,w)\
E(d,o,d)\
A(g,288(%[a]))\
z(96(%[y]),g)\
u(k,l,o)\
z(64(%[y]),f)\
s(q,i,p)\
A(d,384(%[a]))\
e(c,n,c)\
z(100(%[y]),n)\
e(c,p,p)\
v(r,o,o)\
s(c,i,c)\
v(r,p,i)\
u(k,p,d)\
D(C,w,o,w)\
t(h,l,l)\
u(k,i,k)\
t(B,w,w)\
z(68(%[y]),i)\
e(q,c,o)\
t(h,p,h)\
E(o,c,o)\
s(q,m,c)\
s(N,m,m)\
e(N,c,c)\
v(r,d,d)\
A(o,416(%[a]))\
s(q,j,o)\
e(q,m,p)\
e(O,o,o)\
s(O,j,j)\
E(p,m,p)\
D(C,k,d,k)\
v(r,c,d)\
u(g,c,m)\
t(f,c,f)\
e(q,j,c)\
u(g,d,g)\
E(c,j,c)\
t(B,k,k)\
e(w,l,w)\
z(48(%[x]),l)\
z(16(%[x]),j)\
A(w,448(%[a]))\
v(r,o,w)\
u(n,o,d)\
u(n,w,n)\
v(r,m,m)\
s(q,p,w)\
s(c,p,p)\
D(C,g,m,g)\
t(i,o,i)\
e(c,w,c)\
e(k,h,k)\
A(640(%[from]),h)\
A(704(%[from]),w)\
v(r,d,d)\
t(B,g,g)\
v(r,c,o)\
e(q,p,m)\
A(k,480(%[a]))\
D(C,n,d,n)\
E(m,p,m)\
z(104(%[y]),p)\
t(B,n,n)\
A(m,512(%[a]))\
z(72(%[y]),m)\
e(g,f,g)\
s(q,g,f)\
u(l,c,d)\
u(l,o,o)\
e(n,i,n)\
t(j,c,c)\
z(108(%[y]),i)\
e(n,f,f)\
s(n,g,n)\
s(q,h,g)\
s(Q,h,h)\
v(r,f,k)\
v(r,d,d)\
e(Q,g,g)\
u(l,k,k)\
D(C,o,d,o)\
t(j,f,j)\
z(76(%[y]),d)\
u(l,f,l)\
t(B,o,o)\
s(q,w,f)\
s(R,w,w)\
e(R,f,f)\
t(m,g,m)\
v(r,l,l)\
e(o,c,o)\
v(r,g,c)\
u(p,g,g)\
u(p,c,p)\
D(C,k,l,k)\
A(o,576(%[a]))\
v(r,f,o)\
z(52(%[x]),l)\
e(q,h,c)\
u(i,o,o)\
E(c,h,c)\
t(B,k,k)\
e(q,w,h)\
u(i,f,i)\
v(r,g,g)\
E(h,w,h)\
z(20(%[x]),w)\
D(C,p,g,p)\
t(d,f,d)\
s(q,c,f)\
s(h,c,c)\
t(B,p,p)\
e(h,f,h)\
e(k,j,k)\
A(768(%[from]),j)\
e(q,n,f)\
v(r,i,i)\
E(f,n,f)\
A(k,608(%[a]))\
A(832(%[from]),k)\
v(r,h,n)\
e(q,c,g)\
D(C,o,i,o)\
E(g,c,g)\
A(f,544(%[a]))\
z(112(%[y]),f)\
t(B,o,o)\
e(p,m,p)\
A(g,640(%[a]))\
z(80(%[y]),g)\
s(q,p,m)\
u(l,h,c)\
u(l,n,n)\
e(o,d,o)\
t(w,h,h)\
z(116(%[y]),d)\
e(o,m,m)\
v(r,c,c)\
s(o,p,o)\
s(q,j,p)\
z(84(%[y]),i)\
s(S,j,j)\
D(C,n,c,n)\
t(w,m,w)\
e(S,p,p)\
t(B,n,n)\
s(q,k,c)\
s(T,k,k)\
e(T,c,c)\
t(g,p,g)\
e(n,h,n)\
v(r,p,h)\
u(f,p,p)\
u(f,h,f)\
A(n,704(%[a]))\
v(r,c,n)\
e(q,j,h)\
E(h,j,h)\
z(56(%[x]),j)\
u(d,n,n)\
v(r,p,p)\
u(d,c,d)\
D(C,f,p,f)\
t(i,c,i)\
e(q,k,c)\
z(24(%[x]),p)\
t(B,f,f)\
E(c,k,c)\
s(q,h,k)\
v(r,d,d)\
e(c,k,k)\
s(c,h,c)\
v(r,m,h)\
D(C,n,d,n)\
u(l,m,m)\
u(l,h,l)\
v(r,k,h)\
t(B,n,n)\
e(f,g,f)\
A(896(%[from]),g)\
e(q,o,d)\
E(d,o,d)\
A(960(%[from]),o)\
u(j,h,h)\
v(r,m,m)\
A(d,672(%[a]))\
s(q,f,d)\
e(n,i,n)\
D(C,l,m,l)\
z(120(%[y]),m)\
u(j,k,i)\
e(n,d,d)\
t(B,l,l)\
t(p,k,k)\
s(n,f,n)\
z(88(%[y]),f)\
t(p,d,p)\
v(r,i,i)\
D(C,h,i,h)\
e(l,w,l)\
z(124(%[y]),w)\
v(r,d,i)\
u(j,d,d)\
u(j,i,j)\
t(B,h,h)\
A(l,736(%[a]))\
s(q,g,l)\
z(92(%[y]),i)\
s(U,g,g)\
e(U,l,l)\
v(r,d,d)\
D(C,j,d,j)\
e(h,k,h)\
s(q,o,k)\
v(r,l,d)\
e(W,k,k)\
u(m,d,d)\
t(f,l,f)\
s(W,o,o)\
A(h,832(%[a]))\
u(m,l,m)\
t(B,j,j)\
v(r,k,l)\
z(60(%[x]),h)\
u(w,l,l)\
u(w,k,w)\
v(r,m,m)\
t(i,k,i)\
e(j,p,j)\
e(q,g,p)\
z(28(%[x]),k)\
D(C,d,m,d)\
E(p,g,p)\
A(j,864(%[a]))\
e(q,o,j)\
e(q,c,g)\
v(r,w,w)\
t(B,d,d)\
E(j,o,j)\
E(g,c,g)\
D(C,l,w,l)\
s(q,p,w)\
s(j,p,p)\
e(q,n,c)\
A(g,768(%[a]))\
t(B,l,l)\
e(j,w,j)\
e(q,p,w)\
E(c,n,c)\
e(d,f,d)\
v(r,j,f)\
u(h,j,n)\
E(w,p,w)\
A(c,800(%[a]))\
s(q,d,c)\
u(h,f,f)\
A(w,896(%[a]))\
e(l,i,l)\
e(l,c,c)\
v(r,n,n)\
t(k,j,j)\
s(l,d,l)\
v(r,c,d)\
u(h,c,i)\
D(C,f,n,f)\
t(k,c,k)\
u(h,d,h)\
t(B,f,f)\
e(q,l,d)\
E(d,l,d)\
v(r,i,i)\
A(d,928(%[a]))\
D(C,h,i,h)\
e(f,j,f)\
t(B,h,h)\
A(f,960(%[a]))\
e(h,k,h)\
A(h,992(%[a]))\
F($1024,%[a])\
F($1024,%[from])\
F($64,%[x])\
F($128,%[y])\
Y(%[end],%[a])\
Z(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm2,%%ymm12,CP_FPS_ASM_VPSUBD,%%ymm5,%%ymm7,%%ymm11,%%ymm1,%%ymm8,%%ymm3,%%ymm13,%%ymm10,%%ymm6,%%ymm9,%%ymm0,%%ymm14,$32,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm4,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VMOVDQA,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_ADD,160(%[from]),224(%[from]),288(%[from]),32(%[from]),352(%[from]),416(%[from]),480(%[from]),544(%[from]),608(%[from]),672(%[from]),736(%[from]),800(%[from]),864(%[from]),928(%[from]),96(%[from]),992(%[from]),CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [x] "+r"(x), [y] "+r"(y), [from] "+r"(from)
		: [end] "r"(end), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_top8_lower(Vec* a, std::size_t q, const std::uint32_t* table) {
	alignas(32) Vec w[3];
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],y) S(%[P2],x) D(4(%[t]),d) m(d,0(%[w]))\
D(36(%[t]),z)\
D(8(%[t]),d)\
m(d,H)\
D(40(%[t]),E)\
D(12(%[t]),d)\
m(d,I)\
D(44(%[t]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(J,d)\
m(L,o)\
m(K,n)\
m((%[a]),p)\
v(q,o,f)\
u(z,o,i)\
u(z,f,f)\
k(y,n,c)\
s(C,o,e)\
k(d,n,r)\
v(q,d,j)\
u(z,d,g)\
l(d,c,c)\
u(z,j,j)\
v(q,i,i)\
s(C,d,d)\
B(A,f,i,f)\
v(q,c,i)\
v(q,g,g)\
u(z,i,i)\
s(y,f,f)\
B(A,j,g,j)\
u(z,c,g)\
s(C,c,c)\
s(y,j,j)\
l(f,e,f)\
k(x,p,e)\
v(q,g,g)\
l(f,e,e)\
k(f,p,f)\
l(j,d,j)\
B(A,i,g,i)\
k(y,p,g)\
l(x,f,d)\
s(y,i,i)\
k(o,p,p)\
l(o,g,o)\
G(d,f,d)\
k(x,n,f)\
k(j,n,n)\
l(x,e,g)\
l(j,f,j)\
G(g,e,g)\
v(q,n,e)\
u(E,n,f)\
u(E,e,e)\
s(H,n,n)\
l(i,c,i)\
v(q,j,c)\
u(F,c,c)\
v(q,f,f)\
B(A,e,f,e)\
u(F,j,f)\
s(I,j,j)\
s(y,e,e)\
v(q,f,f)\
B(A,c,f,c)\
l(e,n,e)\
m(N,n)\
k(x,p,f)\
k(r,p,p)\
s(y,c,c)\
l(r,f,r)\
m(p,(%[a]))\
m(Q,p)\
k(x,o,f)\
k(i,o,o)\
l(i,f,i)\
m(r,K)\
m(O,r)\
k(x,d,f)\
k(e,d,d)\
l(e,f,e)\
m(o,L)\
m(M,o)\
k(x,g,f)\
l(c,j,c)\
m(i,J)\
k(c,g,g)\
l(c,f,c)\
m(d,(%[b]))\
m(e,(%[b],%[h]))\
m(g,(%[b],%[h],2))\
m(c,(%[b],%[h3]))\
v(q,p,c)\
u(z,p,g)\
u(z,c,c)\
k(y,r,e)\
s(C,p,d)\
k(n,r,f)\
v(q,n,i)\
u(z,n,j)\
l(n,e,e)\
u(z,i,i)\
v(q,g,g)\
s(C,n,n)\
B(A,c,g,c)\
v(q,e,g)\
v(q,j,j)\
u(z,g,g)\
s(y,c,c)\
B(A,i,j,i)\
u(z,e,j)\
s(C,e,e)\
s(y,i,i)\
l(c,d,c)\
k(x,o,d)\
v(q,j,j)\
l(c,d,d)\
k(c,o,c)\
l(i,n,i)\
B(A,g,j,g)\
k(y,o,j)\
l(x,c,n)\
s(y,g,g)\
k(p,o,o)\
l(p,j,p)\
G(n,c,n)\
k(x,r,c)\
k(i,r,r)\
l(x,d,j)\
l(i,c,i)\
G(j,d,j)\
v(q,r,d)\
u(E,r,c)\
u(E,d,d)\
s(H,r,r)\
l(g,e,g)\
v(q,i,e)\
u(F,e,e)\
v(q,c,c)\
B(A,d,c,d)\
u(F,i,c)\
s(I,i,i)\
s(y,d,d)\
v(q,c,c)\
B(A,e,c,e)\
l(d,r,d)\
k(x,o,r)\
k(x,p,c)\
s(y,e,e)\
k(f,o,o)\
l(f,r,f)\
k(g,p,p)\
k(x,n,r)\
l(g,c,g)\
k(d,n,n)\
m(o,M)\
k(x,j,o)\
l(d,r,d)\
m(f,O)\
l(e,i,e)\
m(p,Q)\
k(e,j,j)\
l(e,o,e)\
m(g,N)\
m(n,32(%[b]))\
m(d,32(%[b],%[h]))\
m(j,32(%[b],%[h],2))\
m(e,32(%[b],%[h3]))\
R($64,%[a])\
R($64,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm0,%%ymm7,%%ymm4,%%ymm10,%%ymm5,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPSUBD,CP_FPS_ASM_VMOVDQA,%%ymm2,%%ymm1,%%ymm3,$32,%%ymm8,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm14,%%ymm15,%%ymm11,$170,CP_FPS_ASM_VPBLENDD,(%[w]),CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,CP_FPS_ASM_VPMINUD,32(%[w]),64(%[w]),(%[a],%[h3]),(%[a],%[h]),(%[a],%[h],2),32(%[a]),32(%[a],%[h3]),32(%[a],%[h]),32(%[a],%[h],2),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [t] "r"(table), [w] "r"(w), [P] "m"(kP),
		  [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

[[gnu::noinline]] inline void forward_top8_upper(Vec* a, std::size_t q, const std::uint32_t* table) {
	alignas(32) Vec w[3];
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],o) S(%[P2],y) D(4(%[t]),n) m(n,0(%[w]))\
D(36(%[t]),z)\
D(8(%[t]),n)\
m(n,H)\
D(40(%[t]),E)\
D(12(%[t]),n)\
m(n,I)\
D(44(%[t]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(J,n)\
m(L,v)\
m(K,e)\
m((%[b]),k)\
j(n,o,c)\
j(v,o,f)\
u(q,f,p)\
s(z,f,d)\
j(e,o,g)\
l(o,e,i)\
s(z,p,p)\
j(n,i,i)\
r(C,f,f)\
j(k,o,x)\
l(n,e,n)\
u(q,c,e)\
s(z,e,e)\
u(q,d,d)\
B(A,p,d,p)\
s(z,c,d)\
r(C,c,c)\
r(o,p,p)\
u(q,d,d)\
B(A,e,d,e)\
j(p,f,p)\
u(q,i,f)\
s(z,i,d)\
r(o,e,e)\
s(z,f,f)\
r(C,i,i)\
u(q,d,d)\
j(e,c,e)\
B(A,f,d,f)\
l(y,x,d)\
l(p,x,x)\
l(e,g,c)\
r(o,f,f)\
j(p,d,p)\
l(o,k,d)\
l(y,g,g)\
l(v,k,k)\
j(v,d,v)\
u(q,c,d)\
j(e,g,e)\
s(E,d,d)\
s(E,c,g)\
r(H,c,c)\
j(f,i,f)\
u(q,e,i)\
s(F,i,i)\
u(q,g,g)\
B(A,d,g,d)\
s(F,e,g)\
r(I,e,e)\
r(o,d,d)\
u(q,g,g)\
B(A,i,g,i)\
j(d,c,d)\
j(y,x,c)\
j(y,p,g)\
r(o,i,i)\
G(c,x,c)\
G(g,p,g)\
m(N,p)\
l(y,k,x)\
l(n,k,k)\
j(n,x,n)\
m(Q,x)\
m(k,(%[a]))\
m(O,k)\
j(i,e,i)\
m(n,(%[a],%[h]))\
m(M,n)\
l(y,v,e)\
l(f,v,v)\
j(f,e,f)\
m(v,(%[a],%[h],2))\
l(y,c,v)\
l(y,g,e)\
l(d,c,c)\
j(d,v,d)\
l(i,g,g)\
j(i,e,i)\
m(f,(%[a],%[h3]))\
m(c,(%[b]))\
m(d,K)\
j(p,o,d)\
m(g,L)\
j(x,o,g)\
m(i,J)\
u(q,g,i)\
s(z,g,c)\
j(k,o,f)\
l(o,k,e)\
s(z,i,i)\
j(p,e,e)\
r(C,g,g)\
j(n,o,v)\
l(p,k,p)\
u(q,d,k)\
s(z,k,k)\
u(q,c,c)\
B(A,i,c,i)\
s(z,d,c)\
r(C,d,d)\
r(o,i,i)\
u(q,c,c)\
B(A,k,c,k)\
j(i,g,i)\
u(q,e,g)\
s(z,e,c)\
r(o,k,k)\
s(z,g,g)\
r(C,e,e)\
u(q,c,c)\
j(k,d,k)\
B(A,g,c,g)\
l(y,v,c)\
l(i,v,v)\
l(k,f,d)\
r(o,g,g)\
j(i,c,i)\
l(o,n,c)\
l(y,f,f)\
l(x,n,n)\
j(x,c,x)\
u(q,d,c)\
j(k,f,k)\
s(E,c,c)\
s(E,d,f)\
r(H,d,d)\
j(g,e,g)\
u(q,k,e)\
s(F,e,e)\
u(q,f,f)\
B(A,c,f,c)\
s(F,k,f)\
r(I,k,k)\
r(o,c,c)\
u(q,f,f)\
B(A,e,f,e)\
j(c,d,c)\
j(y,v,d)\
j(y,i,f)\
r(o,e,e)\
G(d,v,d)\
G(f,i,f)\
l(y,n,i)\
l(y,x,v)\
l(p,n,n)\
j(p,i,p)\
l(g,x,x)\
l(y,d,i)\
j(g,v,g)\
l(c,d,d)\
m(n,32(%[a]))\
j(e,k,e)\
l(y,f,k)\
j(c,i,c)\
m(p,32(%[a],%[h]))\
l(e,f,f)\
j(e,k,e)\
m(x,32(%[a],%[h],2))\
m(g,32(%[a],%[h3]))\
m(d,M)\
m(c,O)\
m(f,Q)\
m(e,N)\
R($64,%[a])\
R($64,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm4,%%ymm7,%%ymm2,%%ymm5,%%ymm8,%%ymm9,CP_FPS_ASM_VPSUBD,%%ymm3,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VMOVDQA,%%ymm0,%%ymm15,%%ymm6,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,%%ymm1,%%ymm10,%%ymm14,%%ymm11,$170,CP_FPS_ASM_VPBLENDD,(%[w]),CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,CP_FPS_ASM_VPMINUD,32(%[w]),64(%[w]),(%[b],%[h3]),(%[b],%[h]),(%[b],%[h],2),32(%[b]),32(%[b],%[h3]),32(%[b],%[h]),32(%[b],%[h],2),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [t] "r"(table), [w] "r"(w), [P] "m"(kP),
		  [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}

namespace blocked_poly {

inline constexpr std::uint32_t kModulus = blocked_ntt::kModulus;

enum class Half { kBoth, kLower, kUpper };

class Arena {
public:
	explicit Arena(std::size_t words)
		: storage_(allocate_fps_words(words)), next_(storage_.get()), end_(next_ + words) {}
	Arena(const Arena &) = delete;
	Arena &operator=(const Arena &) = delete;
	static constexpr std::size_t footprint(std::size_t n) { return (n + 7) / 8 * 8 + 16; }
	std::span<std::uint32_t> take(std::size_t n) {
		if (std::size_t(end_ - next_) < footprint(n)) throw std::bad_alloc();
		auto *start = next_;
		next_ += footprint(n);
		return {start, n};
	}

	bool zero_initialized() const { return storage_.get_deleter().region != nullptr; }

private:
	FpsWords storage_;
	std::uint32_t *next_, *end_;
};

namespace detail {

using blocked_ntt::detail::add;
using blocked_ntt::detail::broadcast;
using blocked_ntt::detail::diff;
using blocked_ntt::detail::Factor;
using blocked_ntt::detail::kP;
using blocked_ntt::detail::kR;
using blocked_ntt::detail::reduce;
using blocked_ntt::detail::slot;
using blocked_ntt::detail::Vec;

inline constexpr auto kInverseScales = [] {
	std::array<std::array<std::uint32_t, blocked_ntt::kMaxLog + 1>, 2> s{};
	for (int lg = 3; lg <= blocked_ntt::kMaxLog; ++lg) {
		s[0][lg] = blocked_ntt::detail::power(std::uint32_t(1) << (lg - 3), kP - 2);
		s[1][lg] = blocked_ntt::detail::multiply_mod(s[0][lg], kR);
	}
	return s;
}();

inline Vec load(const std::uint32_t* p) { return _mm256_load_si256(reinterpret_cast<const Vec*>(p)); }
inline void store(std::uint32_t* p, Vec x) { _mm256_store_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec low(Vec x) { return reduce(x, 2 * kP); }
inline Vec canonical(Vec x) { return reduce(reduce(x, 2 * kP), kP); }

inline Vec low_difference(Vec x, Vec y) {
	const Vec t = _mm256_sub_epi32(x, y);
	return _mm256_min_epu32(t, _mm256_add_epi32(t, broadcast(2 * kP)));
}

inline Vec difference(Vec x, Vec y) { return reduce(_mm256_sub_epi32(add(x, broadcast(kP)), y), kP); }

inline Vec times(Vec x, const Factor& w) { return blocked_ntt::detail::multiply(x, w); }

inline Factor entry(const std::uint32_t* table, std::size_t k) {
	const std::uint32_t* e = table + slot(k);
	return Factor(e[0], e[8]);
}

inline Factor leaf_weight(const std::uint32_t* roots, std::size_t p) {
	const std::uint32_t* e = roots + slot(p >> 1);
	return p & 1 ? Factor(kP - e[0], ~e[8]) : Factor(e[0], e[8]);
}

struct Group {
	Factor x, y, z;
	[[gnu::always_inline]] Group(const std::uint32_t* table, std::size_t g)
		: x(entry(table, g)), y(entry(table, 2 * g)), z(entry(table, 2 * g + 1)) {}
};

[[gnu::always_inline]] inline void forward_h1(Vec (&f)[4], const Group& w) {
	const Vec a = low(f[0]), b = low(f[1]), c = times(f[2], w.x), d = times(f[3], w.x);
	const Vec ac = low(add(a, c)), amc = low_difference(a, c);
	const Vec bd = times(add(b, d), w.y), bmd = times(diff(b, d), w.z);
	f[0] = add(ac, bd), f[1] = diff(ac, bd), f[2] = add(amc, bmd), f[3] = diff(amc, bmd);
}

[[gnu::always_inline]] inline void inverse_h1(Vec (&f)[4], const Group& w) {
	const Vec ab = low(add(f[0], f[1])), cd = low(add(f[2], f[3]));
	const Vec amb = times(diff(f[0], f[1]), w.y), cmd = times(diff(f[2], f[3]), w.z);
	f[0] = low(add(ab, cd)), f[1] = low(add(amb, cmd));
	f[2] = times(diff(ab, cd), w.x), f[3] = times(diff(amb, cmd), w.x);
}

struct alignas(64) Window {
	std::uint32_t word[17];
};

inline void fill_window(Window& window, Vec a, const Factor& w) {
	window.word[16] = 0;
	store(window.word + 8, a);
	store(window.word, reduce(times(a, w), kP));
}

[[gnu::always_inline]] inline void fill_windows(Window (&window)[4], const Vec (&a)[4], const std::uint32_t* roots,
												std::size_t g) {
	const Factor y = entry(roots, 2 * g), z = entry(roots, 2 * g + 1);
#pragma GCC unroll 4
	for (int t = 0; t < 4; ++t) {
		window[t].word[16] = 0;
		store(window[t].word + 8, a[t]);
		const Vec wa = times(a[t], t < 2 ? y : z);
		store(window[t].word, reduce(t % 2 ? _mm256_sub_epi32(broadcast(2 * kP), wa) : wa, kP));
	}
}

[[gnu::always_inline]] inline Vec leaf_product(const Window& window, const std::uint32_t* b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
	Vec even, odd, bi, t0, t1, t2, t3;
	#define CP_FPS_ASM_BLOCK(a,c,d,e,f,g,h,i,j,k,l,m,n,o,q,r,s,t,u,v) \
i((%[b]),a) d(36(%[w]),a,e) d(r,a,c) i(4(%[b]),a)\
d(r,a,g)\
d(q,a,h)\
i(8(%[b]),a)\
d(q,a,j)\
d(o,a,k)\
f(g,e,e)\
f(h,c,c)\
i(12(%[b]),a)\
d(o,a,g)\
d(n,a,h)\
f(j,e,e)\
f(k,c,c)\
i(16(%[b]),a)\
d(n,a,j)\
d(m,a,k)\
f(g,e,e)\
f(h,c,c)\
i(20(%[b]),a)\
d(m,a,g)\
d(l,a,h)\
f(j,e,e)\
f(k,c,c)\
i(24(%[b]),a)\
d(l,a,j)\
d(s,a,k)\
f(g,e,e)\
f(h,c,c)\
i(28(%[b]),a)\
d(s,a,g)\
d(4(%[w]),a,h)\
f(j,e,e)\
f(k,c,c)\
f(g,e,e)\
f(h,c,c)\
d(%[ni],c,g)\
d(%[ni],e,h)\
d(%[p],g,g)\
d(%[p],h,h)\
f(g,c,c)\
f(h,e,e)\
u($32,c,c)\
t($0xaa,e,c,c)\
v(%[p2],c,g)\
"vpminud %[t0], %[even], %[even]"
	asm(
		CP_FPS_ASM_BLOCK(%[bi],%[even],CP_FPS_ASM_VPMULUDQ,%[odd],CP_FPS_ASM_VPADDQ,%[t0],%[t1],CP_FPS_ASM_VPBROADCASTD,%[t2],%[t3],12(%[w]),16(%[w]),20(%[w]),24(%[w]),28(%[w]),32(%[w]),8(%[w]),CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VPSUBD)
#undef CP_FPS_ASM_BLOCK
		: [even] "=&x"(even), [odd] "=&x"(odd), [bi] "=&x"(bi), [t0] "=&x"(t0), [t1] "=&x"(t1), [t2] "=&x"(t2),
		[t3] "=&x"(t3)
		: [w] "r"(window.word), [b] "r"(b), [ni] "x"(ni), [p] "x"(p), [p2] "x"(p2),
		"m"(*reinterpret_cast<const std::uint32_t(*)[17]>(window.word)),
		"m"(*reinterpret_cast<const std::uint32_t(*)[8]>(b)));
	return even;
}

struct ForwardBottom {
	static constexpr bool kForward = true, kInverse = false;
	const std::uint32_t* roots;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		if (count % 8 == 0)
			return kernels::forward_bottom(reinterpret_cast<Vec*>(a), count, roots + slot(first), roots + slot(2 * first));
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			const Group w(roots, first + j);
			Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
			forward_h1(f, w);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t) store(a + 8 * t, canonical(f[t]));
		}
	}
};

struct InverseBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* in;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const std::uint32_t* from = in + 32 * first;
		if (count % 8 == 0)
			return kernels::inverse_bottom(reinterpret_cast<Vec*>(a), count, inverse_roots + slot(first),
										inverse_roots + slot(2 * first), reinterpret_cast<const Vec*>(from));
		for (std::size_t j = 0; j < count; ++j, a += 32, from += 32) {
			const Group w(inverse_roots, first + j);
			Vec f[4] = {load(from), load(from + 8), load(from + 16), load(from + 24)};
			inverse_h1(f, w);
			for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
		}
	}
};

struct ProductBottom {
	static constexpr bool kForward = true, kInverse = true;
	const std::uint32_t* roots;
	const std::uint32_t* inverse_roots;
	const std::uint32_t* b;

	[[gnu::always_inline]] void prepare(const std::uint32_t* a, std::size_t g, Window (&window)[4]) const {
		Vec f[4] = {load(a), load(a + 8), load(a + 16), load(a + 24)};
		forward_h1(f, Group(roots, g));
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = canonical(f[t]);
		fill_windows(window, f, roots, g);
	}

	[[gnu::always_inline]] void finish(std::uint32_t* a, std::size_t g, const Window (&window)[4]) const {
		Vec f[4];
#pragma GCC unroll 4
		for (int t = 0; t < 4; ++t) f[t] = leaf_product(window[t], b + 8 * (4 * g + t));
		inverse_h1(f, Group(inverse_roots, g));
		for (int t = 0; t < 4; ++t) store(a + 8 * t, f[t]);
	}

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			finish(a, first + j, window[j & 1]);
		}
	}
};

struct InverseProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	ProductBottom product;
	const std::uint32_t* a;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[4]) const {
		const std::uint32_t* leaves = a + 32 * g;
		const Vec f[4] = {load(leaves), load(leaves + 8), load(leaves + 16), load(leaves + 24)};
		fill_windows(window, f, product.roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			product.finish(out, first + j, window[j & 1]);
		}
	}
};

template <std::size_t K>
struct InverseProductSumBottom {
	static constexpr bool kForward = false, kInverse = true;
	std::array<InverseProductBottom, K> terms;

	void prepare(std::size_t g, Window (&window)[K][4]) const {
		for (std::size_t k = 0; k < K; ++k) terms[k].prepare(g, window[k]);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][K][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				f[t] = leaf_product(window[j & 1][0][t], terms[0].product.b + 8 * (4 * g + t));
				for (std::size_t k = 1; k < K; ++k)
					f[t] = low(add(f[t], leaf_product(window[j & 1][k][t], terms[k].product.b + 8 * (4 * g + t))));
			}
			inverse_h1(f, Group(terms[0].product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct ForwardProductBottom {
	static constexpr bool kForward = true, kInverse = false;
	ProductBottom product;

	void operator()(std::uint32_t* a, std::size_t count, std::size_t first) const {
		const Factor undo_montgomery(kR);
		Window window[2][4];
		product.prepare(a, first, window[0]);
		for (std::size_t j = 0; j < count; ++j, a += 32) {
			if (j + 1 < count) product.prepare(a + 32, first + j + 1, window[(j + 1) & 1]);
			const std::uint32_t* b = product.b + 32 * (first + j);
#pragma GCC unroll 4
			for (int t = 0; t < 4; ++t)
				store(a + 8 * t, reduce(times(leaf_product(window[j & 1][t], b + 8 * t), undo_montgomery), kP));
		}
	}
};

template <class Bottom>
class Recursion {
public:
	Recursion(const std::uint32_t* roots, const std::uint32_t* inverse_roots, const Bottom& bottom)
		: r_(roots), ir_(inverse_roots), bottom_(bottom) {}

	void visit(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if (nv <= kTile) return tile(a, nv, k);
		const std::size_t h = nv / 4;
		if constexpr (Bottom::kForward) forward(a, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

	static constexpr bool radix8(std::size_t nv) { return nv / 2 > kTile; }

	void below(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		const std::size_t h = nv / 4;
		for (std::size_t t = 0; t < 4; ++t) visit(a + 8 * t * h, h, 4 * k + t);
		if constexpr (Bottom::kInverse) inverse(a, h, k);
	}

private:
	static constexpr std::size_t kTile = 256;

	void tile(std::uint32_t* a, std::size_t nv, std::size_t k) const {
		if constexpr (Bottom::kForward) {
			for (std::size_t h = nv / 4; h >= 16; h /= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) forward(a + 8 * j, h, g);
			if (nv >= 16) kernels::forward_h4(vectors(a), nv / 16, r_, k * (nv / 16));
		}
		bottom_(a, nv / 4, k * (nv / 4));
		if constexpr (Bottom::kInverse) {
			if (nv >= 16) kernels::inverse_h4(vectors(a), nv / 16, ir_, k * (nv / 16));
			for (std::size_t h = 16; h < nv; h *= 4)
				for (std::size_t j = 0, g = k * (nv / (4 * h)); j < nv; j += 4 * h, ++g) inverse(a + 8 * j, h, g);
		}
	}

	static Vec* vectors(std::uint32_t* a) { return reinterpret_cast<Vec*>(a); }

	void forward(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::forward_identity(vectors(a), h, r_);
		blocked_ntt::kernels::forward(vectors(a), h, r_ + slot(k), r_ + slot(2 * k));
	}

	void inverse(std::uint32_t* a, std::size_t h, std::size_t k) const {
		if (k == 0) return blocked_ntt::kernels::inverse_identity(vectors(a), h, ir_);
		blocked_ntt::kernels::inverse(vectors(a), h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	const std::uint32_t *r_, *ir_;
	Bottom bottom_;
};

class Source {
public:
	Source(const std::uint32_t* in, std::size_t size, std::size_t shift)
		: in_(in), shift_(shift), end_(shift + size), first_((shift + 7) / 8), first_touched_(shift / 8) {
		count_ = end_ / 8 > first_ ? end_ / 8 - first_ : 0;
		touched_ = (end_ + 7) / 8 - first_touched_;
	}

	Vec operator()(std::size_t v) const {
		if (v - first_ < count_) return _mm256_loadu_si256(reinterpret_cast<const Vec*>(in_ + (8 * v - shift_)));
		if (v - first_touched_ >= touched_) return _mm256_setzero_si256();
		return edge(in_, shift_, end_, v);
	}

	bool in_place(const std::uint32_t* out) const { return in_ == out + shift_; }

	void zero_outside(std::uint32_t* out, std::size_t lo, std::size_t hi) const {
		const std::size_t begin = std::clamp(shift_, lo, hi), end = std::clamp(end_, begin, hi);
		std::fill(out + lo, out + begin, 0);
		std::fill(out + end, out + hi, 0);
	}


	bool within(std::size_t lo, std::size_t hi) const {
		return touched_ == 0 || (first_touched_ >= lo && first_touched_ + touched_ <= hi);
	}

private:

	[[gnu::noinline]] static Vec edge(const std::uint32_t* in, std::size_t shift, std::size_t end, std::size_t v) {
		alignas(32) std::uint32_t x[8] = {};
		for (std::size_t i = 8 * v; i < 8 * v + 8; ++i)
			if (i >= shift && i < end) x[i - 8 * v] = in[i - shift];
		return load(x);
	}

	const std::uint32_t* in_;
	std::size_t shift_, end_;
	std::size_t first_, count_;
	std::size_t first_touched_, touched_;
};

template <class Input>
[[gnu::always_inline]] inline void forward_top4_body(Input in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	const Factor z(roots[1], roots[9]);
	const Vec p = broadcast(kP);
	if (in.within(0, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = in(j), b = in(j + h), zb = times(b, z);
			out[j] = add(a, b), out[j + h] = diff(a, b), out[j + 2 * h] = add(a, zb), out[j + 3 * h] = diff(a, zb);
		}
		return;
	}
	if (in.within(2 * h, 4 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec c = in(j + 2 * h), d = in(j + 3 * h);
			const Vec mc = _mm256_sub_epi32(p, c), zmd = times(_mm256_sub_epi32(p, d), z);
			out[j] = add(c, d), out[j + h] = diff(c, d), out[j + 2 * h] = add(mc, zmd), out[j + 3 * h] = diff(mc, zmd);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec a = in(j), b = in(j + h), c = in(j + 2 * h), d = in(j + 3 * h);
		const Vec ac = add(a, c), amc = _mm256_sub_epi32(add(a, p), c);
		const Vec bd = add(b, d), zbmd = times(_mm256_sub_epi32(add(b, p), d), z);
		out[j] = add(ac, bd), out[j + h] = diff(ac, bd);
		out[j + 2 * h] = add(amc, zbmd), out[j + 3 * h] = diff(amc, zbmd);
	}
}

template <class Input>
[[gnu::always_inline]] inline void forward_top2_body(Input in, Vec* out, std::size_t h) {
	const Vec p = broadcast(kP);
	if (in.within(0, h)) {
		for (std::size_t j = 0; j < h; ++j) out[j] = out[j + h] = in(j);
		return;
	}
	if (in.within(h, 2 * h)) {
		for (std::size_t j = 0; j < h; ++j) {
			const Vec v = in(j + h);
			out[j] = v, out[j + h] = _mm256_sub_epi32(p, v);
		}
		return;
	}
	for (std::size_t j = 0; j < h; ++j) {
		const Vec u = in(j), v = in(j + h);
		out[j] = add(u, v), out[j + h] = _mm256_sub_epi32(add(u, p), v);
	}
}

[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}

inline bool forward_top8(const Source& in, std::uint32_t* a, std::size_t nv, const std::uint32_t* roots) {
	const bool lower = in.within(0, nv / 2);
	if (!in.in_place(a) || !(lower || in.within(nv / 2, nv))) return false;
	const std::size_t half = 4 * nv;
	in.zero_outside(a, lower ? 0 : half, lower ? half : 2 * half);
	auto* v = reinterpret_cast<Vec*>(a);
	if (lower) kernels::forward_top8_lower(v, nv / 8, roots);
	else kernels::forward_top8_upper(v, nv / 8, roots);
	return true;
}

template <class Bottom>
void radix2_halves(const Recursion<Bottom>& recursion, const Source& in, std::uint32_t* a, std::size_t nv,
				   const std::uint32_t* roots) {
	const std::size_t h = nv / 2;
	if constexpr (Bottom::kForward) {
		if (recursion.radix8(nv) && forward_top8(in, a, nv, roots)) {
			recursion.below(a, h, 0);
			recursion.below(a + 8 * h, h, 1);
			return;
		}
		forward_top2(in, reinterpret_cast<Vec*>(a), h);
	}
	recursion.visit(a, h, 0);
	recursion.visit(a + 8 * h, h, 1);
}

inline void inverse_top4(Vec* f, std::size_t h, Half output, const std::uint32_t* inverse_roots, const Factor& s) {
	if (output == Half::kBoth) return blocked_ntt::detail::inverse_radix4(f, h, inverse_roots, s);
	const Factor z(inverse_roots[1], inverse_roots[9]);
	const auto scale = [&s](Vec x) __attribute__((target("avx2"))) { return reduce(times(x, s), kP); };
	for (std::size_t j = 0; j < h; ++j) {
		const Vec p0 = f[j], p1 = f[j + h], p2 = f[j + 2 * h], p3 = f[j + 3 * h];
		const Vec ab = low(add(p0, p1)), cd = low(add(p2, p3));
		const Vec amb = low(diff(p0, p1)), cmd = times(diff(p2, p3), z);
		if (output == Half::kLower) {
			f[j] = scale(add(ab, cd)), f[j + h] = scale(add(amb, cmd));
		} else {
			f[j + 2 * h] = scale(diff(ab, cd)), f[j + 3 * h] = scale(diff(amb, cmd));
		}
	}
}

inline void inverse_top2(Vec* f, std::size_t h, Half output, std::uint32_t scale) {
	if (output == Half::kBoth) {
		alignas(32) std::uint32_t s[16] = {};
		s[1] = scale;
		s[9] = blocked_ntt::detail::quotient(scale);
		return blocked_ntt::kernels::scale_radix2(f, h, s);
	}
	const Factor s(scale);
	for (std::size_t j = 0; j < h; ++j) {
		if (output == Half::kLower) f[j] = reduce(times(add(f[j], f[j + h]), s), kP);
		else f[j + h] = reduce(times(diff(f[j], f[j + h]), s), kP);
	}
}

}

class Transform {
public:
	static constexpr int kMinLog = 6, kMaxLog = blocked_ntt::kMaxLog;

	static constexpr std::size_t words(int lg_max) { return Arena::footprint(2 * blocked_ntt::detail::table_words(lg_max)); }

	Transform(Arena& arena, int lg_max) : lg_max_(lg_max) {
		if (lg_max < kMinLog || lg_max > kMaxLog) std::abort();
		const std::size_t table = blocked_ntt::detail::table_words(lg_max), entries = (std::size_t(1) << lg_max) / 16;
		roots_ = arena.take(2 * table).data();
		std::fill_n(roots_, 2 * table, 0u);
		inverse_roots_ = roots_ + table;
		blocked_ntt::detail::build_table(roots_, entries, blocked_ntt::detail::kRoots[0]);
		blocked_ntt::detail::build_table(inverse_roots_, entries, blocked_ntt::detail::kRoots[1]);
	}

	int lg_max() const { return lg_max_; }

	const std::uint32_t* roots() const { return roots_; }
	const std::uint32_t* inverse_roots() const { return inverse_roots_; }

	void forward(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		run(out, source(in, shift, out.size()), detail::ForwardBottom{roots_}, 0, Half::kBoth);
	}

	void forward(std::span<std::uint32_t> a) const { forward(a, 0, a); }

	void forward_upper(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out) const {
		using namespace detail;
		check_length(2 * out.size());
		const Source src = source(in, shift, out.size());
		const detail::Recursion recursion(roots_, inverse_roots_, ForwardBottom{roots_});
		const std::size_t nv = out.size() / 8;
		auto* v = reinterpret_cast<Vec*>(out.data());
		if (std::countr_zero(nv) % 2 == 0) {
			for (std::size_t j = 0; j < nv; ++j) v[j] = src(j);
			return recursion.visit(out.data(), nv, 1);
		}

		const std::size_t h = nv / 2;
		const Factor z(roots_[1], roots_[9]);
		for (std::size_t j = 0; j < h; ++j) {
			const Vec a = src(j), zb = times(src(j + h), z);
			v[j] = add(a, zb), v[j + h] = diff(a, zb);
		}
		recursion.visit(out.data(), h, 2);
		recursion.visit(out.data() + 8 * h, h, 3);
	}

	void inverse(std::span<const std::uint32_t> in, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), false);
		run(out, detail::Source(nullptr, 0, 0), detail::InverseBottom{inverse_roots_, in.data()}, scale, output);
	}

	void inverse(std::span<std::uint32_t> a, Half output = Half::kBoth) const { inverse(a, a, output); }

	void cyclic_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b, Half output = Half::kBoth, std::uint32_t c = 1) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = multiply_mod(inverse_scale(out.size(), true), c);
		run(out, source(in, shift, out.size()), detail::ProductBottom{roots_, inverse_roots_, b.data()}, scale, output);
	}

	void cyclic_product(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const { cyclic_product(a, 0, a, b); }

	void inverse_product(std::span<const std::uint32_t> a, std::span<const std::uint32_t> b, std::span<std::uint32_t> out,
						Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, detail::Source(nullptr, 0, 0), detail::InverseProductBottom{product, a.data()}, scale, output);
	}

	struct Pair {
		std::span<const std::uint32_t> a, b;
	};

	void inverse_product_sum(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		switch (pairs.size()) {
			case 1: return inverse_product(pairs[0].a, pairs[0].b, out, output);
			case 2: return inverse_products<2>(pairs, out, output);
			case 3: return inverse_products<3>(pairs, out, output);
			default: std::abort();
		}
	}

	template <class Bottom>
	void inverse_with(const Bottom& bottom, std::span<std::uint32_t> out, Half output = Half::kBoth) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	void forward_product(std::span<const std::uint32_t> in, std::size_t shift, std::span<std::uint32_t> out,
						std::span<const std::uint32_t> b) const {
		const detail::ProductBottom product{roots_, inverse_roots_, b.data()};
		run(out, source(in, shift, out.size()), detail::ForwardProductBottom{product}, 0, Half::kBoth);
	}

	void multiply(std::span<std::uint32_t> a, std::span<const std::uint32_t> b) const {
		products(a.data(), a.data(), b.data(), a.size(), [&](detail::Vec, detail::Vec p) __attribute__((target("avx2"))) { return p; });
	}

	void multiply_add(std::span<std::uint32_t> c, std::span<const std::uint32_t> a,
					std::span<const std::uint32_t> b) const {
		products(c.data(), a.data(), b.data(), a.size(), [&](detail::Vec c, detail::Vec p) __attribute__((target("avx2"))) { return detail::add(c, p); });
	}

private:
	std::uint32_t inverse_scale(std::size_t n, bool montgomery) const {
		check_length(n);
		return detail::kInverseScales[montgomery][std::countr_zero(n)];
	}

	void check_length(std::size_t n) const {
		const int lg = std::countr_zero(n);
		if (!std::has_single_bit(n) || lg < kMinLog || lg > lg_max_) std::abort();
	}

	template <std::size_t K>
	void inverse_products(std::span<const Pair> pairs, std::span<std::uint32_t> out, Half output) const {
		using namespace blocked_ntt::detail;
		const std::uint32_t scale = inverse_scale(out.size(), true);
		detail::InverseProductSumBottom<K> bottom{};
		for (std::size_t k = 0; k < K; ++k) bottom.terms[k] = {{roots_, inverse_roots_, pairs[k].b.data()}, pairs[k].a.data()};
		run(out, detail::Source(nullptr, 0, 0), bottom, scale, output);
	}

	static detail::Source source(std::span<const std::uint32_t> in, std::size_t shift, std::size_t n) {
		if (shift > n || in.size() > n - shift) std::abort();
		return detail::Source(in.data(), in.size(), shift);
	}

	template <class Bottom>
	void run(std::span<std::uint32_t> a, const detail::Source& in, const Bottom& bottom, std::uint32_t scale,
			Half output) const {
		using namespace detail;
		check_length(a.size());
		const detail::Recursion recursion(roots_, inverse_roots_, bottom);
		const std::size_t nv = a.size() / 8;
		auto* v = reinterpret_cast<Vec*>(a.data());
		if (std::countr_zero(nv) % 2 == 0) {
			const std::size_t h = nv / 4;
			if constexpr (Bottom::kForward) forward_top4(in, v, h, roots_);
			for (std::size_t t = 0; t < 4; ++t) recursion.visit(a.data() + 8 * t * h, h, t);
			if constexpr (Bottom::kInverse) inverse_top4(v, h, output, inverse_roots_, Factor(scale));
		} else {
			const std::size_t h = nv / 2;
			radix2_halves(recursion, in, a.data(), nv, roots_);
			if constexpr (Bottom::kInverse) inverse_top2(v, h, output, scale);
		}
	}

	template <class Combine>
	void products(std::uint32_t* out, const std::uint32_t* a, const std::uint32_t* b, std::size_t n, Combine combine) const {
		using namespace detail;
		check_length(n);
		const Factor undo_montgomery(kR);
		Window window[2];
		fill_window(window[0], load(a), leaf_weight(roots_, 0));
		for (std::size_t p = 0; p < n / 8; ++p) {
			if (p + 1 < n / 8) fill_window(window[(p + 1) & 1], load(a + 8 * p + 8), leaf_weight(roots_, p + 1));
			const Vec product = reduce(times(leaf_product(window[p & 1], b + 8 * p), undo_montgomery), kP);
			store(out + 8 * p, canonical(combine(load(out + 8 * p), product)));
		}
	}

	int lg_max_;
	std::uint32_t *roots_, *inverse_roots_;
};

}

namespace blocked_poly {

namespace detail {

inline Vec load_unaligned(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline void store_unaligned(std::uint32_t* p, Vec x) { _mm256_storeu_si256(reinterpret_cast<Vec*>(p), x); }

inline Vec montgomery(Vec a, Vec b) {
	const Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP);
	Vec even = _mm256_mul_epu32(a, b);
	Vec odd = _mm256_mul_epu32(_mm256_srli_epi64(a, 32), _mm256_srli_epi64(b, 32));
	even = _mm256_add_epi64(even, _mm256_mul_epu32(_mm256_mul_epu32(even, ni), p));
	odd = _mm256_add_epi64(odd, _mm256_mul_epu32(_mm256_mul_epu32(odd, ni), p));
	return _mm256_blend_epi32(_mm256_srli_epi64(even, 32), odd, 0xAA);
}

class Indices {
public:
	explicit Indices(std::size_t k) {
		alignas(32) std::uint32_t x[8];
		for (std::uint32_t i = 0; i < 8; ++i) x[i] = std::uint32_t((std::uint64_t(k + i) << 32) % kP);
		value_ = load(x);
	}

	Vec value() const { return value_; }
	void next() { value_ = reduce(add(value_, broadcast(kStep)), kP); }

private:
	static constexpr std::uint32_t kStep = std::uint32_t((std::uint64_t(8) << 32) % kP);
	Vec value_;
};

inline std::uint32_t scalar_inverse(std::uint32_t x) { return blocked_ntt::detail::power(x, kP - 2); }

template <class A>
void divide_by_index(std::size_t first, std::span<std::uint32_t> out, const A& a, std::uint32_t factor = 1) {
	constexpr std::size_t kLanes = 32;
	const std::size_t n = out.size();
	if (n == 0) return;
	const std::size_t blocks = (n + kLanes - 1) / kLanes, last = kLanes * (blocks - 1);
	const Vec one = broadcast(1), step = broadcast(kLanes);

	Vec x[4];
	for (int v = 0; v < 4; ++v)
		x[v] = _mm256_add_epi32(broadcast(std::uint32_t(first + 8 * v)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
	const auto valid = [n](std::size_t at) __attribute__((target("avx2"))) {
		const Vec lanes = _mm256_add_epi32(broadcast(std::uint32_t(at)), _mm256_setr_epi32(0, 1, 2, 3, 4, 5, 6, 7));
		return _mm256_cmpgt_epi32(broadcast(std::uint32_t(n)), lanes);
	};
	const auto masked = [&](Vec value, std::size_t at) __attribute__((target("avx2"))) { return at + 8 <= n ? value : _mm256_blendv_epi8(one, value, valid(at)); };

	Vec prefix[4];
	for (int v = 0; v < 4; ++v) prefix[v] = masked(x[v], 8 * v);
	for (std::size_t b = kLanes; b < n; b += kLanes) {
		for (int v = 0; v < 4; ++v) {
			store_unaligned(out.data() + b - kLanes + 8 * v, prefix[v]);
			x[v] = _mm256_add_epi32(x[v], step);
			prefix[v] = montgomery(prefix[v], masked(x[v], b + 8 * v));
		}
	}

	alignas(32) std::uint32_t total[kLanes], inverse[kLanes];
	for (int v = 0; v < 4; ++v) store(total + 8 * v, canonical(prefix[v]));
	std::uint32_t running = 1;
	for (std::size_t l = 0; l < kLanes; ++l) {
		inverse[l] = running;
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}
	running = blocked_ntt::detail::multiply_mod(scalar_inverse(running), blocked_ntt::detail::multiply_mod(kR, factor));
	for (std::size_t l = kLanes; l-- > 0;) {
		inverse[l] = blocked_ntt::detail::multiply_mod(inverse[l], running);
		running = blocked_ntt::detail::multiply_mod(running, total[l]);
	}

	Vec inv[4];
	for (int v = 0; v < 4; ++v) inv[v] = load(inverse + 8 * v);
	const auto put = [&](std::size_t at, Vec reciprocal) __attribute__((target("avx2"))) {
		const Vec q = canonical(montgomery(reciprocal, a(at)));
		if (at + 8 <= n) return store_unaligned(out.data() + at, q);
		_mm256_maskstore_epi32(reinterpret_cast<int*>(out.data() + at), valid(at), q);
	};
	for (std::size_t b = last; b > 0; b -= kLanes) {
		for (int v = 0; v < 4; ++v) {
			const std::size_t at = b + 8 * v;
			if (at < n) put(at, montgomery(load_unaligned(out.data() + at - kLanes), inv[v]));
			inv[v] = montgomery(inv[v], masked(x[v], at));
			x[v] = _mm256_sub_epi32(x[v], step);
		}
	}
	for (int v = 0; v < 4; ++v)
		if (8 * std::size_t(v) < n) put(8 * v, inv[v]);
}

}

inline void derivative(std::span<const std::uint32_t> f, std::span<std::uint32_t> d) {
	using namespace detail;
	const std::size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(load_unaligned(f.data() + i + 1), index.value())));
	for (std::size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(std::uint32_t(i + 1), f[i + 1]) : 0;
}

inline void divide_by_index(std::span<const std::uint32_t> a, std::size_t first, std::span<std::uint32_t> q) {
	if (a.size() != q.size() || first == 0 || first + a.size() > kModulus) std::abort();
	const std::size_t n = a.size();
	detail::divide_by_index(first, q, [a, n](std::size_t at) __attribute__((target("avx2"))) {
		if (at + 8 <= n) return detail::load_unaligned(a.data() + at);
		alignas(32) std::uint32_t rest[8] = {};
		std::copy(a.begin() + at, a.end(), rest);
		return detail::load(rest);
	});
}

}

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <span>

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kInverseBase = 32;

inline void inverse_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	const std::uint32_t inv0 = blocked_ntt::detail::power(f[0], kP - 2);
	g[0] = inv0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = 0;
		for (std::size_t j = 1; j <= std::min(i, f.size() - 1); ++j) {
			sum += std::uint64_t(f[j]) * g[i - j] % kP;
		}
		g[i] = multiply_mod(std::uint32_t(sum % kP ? kP - sum % kP : 0), inv0);
	}
}

}

inline int inverse_log(std::size_t n) {
	return std::max(Transform::kMinLog, int(std::bit_width(std::max<std::size_t>(n, 2) - 1)));
}

inline std::size_t inverse_scratch(std::size_t n) { return 2 * Arena::footprint(std::size_t(1) << inverse_log(n)); }

inline void inverse_step(const Transform& t, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
						std::span<std::uint32_t> to, std::span<std::uint32_t> gt, std::span<std::uint32_t> work) {
	const std::size_t k = g.size();
	t.forward(g, 0, gt);
	t.cyclic_product(f.first(std::min(2 * k, f.size())), 0, work, gt, Half::kUpper);
	t.cyclic_product(work.subspan(k), k, work, gt, Half::kUpper, detail::kP - 1);
	if (to.data() != work.data() + k) std::copy_n(work.begin() + std::ptrdiff_t(k), to.size(), to.begin());
}

inline void inverse(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
					std::span<std::uint32_t> scratch) {
	const std::size_t n = g.size();
	std::size_t k = std::min(n, detail::kInverseBase);
	detail::inverse_direct(f, g.first(k));
	for (; k < n; k *= 2) {
		const std::size_t len = 2 * k;
		inverse_step(t, f, g.first(k), g.subspan(k, std::min(k, n - k)), scratch.first(len),
					scratch.subspan(Arena::footprint(len), len));
	}
}

}

namespace blocked_poly {

namespace detail {

inline constexpr std::size_t kLogBase = 64;

inline constexpr std::size_t kLogBlocks = 4;

inline void log_direct(std::span<const std::uint32_t> f, std::span<std::uint32_t> g) {
	using blocked_ntt::detail::multiply_mod;
	std::uint32_t a[kLogBase] = {}, inv[kLogBase] = {0, 1}, kg[kLogBase] = {};
	std::copy_n(f.begin(), std::min(f.size(), g.size()), a);
	for (std::size_t i = 2; i < g.size(); ++i) inv[i] = multiply_mod(kP - kP / std::uint32_t(i), inv[kP % i]);
	g[0] = 0;
	for (std::size_t i = 1; i < g.size(); ++i) {
		std::uint64_t sum = multiply_mod(std::uint32_t(i), a[i]);
		for (std::size_t k = 1; k < i; ++k) sum += kP - std::uint64_t(kg[k]) * a[i - k] % kP;
		kg[i] = std::uint32_t(sum % kP);
		g[i] = multiply_mod(kg[i], inv[i]);
	}
}

inline std::size_t log_block(std::size_t n) {
	return std::max<std::size_t>(32, std::bit_ceil((n - 1 + kLogBlocks - 1) / kLogBlocks));
}

inline std::size_t log_blocks(std::size_t n) { return (n - 1 + log_block(n) - 1) / log_block(n); }

inline void subtract_from_derivative(std::span<const std::uint32_t> f, std::size_t first, std::size_t count,
									std::uint32_t* out, std::size_t size) {
	const std::size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (std::size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(load_unaligned(f.data() + first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (std::size_t c = full; c < count; ++c) {
		const std::uint32_t d = c < inside ? blocked_ntt::detail::multiply_mod(std::uint32_t(first + 1 + c), f[first + 1 + c]) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct ToeplitzBottom {
	static constexpr bool kForward = false, kInverse = true;
	const std::uint32_t *roots, *inverse_roots, *w1, *w2, *q0, *q1;
	std::uint32_t* w3;

	[[gnu::always_inline]] void prepare(std::size_t g, Window (&window)[3][4]) const {
		Vec sum[4], d1[4], d3[4];
#pragma GCC unroll 4
		for (std::size_t t = 0; t < 4; ++t) {
			const std::size_t leaf = 8 * (4 * g + t);
			const Vec v2 = load(w2 + leaf);
			sum[t] = reduce(add(load(q0 + leaf), load(q1 + leaf)), kP);
			d1[t] = difference(load(w1 + leaf), v2);
			d3[t] = difference(load(w3 + leaf), v2);
		}
		fill_windows(window[0], sum, roots, g);
		fill_windows(window[1], d1, roots, g);
		fill_windows(window[2], d3, roots, g);
	}

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][3][4];
		prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			const auto& w = window[j & 1];
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				const Vec m1 = leaf_product(w[0][t], w2 + leaf);
				f[t] = low(add(m1, leaf_product(w[1][t], q1 + leaf)));
				store(w3 + leaf, low(add(m1, leaf_product(w[2][t], q0 + leaf))));
			}
			inverse_h1(f, Group(inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

struct SideProductBottom {
	static constexpr bool kForward = false, kInverse = true;
	InverseProductBottom term;
	const std::uint32_t* side;

	void operator()(std::uint32_t* out, std::size_t count, std::size_t first) const {
		Window window[2][4];
		term.prepare(first, window[0]);
		for (std::size_t j = 0; j < count; ++j, out += 32) {
			if (j + 1 < count) term.prepare(first + j + 1, window[(j + 1) & 1]);
			const std::size_t g = first + j;
			Vec f[4];
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) {
				const std::size_t leaf = 8 * (4 * g + t);
				f[t] = low(add(leaf_product(window[j & 1][t], term.product.b + leaf), load(side + leaf)));
			}
			inverse_h1(f, Group(term.product.inverse_roots, g));
#pragma GCC unroll 4
			for (std::size_t t = 0; t < 4; ++t) store(out + 8 * t, f[t]);
		}
	}
};

inline std::size_t log_buffers(std::size_t n) { return std::max<std::size_t>(log_blocks(n) + 2, 3); }

inline unsigned mint_scalar(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec mint_load(std::span<const Mint> f, std::size_t at) {
	if (at + 8 <= f.size()) {
		Vec value;
		std::memcpy(&value, f.data() + at, sizeof(value));
		return reduce(value, kP);
	}
	alignas(32) unsigned values[8] = {};
	for (std::size_t i = 0; i < 8 && at + i < f.size(); ++i) values[i] = mint_scalar(f[at + i]);
	return load(values);
}
class MintSource {
public:
	explicit MintSource(std::span<const Mint> f) : f_(f), full_(f.size() / 8), touched_((f.size() + 7) / 8) {}
	Vec operator()(std::size_t v) const {
		if (v < full_) {
			Vec value;
			std::memcpy(&value, f_.data() + 8 * v, sizeof(value));
			return reduce(value, kP);
		}
		if (v >= touched_) return _mm256_setzero_si256();
		return edge(f_, 8 * v);
	}
	bool within(std::size_t lo, std::size_t hi) const { return touched_ == 0 || (lo == 0 && touched_ <= hi); }
private:
	[[gnu::noinline]] static Vec edge(std::span<const Mint> f, std::size_t at) { return mint_load(f, at); }
	std::span<const Mint> f_;
	std::size_t full_, touched_;
};
[[gnu::noinline]] inline void mint_forward_top4(MintSource in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::noinline]] inline void mint_forward_top2(MintSource in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}

inline void mint_forward(const Transform& t, std::span<const Mint> f, std::span<std::uint32_t> out) {
	const MintSource in(f);
	const detail::Recursion recursion(t.roots(), t.inverse_roots(), ForwardBottom{t.roots()});
	const std::size_t nv = out.size() / 8;
	auto* v = reinterpret_cast<Vec*>(out.data());
	if (std::countr_zero(nv) % 2 == 0) {
		const std::size_t h = nv / 4;
		mint_forward_top4(in, v, h, t.roots());
		for (std::size_t q = 0; q < 4; ++q) recursion.visit(out.data() + 8 * q * h, h, q);
	} else {
		const std::size_t h = nv / 2;
		mint_forward_top2(in, v, h);
		recursion.visit(out.data(), h, 0);
		recursion.visit(out.data() + 8 * h, h, 1);
	}
}
inline void mint_derivative(std::span<const Mint> f, std::span<std::uint32_t> d) {
	using namespace detail;
	const std::size_t n = d.size(), full = std::min(n, f.size() > 0 ? f.size() - 1 : 0) / 8 * 8;
	Indices index(1);
	for (std::size_t i = 0; i < full; i += 8, index.next())
		store_unaligned(d.data() + i, canonical(montgomery(mint_load(f, i + 1), index.value())));
	for (std::size_t i = full; i < n; ++i)
		d[i] = i + 1 < f.size() ? blocked_ntt::detail::multiply_mod(std::uint32_t(i + 1), mint_scalar(f[i + 1])) : 0;
}

inline void mint_subtract_from_derivative(std::span<const Mint> f, std::size_t first, std::size_t count,
									std::uint32_t* out, std::size_t size) {
	const std::size_t inside = f.size() > first + 1 ? std::min(count, f.size() - first - 1) : 0, full = inside / 8 * 8;
	Indices index(first + 1);
	for (std::size_t c = 0; c < full; c += 8, index.next()) {
		const Vec d = canonical(montgomery(mint_load(f, first + 1 + c), index.value()));
		store_unaligned(out + c, reduce(_mm256_sub_epi32(add(d, broadcast(kP)), load_unaligned(out + size + c)), kP));
	}
	for (std::size_t c = full; c < count; ++c) {
		const std::uint32_t d = c < inside ? blocked_ntt::detail::multiply_mod(std::uint32_t(first + 1 + c), mint_scalar(f[first + 1 + c])) : 0;
		out[c] = (d + kP - out[size + c]) % kP;
	}
}

struct NoLogTransform {
	void operator()(std::size_t, std::span<const std::uint32_t>) const {}
};

template <class Input, class Sink, class Transformed>
[[gnu::always_inline]] inline void log_derivative_impl(const Transform& t, Input f, std::size_t n,
												std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink, const Transformed& transformed = {}) {
	const std::size_t k = log_block(n), len = 2 * k, blocks = log_blocks(n);
	const auto buffer = [&scratch, len](std::size_t i) __attribute__((target("avx2"))) { return scratch.subspan(i * Arena::footprint(len), len); };
	const std::span<std::uint32_t> ht = buffer(0), q0 = buffer(blocks);
	const auto window = [&buffer](std::size_t t) __attribute__((target("avx2"))) { return buffer(t); };

	inverse(t, prefix, ht.first(k), scratch.subspan(Arena::footprint(len), inverse_scratch(k)));
	t.forward(ht.first(k), 0, ht);
	for (std::size_t s = 1; s < blocks; ++s) {
		const std::size_t from = std::min(f.size(), (s - 1) * k);
		if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_forward(t, f.subspan(from, std::min(f.size() - from, len)), window(s));
		else t.forward(f.subspan(from, std::min(f.size() - from, len)), 0, window(s));
	}
	for (std::size_t j = 0; j < blocks; ++j) {
		const std::size_t first = j * k, count = std::min(k, n - 1 - first);
		const std::span<std::uint32_t> work = j == 0 ? q0 : buffer(blocks + 1);
		if (j == 0) {
			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
			else derivative(f.first(std::min(f.size(), count + 1)), work.first(count));
		} else {
			if (j == 1) {
				t.inverse_product(window(1), q0, work, Half::kUpper);
			} else if (blocks == 3) {
				const Transform::Pair pairs[2] = {{window(2), q0}, {window(1), work}};
				t.inverse_product_sum(pairs, work, Half::kUpper);
			} else if (j == 2) {
				t.inverse_with(ToeplitzBottom{t.roots(), t.inverse_roots(), window(1).data(), window(2).data(), q0.data(),
											work.data(), window(3).data()},
							work, Half::kUpper);
			} else {
				const InverseProductBottom product{{t.roots(), t.inverse_roots(), work.data()}, window(1).data()};
				t.inverse_with(SideProductBottom{product, window(3).data()}, work, Half::kUpper);
			}
			if constexpr (std::is_same_v<Input, std::span<const Mint>>) mint_subtract_from_derivative(f, first, count, work.data(), k);
			else subtract_from_derivative(f, first, count, work.data(), k);
		}
		t.cyclic_product(work.first(count), 0, work, ht, Half::kLower);
		sink(first, std::span<const std::uint32_t>(work.first(count)));
		if (j + 1 < blocks) {
			t.forward(work.first(k), 0, work);
			transformed(j, std::span<const std::uint32_t>(work));
		}
	}
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void log_derivative(const Transform& t, std::span<const std::uint32_t> f, std::size_t n,
		std::span<std::uint32_t> scratch, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, f, sink, transformed);
}

template <class Sink, class Transformed = NoLogTransform>
[[gnu::always_inline]] inline void mint_log_derivative(const Transform& t, std::span<const Mint> f, std::size_t n,
		std::span<std::uint32_t> scratch, std::span<const std::uint32_t> prefix, const Sink& sink, const Transformed& transformed = {}) {
	log_derivative_impl(t, f, n, scratch, prefix, sink, transformed);
}

inline int log_derivative_log(std::size_t n) { return std::countr_zero(2 * log_block(n)); }

inline std::size_t log_derivative_scratch(std::size_t n) { return log_buffers(n) * Arena::footprint(2 * log_block(n)); }

}

inline int log_log(std::size_t n) {
	return n <= detail::kLogBase ? Transform::kMinLog : std::countr_zero(2 * detail::log_block(n));
}

inline std::size_t log_scratch(std::size_t n) {
	return n <= detail::kLogBase ? 0 : detail::log_buffers(n) * Arena::footprint(2 * detail::log_block(n));
}

inline void log(const Transform& t, std::span<const std::uint32_t> f, std::span<std::uint32_t> g,
				std::span<std::uint32_t> scratch) {
	using namespace detail;
	const std::size_t n = g.size();
	if (n <= kLogBase) return log_direct(f, g);
	log_derivative(t, f, n, scratch, [g](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		detail::divide_by_index(first + 1, g.subspan(first + 1, q.size()), [q](std::size_t i) __attribute__((target("avx2"))) { return load(q.data() + i); });
	});
	g[0] = 0;
}

}

namespace blocked_poly {
inline void native_log(const Mint *in, std::size_t input, std::vector<Mint> &out, std::size_t n, bool integrate) {
	prepare_fps_output(out, n);
	out.emplace_back(0);
	const std::size_t c = std::min(input, n);
	const int lg = detail::log_derivative_log(n);
	const bool direct = n >= 262144;
	const auto prefix_size = direct ? std::min(c, detail::log_block(n)) : c;
	Arena arena(Transform::words(lg) + Arena::footprint(prefix_size) + Arena::footprint(log_scratch(n)));
	const Transform t(arena, lg);
	auto f = arena.take(prefix_size), scratch = arena.take(log_scratch(n));
	for (std::size_t i = 0; i < prefix_size; ++i) {
		const unsigned v = in[i].v;
		f[i] = std::min(v, v - Mint::mod());
	}
	const auto sink = [&out, integrate, n](std::size_t first, std::span<const std::uint32_t> q) __attribute__((target("avx2"))) {
		if (integrate) {
			auto *work = const_cast<std::uint32_t *>(q.data()) + detail::log_block(n);
			detail::divide_by_index(first + 1, {work, q.size()}, [q](std::size_t i) __attribute__((target("avx2"))) { return detail::load(q.data() + i); }, blocked_ntt::detail::kR);
		}
		const auto *values = integrate ? q.data() + detail::log_block(n) : q.data();
		if (integrate) append_fps_words<true>(out, values, q.size());
		else append_fps_words<false>(out, values, q.size());
	};
	if (direct) detail::mint_log_derivative(t, {in, c}, n, scratch, f, sink);
	else detail::log_derivative(t, f, n, scratch, sink);
}
}
}
#pragma GCC pop_options
#endif
#endif
