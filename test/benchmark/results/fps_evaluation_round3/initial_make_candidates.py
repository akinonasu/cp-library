from pathlib import Path
import shutil
W=Path('/tmp/fps-evaluation-round3')
B=W/'base'
for variant in ['evex','wide_chain']:
 C=W/variant
 if C.exists(): shutil.rmtree(C)
 shutil.copytree(B,C)
 p=C/'components/fps_taylor_blocked.hpp'
 s=p.read_text().replace('GCC target(\\\"avx2\\\")','GCC target(\\\"avx2,avx512f,avx512dq,avx512vl\\\")')
 if variant=='wide_chain':
  lo=s.index('struct Chain {');hi=s.index('\nclass TaylorShift',lo)
  s=s[:lo]+'''struct Chain {
	using Wide = __m512i;
	Wide x[2], base[2], base_odd[2], u, step;
	template <class X, class Base>
	Chain(X initial, Base base_of, std::uint32_t step_value) : u(_mm512_setzero_si512()), step(_mm512_set1_epi32(step_value)) {
		alignas(64) std::uint32_t initial_values[32], base_values[32];
		for (int s = 0; s < 32; ++s) initial_values[s] = initial(s), base_values[s] = base_of(s);
		for (int v = 0; v < 2; ++v) {
			x[v] = _mm512_load_si512(initial_values + 16 * v);
			base[v] = _mm512_load_si512(base_values + 16 * v);
			base_odd[v] = _mm512_srli_epi64(base[v], 32);
		}
	}
	[[gnu::always_inline]] void advance(Block& block, int t) {
		const auto ni = _mm512_set1_epi32(ntt::kernels::kNI), p = _mm512_set1_epi32(kP);
#pragma GCC unroll 2
		for (int v = 0; v < 2; ++v) {
			block[2 * v][t] = _mm512_castsi512_si256(x[v]);
			block[2 * v + 1][t] = _mm512_extracti64x4_epi64(x[v], 1);
			auto even = _mm512_mul_epu32(x[v], _mm512_add_epi32(base[v], u));
			auto odd = _mm512_mul_epu32(_mm512_srli_epi64(x[v], 32), _mm512_add_epi32(base_odd[v], u));
			even = _mm512_add_epi64(even, _mm512_mul_epu32(_mm512_mul_epu32(even, ni), p));
			odd = _mm512_add_epi64(odd, _mm512_mul_epu32(_mm512_mul_epu32(odd, ni), p));
			x[v] = _mm512_mask_blend_epi32(0xaaaa, _mm512_srli_epi64(even, 32), odd);
		}
		u = _mm512_add_epi32(u, step);
		u = _mm512_min_epu32(u, _mm512_sub_epi32(u, p));
	}
};''' + s[hi:]
 p.write_text(s)
 print(variant)
