#ifndef CP_COMPONENTS_FPS_CONVOLUTION_HPP
#define CP_COMPONENTS_FPS_CONVOLUTION_HPP
#include "fps_asm.hpp"
#include "fps_blocked.hpp"
#include "fps_output.hpp"
#include "fps_convolution_unbalanced.hpp"
#include "fps_convolution_constant.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

#if defined(__linux__) && defined(__x86_64__) && defined(__GNUC__) && !defined(__clang__)
#pragma GCC push_options
#pragma GCC optimize("O3")
_Pragma("GCC target(\"avx2\")")

namespace fps_detail::multiply_detail {
namespace ntt::kernels {
using namespace blocked_ntt::kernels;
inline void forward_pair(Vec* a, Vec* b, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
	alignas(32) Vec w[3];
	Vec* const end = a + h;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T) \
R(%[P],z) R(%[P2],r) D((%[x]),c) m(c,G)\
D(32(%[x]),w)\
D((%[y]),c)\
m(c,L)\
D(32(%[y]),E)\
D(4(%[y]),c)\
m(c,M)\
D(36(%[y]),F)\
".p2align 5\n\t" \
"1:\n\t" \
m(H,c)\
m(J,d)\
m(N,f)\
m(O,g)\
q(H,w,i)\
q(J,w,t)\
m(I,l)\
q(4(%[a],%[h3]),w,e)\
q(4(%[b],%[h3]),w,j)\
m(K,n)\
m((%[a]),v)\
u(s,i,i)\
u(s,t,t)\
B(A,e,i,e)\
B(A,j,t,j)\
m((%[b]),t)\
q(I,w,i)\
p(z,e,e)\
p(z,j,j)\
p(G,c,c)\
p(G,d,d)\
u(s,i,i)\
p(G,l,l)\
k(e,c,e)\
k(j,d,j)\
p(G,n,n)\
k(r,f,d)\
C(d,f,d)\
k(r,g,f)\
q(K,w,c)\
C(f,g,f)\
q(4(%[a],%[h],2),w,g)\
u(s,c,c)\
B(A,g,i,g)\
q(4(%[b],%[h],2),w,i)\
p(z,g,g)\
B(A,i,c,i)\
o(r,d,c)\
o(e,d,d)\
k(e,c,e)\
p(z,i,i)\
k(g,l,g)\
o(r,f,l)\
o(j,f,f)\
k(j,l,j)\
u(s,d,l)\
q(E,d,c)\
q(E,l,l)\
p(L,d,d)\
k(i,n,i)\
u(s,e,n)\
q(F,n,n)\
u(s,c,c)\
B(A,l,c,l)\
q(F,e,c)\
p(M,e,e)\
p(z,l,l)\
u(s,c,c)\
B(A,n,c,n)\
k(l,d,l)\
u(s,f,d)\
q(E,f,c)\
q(E,d,d)\
p(z,n,n)\
p(L,f,f)\
u(s,c,c)\
B(A,d,c,d)\
k(n,e,n)\
u(s,j,e)\
q(F,j,c)\
q(F,e,e)\
p(z,d,d)\
p(M,j,j)\
u(s,c,c)\
B(A,e,c,e)\
k(d,f,d)\
k(r,v,f)\
k(r,t,c)\
C(f,v,f)\
C(c,t,c)\
p(z,e,e)\
o(g,f,t)\
k(g,f,g)\
o(i,c,f)\
k(i,c,i)\
k(r,t,c)\
o(r,g,v)\
C(c,t,c)\
C(v,g,v)\
k(e,j,e)\
k(r,f,j)\
o(r,i,g)\
C(j,f,j)\
o(r,c,f)\
o(r,v,t)\
C(g,i,g)\
o(l,c,c)\
k(l,f,l)\
o(n,v,v)\
o(r,j,f)\
o(r,g,i)\
k(n,t,n)\
o(d,j,j)\
m(c,(%[a]))\
k(d,f,d)\
o(e,g,g)\
k(e,i,e)\
m(l,N)\
m(v,I)\
m(n,H)\
m(j,(%[b]))\
m(d,O)\
m(g,K)\
m(e,J)\
Q(s,%[b])\
Q(s,%[a])\
S(%[end],%[a])\
T(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm0,%%ymm1,%%ymm7,%%ymm2,%%ymm3,%%ymm4,%%ymm8,CP_FPS_ASM_VPSUBD,%%ymm6,CP_FPS_ASM_VMOVDQA,%%ymm9,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,%%ymm5,CP_FPS_ASM_VPSRLQ,%%ymm10,%%ymm11,%%ymm15,$170,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VBROADCASTSS,%%ymm12,%%ymm13,(%[w]),(%[a],%[h3]),(%[a],%[h],2),(%[b],%[h3]),(%[b],%[h],2),32(%[w]),64(%[w]),(%[a],%[h]),(%[b],%[h]),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * h), [h3] "r"(96 * h), [y] "r"(y), [x] "r"(x), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

inline void bottom_first(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R) \
H(%[P],p) H(%[P2],s) H(%[NI],%%ymm13) x(32(%[a]),k)\
m(D,A)\
m(D,e)\
m(E,h)\
x((%[a]),f)\
m(D,v)\
g(s,k,l)\
m(D,c)\
r(N,A,A)\
o(l,k,l)\
r(68(%[a]),e,e)\
q(N,h,h)\
g(s,f,k)\
m(E,i)\
r(Q,v,v)\
o(k,f,k)\
u(t,A,A)\
C(B,e,A,e)\
u(t,v,v)\
q(p,e,e)\
r(100(%[a]),c,c)\
g(e,h,e)\
z(s,l,h)\
z(e,k,A)\
g(e,k,e)\
m(F,k)\
q(Q,i,i)\
g(s,A,f)\
z(s,e,j)\
m(F,d)\
C(B,c,v,c)\
o(f,A,f)\
o(j,e,j)\
q(p,c,c)\
m((%[y]),e)\
g(c,i,c)\
m(G,i)\
z(c,l,l)\
g(c,h,c)\
m(G,h)\
u(t,l,A)\
u(t,c,v)\
m(M,n)\
r(k,l,k)\
r(d,A,d)\
q(e,l,e)\
u(t,k,k)\
C(B,d,k,d)\
z(s,f,k)\
q(p,d,d)\
r(i,c,i)\
z(s,j,l)\
r(h,v,h)\
q(n,c,n)\
g(d,e,d)\
u(t,i,i)\
z(d,f,f)\
g(d,k,d)\
C(B,h,i,h)\
m(I,i)\
g(s,f,k)\
q(p,h,h)\
m(I,e)\
o(k,f,k)\
m((%[w]),f)\
g(p,k,c)\
o(c,k,c)\
g(h,n,h)\
u(t,c,n)\
x(c,32(%[buf]))\
z(h,j,j)\
g(h,l,h)\
g(s,d,l)\
o(l,d,l)\
g(p,l,d)\
r(i,c,i)\
o(d,l,d)\
r(e,n,e)\
m(J,n)\
u(t,d,l)\
q(f,c,f)\
x(d,96(%[buf]))\
m(J,c)\
u(t,i,i)\
m(4(%[w]),k)\
C(B,e,i,e)\
q(p,e,e)\
g(e,f,e)\
g(p,e,f)\
r(n,d,n)\
o(f,e,f)\
g(s,j,e)\
r(c,l,c)\
x(f,(%[buf]))\
o(e,j,e)\
q(k,d,k)\
m(K,d)\
g(p,e,j)\
u(t,n,n)\
m(K,f)\
o(j,e,j)\
C(B,c,n,c)\
m(8(%[w]),n)\
u(t,j,e)\
q(p,c,c)\
x(j,160(%[buf]))\
g(c,k,c)\
g(p,c,k)\
o(k,c,k)\
g(s,h,c)\
r(d,j,d)\
x(k,64(%[buf]))\
o(c,h,c)\
r(f,e,f)\
g(p,c,e)\
q(n,j,n)\
m(L,j)\
o(e,c,e)\
u(t,d,d)\
m(L,c)\
u(t,e,h)\
C(B,f,d,f)\
x(e,224(%[buf]))\
m(12(%[w]),d)\
q(p,f,f)\
g(f,n,f)\
g(p,f,n)\
o(n,f,n)\
x((%[b]),f)\
r(j,e,j)\
x(n,128(%[buf]))\
x(32(%[b]),n)\
r(c,h,c)\
q(d,e,d)\
m(D,e)\
u(t,j,j)\
m(D,h)\
C(B,c,j,c)\
q(p,c,c)\
g(s,f,j)\
g(c,d,c)\
m(E,d)\
g(s,n,k)\
o(j,f,j)\
g(p,c,f)\
m(D,l)\
o(k,n,k)\
o(f,c,f)\
m(D,c)\
r(O,e,e)\
x(f,192(%[buf]))\
m(E,f)\
r(68(%[b]),h,h)\
u(t,e,e)\
C(B,h,e,h)\
q(p,h,h)\
q(O,d,d)\
r(R,l,l)\
r(100(%[b]),c,c)\
q(R,f,f)\
g(h,d,h)\
u(t,l,l)\
z(s,k,d)\
z(h,j,e)\
g(h,j,h)\
m(F,j)\
C(B,c,l,c)\
g(s,e,l)\
z(s,h,n)\
m(F,i)\
q(p,c,c)\
o(l,e,l)\
o(n,h,n)\
m((%[y]),h)\
g(c,f,c)\
m(G,f)\
z(c,k,k)\
g(c,d,c)\
m(G,d)\
u(t,k,e)\
u(t,c,v)\
m(M,A)\
r(j,k,j)\
r(i,e,i)\
q(h,k,h)\
u(t,j,j)\
C(B,i,j,i)\
z(s,l,j)\
r(f,c,f)\
q(p,i,i)\
r(d,v,d)\
z(s,n,v)\
q(A,c,A)\
u(t,f,f)\
g(i,h,i)\
C(B,d,f,d)\
z(i,l,l)\
g(i,j,i)\
q(p,d,d)\
g(s,l,j)\
g(s,i,f)\
o(j,l,j)\
o(f,i,f)\
g(p,j,i)\
g(p,f,l)\
g(d,A,d)\
o(i,j,i)\
o(l,f,l)\
z(d,n,n)\
g(d,v,d)\
x(i,256(%[buf]))\
g(s,n,i)\
g(s,d,v)\
x(l,288(%[buf]))\
o(i,n,i)\
o(v,d,v)\
g(p,i,d)\
g(p,v,n)\
o(d,i,d)\
o(n,v,n)\
x(d,320(%[buf]))\
x(n,352(%[buf]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm7,%%ymm10,%%ymm2,%%ymm4,CP_FPS_ASM_VPSUBD,%%ymm3,%%ymm8,%%ymm9,%%ymm0,%%ymm6,CP_FPS_ASM_VBROADCASTSS,%%ymm11,CP_FPS_ASM_VPMINUD,%%ymm15,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,CP_FPS_ASM_VPSRLQ,%%ymm5,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,%%ymm1,$170,CP_FPS_ASM_VPBLENDD,32(%[x]),(%[x]),32(%[y]),36(%[y]),CP_FPS_ASM_VPBROADCASTD,16(%[w]),20(%[w]),24(%[w]),28(%[w]),4(%[y]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [w] "r"(w),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

inline void bottom_last(void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14) \
E(%[P],t) E(%[P2],s) E(%[NI],w) c(G,n)\
p(28(%[cur]),f)\
c(I,i)\
y(32(%[cur]),k)\
c(H,h)\
p(36(%[cur]),l)\
c(I,q)\
p(24(%[cur]),b)\
c(G,j)\
c(H,e)\
c(J,d)\
a(n,k,n)\
a(i,f,i)\
a(h,f,h)\
p(20(%[cur]),f)\
a(q,b,q)\
c(J,o)\
a(j,l,j)\
a(e,k,e)\
c(K,k)\
g(h,n,h)\
a(d,b,d)\
p(16(%[cur]),b)\
c(K,n)\
c(L,l)\
g(q,h,q)\
g(e,j,e)\
p(12(%[cur]),j)\
g(i,e,i)\
c(L,e)\
g(d,i,d)\
c(M,i)\
a(o,f,o)\
a(k,f,k)\
p(8(%[cur]),f)\
c(M,h)\
a(n,b,n)\
g(o,q,o)\
a(l,b,l)\
c(N,b)\
g(k,d,k)\
c(N,d)\
p(4(%[cur]),q)\
a(e,j,e)\
a(i,j,i)\
g(n,o,n)\
g(l,k,l)\
g(e,n,e)\
g(i,l,i)\
a(h,f,h)\
a(b,f,b)\
a(d,q,d)\
g(h,e,h)\
g(b,i,b)\
p(100(%[cur]),i)\
g(d,h,d)\
a(w,b,h)\
c(O,e)\
y(96(%[cur]),q)\
c(O,f)\
a(w,d,l)\
c(Q,n)\
a(t,h,h)\
p(92(%[cur]),k)\
c(Q,o)\
a(t,l,l)\
g(h,b,h)\
c(R,b)\
g(l,d,l)\
a(e,i,e)\
v(u,l,l)\
p(88(%[cur]),i)\
a(f,q,f)\
A(x,h,l,h)\
c(R,l)\
c(S,d)\
a(n,q,n)\
r(s,h,q)\
B(q,h,q)\
c(S,h)\
p(84(%[cur]),j)\
a(o,k,o)\
c(T,m)\
g(n,e,n)\
a(b,k,b)\
p(80(%[cur]),k)\
g(o,f,o)\
c(T,f)\
c(U,e)\
g(b,n,b)\
a(l,i,l)\
a(d,i,d)\
a(h,j,h)\
c(U,i)\
p(76(%[cur]),n)\
a(m,j,m)\
c(V,j)\
g(d,b,d)\
g(l,o,l)\
g(h,l,h)\
c(V,l)\
p(72(%[cur]),o)\
a(f,k,f)\
g(m,d,m)\
c(W,d)\
a(e,k,e)\
p(68(%[cur]),k)\
c(W,b)\
g(f,h,f)\
a(i,n,i)\
g(e,m,e)\
a(j,n,j)\
a(l,o,l)\
g(i,f,i)\
a(d,o,d)\
g(j,e,j)\
a(b,k,b)\
g(l,i,l)\
g(d,j,d)\
p(164(%[cur]),j)\
a(w,d,i)\
c(Y,k)\
g(b,l,b)\
y(160(%[cur]),l)\
a(w,b,e)\
c(Z,o)\
c(Y,f)\
a(t,i,i)\
p(156(%[cur]),n)\
a(t,e,e)\
c(Z,m)\
g(i,d,i)\
c(_0,d)\
g(e,b,e)\
a(k,j,k)\
v(u,e,e)\
c(_0,j)\
p(152(%[cur]),b)\
a(f,l,f)\
a(o,l,o)\
A(x,i,e,i)\
c(_1,e)\
r(s,i,l)\
a(m,n,m)\
B(l,i,l)\
p(148(%[cur]),i)\
g(o,k,o)\
c(_1,k)\
c(_2,h)\
a(d,n,d)\
p(144(%[cur]),n)\
g(m,f,m)\
c(_2,f)\
a(j,b,j)\
g(d,o,d)\
c(_3,o)\
a(e,b,e)\
p(140(%[cur]),b)\
g(j,m,j)\
c(_3,m)\
a(k,i,k)\
a(h,i,h)\
c(_4,i)\
g(e,d,e)\
p(136(%[cur]),d)\
a(f,n,f)\
g(k,j,k)\
g(h,e,h)\
c(_4,e)\
a(o,n,o)\
c(_5,n)\
g(f,k,f)\
p(132(%[cur]),k)\
c(_5,j)\
a(m,b,m)\
g(o,h,o)\
a(i,b,i)\
g(m,f,m)\
a(e,d,e)\
g(i,o,i)\
a(n,d,n)\
a(j,k,j)\
g(e,m,e)\
g(n,i,n)\
p(228(%[cur]),i)\
a(w,n,m)\
c(_6,k)\
g(j,e,j)\
c(_6,e)\
y(224(%[cur]),d)\
c(_7,o)\
a(w,j,f)\
a(t,m,m)\
p(220(%[cur]),b)\
c(_7,h)\
a(t,f,f)\
g(m,n,m)\
c(_9,n)\
a(k,i,k)\
g(f,j,f)\
a(e,d,e)\
v(u,f,f)\
p(216(%[cur]),j)\
a(o,d,o)\
A(x,m,f,m)\
c(_9,f)\
c(_10,d)\
r(s,m,i)\
a(h,b,h)\
B(i,m,i)\
p(212(%[cur]),m)\
g(o,k,o)\
c(_10,k)\
a(n,b,n)\
c(_11,b)\
g(h,e,h)\
p(208(%[cur]),e)\
g(n,o,n)\
c(_11,o)\
a(f,j,f)\
a(d,j,d)\
c(_12,j)\
a(k,m,k)\
g(f,h,f)\
c(_12,h)\
a(b,m,b)\
g(d,n,d)\
p(204(%[cur]),n)\
p(200(%[cur]),m)\
g(k,f,k)\
c(_13,f)\
a(o,e,o)\
g(b,d,b)\
c(_13,d)\
a(j,e,j)\
c(_14,e)\
g(o,k,o)\
c(_14,k)\
a(h,n,h)\
g(j,b,j)\
p(196(%[cur]),b)\
a(f,m,f)\
a(d,n,d)\
g(h,o,h)\
a(e,m,e)\
g(f,h,f)\
g(d,j,d)\
a(k,b,k)\
g(e,d,e)\
a(w,e,d)\
z(l,q,b)\
r(s,b,j)\
g(k,f,k)\
B(j,b,j)\
a(w,k,b)\
a(t,d,d)\
z(s,q,q)\
r(l,q,l)\
a(t,b,b)\
c(X,q)\
g(d,e,d)\
v(u,l,e)\
c(X,f)\
g(b,k,b)\
c((%[iy]),k)\
v(u,b,b)\
A(x,d,b,d)\
r(s,d,b)\
z(s,i,h)\
B(b,d,b)\
a(q,l,q)\
r(b,h,h)\
z(b,i,b)\
c(_8,i)\
c(_8,d)\
a(f,e,f)\
v(u,h,e)\
r(s,b,m)\
B(m,b,m)\
c(4(%[iy]),b)\
v(u,q,q)\
A(x,f,q,f)\
C(k,l,k)\
z(m,j,l)\
C(t,f,f)\
r(s,l,q)\
B(q,l,q)\
y(q,(%[out]))\
r(f,k,f)\
a(i,h,i)\
a(d,e,d)\
z(s,j,j)\
c(D,e)\
C(b,h,b)\
r(m,j,m)\
c(D,j)\
v(u,m,h)\
v(u,i,i)\
A(x,d,i,d)\
c(F,i)\
C(t,d,d)\
r(d,b,d)\
a(e,m,e)\
z(d,f,b)\
z(s,f,f)\
a(j,h,j)\
r(s,b,h)\
c(D,k)\
r(d,f,d)\
B(h,b,h)\
c(D,b)\
v(u,d,f)\
v(u,e,e)\
C(i,m,i)\
y(h,32(%[out]))\
c(F,h)\
A(x,j,e,j)\
C(t,j,j)\
r(j,i,j)\
y(j,64(%[out]))\
a(k,d,k)\
a(b,f,b)\
C(h,d,h)\
v(u,k,k)\
A(x,b,k,b)\
C(t,b,b)\
r(b,h,b)\
y(b,96(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm7,CP_FPS_ASM_VBROADCASTSS,%%ymm10,%%ymm9,%%ymm1,CP_FPS_ASM_VPADDQ,%%ymm4,%%ymm2,%%ymm8,%%ymm3,%%ymm5,%%ymm12,%%ymm0,%%ymm11,CP_FPS_ASM_VMOVDQU,%%ymm6,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm15,$32,CP_FPS_ASM_VPSRLQ,%%ymm13,$170,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VPMULLD,32(%[ix]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),256(%[cur]),260(%[cur]),264(%[cur]),268(%[cur]),272(%[cur]),276(%[cur]),280(%[cur]),284(%[cur]),288(%[cur]),292(%[cur]),296(%[cur]),300(%[cur]),304(%[cur]),308(%[cur]),312(%[cur]),316(%[cur]),32(%[iy]),320(%[cur]),324(%[cur]),328(%[cur]),332(%[cur]),336(%[cur]),340(%[cur]),344(%[cur]),348(%[cur]),352(%[cur]),356(%[cur]),36(%[iy]),360(%[cur]),364(%[cur]),368(%[cur]),372(%[cur]),376(%[cur]),380(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

inline void bottom_both(const void* a, const void* b, void* buf, const void* x, const void* y, const void* w, void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31) \
M(%[P],t) M(%[P2],v) M(%[NI],G) C(32(%[a]),k)\
z(36(%[cur]),d)\
f(S,j)\
f(H,n)\
f(H,e)\
f(I,o)\
C(32(%[cur]),c)\
f(S,m)\
C((%[a]),s)\
q(v,k,h)\
u(h,k,h)\
f(T,k)\
g(j,d,j)\
g(_28,n,n)\
f(H,d)\
g(68(%[a]),e,e)\
z(28(%[cur]),i)\
f(T,l)\
g(m,c,m)\
B(x,n,n)\
A(_28,o,o)\
q(v,s,r)\
F(E,e,n,e)\
u(r,s,r)\
f(U,s)\
f(H,n)\
A(t,e,e)\
g(k,c,k)\
q(e,o,e)\
z(24(%[cur]),o)\
f(U,c)\
g(_30,d,d)\
g(l,i,l)\
p(k,j,k)\
f(I,j)\
g(s,i,s)\
g(100(%[a]),n,n)\
B(x,d,d)\
f(V,i)\
p(l,m,l)\
z(20(%[cur]),m)\
p(s,k,s)\
F(E,n,d,n)\
f(V,d)\
D(e,r,k)\
A(t,n,n)\
q(e,r,e)\
q(v,k,r)\
g(c,o,c)\
u(r,k,r)\
f(W,k)\
A(_30,j,j)\
g(i,o,i)\
p(c,l,c)\
z(16(%[cur]),l)\
D(v,e,o)\
u(o,e,o)\
f(W,e)\
q(n,j,n)\
f(X,j)\
g(d,m,d)\
p(i,s,i)\
D(v,h,s)\
D(n,h,h)\
q(n,s,n)\
f(X,s)\
g(k,m,k)\
z(12(%[cur]),m)\
p(d,c,d)\
f(K,c)\
p(k,i,k)\
f(Z,i)\
g(e,l,e)\
g(j,l,j)\
f(K,l)\
p(e,d,e)\
z(8(%[cur]),d)\
g(s,m,s)\
p(j,k,j)\
f(Z,k)\
g(c,h,c)\
g(i,m,i)\
p(s,e,s)\
f(_0,e)\
f((%[y]),m)\
B(x,c,c)\
p(i,j,i)\
z(4(%[cur]),j)\
g(k,d,k)\
p(k,s,k)\
g(e,d,e)\
A(m,h,m)\
f(_0,d)\
B(x,n,s)\
B(x,h,h)\
g(l,h,l)\
f(L,h)\
p(e,i,e)\
f(L,i)\
F(E,l,c,l)\
g(G,e,c)\
A(t,l,l)\
g(d,j,d)\
g(t,c,c)\
f(_27,j)\
q(l,m,l)\
g(h,n,h)\
p(d,k,d)\
p(c,e,c)\
g(i,s,i)\
z(100(%[cur]),s)\
g(G,d,e)\
f(_1,k)\
B(x,h,h)\
F(E,i,h,i)\
D(l,r,h)\
f(_1,m)\
g(t,e,e)\
A(t,i,i)\
D(v,r,r)\
A(j,n,j)\
q(l,r,l)\
C(96(%[cur]),r)\
f(_2,n)\
p(e,d,e)\
z(92(%[cur]),d)\
B(x,e,e)\
F(E,c,e,c)\
q(i,j,i)\
f(_2,j)\
D(v,o,e)\
g(k,s,k)\
D(i,o,o)\
q(i,e,i)\
f(_3,e)\
q(v,c,s)\
u(s,c,s)\
z(88(%[cur]),c)\
g(m,r,m)\
g(n,r,n)\
f(_3,r)\
g(j,d,j)\
g(e,d,e)\
p(n,k,n)\
q(v,h,k)\
f(_4,d)\
u(k,h,k)\
p(j,m,j)\
f(O,m)\
q(t,k,h)\
p(e,n,e)\
u(h,k,h)\
f(_4,k)\
z(84(%[cur]),n)\
C(h,32(%[buf]))\
g(r,c,r)\
p(r,j,r)\
f(_5,j)\
g(d,c,d)\
f(O,c)\
g(m,h,m)\
g(k,n,k)\
p(d,e,d)\
f(_5,e)\
B(x,m,m)\
p(k,r,k)\
z(80(%[cur]),r)\
g(j,n,j)\
f(_6,n)\
p(j,d,j)\
f((%[w]),d)\
g(e,r,e)\
g(n,r,n)\
z(76(%[cur]),r)\
p(e,k,e)\
f(_6,k)\
p(n,j,n)\
A(d,h,d)\
q(v,l,j)\
B(x,h,h)\
u(j,l,j)\
g(c,h,c)\
f(_7,h)\
z(72(%[cur]),l)\
F(E,c,m,c)\
q(t,j,m)\
u(m,j,m)\
A(t,c,c)\
f(_7,j)\
g(k,r,k)\
C(m,96(%[buf]))\
q(c,d,c)\
f(_8,d)\
g(h,r,h)\
p(k,e,k)\
f(Q,e)\
B(x,m,r)\
p(h,n,h)\
f(_8,n)\
g(j,l,j)\
g(d,l,d)\
p(j,k,j)\
z(68(%[cur]),k)\
q(t,c,l)\
g(e,m,e)\
u(l,c,l)\
f(Q,c)\
C(l,(%[buf]))\
f(4(%[w]),l)\
p(d,h,d)\
g(G,d,h)\
B(x,e,e)\
g(t,h,h)\
g(n,k,n)\
g(c,r,c)\
p(h,d,h)\
A(l,m,l)\
p(n,j,n)\
g(G,n,j)\
F(E,c,e,c)\
z(164(%[cur]),e)\
A(t,c,c)\
f(_10,m)\
g(t,j,j)\
q(v,o,d)\
f(_10,r)\
C(160(%[cur]),k)\
u(d,o,d)\
q(c,l,c)\
f(_11,l)\
p(j,n,j)\
q(t,d,n)\
q(t,c,o)\
B(x,j,j)\
u(n,d,n)\
u(o,c,o)\
f(_11,c)\
z(156(%[cur]),d)\
F(E,h,j,h)\
C(o,64(%[buf]))\
f(R,o)\
B(x,n,j)\
g(m,e,m)\
C(n,160(%[buf]))\
f(_12,e)\
g(r,k,r)\
g(l,k,l)\
z(152(%[cur]),k)\
g(c,d,c)\
p(l,m,l)\
g(o,n,o)\
f(_12,m)\
g(e,d,e)\
f(_13,d)\
p(c,r,c)\
q(v,h,r)\
u(r,h,r)\
B(x,o,o)\
f(R,h)\
p(e,l,e)\
f(_13,l)\
g(m,k,m)\
g(d,k,d)\
z(148(%[cur]),k)\
p(m,c,m)\
g(h,j,h)\
f(_14,j)\
f(8(%[w]),c)\
p(d,e,d)\
z(144(%[cur]),e)\
F(E,h,o,h)\
q(v,i,o)\
A(t,h,h)\
u(o,i,o)\
f(_14,i)\
g(l,k,l)\
g(j,k,j)\
A(c,n,c)\
f(_15,n)\
q(t,o,k)\
p(l,m,l)\
u(k,o,k)\
f(_15,o)\
z(140(%[cur]),m)\
p(j,d,j)\
q(h,c,h)\
C(k,224(%[buf]))\
B(x,k,c)\
f(_16,d)\
g(i,e,i)\
g(n,e,n)\
p(i,l,i)\
f(Y,l)\
f(Y,e)\
g(o,m,o)\
p(n,j,n)\
f(_16,j)\
g(d,m,d)\
z(136(%[cur]),m)\
p(o,i,o)\
f(_17,i)\
p(d,n,d)\
f(12(%[w]),n)\
g(e,c,e)\
g(l,k,l)\
q(t,h,c)\
u(c,h,c)\
f(_17,h)\
C(c,128(%[buf]))\
z(132(%[cur]),c)\
B(x,l,l)\
g(j,m,j)\
F(E,e,l,e)\
g(i,m,i)\
A(t,e,e)\
p(j,o,j)\
A(n,k,n)\
p(i,d,i)\
C((%[b]),d)\
g(G,i,k)\
g(h,c,h)\
q(e,n,e)\
q(t,e,n)\
g(t,k,k)\
u(n,e,n)\
p(h,j,h)\
C(32(%[b]),j)\
C(n,192(%[buf]))\
z(228(%[cur]),n)\
g(G,h,e)\
f(_18,c)\
p(k,i,k)\
f(H,i)\
q(v,d,o)\
C(224(%[cur]),m)\
g(t,e,e)\
u(o,d,o)\
f(_18,d)\
f(H,l)\
p(e,h,e)\
f(_19,h)\
B(x,e,e)\
F(E,k,e,k)\
q(v,j,e)\
g(c,n,c)\
u(e,j,e)\
f(_19,j)\
z(220(%[cur]),n)\
g(_29,i,i)\
g(d,m,d)\
g(68(%[b]),l,l)\
B(x,i,i)\
g(h,m,h)\
f(_21,m)\
F(E,l,i,l)\
z(216(%[cur]),i)\
A(t,l,l)\
g(j,n,j)\
p(h,c,h)\
f(_21,c)\
p(j,d,j)\
f(H,d)\
g(m,n,m)\
f(_22,n)\
g(c,i,c)\
p(m,h,m)\
f(H,h)\
p(c,j,c)\
g(_31,d,d)\
z(212(%[cur]),j)\
g(n,i,n)\
f(_22,i)\
B(x,d,d)\
p(n,m,n)\
g(100(%[b]),h,h)\
f(_23,m)\
F(E,h,d,h)\
f(I,d)\
A(t,h,h)\
g(i,j,i)\
p(i,c,i)\
g(m,j,m)\
f(_23,j)\
z(208(%[cur]),c)\
p(m,n,m)\
A(_31,d,d)\
q(v,k,n)\
u(n,k,n)\
f(_24,k)\
q(h,d,h)\
f(I,d)\
g(j,c,j)\
p(j,i,j)\
z(204(%[cur]),i)\
g(k,c,k)\
f(_24,c)\
A(_29,d,d)\
p(k,m,k)\
f(_25,m)\
q(l,d,l)\
D(v,e,d)\
D(h,e,e)\
q(h,d,h)\
z(200(%[cur]),d)\
g(c,i,c)\
g(m,i,m)\
p(c,j,c)\
f(_25,j)\
B(x,e,i)\
p(m,k,m)\
f(_26,k)\
g(j,d,j)\
g(k,d,k)\
p(j,c,j)\
f(K,c)\
f(K,d)\
p(k,m,k)\
z(196(%[cur]),m)\
g(d,i,d)\
g(c,e,c)\
f(_26,i)\
B(x,c,c)\
F(E,d,c,d)\
f((%[y]),c)\
g(i,m,i)\
q(l,o,m)\
D(l,o,l)\
g(G,k,o)\
A(t,d,d)\
p(i,j,i)\
D(v,m,j)\
g(t,o,o)\
A(c,e,c)\
u(j,m,j)\
B(x,h,m)\
g(G,i,e)\
p(o,k,o)\
q(d,c,d)\
f(L,c)\
q(v,l,k)\
g(t,e,e)\
u(k,l,k)\
f(L,l)\
p(e,i,e)\
D(r,s,i)\
B(x,e,e)\
F(E,o,e,o)\
q(v,i,e)\
D(v,s,s)\
u(e,i,e)\
q(v,o,i)\
u(i,o,i)\
q(r,s,r)\
g(c,h,c)\
f(_27,s)\
f(_9,o)\
g(l,m,l)\
f(_9,m)\
B(x,c,c)\
F(E,l,c,l)\
B(x,r,c)\
A(t,l,l)\
A(s,h,s)\
f((%[iy]),h)\
g(o,r,o)\
g(m,c,m)\
D(i,n,c)\
q(l,s,l)\
q(v,c,s)\
D(v,n,n)\
B(x,o,o)\
u(s,c,s)\
D(v,j,c)\
q(i,n,i)\
F(E,m,o,m)\
q(l,c,c)\
D(l,j,l)\
f(_20,j)\
f(_20,o)\
B(x,i,n)\
A(t,m,m)\
A(h,r,h)\
D(v,k,r)\
D(d,k,k)\
q(d,r,d)\
f(4(%[iy]),r)\
q(m,h,m)\
q(v,d,h)\
u(h,d,h)\
q(v,k,d)\
g(j,i,j)\
u(d,k,d)\
D(s,e,k)\
g(o,n,o)\
q(t,d,n)\
u(n,d,n)\
q(t,h,d)\
B(x,j,j)\
C(n,256(%[buf]))\
u(d,h,d)\
q(v,k,h)\
F(E,o,j,o)\
A(r,i,r)\
q(v,l,i)\
C(d,288(%[buf]))\
u(h,k,h)\
A(t,o,o)\
D(v,e,e)\
u(i,l,i)\
C(h,(%[out]))\
f(J,h)\
q(s,e,s)\
q(t,i,e)\
f(J,l)\
B(x,s,k)\
q(v,c,d)\
u(e,i,e)\
q(o,r,o)\
u(d,c,d)\
C(e,320(%[buf]))\
f(N,e)\
D(o,m,c)\
q(t,d,r)\
q(v,c,i)\
u(r,d,r)\
D(v,m,m)\
u(i,c,i)\
C(r,352(%[buf]))\
f(J,r)\
q(o,m,o)\
C(i,32(%[out]))\
f(J,i)\
B(x,o,m)\
g(h,s,h)\
f(N,c)\
g(l,k,l)\
B(x,h,h)\
A(e,s,e)\
F(E,l,h,l)\
A(t,l,l)\
g(r,o,r)\
g(i,m,i)\
q(l,e,l)\
B(x,r,r)\
A(c,o,c)\
C(l,64(%[out]))\
F(E,i,r,i)\
A(t,i,i)\
q(i,c,i)\
C(i,96(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm6,%%ymm1,%%ymm4,CP_FPS_ASM_VBROADCASTSS,CP_FPS_ASM_VPMULUDQ,%%ymm9,%%ymm10,%%ymm2,%%ymm0,%%ymm11,%%ymm7,%%ymm3,%%ymm5,CP_FPS_ASM_VPADDQ,CP_FPS_ASM_VPSUBD,%%ymm12,%%ymm8,%%ymm15,CP_FPS_ASM_VPMINUD,%%ymm14,$32,CP_FPS_ASM_VMOVDQU,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VPBLENDD,%%ymm13,32(%[x]),(%[x]),32(%[ix]),32(%[y]),36(%[y]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),16(%[w]),20(%[w]),24(%[w]),256(%[cur]),260(%[cur]),264(%[cur]),268(%[cur]),272(%[cur]),276(%[cur]),28(%[w]),280(%[cur]),284(%[cur]),288(%[cur]),292(%[cur]),296(%[cur]),300(%[cur]),304(%[cur]),308(%[cur]),312(%[cur]),316(%[cur]),32(%[iy]),320(%[cur]),324(%[cur]),328(%[cur]),332(%[cur]),336(%[cur]),340(%[cur]),344(%[cur]),348(%[cur]),352(%[cur]),356(%[cur]),36(%[iy]),360(%[cur]),364(%[cur]),368(%[cur]),372(%[cur]),376(%[cur]),380(%[cur]),4(%[y]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [w] "r"(w), [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

}
namespace ntt {
inline constexpr int kMaxLog = 23;
namespace detail {
using namespace blocked_ntt::detail;
class Recursion {
public:
	Recursion(const std::uint32_t* roots, const std::uint32_t* inverse_roots) : r_(roots), ir_(inverse_roots) {}

	void visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
		switch (nv) {
		case 4: return tile<4>(a, b, k);
		case 16: return tile<16>(a, b, k);
		case 64: return tile<64>(a, b, k);
		case 256: return tile<256>(a, b, k);
		}
		const std::size_t h = nv / 4;
		forward(a, b, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + t * h, b + t * h, h, 4 * k + t);
		inverse(a, h, k);
	}

private:
	
	template <std::size_t NV>
	[[gnu::noinline]] void tile(Vec* a, Vec* b, std::size_t k) const {
		for (std::size_t h = NV / 4; h >= 4; h /= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) forward(a + j, b + j, h, g);
		bottom(a, b, NV, k * (NV / 4));
		for (std::size_t h = 4; h < NV; h *= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) inverse(a + j, h, g);
	}

	void forward(Vec* a, Vec* b, std::size_t h, std::size_t k) const {
		if (k == 0) {
			kernels::forward_identity(a, h, r_);
			kernels::forward_identity(b, h, r_);
			return;
		}
		const std::uint32_t *x = r_ + slot(k), *y = r_ + slot(2 * k);
		if (h == 4) return kernels::forward_pair(a, b, h, x, y);
		kernels::forward(a, h, x, y);
		kernels::forward(b, h, x, y);
	}

	void inverse(Vec* a, std::size_t h, std::size_t k) const {
		if (k == 0) return kernels::inverse_identity(a, h, ir_);
		kernels::inverse(a, h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	struct alignas(64) Leaves {
		std::uint32_t window[4][16];
		std::uint32_t coefficients[4][8];
	};

	void leaf_weights(std::size_t k, std::uint32_t* w) const {
		const std::uint32_t* y = r_ + slot(2 * k);
		w[0] = y[0], w[1] = kP - y[0], w[2] = y[1], w[3] = kP - y[1];
		w[4] = y[8], w[5] = ~y[8], w[6] = y[9], w[7] = ~y[9];
	}

	
	void bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {
		Leaves leaves[2]{};
		alignas(32) std::uint32_t w[2][8];
		leaf_weights(first, w[0]);
		kernels::bottom_first(a, b, &leaves[0], r_ + slot(first), r_ + slot(2 * first), w[0]);
		for (std::size_t j = 0, k = first; j < nv; j += 4, ++k) {
			const std::size_t cur = j / 4 % 2, next = cur ^ 1;
			const std::uint32_t *ix = ir_ + slot(k), *iy = ir_ + slot(2 * k);
			if (j + 4 == nv) {
				kernels::bottom_last(a + j, &leaves[cur], ix, iy);
				break;
			}
			leaf_weights(k + 1, w[next]);
			kernels::bottom_both(a + j + 4, b + j + 4, &leaves[next], r_ + slot(k + 1), r_ + slot(2 * k + 2), w[next],
								 a + j, &leaves[cur], ix, iy);
		}
	}

	const std::uint32_t *r_, *ir_;
};



inline void forward_radix2(Vec* f, std::size_t h, bool sparse) {
	if (sparse) {
		std::memcpy(f + h, f, h * sizeof(Vec));
		return;
	}
	for (std::size_t i = 0; i < h; ++i) {
		const Vec x = f[i], y = f[i + h];
		f[i] = reduce(add(x, y), 2 * kP);
		f[i + h] = reduce(diff(x, y), 2 * kP);
	}
}

inline void forward_radix4(Vec* f, std::size_t h, bool sparse, const std::uint32_t* roots) {
	if (!sparse) return kernels::forward_identity(f, h, roots);
	const Factor z(roots[1], roots[9]);
	for (std::size_t j = 0; j < h; ++j) {
		const Vec a = f[j], b = f[j + h], zb = multiply(b, z);
		f[j] = add(a, b), f[j + h] = diff(a, b), f[j + 2 * h] = add(a, zb), f[j + 3 * h] = diff(a, zb);
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
class Convolution {
public:
	Convolution(std::size_t n, std::size_t m) : Convolution(n, m, nullptr) {}
	Convolution(std::size_t n, std::size_t m, std::uint32_t* borrowed_a)
		: n_(n), m_(m), lg_(std::max(6, int(std::bit_width(n + m - 2)))) {
		if (n == 0 || m == 0 || lg_ > kMaxLog) std::abort();
		const std::size_t len = length(), words = (borrowed_a ? 1 : 2) * (len + kPadding) + 2 * detail::table_words(lg_) + (borrowed_a ? 1024 : 0);
		storage_ = allocate_fps_words(words);
		a_ = borrowed_a ? borrowed_a : storage_.get();
		const std::size_t color = ((reinterpret_cast<std::uintptr_t>(a_) >> 6) - (reinterpret_cast<std::uintptr_t>(storage_.get()) >> 6) + 1) & 63;
		b_ = storage_.get() + (borrowed_a ? 16 * color : len + kPadding);
		roots_ = b_ + len + kPadding;
		inverse_roots_ = roots_ + detail::table_words(lg_);
		std::fill(roots_, inverse_roots_ + detail::table_words(lg_), 0);
	}

	Convolution(const Convolution&) = delete;
	Convolution& operator=(const Convolution&) = delete;

	std::uint32_t* a() { return a_; }
	std::uint32_t* b() { return b_; }

	const std::uint32_t* multiply() {
		using namespace detail;
		const std::size_t len = length(), nv = len / 8;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		const Recursion recursion(roots_, inverse_roots_);
		const std::uint32_t scale = power(std::uint32_t(nv), kP - 2);
		auto* a = reinterpret_cast<Vec*>(a_);
		auto* b = reinterpret_cast<Vec*>(b_);
		const bool sparse_a = n_ <= len / 2, sparse_b = m_ <= len / 2;
		if (std::countr_zero(nv) % 2) {
			const std::size_t h = nv / 2;
			forward_radix2(a, h, sparse_a);
			forward_radix2(b, h, sparse_b);
			recursion.visit(a, b, h, 0);
			recursion.visit(a + h, b + h, h, 1);
			alignas(32) std::uint32_t s[16] = {};
			s[1] = scale;
			s[9] = quotient(scale);
			kernels::scale_radix2(a, h, s);
		} else {
			const std::size_t h = nv / 4;
			forward_radix4(a, h, sparse_a, roots_);
			forward_radix4(b, h, sparse_b, roots_);
			for (std::size_t t = 0; t < 4; ++t) recursion.visit(a + t * h, b + t * h, h, t);
			detail::inverse_radix4(a, h, inverse_roots_, Factor(scale));
		}
		return a_;
	}

private:
	static constexpr std::size_t kPadding = 16;

	std::size_t length() const { return std::size_t(1) << lg_; }

	std::size_t n_, m_;
	int lg_;
	FpsWords storage_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
};

}
namespace bottom_kernels {

using ntt::kernels::k2P;
using ntt::kernels::kNI;
using ntt::kernels::kP;

[[gnu::always_inline]] inline void first(const void* a, const void* b, void* buf, const void* x, const void* y) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W) \
N(%[P],q) N(%[P2],u) N(%[NI],%%ymm13) x(32(%[a]),j)\
m(B,l)\
m(B,n)\
m(H,f)\
x((%[a]),o)\
m(B,k)\
e(u,j,i)\
m(B,c)\
t(T,l,l)\
p(i,j,i)\
t(68(%[a]),n,n)\
s(T,f,f)\
e(u,o,j)\
m(H,g)\
t(V,k,k)\
p(j,o,j)\
w(v,l,l)\
A(z,n,l,n)\
w(v,k,k)\
s(q,n,n)\
t(100(%[a]),c,c)\
e(n,f,n)\
y(u,i,f)\
y(n,j,l)\
e(n,j,n)\
m(C,j)\
s(V,g,g)\
e(u,l,o)\
y(u,n,r)\
m(C,h)\
A(z,c,k,c)\
p(o,l,o)\
p(r,n,r)\
s(q,c,c)\
m(I,n)\
e(c,g,c)\
m(E,g)\
y(c,i,i)\
e(c,f,c)\
m(E,f)\
w(v,i,l)\
w(v,c,k)\
m(L,d)\
t(j,i,j)\
t(h,l,h)\
s(n,i,n)\
w(v,j,j)\
A(z,h,j,h)\
y(u,o,j)\
s(q,h,h)\
t(g,c,g)\
y(u,r,i)\
t(f,k,f)\
s(d,c,d)\
e(h,n,h)\
w(v,g,g)\
e(h,j,j)\
y(h,o,h)\
A(z,f,g,f)\
m(C,g)\
e(u,h,o)\
s(q,f,f)\
m(C,n)\
p(o,h,o)\
m(I,h)\
e(q,o,c)\
p(c,o,c)\
e(f,d,f)\
w(v,c,d)\
x(c,32(%[buf]))\
e(f,i,i)\
y(f,r,f)\
e(u,j,r)\
p(r,j,r)\
e(q,r,j)\
t(g,c,g)\
p(j,r,j)\
t(n,d,n)\
m(C,d)\
w(v,j,r)\
s(h,c,h)\
x(j,96(%[buf]))\
m(C,c)\
w(v,g,g)\
m(I,o)\
A(z,n,g,n)\
s(q,n,n)\
e(n,h,n)\
e(q,n,h)\
t(d,j,d)\
p(h,n,h)\
e(u,f,n)\
t(c,r,c)\
x(h,(%[buf]))\
p(n,f,n)\
s(o,j,o)\
e(q,n,j)\
w(v,d,d)\
m(E,f)\
p(j,n,j)\
A(z,c,d,c)\
m(E,d)\
w(v,j,n)\
s(q,c,c)\
x(j,160(%[buf]))\
m(L,h)\
e(c,o,c)\
e(q,c,o)\
p(o,c,o)\
e(o,q,o)\
e(u,i,c)\
t(f,j,f)\
x(o,64(%[buf]))\
p(c,i,c)\
t(d,n,d)\
e(q,c,n)\
s(h,j,h)\
m(E,j)\
p(n,c,n)\
w(v,f,f)\
m(E,c)\
w(v,n,i)\
A(z,d,f,d)\
x(n,224(%[buf]))\
m(L,f)\
s(q,d,d)\
e(d,h,d)\
e(q,d,h)\
p(h,d,h)\
t(j,n,j)\
x(h,128(%[buf]))\
x((%[b]),h)\
t(c,i,c)\
x(32(%[b]),i)\
s(f,n,f)\
w(v,j,j)\
m(B,n)\
A(z,c,j,c)\
m(B,j)\
s(q,c,c)\
e(c,f,c)\
e(u,h,f)\
e(q,c,d)\
m(H,o)\
e(u,i,r)\
p(f,h,f)\
p(d,c,d)\
m(B,c)\
p(r,i,r)\
e(d,q,d)\
m(B,i)\
t(U,n,n)\
x(d,192(%[buf]))\
m(H,d)\
t(68(%[b]),j,j)\
w(v,n,n)\
A(z,j,n,j)\
s(q,j,j)\
s(U,o,o)\
t(W,c,c)\
t(100(%[b]),i,i)\
s(W,d,d)\
e(j,o,j)\
w(v,c,c)\
y(u,r,o)\
y(j,f,n)\
e(j,f,j)\
m(C,f)\
A(z,i,c,i)\
e(u,n,c)\
y(u,j,h)\
m(C,g)\
s(q,i,i)\
p(c,n,c)\
p(h,j,h)\
m(I,j)\
e(i,d,i)\
m(E,d)\
y(i,r,r)\
e(i,o,i)\
m(E,o)\
w(v,r,n)\
w(v,i,k)\
m(L,l)\
t(f,r,f)\
t(g,n,g)\
s(j,r,j)\
w(v,f,f)\
A(z,g,f,g)\
y(u,c,f)\
t(d,i,d)\
s(q,g,g)\
t(o,k,o)\
y(u,h,k)\
s(l,i,l)\
w(v,d,d)\
e(g,j,g)\
A(z,o,d,o)\
y(g,c,c)\
e(g,f,g)\
s(q,o,o)\
e(u,c,f)\
e(u,g,d)\
p(f,c,f)\
p(d,g,d)\
e(q,f,g)\
e(q,d,c)\
e(o,l,o)\
p(g,f,g)\
p(c,d,c)\
y(o,h,h)\
e(o,k,o)\
x(g,256(%[buf]))\
x(128(%[a]),g)\
e(u,h,k)\
e(u,o,d)\
x(c,288(%[buf]))\
x(160(%[a]),c)\
p(k,h,k)\
p(d,o,d)\
m(D,o)\
e(q,k,h)\
e(q,d,f)\
m(D,l)\
p(h,k,h)\
p(f,d,f)\
m(K,d)\
x(h,320(%[buf]))\
x(f,352(%[buf]))\
m(D,f)\
e(u,g,h)\
m(D,k)\
e(u,c,j)\
p(h,g,h)\
t(O,o,o)\
p(j,c,j)\
m(K,c)\
t(196(%[a]),l,l)\
s(O,d,d)\
w(v,o,o)\
t(R,f,f)\
A(z,l,o,l)\
s(q,l,l)\
t(228(%[a]),k,k)\
w(v,f,f)\
e(l,d,l)\
s(R,c,c)\
y(u,j,d)\
y(l,h,o)\
e(l,h,l)\
m(F,h)\
A(z,k,f,k)\
e(u,o,f)\
y(u,l,g)\
m(F,i)\
s(q,k,k)\
p(f,o,f)\
p(g,l,g)\
m(M,l)\
e(k,c,k)\
m(G,c)\
y(k,j,j)\
e(k,d,k)\
m(G,d)\
w(v,j,o)\
w(v,k,r)\
m(J,n)\
t(h,j,h)\
t(i,o,i)\
s(l,j,l)\
w(v,h,h)\
A(z,i,h,i)\
y(u,f,h)\
t(c,k,c)\
s(q,i,i)\
t(d,r,d)\
y(u,g,r)\
s(n,k,n)\
w(v,c,c)\
e(i,l,i)\
A(z,d,c,d)\
e(i,h,h)\
y(i,f,i)\
s(q,d,d)\
m(F,f)\
e(u,i,c)\
m(F,l)\
p(c,i,c)\
m(M,i)\
e(q,c,k)\
e(d,n,d)\
p(k,c,k)\
e(d,r,r)\
y(d,g,d)\
w(v,k,g)\
x(k,416(%[buf]))\
e(u,h,c)\
p(c,h,c)\
e(q,c,h)\
p(h,c,h)\
t(f,k,f)\
m(F,c)\
w(v,h,n)\
t(l,g,l)\
x(h,480(%[buf]))\
m(F,g)\
s(i,k,i)\
m(M,k)\
w(v,f,f)\
A(z,l,f,l)\
s(q,l,l)\
e(l,i,l)\
t(c,h,c)\
e(q,l,i)\
t(g,n,g)\
p(i,l,i)\
e(u,d,l)\
s(k,h,k)\
x(i,384(%[buf]))\
p(l,d,l)\
w(v,c,c)\
m(G,d)\
e(q,l,i)\
A(z,g,c,g)\
m(G,c)\
p(i,l,i)\
s(q,g,g)\
m(J,l)\
w(v,i,h)\
x(i,544(%[buf]))\
e(g,k,g)\
e(q,g,k)\
p(k,g,k)\
e(k,q,k)\
e(u,r,g)\
t(d,i,d)\
x(k,448(%[buf]))\
p(g,r,g)\
t(c,h,c)\
e(q,g,h)\
s(l,i,l)\
m(G,i)\
p(h,g,h)\
w(v,d,d)\
m(G,g)\
w(v,h,r)\
A(z,c,d,c)\
x(h,608(%[buf]))\
m(J,d)\
s(q,c,c)\
e(c,l,c)\
e(q,c,l)\
p(l,c,l)\
t(i,h,i)\
x(l,512(%[buf]))\
x(128(%[b]),l)\
t(g,r,g)\
x(160(%[b]),r)\
s(d,h,d)\
w(v,i,i)\
m(D,h)\
A(z,g,i,g)\
m(D,i)\
s(q,g,g)\
e(g,d,g)\
e(u,l,d)\
e(q,g,c)\
m(K,k)\
e(u,r,n)\
p(d,l,d)\
p(c,g,c)\
m(D,g)\
p(n,r,n)\
e(c,q,c)\
m(D,r)\
t(Q,h,h)\
x(c,576(%[buf]))\
m(K,c)\
t(196(%[b]),i,i)\
w(v,h,h)\
A(z,i,h,i)\
s(q,i,i)\
s(Q,k,k)\
t(S,g,g)\
t(228(%[b]),r,r)\
s(S,c,c)\
e(i,k,i)\
w(v,g,g)\
y(u,n,k)\
y(i,d,h)\
e(i,d,i)\
m(F,d)\
A(z,r,g,r)\
e(u,h,g)\
y(u,i,l)\
m(F,f)\
s(q,r,r)\
p(g,h,g)\
p(l,i,l)\
m(M,i)\
e(r,c,r)\
m(G,c)\
y(r,n,n)\
e(r,k,r)\
m(G,k)\
w(v,n,h)\
w(v,r,j)\
m(J,o)\
t(d,n,d)\
t(f,h,f)\
s(i,n,i)\
w(v,d,d)\
A(z,f,d,f)\
y(u,g,d)\
t(c,r,c)\
s(q,f,f)\
t(k,j,k)\
y(u,l,j)\
s(o,r,o)\
w(v,c,c)\
e(f,i,f)\
A(z,k,c,k)\
y(f,g,g)\
e(f,d,f)\
s(q,k,k)\
e(u,g,d)\
e(u,f,c)\
p(d,g,d)\
p(c,f,c)\
e(q,d,f)\
e(q,c,g)\
e(k,o,k)\
p(f,d,f)\
p(g,c,g)\
y(k,l,l)\
e(k,j,k)\
x(f,640(%[buf]))\
e(u,l,f)\
e(u,k,j)\
x(g,672(%[buf]))\
p(f,l,f)\
p(j,k,j)\
e(q,f,k)\
e(q,j,l)\
p(k,f,k)\
p(l,j,l)\
x(k,704(%[buf]))\
x(l,736(%[buf]))
	asm volatile(
		CP_FPS_ASM_BLOCK(%%ymm7,%%ymm11,CP_FPS_ASM_VPSUBD,%%ymm3,%%ymm8,%%ymm10,%%ymm6,%%ymm0,%%ymm5,%%ymm1,CP_FPS_ASM_VBROADCASTSS,%%ymm2,%%ymm4,CP_FPS_ASM_VPMINUD,%%ymm15,%%ymm9,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,%%ymm14,$32,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VPBLENDD,32(%[x]),32(%[y]),36(%[x]),36(%[y]),40(%[y]),44(%[y]),(%[x]),(%[y]),12(%[y]),4(%[x]),4(%[y]),8(%[y]),CP_FPS_ASM_VPBROADCASTD,192(%[a]),192(%[b]),224(%[a]),224(%[b]),64(%[a]),64(%[b]),96(%[a]),96(%[b]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

[[gnu::always_inline]] inline void last(void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39) \
C(%[P],q) C(%[P2],p) C(%[NI],t) k(256(%[cur]),l)\
k(260(%[cur]),c)\
k(268(%[cur]),n)\
k(264(%[cur]),e)\
a(X,c,g)\
a(36(%[cur]),l,i)\
a(Y,l,l)\
a(W,e,b)\
k(272(%[cur]),h)\
a(Y,c,c)\
a(N,n,j)\
a(X,e,e)\
k(276(%[cur]),f)\
a(W,n,n)\
d(g,l,g)\
d(c,i,c)\
d(b,g,b)\
k(280(%[cur]),g)\
d(j,b,j)\
d(e,c,e)\
d(n,e,n)\
a(N,h,e)\
a(L,h,h)\
k(284(%[cur]),c)\
a(E,f,b)\
a(L,f,f)\
d(e,n,e)\
d(h,j,h)\
a(_34,g,j)\
a(E,g,g)\
d(f,e,f)\
d(b,h,b)\
d(j,b,j)\
d(g,f,g)\
a(4(%[cur]),c,f)\
a(_34,c,c)\
d(f,j,f)\
d(c,g,c)\
k(288(%[cur]),g)\
a(t,f,j)\
a(t,c,b)\
k(292(%[cur]),h)\
a(q,j,j)\
a(q,b,b)\
d(j,f,j)\
d(b,c,b)\
k(296(%[cur]),c)\
s(r,j,j)\
a(100(%[cur]),g,f)\
a(_39,g,g)\
w(u,b,j,b)\
k(300(%[cur]),j)\
a(_39,h,e)\
o(p,b,n)\
a(_38,h,h)\
x(n,b,n)\
k(304(%[cur]),b)\
d(e,f,e)\
d(h,g,h)\
a(_37,c,g)\
a(_38,c,c)\
k(308(%[cur]),f)\
a(_36,j,i)\
a(_37,j,j)\
d(g,h,g)\
d(c,e,c)\
k(312(%[cur]),e)\
a(_35,b,h)\
a(_36,b,b)\
d(i,g,i)\
d(j,c,j)\
k(316(%[cur]),c)\
d(h,i,h)\
d(b,j,b)\
a(_35,f,j)\
a(_33,f,f)\
a(_32,e,i)\
d(f,h,f)\
a(_33,e,e)\
d(j,b,j)\
a(_32,c,b)\
a(68(%[cur]),c,c)\
d(i,f,i)\
d(e,j,e)\
d(c,i,c)\
d(b,e,b)\
k(320(%[cur]),e)\
a(t,c,i)\
a(t,b,j)\
k(324(%[cur]),f)\
a(q,i,i)\
a(q,j,j)\
d(j,b,j)\
d(i,c,i)\
k(328(%[cur]),c)\
s(r,i,i)\
a(M,e,b)\
a(164(%[cur]),e,e)\
w(u,j,i,j)\
k(332(%[cur]),i)\
a(K,f,h)\
o(p,j,g)\
a(M,f,f)\
x(g,j,g)\
k(336(%[cur]),j)\
d(h,b,h)\
d(f,e,f)\
a(J,c,e)\
a(K,c,c)\
k(340(%[cur]),b)\
a(I,i,l)\
a(J,i,i)\
d(e,h,e)\
d(c,f,c)\
k(344(%[cur]),f)\
a(I,j,h)\
a(H,j,j)\
d(l,e,l)\
d(i,c,i)\
k(348(%[cur]),c)\
d(h,i,h)\
d(j,l,j)\
a(G,b,l)\
a(H,b,b)\
a(F,f,i)\
a(G,f,f)\
d(b,h,b)\
d(l,j,l)\
a(132(%[cur]),c,j)\
a(F,c,c)\
d(i,l,i)\
d(f,b,f)\
d(j,i,j)\
d(c,f,c)\
k(352(%[cur]),f)\
a(t,j,i)\
a(t,c,b)\
k(356(%[cur]),l)\
a(q,i,i)\
a(q,b,b)\
d(i,j,i)\
d(b,c,b)\
k(360(%[cur]),c)\
s(r,i,i)\
a(V,f,j)\
a(228(%[cur]),f,f)\
w(u,b,i,b)\
k(364(%[cur]),i)\
a(U,l,h)\
o(p,b,e)\
a(V,l,l)\
x(e,b,e)\
k(368(%[cur]),b)\
d(h,j,h)\
d(l,f,l)\
a(T,c,f)\
a(U,c,c)\
k(372(%[cur]),j)\
a(T,i,m)\
a(S,i,i)\
d(c,l,c)\
d(f,h,f)\
k(376(%[cur]),h)\
a(R,b,l)\
d(i,f,i)\
a(S,b,b)\
d(m,c,m)\
k(380(%[cur]),c)\
d(l,i,l)\
d(b,m,b)\
a(Q,j,m)\
a(R,j,j)\
a(O,h,i)\
a(Q,h,h)\
d(m,l,m)\
d(j,b,j)\
a(196(%[cur]),c,b)\
a(O,c,c)\
d(i,m,i)\
d(h,j,h)\
d(b,i,b)\
d(c,h,c)\
a(t,b,h)\
a(t,c,i)\
v(g,n,j)\
o(p,j,m)\
x(m,j,m)\
a(q,h,h)\
a(q,i,i)\
v(p,n,n)\
k(Z,j)\
o(g,n,g)\
k(Z,n)\
s(r,g,l)\
d(h,b,h)\
d(i,c,i)\
s(r,h,h)\
k((%[iy]),c)\
w(u,i,h,i)\
o(p,i,h)\
v(p,e,b)\
x(h,i,h)\
o(h,b,b)\
v(h,e,h)\
k(_0,e)\
a(j,g,j)\
s(r,b,i)\
o(p,h,f)\
a(n,l,n)\
x(f,h,f)\
k(_0,h)\
k(4(%[iy]),l)\
s(r,j,j)\
y(c,g,c)\
w(u,n,j,n)\
v(f,m,j)\
y(q,n,n)\
o(p,j,g)\
x(g,j,g)\
z(g,(%[out]))\
a(e,b,e)\
o(n,c,n)\
a(h,i,h)\
v(p,m,m)\
k(A,i)\
y(l,b,l)\
o(f,m,f)\
s(r,e,e)\
k(A,m)\
s(r,f,b)\
w(u,h,e,h)\
k(D,e)\
y(q,h,h)\
o(h,l,h)\
v(h,n,l)\
v(p,n,n)\
a(i,f,i)\
o(p,l,c)\
k(A,g)\
o(h,n,h)\
a(m,b,m)\
x(c,l,c)\
k(A,l)\
s(r,h,b)\
y(e,f,e)\
z(c,32(%[out]))\
k(D,c)\
s(r,i,i)\
w(u,m,i,m)\
k(640(%[cur]),i)\
y(q,m,m)\
o(m,e,m)\
k(644(%[cur]),e)\
a(g,h,g)\
z(m,64(%[out]))\
a(l,b,l)\
y(c,h,c)\
k(648(%[cur]),h)\
s(r,g,g)\
a(_9,i,b)\
a(420(%[cur]),i,i)\
w(u,l,g,l)\
k(652(%[cur]),g)\
y(q,l,l)\
a(_8,e,m)\
a(_9,e,e)\
o(l,c,l)\
k(656(%[cur]),c)\
z(l,96(%[out]))\
a(_8,h,l)\
a(_7,h,h)\
d(e,i,e)\
d(m,b,m)\
k(660(%[cur]),b)\
a(_6,g,i)\
a(_7,g,g)\
d(h,m,h)\
d(l,e,l)\
k(664(%[cur]),e)\
d(i,h,i)\
d(g,l,g)\
a(_5,c,l)\
a(_6,c,c)\
k(668(%[cur]),h)\
a(_2,b,m)\
a(_5,b,b)\
d(l,i,l)\
d(c,g,c)\
a(_1,e,g)\
a(_2,e,e)\
d(m,l,m)\
d(b,c,b)\
a(388(%[cur]),h,c)\
a(_1,h,h)\
d(g,m,g)\
d(e,b,e)\
d(h,e,h)\
d(c,g,c)\
k(672(%[cur]),g)\
a(t,c,e)\
a(t,h,b)\
k(676(%[cur]),m)\
a(q,e,e)\
a(q,b,b)\
d(e,c,e)\
d(b,h,b)\
k(680(%[cur]),h)\
s(r,e,e)\
a(484(%[cur]),g,c)\
a(_17,g,g)\
w(u,b,e,b)\
k(684(%[cur]),e)\
a(_16,m,l)\
o(p,b,i)\
a(_17,m,m)\
x(i,b,i)\
k(688(%[cur]),b)\
d(l,g,l)\
d(m,c,m)\
a(_16,h,c)\
a(_15,h,h)\
k(692(%[cur]),g)\
a(_14,e,f)\
a(_15,e,e)\
d(c,m,c)\
d(h,l,h)\
k(696(%[cur]),l)\
a(_13,b,m)\
a(_14,b,b)\
d(f,h,f)\
d(e,c,e)\
k(700(%[cur]),c)\
d(m,f,m)\
d(b,e,b)\
a(_13,g,e)\
a(_12,g,g)\
a(_11,l,f)\
a(_12,l,l)\
d(g,m,g)\
d(e,b,e)\
a(452(%[cur]),c,b)\
a(_11,c,c)\
d(f,g,f)\
d(l,e,l)\
d(b,f,b)\
d(c,l,c)\
k(704(%[cur]),l)\
a(t,b,f)\
a(t,c,e)\
k(708(%[cur]),g)\
a(q,f,f)\
a(q,e,e)\
d(e,c,e)\
d(f,b,f)\
k(712(%[cur]),b)\
s(r,f,f)\
a(_24,l,c)\
a(548(%[cur]),l,l)\
w(u,e,f,e)\
k(716(%[cur]),f)\
a(_23,g,m)\
o(p,e,h)\
a(_24,g,g)\
x(h,e,h)\
k(720(%[cur]),e)\
d(m,c,m)\
d(g,l,g)\
a(_22,b,l)\
a(_23,b,b)\
k(724(%[cur]),c)\
a(_21,f,n)\
a(_22,f,f)\
d(l,m,l)\
d(b,g,b)\
k(728(%[cur]),g)\
a(_20,e,m)\
a(_21,e,e)\
d(n,l,n)\
d(f,b,f)\
k(732(%[cur]),b)\
d(m,n,m)\
d(e,f,e)\
a(_19,c,f)\
a(_20,c,c)\
a(_18,g,n)\
d(f,m,f)\
a(_19,g,g)\
d(c,e,c)\
a(516(%[cur]),b,e)\
a(_18,b,b)\
d(n,f,n)\
d(g,c,g)\
d(b,g,b)\
d(e,n,e)\
k(736(%[cur]),n)\
a(t,e,g)\
a(t,b,c)\
k(740(%[cur]),f)\
a(q,g,g)\
a(q,c,c)\
d(g,e,g)\
d(c,b,c)\
k(744(%[cur]),b)\
s(r,g,g)\
a(_31,n,e)\
a(612(%[cur]),n,n)\
w(u,c,g,c)\
k(748(%[cur]),g)\
a(_30,f,m)\
o(p,c,l)\
a(_31,f,f)\
x(l,c,l)\
k(752(%[cur]),c)\
d(m,e,m)\
d(f,n,f)\
a(_29,b,n)\
a(_30,b,b)\
k(756(%[cur]),e)\
a(_29,g,j)\
a(_28,g,g)\
d(n,m,n)\
d(b,f,b)\
k(760(%[cur]),f)\
a(_27,c,m)\
a(_28,c,c)\
d(g,n,g)\
d(j,b,j)\
k(764(%[cur]),b)\
d(m,g,m)\
d(c,j,c)\
a(_26,e,j)\
a(_27,e,e)\
a(_25,f,g)\
a(_26,f,f)\
d(j,m,j)\
d(e,c,e)\
a(580(%[cur]),b,c)\
a(_25,b,b)\
d(f,e,f)\
d(g,j,g)\
d(c,g,c)\
d(b,f,b)\
a(t,c,f)\
a(t,b,g)\
v(h,i,j)\
o(p,j,e)\
x(e,j,e)\
a(q,g,g)\
a(q,f,f)\
v(p,i,i)\
k(_4,j)\
o(h,i,h)\
k(_4,i)\
s(r,h,m)\
d(f,c,f)\
d(g,b,g)\
s(r,f,f)\
k(8(%[iy]),b)\
w(u,g,f,g)\
o(p,g,f)\
v(p,l,c)\
x(f,g,f)\
o(f,c,c)\
v(f,l,f)\
k(_10,l)\
a(j,h,j)\
s(r,c,g)\
o(p,f,n)\
a(i,m,i)\
x(n,f,n)\
k(_10,f)\
k(12(%[iy]),m)\
s(r,j,j)\
y(b,h,b)\
w(u,i,j,i)\
v(n,e,j)\
y(q,i,i)\
o(p,j,h)\
x(h,j,h)\
z(h,128(%[out]))\
a(l,c,l)\
o(i,b,i)\
a(f,g,f)\
v(p,e,e)\
k(B,g)\
y(m,c,m)\
o(n,e,n)\
s(r,l,l)\
k(B,e)\
s(r,n,c)\
w(u,f,l,f)\
k(_3,l)\
y(q,f,f)\
o(f,m,f)\
v(f,i,m)\
v(p,i,i)\
a(g,n,g)\
o(p,m,b)\
k(B,h)\
o(f,i,f)\
a(e,c,e)\
x(b,m,b)\
k(B,m)\
s(r,f,c)\
y(l,n,l)\
z(b,160(%[out]))\
k(_3,b)\
s(r,g,g)\
w(u,e,g,e)\
y(q,e,e)\
o(e,l,e)\
a(h,f,h)\
z(e,192(%[out]))\
a(m,c,m)\
y(b,f,b)\
s(r,h,h)\
w(u,m,h,m)\
y(q,m,m)\
o(m,b,m)\
z(m,224(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm6,%%ymm1,CP_FPS_ASM_VPADDQ,%%ymm3,%%ymm9,%%ymm4,%%ymm7,%%ymm5,%%ymm8,CP_FPS_ASM_VBROADCASTSS,%%ymm0,%%ymm10,%%ymm2,CP_FPS_ASM_VPSUBD,%%ymm14,%%ymm15,$32,CP_FPS_ASM_VPSRLQ,%%ymm13,$170,CP_FPS_ASM_VPADDD,CP_FPS_ASM_VPBLENDD,CP_FPS_ASM_VPMINUD,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VMOVDQA,32(%[ix]),36(%[ix]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),12(%[cur]),136(%[cur]),140(%[cur]),144(%[cur]),148(%[cur]),152(%[cur]),156(%[cur]),16(%[cur]),160(%[cur]),20(%[cur]),200(%[cur]),204(%[cur]),208(%[cur]),212(%[cur]),216(%[cur]),220(%[cur]),224(%[cur]),24(%[cur]),28(%[cur]),32(%[cur]),32(%[iy]),36(%[iy]),392(%[cur]),396(%[cur]),4(%[ix]),40(%[iy]),400(%[cur]),404(%[cur]),408(%[cur]),412(%[cur]),416(%[cur]),44(%[iy]),456(%[cur]),460(%[cur]),464(%[cur]),468(%[cur]),472(%[cur]),476(%[cur]),480(%[cur]),520(%[cur]),524(%[cur]),528(%[cur]),532(%[cur]),536(%[cur]),540(%[cur]),544(%[cur]),584(%[cur]),588(%[cur]),592(%[cur]),596(%[cur]),600(%[cur]),604(%[cur]),608(%[cur]),72(%[cur]),76(%[cur]),8(%[cur]),80(%[cur]),84(%[cur]),88(%[cur]),92(%[cur]),96(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

[[gnu::always_inline]] inline void both(const void* a, const void* b, void* buf, const void* x, const void* y, void* out, const void* cur, const void* ix, const void* iy) {
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U,V,W,X,Y,Z,_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,_33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,_49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63) \
T(%[P],t) T(%[P2],v) T(%[NI],D) l(E,p)\
l(E,h)\
B(32(%[a]),m)\
l(K,g)\
l(E,d)\
l(256(%[cur]),n)\
l(E,j)\
l(K,k)\
B((%[a]),e)\
l(260(%[cur]),i)\
c(_61,p,p)\
c(100(%[a]),h,h)\
s(v,m,q)\
x(_61,g,g)\
c(_52,d,d)\
u(q,m,q)\
c(_18,n,m)\
c(36(%[cur]),n,n)\
y(w,p,p)\
l(264(%[cur]),f)\
C(A,h,p,h)\
c(68(%[a]),j,j)\
s(v,e,p)\
c(_17,i,o)\
c(_18,i,i)\
y(w,d,d)\
u(p,e,p)\
x(t,h,h)\
z(v,q,e)\
x(_52,k,k)\
C(A,j,d,j)\
l(268(%[cur]),d)\
r(o,m,o)\
r(i,n,i)\
x(t,j,j)\
l(F,n)\
s(h,g,h)\
l(272(%[cur]),g)\
z(h,q,q)\
s(h,e,h)\
l(F,e)\
y(w,q,m)\
s(j,k,j)\
c(_16,f,k)\
c(_17,f,f)\
r(k,o,k)\
r(f,i,f)\
c(_6,d,i)\
c(_16,d,d)\
l(276(%[cur]),o)\
c(n,q,n)\
r(i,k,i)\
r(d,f,d)\
c(_2,g,f)\
c(_6,g,g)\
l(L,k)\
c(e,m,e)\
z(j,p,m)\
s(j,p,j)\
l(280(%[cur]),p)\
y(w,n,n)\
r(f,i,f)\
r(g,d,g)\
l(H,d)\
l(H,i)\
C(A,e,n,e)\
l(284(%[cur]),n)\
x(t,e,e)\
x(k,q,k)\
c(V,o,q)\
c(_2,o,o)\
c(d,h,d)\
r(q,f,q)\
s(e,k,e)\
c(_56,p,k)\
c(V,p,p)\
c(4(%[cur]),n,f)\
c(_56,n,n)\
r(o,g,o)\
y(w,h,g)\
c(i,g,i)\
y(w,d,d)\
l(R,g)\
r(k,q,k)\
r(p,o,p)\
s(v,m,o)\
z(v,j,q)\
r(f,k,f)\
r(n,p,n)\
u(o,m,o)\
u(q,j,q)\
c(D,f,j)\
c(D,n,m)\
C(A,i,d,i)\
z(v,o,d)\
l(288(%[cur]),p)\
x(t,i,i)\
z(v,q,k)\
s(e,d,d)\
z(e,o,e)\
l(292(%[cur]),o)\
c(t,j,j)\
c(t,m,m)\
x(g,h,g)\
s(v,e,h)\
r(j,f,j)\
r(m,n,m)\
u(h,e,h)\
l(F,e)\
l(296(%[cur]),n)\
y(w,j,j)\
l(F,f)\
C(A,m,j,m)\
s(i,g,i)\
s(t,h,g)\
c(_63,p,j)\
c(100(%[cur]),p,p)\
s(i,k,k)\
z(i,q,i)\
l(300(%[cur]),q)\
u(g,h,g)\
c(_60,o,h)\
c(_63,o,o)\
B(g,32(%[buf]))\
r(h,j,h)\
r(o,p,o)\
c(e,g,e)\
s(v,m,p)\
l(L,j)\
u(p,m,p)\
l(304(%[cur]),m)\
y(w,e,e)\
x(j,g,j)\
y(w,g,g)\
c(f,g,f)\
c(_59,n,g)\
c(_60,n,n)\
r(g,h,g)\
C(A,f,e,f)\
l(308(%[cur]),e)\
c(_58,q,h)\
c(_59,q,q)\
r(n,o,n)\
x(t,f,f)\
s(v,d,o)\
u(o,d,o)\
l(312(%[cur]),d)\
r(h,g,h)\
c(_57,m,g)\
c(_58,m,m)\
r(q,n,q)\
s(f,j,f)\
l(F,j)\
l(F,n)\
r(g,h,g)\
r(m,q,m)\
s(t,o,q)\
s(t,f,h)\
u(q,o,q)\
u(h,f,h)\
l(316(%[cur]),f)\
c(_55,e,o)\
c(_57,e,e)\
B(h,(%[buf]))\
l(L,h)\
B(q,96(%[buf]))\
r(o,g,o)\
r(e,m,e)\
c(_54,d,m)\
c(_55,d,d)\
y(w,q,g)\
c(j,q,j)\
c(n,g,n)\
r(m,o,m)\
r(d,e,d)\
c(68(%[cur]),f,e)\
c(_54,f,f)\
y(w,j,j)\
s(v,i,o)\
C(A,n,j,n)\
u(o,i,o)\
x(h,q,h)\
x(t,n,n)\
l(320(%[cur]),q)\
r(e,m,e)\
r(f,d,f)\
s(t,o,d)\
c(D,e,m)\
c(D,f,i)\
u(d,o,d)\
l(324(%[cur]),o)\
l(H,j)\
y(w,d,g)\
s(n,h,n)\
B(d,160(%[buf]))\
l(H,h)\
c(t,m,m)\
c(t,i,i)\
r(m,e,m)\
r(i,f,i)\
l(328(%[cur]),f)\
s(t,n,e)\
y(w,m,m)\
u(e,n,e)\
c(_3,q,n)\
c(164(%[cur]),q,q)\
C(A,i,m,i)\
s(e,t,e)\
l(R,m)\
c(j,d,j)\
B(e,64(%[buf]))\
l(332(%[cur]),e)\
c(h,g,h)\
c(_1,o,g)\
c(_3,o,o)\
y(w,j,j)\
r(g,n,g)\
C(A,h,j,h)\
l(336(%[cur]),j)\
s(v,i,n)\
r(o,q,o)\
u(n,i,n)\
x(t,h,h)\
c(_0,f,i)\
c(_1,f,f)\
s(v,k,q)\
u(q,k,q)\
x(m,d,m)\
l(340(%[cur]),d)\
c(Z,e,k)\
c(_0,e,e)\
r(i,g,i)\
s(t,q,g)\
r(f,o,f)\
u(g,q,g)\
l(H,q)\
l(344(%[cur]),o)\
s(h,m,h)\
B(g,224(%[buf]))\
r(k,i,k)\
r(e,f,e)\
l(H,f)\
y(w,g,i)\
s(t,h,m)\
u(m,h,m)\
l(348(%[cur]),h)\
B(m,128(%[buf]))\
c(Y,j,m)\
c(Z,j,j)\
r(m,k,m)\
r(j,e,j)\
c(X,d,e)\
c(Y,d,d)\
l(R,k)\
c(q,g,q)\
r(e,m,e)\
r(d,j,d)\
c(f,i,f)\
c(W,o,i)\
c(X,o,o)\
c(132(%[cur]),h,j)\
y(w,q,q)\
c(W,h,h)\
B((%[b]),m)\
r(i,e,i)\
C(A,f,q,f)\
r(o,d,o)\
r(j,i,j)\
x(t,f,f)\
r(h,o,h)\
c(D,j,o)\
B(32(%[b]),i)\
l(352(%[cur]),d)\
c(D,h,q)\
x(k,g,k)\
l(E,g)\
c(t,o,o)\
c(t,q,q)\
l(E,e)\
s(f,k,f)\
l(356(%[cur]),k)\
r(o,j,o)\
s(v,m,j)\
r(q,h,q)\
y(w,o,o)\
u(j,m,j)\
s(t,f,m)\
l(360(%[cur]),h)\
C(A,q,o,q)\
l(K,o)\
u(m,f,m)\
c(_15,d,f)\
c(228(%[cur]),d,d)\
s(m,t,m)\
c(_53,g,g)\
B(m,192(%[buf]))\
s(v,q,m)\
u(m,q,m)\
l(364(%[cur]),q)\
c(68(%[b]),e,e)\
y(w,g,g)\
C(A,e,g,e)\
c(_12,k,g)\
c(_15,k,k)\
x(_53,o,o)\
x(t,e,e)\
r(g,f,g)\
r(k,d,k)\
l(368(%[cur]),d)\
c(_11,h,f)\
c(_12,h,h)\
s(e,o,e)\
c(_10,q,o)\
c(_11,q,q)\
r(f,g,f)\
l(E,g)\
r(h,k,h)\
r(o,f,o)\
l(372(%[cur]),f)\
l(E,k)\
r(q,h,q)\
l(K,h)\
c(_62,g,g)\
c(100(%[b]),k,k)\
x(_62,h,h)\
y(w,g,g)\
C(A,k,g,k)\
l(376(%[cur]),g)\
x(t,k,k)\
s(k,h,k)\
c(_9,d,h)\
c(_10,d,d)\
r(h,o,h)\
r(d,q,d)\
c(_8,f,q)\
c(_9,f,f)\
l(380(%[cur]),o)\
r(q,h,q)\
r(f,d,f)\
c(_7,g,d)\
c(_8,g,g)\
l(F,h)\
r(d,q,d)\
r(g,f,g)\
l(F,f)\
s(v,i,q)\
u(q,i,q)\
z(e,j,i)\
s(e,j,e)\
z(v,q,j)\
z(k,q,q)\
s(k,j,k)\
c(196(%[cur]),o,j)\
c(_7,o,o)\
r(j,d,j)\
r(o,g,o)\
c(h,q,h)\
l(L,g)\
y(w,q,d)\
c(f,d,f)\
c(D,j,d)\
y(w,h,h)\
c(t,d,d)\
C(A,f,h,f)\
c(D,o,h)\
x(t,f,f)\
c(t,h,h)\
r(d,j,d)\
x(g,q,g)\
l(H,q)\
l(H,j)\
y(w,d,d)\
r(h,o,h)\
s(f,g,f)\
z(v,p,g)\
y(w,k,o)\
C(A,h,d,h)\
s(n,g,g)\
z(n,p,n)\
l(R,p)\
s(v,i,d)\
u(d,i,d)\
l(_19,i)\
c(q,k,q)\
c(j,o,j)\
l(_19,o)\
y(w,q,q)\
C(A,j,q,j)\
x(p,k,p)\
s(v,h,k)\
y(w,g,q)\
u(k,h,k)\
c(i,g,i)\
x(t,j,j)\
l((%[iy]),h)\
c(o,q,o)\
y(w,i,i)\
s(j,p,j)\
s(v,n,p)\
z(v,m,q)\
z(k,m,m)\
u(p,n,p)\
z(v,e,n)\
s(k,q,k)\
u(n,e,n)\
l(_20,e)\
s(v,m,q)\
C(A,o,i,o)\
u(q,m,q)\
l(_20,m)\
y(w,k,i)\
x(t,o,o)\
x(h,g,h)\
z(v,d,g)\
z(f,d,d)\
s(f,g,f)\
l(4(%[iy]),g)\
s(o,h,o)\
z(v,n,h)\
z(j,n,n)\
s(j,h,j)\
s(v,d,h)\
c(e,k,e)\
u(h,d,h)\
s(v,f,d)\
c(m,i,m)\
u(d,f,d)\
s(v,n,f)\
z(q,p,i)\
u(f,n,f)\
s(v,j,n)\
y(w,e,e)\
u(n,j,n)\
s(t,h,j)\
C(A,m,e,m)\
x(g,k,g)\
u(j,h,j)\
s(t,d,h)\
x(t,m,m)\
z(v,p,p)\
s(t,f,k)\
s(v,i,e)\
B(j,256(%[buf]))\
B(128(%[a]),j)\
s(q,p,q)\
s(t,n,p)\
u(h,d,h)\
u(k,f,k)\
B(160(%[a]),f)\
y(w,q,d)\
u(e,i,e)\
u(p,n,p)\
B(h,288(%[buf]))\
l(G,h)\
l(N,n)\
s(m,g,m)\
B(e,(%[out]))\
l(G,e)\
l(N,g)\
z(m,o,i)\
B(k,320(%[buf]))\
l(U,k)\
B(p,352(%[buf]))\
z(v,o,o)\
s(v,i,p)\
s(m,o,m)\
u(p,i,p)\
l(Q,i)\
l(N,o)\
B(p,32(%[out]))\
s(v,j,p)\
u(p,j,p)\
s(v,f,j)\
c(_4,h,h)\
u(j,f,j)\
c(n,q,n)\
l(N,f)\
c(196(%[a]),e,e)\
c(g,d,g)\
l(G,d)\
x(k,q,k)\
l(G,q)\
y(w,h,h)\
y(w,n,n)\
C(A,e,h,e)\
x(_4,i,i)\
C(A,g,n,g)\
l(U,n)\
y(w,m,h)\
x(t,e,e)\
c(o,m,o)\
x(t,g,g)\
s(e,i,e)\
c(f,h,f)\
y(w,o,o)\
l(640(%[cur]),h)\
l(Q,i)\
c(_13,d,d)\
s(g,k,g)\
l(644(%[cur]),k)\
c(228(%[a]),q,q)\
B(g,64(%[out]))\
z(e,p,g)\
s(e,p,e)\
C(A,f,o,f)\
l(648(%[cur]),o)\
s(v,g,p)\
y(w,d,d)\
u(p,g,p)\
x(n,m,n)\
x(t,f,f)\
C(A,q,d,q)\
z(v,j,d)\
z(v,e,m)\
l(652(%[cur]),g)\
x(t,q,q)\
u(m,e,m)\
l(I,e)\
s(f,n,f)\
c(_29,h,n)\
c(420(%[cur]),h,h)\
B(f,96(%[out]))\
l(656(%[cur]),f)\
x(_13,i,i)\
s(q,i,q)\
c(_28,k,i)\
c(_29,k,k)\
z(q,j,j)\
s(q,d,q)\
c(_27,o,d)\
c(_28,o,o)\
r(i,n,i)\
r(k,h,k)\
c(e,j,e)\
l(660(%[cur]),h)\
c(_26,g,n)\
c(_27,g,g)\
r(d,i,d)\
r(o,k,o)\
l(I,k)\
y(w,j,i)\
r(n,d,n)\
y(w,e,e)\
l(664(%[cur]),d)\
r(g,o,g)\
c(_25,f,o)\
c(_26,f,f)\
r(o,n,o)\
r(f,g,f)\
l(J,g)\
l(J,n)\
c(k,i,k)\
l(668(%[cur]),i)\
C(A,k,e,k)\
c(_22,h,e)\
c(_25,h,h)\
x(t,k,k)\
r(e,o,e)\
r(h,f,h)\
c(g,q,g)\
c(_21,d,f)\
l(S,o)\
c(_22,d,d)\
r(f,e,f)\
y(w,g,g)\
c(388(%[cur]),i,e)\
c(_21,i,i)\
r(d,h,d)\
l(M,h)\
r(e,f,e)\
r(i,d,i)\
y(w,q,d)\
z(v,p,f)\
c(n,d,n)\
c(D,e,d)\
x(o,j,o)\
c(D,i,j)\
c(t,d,d)\
C(A,n,g,n)\
l(672(%[cur]),g)\
c(t,j,j)\
x(t,n,n)\
s(k,o,k)\
z(v,m,o)\
x(h,q,h)\
s(k,f,f)\
z(k,p,k)\
l(676(%[cur]),p)\
r(d,e,d)\
s(v,k,e)\
r(j,i,j)\
y(w,d,d)\
l(I,i)\
u(e,k,e)\
l(680(%[cur]),k)\
C(A,j,d,j)\
s(n,h,n)\
l(I,h)\
s(t,e,d)\
s(v,j,q)\
s(n,o,o)\
z(n,m,n)\
u(d,e,d)\
l(684(%[cur]),e)\
u(q,j,q)\
l(S,j)\
y(w,d,m)\
B(d,416(%[buf]))\
c(i,d,i)\
c(h,m,h)\
c(_37,g,m)\
c(484(%[cur]),g,g)\
y(w,i,i)\
x(j,d,j)\
l(688(%[cur]),d)\
C(A,h,i,h)\
c(_36,p,i)\
c(_37,p,p)\
x(t,h,h)\
r(i,m,i)\
r(p,g,p)\
c(_35,k,g)\
c(_36,k,k)\
l(692(%[cur]),m)\
s(h,j,h)\
c(_34,e,j)\
c(_35,e,e)\
r(g,i,g)\
r(k,p,k)\
s(v,f,p)\
s(t,h,i)\
u(p,f,p)\
u(i,h,i)\
l(696(%[cur]),h)\
c(_33,d,f)\
c(_34,d,d)\
r(j,g,j)\
r(e,k,e)\
B(i,384(%[buf]))\
l(I,i)\
s(t,p,k)\
l(700(%[cur]),g)\
u(k,p,k)\
r(f,j,f)\
l(I,j)\
l(S,p)\
B(k,480(%[buf]))\
r(d,e,d)\
y(w,k,e)\
c(i,k,i)\
c(j,e,j)\
x(p,k,p)\
c(_32,m,k)\
c(_33,m,m)\
y(w,i,i)\
c(_31,h,e)\
c(_32,h,h)\
C(A,j,i,j)\
c(452(%[cur]),g,i)\
c(_31,g,g)\
r(k,f,k)\
r(m,d,m)\
x(t,j,j)\
s(v,n,d)\
r(e,k,e)\
r(h,m,h)\
u(d,n,d)\
l(704(%[cur]),n)\
r(i,e,i)\
r(g,h,g)\
s(t,d,h)\
c(D,i,e)\
c(D,g,m)\
u(h,d,h)\
s(j,p,j)\
l(708(%[cur]),p)\
l(J,d)\
y(w,h,k)\
s(t,j,f)\
B(h,544(%[buf]))\
u(f,j,f)\
l(J,j)\
c(t,e,e)\
c(t,m,m)\
s(f,t,f)\
B(f,448(%[buf]))\
l(712(%[cur]),f)\
r(e,i,e)\
r(m,g,m)\
l(M,g)\
c(_44,n,i)\
c(548(%[cur]),n,n)\
y(w,e,e)\
C(A,m,e,m)\
l(716(%[cur]),e)\
c(d,h,d)\
c(j,k,j)\
c(_43,p,k)\
c(_44,p,p)\
y(w,d,d)\
r(k,i,k)\
C(A,j,d,j)\
l(720(%[cur]),d)\
c(_42,f,i)\
c(_43,f,f)\
r(p,n,p)\
x(g,h,g)\
s(v,m,h)\
u(h,m,h)\
s(v,o,m)\
x(t,j,j)\
l(724(%[cur]),n)\
r(i,k,i)\
u(m,o,m)\
c(_41,e,o)\
c(_42,e,e)\
r(f,p,f)\
s(t,m,p)\
l(J,k)\
u(p,m,p)\
s(j,g,j)\
l(728(%[cur]),g)\
l(J,m)\
B(p,608(%[buf]))\
r(o,i,o)\
r(e,f,e)\
y(w,p,f)\
s(t,j,i)\
u(i,j,i)\
l(732(%[cur]),j)\
B(i,512(%[buf]))\
c(_40,d,i)\
c(_41,d,d)\
r(i,o,i)\
r(d,e,d)\
c(_39,n,e)\
c(_40,n,n)\
l(M,o)\
c(k,p,k)\
c(m,f,m)\
c(_38,g,f)\
c(_39,g,g)\
r(e,i,e)\
r(n,d,n)\
c(516(%[cur]),j,d)\
c(_38,j,j)\
y(w,k,k)\
r(f,e,f)\
B(128(%[b]),e)\
C(A,m,k,m)\
r(g,n,g)\
x(t,m,m)\
r(d,f,d)\
r(j,g,j)\
B(160(%[b]),g)\
c(D,d,f)\
c(D,j,n)\
l(736(%[cur]),k)\
l(G,i)\
x(o,p,o)\
c(t,f,f)\
c(t,n,n)\
l(740(%[cur]),p)\
s(m,o,m)\
l(G,o)\
r(f,d,f)\
r(n,j,n)\
s(v,e,j)\
s(t,m,d)\
y(w,f,f)\
u(j,e,j)\
u(d,m,d)\
s(v,g,m)\
l(744(%[cur]),e)\
C(A,n,f,n)\
l(Q,f)\
u(m,g,m)\
s(d,t,d)\
c(_51,k,g)\
c(612(%[cur]),k,k)\
B(d,576(%[buf]))\
l(748(%[cur]),d)\
c(_5,i,i)\
c(196(%[b]),o,o)\
y(w,i,i)\
C(A,o,i,o)\
x(_5,f,f)\
c(_50,p,i)\
c(_51,p,p)\
x(t,o,o)\
r(i,g,i)\
l(752(%[cur]),g)\
r(p,k,p)\
s(o,f,o)\
c(_49,e,f)\
c(_50,e,e)\
c(_48,d,k)\
c(_49,d,d)\
r(f,i,f)\
r(e,p,e)\
l(G,p)\
l(G,i)\
r(k,f,k)\
r(d,e,d)\
l(756(%[cur]),e)\
l(Q,f)\
c(_14,p,p)\
c(228(%[b]),i,i)\
x(_14,f,f)\
y(w,p,p)\
C(A,i,p,i)\
l(760(%[cur]),p)\
x(t,i,i)\
s(i,f,i)\
c(_47,g,f)\
c(_48,g,g)\
r(f,k,f)\
r(g,d,g)\
c(_46,e,d)\
c(_47,e,e)\
l(764(%[cur]),k)\
r(d,f,d)\
r(e,g,e)\
c(_45,p,g)\
c(_46,p,p)\
s(v,n,f)\
u(f,n,f)\
l(I,n)\
r(g,d,g)\
r(p,e,p)\
l(I,e)\
z(v,m,d)\
z(i,m,m)\
s(i,d,i)\
y(w,m,d)\
c(n,m,n)\
c(e,d,e)\
c(580(%[cur]),k,d)\
c(_45,k,k)\
y(w,n,n)\
r(d,g,d)\
C(A,e,n,e)\
l(S,n)\
z(o,j,g)\
s(o,j,o)\
r(k,p,k)\
c(D,d,p)\
y(w,i,j)\
x(t,e,e)\
c(t,p,p)\
r(p,d,p)\
c(D,k,d)\
y(w,p,p)\
x(n,m,n)\
l(J,m)\
c(t,d,d)\
s(e,n,e)\
z(v,q,n)\
z(h,q,q)\
s(h,n,h)\
l(J,n)\
r(d,k,d)\
y(w,h,k)\
C(A,d,p,d)\
l(_24,p)\
c(m,i,m)\
y(w,m,m)\
c(n,j,n)\
l(_24,j)\
c(p,h,p)\
C(A,n,m,n)\
s(v,d,m)\
u(m,d,m)\
x(t,n,n)\
l(M,d)\
y(w,p,p)\
c(j,k,j)\
l(8(%[iy]),k)\
C(A,j,p,j)\
s(v,q,p)\
x(t,j,j)\
u(p,q,p)\
x(d,i,d)\
z(m,f,i)\
z(v,f,f)\
s(v,i,q)\
s(m,f,m)\
u(q,i,q)\
l(_30,i)\
s(v,g,f)\
u(f,g,f)\
s(n,d,n)\
l(_30,d)\
y(w,m,g)\
x(k,h,k)\
z(v,o,h)\
u(h,o,h)\
z(v,f,o)\
z(e,f,f)\
s(e,o,e)\
l(12(%[iy]),o)\
s(j,k,j)\
z(v,h,k)\
z(n,h,h)\
s(n,k,n)\
s(v,f,k)\
u(k,f,k)\
s(v,e,f)\
c(i,m,i)\
u(f,e,f)\
s(v,h,e)\
c(d,g,d)\
u(e,h,e)\
z(q,p,h)\
s(t,k,g)\
u(g,k,g)\
s(v,n,k)\
y(w,i,i)\
u(k,n,k)\
B(g,640(%[buf]))\
s(t,f,g)\
s(t,e,n)\
C(A,d,i,d)\
x(o,m,o)\
z(v,p,p)\
u(g,f,g)\
l(O,f)\
l(O,m)\
x(t,d,d)\
s(q,p,q)\
s(v,h,p)\
s(t,k,i)\
B(g,672(%[buf]))\
l(_23,g)\
u(n,e,n)\
u(p,h,p)\
u(i,k,i)\
z(v,j,k)\
y(w,q,h)\
B(p,128(%[out]))\
l(O,p)\
l(O,e)\
s(d,o,d)\
B(n,704(%[buf]))\
l(_23,n)\
s(d,k,k)\
z(d,j,d)\
B(i,736(%[buf]))\
y(w,k,i)\
s(v,d,j)\
u(j,d,j)\
B(j,160(%[out]))\
c(f,q,f)\
c(m,h,m)\
x(g,q,g)\
c(p,k,p)\
c(e,i,e)\
y(w,f,f)\
C(A,m,f,m)\
x(n,k,n)\
x(t,m,m)\
y(w,p,p)\
C(A,e,p,e)\
x(t,e,e)\
s(m,g,m)\
B(m,192(%[out]))\
s(e,n,e)\
B(e,224(%[out]))
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPMULUDQ,%%ymm4,%%ymm8,%%ymm11,%%ymm3,%%ymm1,%%ymm9,%%ymm6,%%ymm7,CP_FPS_ASM_VBROADCASTSS,%%ymm2,%%ymm5,%%ymm12,%%ymm0,%%ymm10,CP_FPS_ASM_VPADDQ,CP_FPS_ASM_VPSUBD,%%ymm15,CP_FPS_ASM_VPMINUD,%%ymm14,$32,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VPADDD,$170,CP_FPS_ASM_VMOVDQA,CP_FPS_ASM_VPBLENDD,%%ymm13,32(%[x]),32(%[y]),36(%[x]),36(%[y]),40(%[y]),44(%[y]),(%[x]),(%[y]),12(%[y]),32(%[ix]),36(%[ix]),4(%[x]),4(%[y]),8(%[y]),CP_FPS_ASM_VPBROADCASTD,(%[ix]),12(%[cur]),136(%[cur]),140(%[cur]),144(%[cur]),148(%[cur]),152(%[cur]),156(%[cur]),16(%[cur]),160(%[cur]),192(%[a]),192(%[b]),20(%[cur]),200(%[cur]),204(%[cur]),208(%[cur]),212(%[cur]),216(%[cur]),220(%[cur]),224(%[a]),224(%[b]),224(%[cur]),24(%[cur]),28(%[cur]),32(%[cur]),32(%[iy]),36(%[iy]),392(%[cur]),396(%[cur]),4(%[ix]),40(%[iy]),400(%[cur]),404(%[cur]),408(%[cur]),412(%[cur]),416(%[cur]),44(%[iy]),456(%[cur]),460(%[cur]),464(%[cur]),468(%[cur]),472(%[cur]),476(%[cur]),480(%[cur]),520(%[cur]),524(%[cur]),528(%[cur]),532(%[cur]),536(%[cur]),540(%[cur]),544(%[cur]),584(%[cur]),588(%[cur]),592(%[cur]),596(%[cur]),600(%[cur]),604(%[cur]),608(%[cur]),64(%[a]),64(%[b]),72(%[cur]),76(%[cur]),8(%[cur]),80(%[cur]),84(%[cur]),88(%[cur]),92(%[cur]),96(%[a]),96(%[b]),96(%[cur]))
#undef CP_FPS_ASM_BLOCK
		:
		: [a] "r"(a), [b] "r"(b), [buf] "r"(buf), [x] "r"(x), [y] "r"(y), [out] "r"(out), [cur] "r"(cur), [ix] "r"(ix), [iy] "r"(iy),
		  [P] "m"(kP), [P2] "m"(k2P), [NI] "m"(kNI)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory");
}

}
namespace top_kernels {

using ntt::kernels::k2P;
using ntt::kernels::kP;
using ntt::kernels::Vec;

inline void inverse_top(Vec* a, std::size_t q, const Vec* w) {
	Vec* b = a + 4 * q;
	Vec* const end = a + q;
	#define CP_FPS_ASM_BLOCK(c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,x,y,z,A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,Q,R,S,T,U) \
S(%[P],v) S(%[P2],j) ".p2align 5\n\t" "1:\n\t" \
A((%[b]),n)\
A(K,f)\
A((%[a]),g)\
A(J,t)\
o(j,n,e)\
o(I,n,n)\
c(I,e,e)\
o(j,f,d)\
c(j,n,r)\
o(H,f,f)\
z(u,e,h)\
y(L,e,m)\
c(H,d,d)\
o(j,g,k)\
y(L,h,h)\
z(u,d,s)\
y(M,d,i)\
o(j,t,p)\
y(M,s,s)\
x(128(%[w]),e,e)\
q(r,n,r)\
c(j,f,n)\
z(u,m,m)\
o(G,g,g)\
c(G,k,k)\
q(n,f,n)\
C(B,h,m,h)\
z(u,i,i)\
o(F,t,t)\
c(F,p,p)\
x(v,h,h)\
C(B,s,i,s)\
x(192(%[w]),d,d)\
z(u,g,i)\
x(v,s,s)\
y(D,g,m)\
z(u,k,f)\
z(u,t,l)\
y(D,i,i)\
y(D,f,f)\
y(D,l,l)\
c(h,e,h)\
x(E,g,g)\
z(u,p,e)\
y(Q,e,e)\
c(s,d,s)\
z(u,m,m)\
y(D,k,d)\
C(B,i,m,i)\
x(E,k,k)\
y(D,t,m)\
x(v,i,i)\
x(E,t,t)\
z(u,d,d)\
z(u,m,m)\
C(B,f,d,f)\
y(Q,p,d)\
x(64(%[w]),p,p)\
C(B,l,m,l)\
x(v,f,f)\
c(i,g,i)\
o(j,r,g)\
o(j,h,m)\
o(n,r,r)\
c(n,g,n)\
x(v,l,l)\
z(u,d,d)\
c(s,m,m)\
z(u,r,g)\
o(s,h,s)\
C(B,e,d,e)\
y(D,r,d)\
z(u,n,h)\
y(D,g,g)\
x(v,e,e)\
y(N,h,h)\
c(f,k,f)\
c(l,t,l)\
y(N,n,t)\
z(u,m,k)\
x(E,r,r)\
y(O,k,k)\
z(u,d,d)\
x(256(%[w]),n,n)\
c(e,p,e)\
C(B,g,d,g)\
y(O,m,d)\
o(l,i,p)\
z(u,t,t)\
c(l,i,l)\
x(v,g,g)\
x(320(%[w]),m,m)\
C(B,h,t,h)\
o(e,f,t)\
c(e,f,e)\
c(j,p,f)\
z(u,d,d)\
x(v,h,h)\
c(j,t,i)\
q(f,p,f)\
C(B,k,d,k)\
o(j,l,d)\
o(j,e,p)\
q(i,t,i)\
x(v,k,k)\
q(d,l,d)\
q(p,e,p)\
c(j,s,e)\
c(g,r,g)\
q(e,s,e)\
c(h,n,h)\
o(j,f,n)\
o(j,i,s)\
o(j,d,r)\
o(j,p,l)\
o(g,f,f)\
c(k,m,k)\
c(g,n,g)\
o(e,i,i)\
c(e,s,e)\
o(h,d,d)\
c(h,r,h)\
o(k,p,p)\
c(k,l,k)\
c(j,f,l)\
c(j,g,r)\
c(j,i,s)\
c(j,e,n)\
c(j,d,m)\
c(j,h,t)\
q(l,f,l)\
q(r,g,r)\
c(j,p,g)\
c(j,k,f)\
q(s,i,s)\
q(n,e,n)\
q(m,d,m)\
q(t,h,t)\
q(g,p,g)\
q(f,k,f)\
c(v,l,k)\
c(v,r,p)\
c(v,s,h)\
c(v,n,d)\
c(v,m,e)\
c(v,t,i)\
q(k,l,k)\
q(p,r,p)\
c(v,g,r)\
c(v,f,l)\
q(h,s,h)\
q(d,n,d)\
A(k,(%[a]))\
q(e,m,e)\
q(i,t,i)\
q(r,g,r)\
q(l,f,l)\
A(p,(%[b]))\
A(h,G)\
A(d,I)\
A(e,J)\
A(i,K)\
A(r,F)\
A(l,H)\
R(u,%[a])\
R(u,%[b])\
T(%[end],%[a])\
U(1b)
	asm volatile(
		CP_FPS_ASM_BLOCK(CP_FPS_ASM_VPSUBD,%%ymm5,%%ymm4,%%ymm1,%%ymm2,%%ymm7,%%ymm11,%%ymm14,%%ymm9,%%ymm13,%%ymm8,%%ymm0,CP_FPS_ASM_VPADDD,%%ymm12,CP_FPS_ASM_VPMINUD,%%ymm6,%%ymm10,%%ymm3,$32,%%ymm15,CP_FPS_ASM_VPMULLD,CP_FPS_ASM_VPMULUDQ,CP_FPS_ASM_VPSRLQ,CP_FPS_ASM_VMOVDQA,$170,CP_FPS_ASM_VPBLENDD,32(%[w]),(%[w]),(%[a],%[h3]),(%[a],%[h]),(%[b],%[h3]),(%[b],%[h]),(%[a],%[h],2),(%[b],%[h],2),160(%[w]),224(%[w]),288(%[w]),352(%[w]),96(%[w]),CP_FPS_ASM_ADD,CP_FPS_ASM_VPBROADCASTD,CP_FPS_ASM_CMP,CP_FPS_ASM_JNE)
#undef CP_FPS_ASM_BLOCK
		: [a] "+r"(a), [b] "+r"(b)
		: [end] "r"(end), [h] "r"(32 * q), [h3] "r"(96 * q), [w] "r"(w), [P] "m"(kP), [P2] "m"(k2P)
		: "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15", "memory", "cc");
}

}

using ntt::detail::add;
using ntt::detail::diff;
using ntt::detail::Factor;
using ntt::detail::kP;
using ntt::detail::reduce;
using ntt::detail::slot;
using ntt::detail::Vec;

inline Vec times(Vec x, const Factor& f) { return ntt::detail::multiply(x, f); }

inline Vec diff_canonical(Vec x, Vec y) {
	return _mm256_sub_epi32(_mm256_add_epi32(x, ntt::detail::broadcast(kP)), y);
}



inline void forward_radix8(Vec* f, std::size_t q, const std::uint32_t* roots) {
	const Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
	for (std::size_t j = 0; j < q; ++j) {
		const Vec f0 = f[j], f1 = f[j + q], f2 = f[j + 2 * q], f3 = f[j + 3 * q];
		
		const Vec g0 = add(f0, f2), g1 = add(f1, f3);
		const Vec h0 = diff_canonical(f0, f2), ih1 = times(diff_canonical(f1, f3), i);
		f[j] = add(g0, g1), f[j + q] = diff(g0, g1);
		f[j + 2 * q] = add(h0, ih1), f[j + 3 * q] = diff(h0, ih1);
		
		const Vec if2 = times(f2, i), if3 = times(f3, i);
		const Vec u0 = reduce(add(f0, if2), 2 * kP), v0 = reduce(diff(f0, if2), 2 * kP);
		const Vec yu1 = times(add(f1, if3), y), zv1 = times(diff(f1, if3), z);
		f[j + 4 * q] = add(u0, yu1), f[j + 5 * q] = diff(u0, yu1);
		f[j + 6 * q] = add(v0, zv1), f[j + 7 * q] = diff(v0, zv1);
	}
}



class Subtrees {
public:
	Subtrees(const std::uint32_t* roots, const std::uint32_t* inverse_roots) : r_(roots), ir_(inverse_roots) {}

	void visit(Vec* a, Vec* b, std::size_t nv, std::size_t k) const {
		switch (nv) {
		case 16: return tile<16>(a, b, k);
		case 64: return tile<64>(a, b, k);
		case 256: return tile<256>(a, b, k);
		}
		const std::size_t h = nv / 4;
		forward(a, b, h, k);
		for (std::size_t t = 0; t < 4; ++t) visit(a + t * h, b + t * h, h, 4 * k + t);
		inverse(a, h, k);
	}

private:
	template <std::size_t NV>
	[[gnu::noinline]] void tile(Vec* a, Vec* b, std::size_t k) const {
		for (std::size_t h = NV / 4; h >= 4; h /= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) forward(a + j, b + j, h, g);
		bottom(a, b, NV, k * (NV / 4));
		for (std::size_t h = 4; h < NV; h *= 4)
			for (std::size_t j = 0, g = k * (NV / (4 * h)); j < NV; j += 4 * h, ++g) inverse(a + j, h, g);
	}

	void forward(Vec* a, Vec* b, std::size_t h, std::size_t k) const {
		if (k == 0) {
			ntt::kernels::forward_identity(a, h, r_);
			ntt::kernels::forward_identity(b, h, r_);
			return;
		}
		const std::uint32_t *x = r_ + slot(k), *y = r_ + slot(2 * k);
		if (h == 4) return ntt::kernels::forward_pair(a, b, h, x, y);
		ntt::kernels::forward(a, h, x, y);
		ntt::kernels::forward(b, h, x, y);
	}

	void inverse(Vec* a, std::size_t h, std::size_t k) const {
		if (k == 0) return ntt::kernels::inverse_identity(a, h, ir_);
		ntt::kernels::inverse(a, h, ir_ + slot(k), ir_ + slot(2 * k));
	}

	struct alignas(64) Leaves {
		std::uint32_t window[4][16];
		std::uint32_t coefficients[4][8];
	};

	
	void bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {
		Leaves leaves[2][2]{};
		bottom_kernels::first(a, b, leaves[0], r_ + slot(first), r_ + slot(2 * first));
		for (std::size_t j = 0, k = first;; j += 8, k += 2) {
			const std::size_t cur = j / 8 % 2, next = cur ^ 1;
			const std::uint32_t *ix = ir_ + slot(k), *iy = ir_ + slot(2 * k);
			if (j + 8 == nv) return bottom_kernels::last(a + j, leaves[cur], ix, iy);
			bottom_kernels::both(a + j + 8, b + j + 8, leaves[next], r_ + slot(k + 2), r_ + slot(2 * k + 4), a + j,
								 leaves[cur], ix, iy);
		}
	}

	const std::uint32_t *r_, *ir_;
};


class Product {
public:
	static bool fits(std::size_t n, std::size_t m) {
		const int lg = log_length(n, m);
		return lg % 2 == 0 && lg >= 10 && 2 * std::max(n, m) <= std::size_t(1) << lg;
	}

	Product(std::size_t n, std::size_t m) : Product(n, m, nullptr) {}
	Product(std::size_t n, std::size_t m, std::uint32_t* borrowed_a) : lg_(log_length(n, m)) {
		const std::size_t len = length(), tables = ntt::detail::table_words(lg_);
		const std::size_t words = (borrowed_a ? 1 : 2) * (len + kPadding) + 2 * tables + (borrowed_a ? 1024 : 0);
		storage_ = allocate_fps_words(words);
		a_ = borrowed_a ? borrowed_a : storage_.get();
		const std::size_t color = ((reinterpret_cast<std::uintptr_t>(a_) >> 6) - (reinterpret_cast<std::uintptr_t>(storage_.get()) >> 6) + 1) & 63;
		b_ = storage_.get() + (borrowed_a ? 16 * color : len + kPadding);
		roots_ = b_ + len + kPadding;
		inverse_roots_ = roots_ + tables;
		std::fill(roots_, inverse_roots_ + tables, 0);
	}

	Product(const Product&) = delete;
	Product& operator=(const Product&) = delete;

	std::uint32_t* a() { return a_; }
	std::uint32_t* b() { return b_; }


	
	const std::uint32_t* multiply() {
		using namespace ntt::detail;
		const std::size_t len = length(), nv = len / 8, q = nv / 8;
		build_table(roots_, len / 16, kRoots[0]);
		build_table(inverse_roots_, len / 16, kRoots[1]);
		auto* a = reinterpret_cast<Vec*>(a_);
		auto* b = reinterpret_cast<Vec*>(b_);
		forward_radix8(a, q, roots_);
		forward_radix8(b, q, roots_);

		const Subtrees subtrees(roots_, inverse_roots_);
		for (std::size_t c = 0; c < 8; ++c) subtrees.visit(a + c * q, b + c * q, q, c);
		const std::uint32_t s = power(std::uint32_t(nv), kP - 2);
		const std::uint32_t *z0 = inverse_roots_ + 1, *x1 = inverse_roots_ + slot(1), *y1 = inverse_roots_ + slot(2);
		const std::uint32_t twiddles[6] = {s, multiply_mod(s, *z0), multiply_mod(s, y1[0]), multiply_mod(s, y1[1]),
										   multiply_mod(s, *x1), *x1};
		alignas(32) Vec w[12];
		for (int i = 0; i < 6; ++i) w[2 * i] = broadcast(twiddles[i]), w[2 * i + 1] = broadcast(quotient(twiddles[i]));
		top_kernels::inverse_top(a, q, w);
		return a_;
	}

private:
	static constexpr std::size_t kPadding = 16;

	static int log_length(std::size_t n, std::size_t m) { return std::max(6, int(std::bit_width(n + m - 2))); }
	std::size_t length() const { return std::size_t(1) << lg_; }

	int lg_;
	FpsWords storage_;
	std::uint32_t *a_, *b_, *roots_, *inverse_roots_;
};




inline void copy_input(const std::vector<Mint> &in, std::uint32_t *out, std::size_t len) {
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		const auto x = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(in.data() + i));
		_mm256_store_si256(reinterpret_cast<__m256i *>(out + i), _mm256_min_epu32(x, _mm256_sub_epi32(x, p)));
	}
	for (; i < in.size(); ++i) out[i] = std::min(in[i].v, in[i].v - Mint::mod());
	std::fill(out + i, out + (i <= len / 2 ? len / 2 : len), 0);
	std::fill(out + len, out + len + 16, 0);
}


inline void copy_input_mint(const std::vector<Mint>& in, Mint* out) {
	static_assert(sizeof(Mint) == sizeof(std::uint32_t) && std::is_trivially_copyable_v<Mint>);
	std::size_t i = 0;
	const auto p = _mm256_set1_epi32(Mint::mod());
	for (; i + 8 <= in.size(); i += 8) {
		__m256i x;
		std::memcpy(&x, in.data() + i, sizeof(x));
		x = _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
		std::memcpy(static_cast<void*>(out + i), &x, sizeof(x));
	}
	for (; i < in.size(); ++i) out[i] = Mint::word(std::min(in[i].v, in[i].v - Mint::mod()));
}

template <class Plan>
inline std::vector<Mint> run_borrowed_as(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	const std::size_t len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	constexpr std::size_t alignment = 32;
	const std::size_t words = len + 16 + alignment / sizeof(Mint) - 1, size = a.size() + b.size() - 1;
	std::vector<Mint> result;
	prepare_fps_output(result, words);
	result.assign(words, Mint::word(0));
	const auto address = reinterpret_cast<std::uintptr_t>(result.data());
	const std::size_t offset = ((alignment - (address & (alignment - 1))) & (alignment - 1)) / sizeof(Mint);
	Mint* const to = result.data() + offset;
	Plan plan(a.size(), b.size(), reinterpret_cast<std::uint32_t*>(to));
	copy_input_mint(a, to);
	copy_input(b, plan.b(), len);
	plan.multiply();
	if (offset) std::memmove(static_cast<void*>(result.data()), to, size * sizeof(Mint));
	result.resize(size);
	return result;
}

inline std::vector<Mint> run_borrowed(const std::vector<Mint>& a, const std::vector<Mint>& b) {
	return run_borrowed_as<Product>(a, b);
}

template <class Plan>
inline std::vector<Mint> run(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	Plan plan(a.size(), b.size());
	const auto len = std::size_t(1) << std::max(6, int(std::bit_width(a.size() + b.size() - 2)));
	copy_input(a, plan.a(), len);
	copy_input(b, plan.b(), len);
	const auto *p = plan.multiply();
	std::vector<Mint> result;
	const std::size_t size = a.size() + b.size() - 1;
	prepare_fps_output(result, size);
	append_fps_words<true>(result, p, size);
	return result;
}

inline std::vector<Mint> multiply(const std::vector<Mint> &a, const std::vector<Mint> &b) {
	assert(!a.empty() && !b.empty() && a.size() + b.size() - 1 <= (1 << 23));
	if (constant::fits(a, b)) return constant::multiply(a, b);
	if (unbalanced::fits(a.size(), b.size())) return unbalanced::multiply(a, b);
	if (Product::fits(a.size(), b.size())) return run_borrowed(a, b);
	return run_borrowed_as<ntt::Convolution>(a, b);
}

}
#pragma GCC pop_options
#endif

#endif
