#ifndef FPS_LEAF_IFMA_PROBE_HPP
#define FPS_LEAF_IFMA_PROBE_HPP
#include "components/fps_blocked.hpp"
namespace leaf_ifma_probe {
using namespace fps_detail::blocked_poly::detail;
template<int Groups=4>
[[gnu::target("avx512f,avx512ifma"),gnu::always_inline]] inline Vec core(const Window& window,const std::uint32_t* b){
 __m512i lows[Groups]{},highs[Groups]{};
#pragma GCC unroll 8
 for(int j=0;j<8;++j){
  const auto a=_mm512_cvtepu32_epi64(_mm256_loadu_si256(reinterpret_cast<const __m256i*>(window.word+8-j)));
  const auto x=_mm512_set1_epi64(static_cast<std::uint64_t>(b[j]));
  lows[j%Groups]=_mm512_madd52lo_epu64(lows[j%Groups],a,x);
  highs[j%Groups]=_mm512_madd52hi_epu64(highs[j%Groups],a,x);
 }
 auto lo=lows[0],hi=highs[0];
#pragma GCC unroll 4
 for(int g=1;g<Groups;++g){lo=_mm512_add_epi64(lo,lows[g]);hi=_mm512_add_epi64(hi,highs[g]);}
 const auto sum=_mm512_add_epi64(lo,_mm512_slli_epi64(hi,52));
 const auto q=_mm512_mul_epu32(sum,_mm512_set1_epi64(fps_detail::blocked_ntt::kernels::kNI));
 const auto qp=_mm512_mul_epu32(q,_mm512_set1_epi64(kP));
 const auto value=_mm512_srli_epi64(_mm512_add_epi64(sum,qp),32);
 const auto packed=_mm512_cvtepi64_epi32(value);
 return _mm256_min_epu32(packed,_mm256_sub_epi32(packed,_mm256_set1_epi32(2*kP)));
}
[[gnu::target("avx512f,avx512ifma"),gnu::noinline]] inline Vec call_g1(const Window& w,const std::uint32_t* b){return core<1>(w,b);}
[[gnu::target("avx512f,avx512ifma"),gnu::noinline]] inline Vec call_g2(const Window& w,const std::uint32_t* b){return core<2>(w,b);}
[[gnu::target("avx512f,avx512ifma"),gnu::noinline]] inline Vec call(const Window& w,const std::uint32_t* b){return core(w,b);}
[[gnu::target("avx2"),gnu::noinline]] inline Vec original_call(const Window& w,const std::uint32_t* b){return leaf_product(w,b);}
[[gnu::target("avx2"),gnu::noinline]] inline Vec dispatch(const Window& w,const std::uint32_t* b){
 if(__builtin_cpu_supports("avx512f")&&__builtin_cpu_supports("avx512ifma"))return call(w,b);
 return leaf_product(w,b);
}
[[gnu::target("avx2"),gnu::noinline]] inline Vec forced_fallback(const Window& w,const std::uint32_t* b){return leaf_product(w,b);}
}
#endif
