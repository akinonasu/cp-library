from pathlib import Path
p=Path('/tmp/fps-avx512-top/wide/components/fps_blocked.hpp');s=p.read_text();needle='inline void inverse_top4(Vec*';at=s.index(needle);s=s[:at]+Path('/tmp/fps-avx512-top/wide.hpp').read_text()+'\n'+s[at:]
needle='inline void inverse_top4(Vec* f, std::size_t h, Half output, const std::uint32_t* inverse_roots, const Factor& s) {';assert s.count(needle)==1;s=s.replace(needle,needle+'\n\tif (h >= 32 && h % 2 == 0 && avx512_top_supported()) return wide_top::inverse4(f, h, output, inverse_roots, s);')
needle='inline void inverse_top2(Vec* f, std::size_t h, Half output, std::uint32_t scale) {';assert s.count(needle)==1;s=s.replace(needle,needle+'\n\tif (h >= 32 && h % 2 == 0 && avx512_top_supported()) return wide_top::inverse2(f, h, output, scale);');p.write_text(s)
