from pathlib import Path
p=Path('/tmp/fps-avx512-top/evex/components/fps_blocked.hpp');s=p.read_text()
needle='[[gnu::noinline]] inline void forward_top4(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {'
new='''inline bool avx512_top_supported() {
	return __builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512dq") && __builtin_cpu_supports("avx512vl");
}
[[gnu::target("avx512f,avx512dq,avx512vl"), gnu::noinline]] inline void forward_top4_evex(Source in, Vec* out, std::size_t h, const std::uint32_t* roots) {
	forward_top4_body(in, out, h, roots);
}
[[gnu::target("avx512f,avx512dq,avx512vl"), gnu::noinline]] inline void forward_top2_evex(Source in, Vec* out, std::size_t h) {
	forward_top2_body(in, out, h);
}
'''
assert s.count(needle)==1;s=s.replace(needle,new+'\n'+needle+'\n\tif (h >= 32 && avx512_top_supported()) return forward_top4_evex(in, out, h, roots);')
needle='[[gnu::noinline]] inline void forward_top2(Source in, Vec* out, std::size_t h) {';assert s.count(needle)==1;s=s.replace(needle,needle+'\n\tif (h >= 32 && avx512_top_supported()) return forward_top2_evex(in, out, h);')
start=s.index('inline void inverse_top4(Vec*');end=s.index('\n}\n\n}',s.index('inline void inverse_top2(Vec*',start))+2
old=s[start:end]
new=old.replace('inline void inverse_top4(','[[gnu::target("avx512f,avx512dq,avx512vl"), gnu::noinline]] inline void inverse_top4_evex(').replace('inline void inverse_top2(','[[gnu::target("avx512f,avx512dq,avx512vl"), gnu::noinline]] inline void inverse_top2_evex(')
s=s[:start]+new+'\n\n'+s[start:]
needle='inline void inverse_top4(Vec* f, std::size_t h, Half output, const std::uint32_t* inverse_roots, const Factor& s) {';s=s.replace(needle,needle+'\n\tif (h >= 32 && avx512_top_supported()) return inverse_top4_evex(f, h, output, inverse_roots, s);')
needle='inline void inverse_top2(Vec* f, std::size_t h, Half output, std::uint32_t scale) {';s=s.replace(needle,needle+'\n\tif (h >= 32 && output != Half::kBoth && avx512_top_supported()) return inverse_top2_evex(f, h, output, scale);')
p.write_text(s)
