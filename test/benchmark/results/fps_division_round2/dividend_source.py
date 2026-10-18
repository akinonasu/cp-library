from pathlib import Path
import shutil,hashlib,json
W=Path('/tmp/fps-division-round2');B=W/'candidate';C=W/'dividend_candidate'
if C.exists():shutil.rmtree(C)
shutil.copytree(B,C)
p=C/'components/fps_division_blocked.hpp';s=p.read_text()
def change(a,b,count=1):
 global s
 assert s.count(a)==count,(a,s.count(a),count)
 s=s.replace(a,b)
change('inline Vec load_any(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }','''inline std::uint32_t input_word(std::uint32_t x) { return x; }
inline std::uint32_t input_word(Mint x) { return std::min(x.v, x.v - Mint::mod()); }
inline Vec load_any(const std::uint32_t* p) { return _mm256_loadu_si256(reinterpret_cast<const Vec*>(p)); }
inline Vec load_any(const Mint* p) {
\tVec value;
\tstd::memcpy(&value, p, sizeof(value));
\treturn reduce(value, kP);
}''')
change('inline void subtract(const std::uint32_t* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {','template<class T>\ninline void subtract(const T* a, const std::uint32_t* b, std::uint32_t* out, std::size_t count) {')
change('out[i] = (a[i] + kP - b[i]) % kP;','out[i] = (input_word(a[i]) + kP - b[i]) % kP;')
change('inline void reverse_difference(std::span<const std::uint32_t> a,','template<class T>\ninline void reverse_difference(std::span<T> a,')
change('const std::uint32_t x = p >= 0 && p < size ? a[std::size_t(p)] : 0;','const std::uint32_t x = p >= 0 && p < size ? input_word(a[std::size_t(p)]) : 0;')
change('inline void fold(std::span<const std::uint32_t> a, std::span<std::uint32_t> out) {','template<class T>\ninline void fold(std::span<T> a, std::span<std::uint32_t> out) {')
change('const std::uint32_t* x = a.data() + from;','const T* x = a.data() + from;')
change('out[j] = (out[j] + x[j]) % kP;','out[j] = (out[j] + input_word(x[j])) % kP;')
change('inline void long_division(std::span<const std::uint32_t> f,','template<class T>\ninline void long_division(std::span<const T> f,')
change('\tstd::copy(f.begin(), f.end(), rest.begin());','\tfor (std::size_t i = 0; i < f.size(); ++i) rest[i] = input_word(f[i]);')
change('inline void remainder_direct(std::span<const std::uint32_t> f,','template<class T>\ninline void remainder_direct(std::span<const T> f,')
change('r[j] = std::uint32_t((f[j] + kP - sum) % kP);','r[j] = std::uint32_t((input_word(f[j]) + kP - sum) % kP);')
change('inline void remainder_wrap(const Transform& t, std::span<const std::uint32_t> f,','template<class T>\ninline void remainder_wrap(const Transform& t, std::span<const T> f,')
change('inline void remainder_split(const Transform& t, std::span<const std::uint32_t> f,','template<class T>\ninline void remainder_split(const Transform& t, std::span<const T> f,')
change('inline void subtract_reversed(const std::uint32_t* a,','template<class T>\ninline void subtract_reversed(const T* a,')
change('out[j] = (a[j] + kP - b[count - 1 - j]) % kP;','out[j] = (input_word(a[j]) + kP - b[count - 1 - j]) % kP;')
change('inline void divide_blocks(const Transform& t, const DivisionPlan& plan, std::span<const std::uint32_t> f,','template<class T>\ninline void divide_blocks(const Transform& t, const DivisionPlan& plan, std::span<const T> f,')
change('inline void divide(Arena& arena, const DivisionPlan& plan, std::span<const std::uint32_t> f,','template<class T>\ninline void divide(Arena& arena, const DivisionPlan& plan, std::span<const T> f,')
change('Arena arena(plan.words() + Arena::footprint(n) + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));','Arena arena(plan.words() + Arena::footprint(m) + Arena::footprint(k) + Arena::footprint(d));')
change('auto f = arena.take(n), g = arena.take(m), q = arena.take(k), r = arena.take(d);\n\tfor (std::size_t i = 0; i < n; ++i) f[i] = std::min(a[i].v, a[i].v - Mint::mod());','const std::span<const Mint> f(a, n);\n\tauto g = arena.take(m), q = arena.take(k), r = arena.take(d);')
p.write_text(s)
(W/'dividend_source.json').write_text(json.dumps({'baseline':'144ae32387675fa2a889ea3e8cd19b03ad5016bf','includes_batch64':True,'source_sha256':hashlib.sha256(p.read_bytes()).hexdigest(),'harness_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},indent=2)+'\n')
print('prepared const dividend candidate, no compilation',flush=True)
