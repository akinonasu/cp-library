from pathlib import Path
import shutil,subprocess,hashlib,json,time
W=Path('/tmp/fps-division-round2');B=Path('/tmp/cp-fps-round2-base');C=W/'inverse_candidate';R=Path('/workspace/cp-library')
if C.exists():shutil.rmtree(C)
shutil.copytree(B,C)
p=C/'components/polynomial_gcd_blocked.hpp';s=p.read_text();a=s.index('inline std::ptrdiff_t inverse_mod(Arena& arena,');b=s.index('\n}\n\n}',a)+2
old=s[a:b]
new=old.replace('inline std::ptrdiff_t inverse_mod(Arena& arena, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,\n\t\t\t\t\t\t\t\t  std::span<std::uint32_t> h)', 'inline InverseModResult inverse_mod_result(Arena& arena, std::span<const std::uint32_t> f,\n\t\t\t\t\t\t\t\t\t\t\t std::span<const std::uint32_t> g)')
assert new!=old
new=new.replace('if (d == 0) return 0;','if (d == 0) return {nullptr, 0, 1};').replace('return -1;','return {};')
new=new.replace('\tfor (std::size_t i = 0; i < t; ++i) h[i] = mod_mul(r.entry[1][i], scale);\n\treturn std::ptrdiff_t(t);','\treturn {r.entry[1], std::ptrdiff_t(t), scale};')
assert 'h[i]' not in new
new='''struct InverseModResult {
\tconst std::uint32_t* data = nullptr;
\tstd::ptrdiff_t size = -1;
\tstd::uint32_t scale = 0;
};

'''+new+'''

inline std::ptrdiff_t inverse_mod(Arena& arena, std::span<const std::uint32_t> f, std::span<const std::uint32_t> g,
\t\t\t\t\t\t\t\t  std::span<std::uint32_t> h) {
\tconst auto result = inverse_mod_result(arena, f, g);
\tfor (std::ptrdiff_t i = 0; i < result.size; ++i) h[std::size_t(i)] = gcd_detail::mod_mul(result.data[i], result.scale);
\treturn result.size;
}'''
s=s[:a]+new+s[b:]
s=s.replace('Arena arena(inverse_mod_words(n, m) + Arena::footprint(n) + Arena::footprint(m) + Arena::footprint(m));','Arena arena(inverse_mod_words(n, m) + Arena::footprint(n) + Arena::footprint(m));').replace('auto f = arena.take(n), g = arena.take(m), h = arena.take(m);','auto f = arena.take(n), g = arena.take(m);').replace('const auto size = inverse_mod(arena, f, g, h);\n\tif (size < 0) return false;\n\tprepare_fps_output(out, std::size_t(size));\n\tdivision_detail::append_words(out, h.data(), std::size_t(size), 932051910);','const auto result = inverse_mod_result(arena, f, g);\n\tif (result.size < 0) return false;\n\tprepare_fps_output(out, std::size_t(result.size));\n\tdivision_detail::append_words(out, result.data, std::size_t(result.size), gcd_detail::mod_mul(result.scale, 932051910));')
p.write_text(s)
cmd=['flock','-s','/tmp/cp-fps-tuning.lock','g++','-std=c++23','-O2','-DEVAL','-DONLINE_JUDGE','-ftrivial-auto-var-init=zero','-march=haswell','-I',str(C),'-DMODE=1',str(W/'driver.cpp'),'-o',str(W/'inverse_mod_fused')]
t=time.monotonic();p=subprocess.run(cmd,capture_output=True,text=True);print(p.stdout+p.stderr,flush=True);assert p.returncode==0
result={'command':cmd,'compile_seconds':time.monotonic()-t,'source_sha256':hashlib.sha256((C/'components/polynomial_gcd_blocked.hpp').read_bytes()).hexdigest(),'driver_sha256':hashlib.sha256((W/'driver.cpp').read_bytes()).hexdigest(),'binary_sha256':hashlib.sha256((W/'inverse_mod_fused').read_bytes()).hexdigest(),'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()};(W/'fused_build.json').write_text(json.dumps(result,indent=2)+'\n');print('fused inv_mod built',flush=True)
