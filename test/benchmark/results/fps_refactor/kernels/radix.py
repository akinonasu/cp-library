exec(open('/workspace/cp-fps-refactor-kernels-study/prepare.py').read().split('variants={}')[0])
block=(out/'all_top/components/fps_blocked.hpp').read_text()
start=block.index('inline void forward(')
end=block.index('\n}\n\n}',block.index('inline void scale_radix2('))+3
code='''struct Radix4 {
\tstatic Vec low(Vec x) {
\t\tconst Vec p = _mm256_set1_epi32(k2P);
\t\treturn _mm256_min_epu32(x, _mm256_sub_epi32(x, p));
\t}
\tstatic Vec sum(Vec a, Vec b) { return _mm256_add_epi32(a, b); }
\tstatic Vec difference(Vec a, Vec b) { return _mm256_sub_epi32(sum(a, _mm256_set1_epi32(k2P)), b); }
\tstruct Weight {
\t\tVec value, quotient;
\t\texplicit Weight(const std::uint32_t* p) : value(_mm256_set1_epi32(p[0])), quotient(_mm256_set1_epi32(p[8])) {}
\t\tVec operator()(Vec x) const {
\t\t\tconst Vec a = _mm256_srli_epi64(_mm256_mul_epu32(x, quotient), 32);
\t\t\tconst Vec b = _mm256_mul_epu32(_mm256_srli_epi64(x, 32), quotient);
\t\t\treturn _mm256_sub_epi32(_mm256_mullo_epi32(x, value), _mm256_mullo_epi32(_mm256_blend_epi32(a, b, 0xaa), _mm256_set1_epi32(kP)));
\t\t}
\t};
\ttemplate<bool inverse, bool identity>
\tstatic void run(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) {
\t\tconst Weight wx(identity ? y : x), wy(y), wz(y + 1);
#pragma GCC unroll 2
\t\tfor (std::size_t j = 0; j < h; ++j) {
\t\t\tVec f0 = low(a[j]), f1 = low(a[j + h]), f2 = low(a[j + 2 * h]), f3 = low(a[j + 3 * h]);
\t\t\tif constexpr (inverse) {
\t\t\t\tconst Vec ab = low(sum(f0, f1)), cd = low(sum(f2, f3));
\t\t\t\tVec amb = low(difference(f0, f1)), cmd = wz(difference(f2, f3));
\t\t\t\tif constexpr (!identity) amb = wy(amb);
\t\t\t\tf0 = low(sum(ab, cd)); f1 = low(sum(amb, cmd));
\t\t\t\tf2 = low(difference(ab, cd)); f3 = low(difference(amb, cmd));
\t\t\t\tif constexpr (!identity) f2 = wx(f2), f3 = wx(f3);
\t\t\t} else {
\t\t\t\tif constexpr (!identity) f2 = wx(f2), f3 = wx(f3);
\t\t\t\tconst Vec ac = low(sum(f0, f2)), amc = low(difference(f0, f2));
\t\t\t\tVec bd = low(sum(f1, f3));
\t\t\t\tif constexpr (!identity) bd = wy(bd);
\t\t\t\tconst Vec bmd = wz(difference(f1, f3));
\t\t\t\tf0 = sum(ac, bd); f1 = difference(ac, bd);
\t\t\t\tf2 = sum(amc, bmd); f3 = difference(amc, bmd);
\t\t\t}
\t\t\ta[j] = f0; a[j + h] = f1; a[j + 2 * h] = f2; a[j + 3 * h] = f3;
\t\t}
\t}
};
inline void forward(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) { Radix4::run<false, false>(a, h, x, y); }
inline void inverse(Vec* a, std::size_t h, const std::uint32_t* x, const std::uint32_t* y) { Radix4::run<true, false>(a, h, x, y); }
inline void forward_identity(Vec* a, std::size_t h, const std::uint32_t* y) { Radix4::run<false, true>(a, h, nullptr, y); }
inline void inverse_identity(Vec* a, std::size_t h, const std::uint32_t* y) { Radix4::run<true, true>(a, h, nullptr, y); }
inline void scale_radix2(Vec* a, std::size_t h, const std::uint32_t* y) {
\tconst Radix4::Weight scale(y + 1);
\tconst Vec p = _mm256_set1_epi32(kP);
\tfor (std::size_t j = 0; j < h; ++j) {
\t\tconst Vec x = Radix4::low(a[j]), z = Radix4::low(a[j + h]);
\t\tconst Vec u = scale(Radix4::low(Radix4::sum(x, z))), v = scale(Radix4::low(Radix4::difference(x, z)));
\t\ta[j] = _mm256_min_epu32(u, _mm256_sub_epi32(u, p));
\t\ta[j + h] = _mm256_min_epu32(v, _mm256_sub_epi32(v, p));
\t}
}'''
block=block[:start]+code+block[end:]
p=out/'radix';p.mkdir(exist_ok=True)
for directory in ['components','fps','math','convolution']:
 shutil.copytree(out/'all_top'/directory,p/directory,dirs_exist_ok=True)
(p/'components/fps_blocked.hpp').write_text(block)
print('blockbytes',len(block),'additional saved',len((out/'all_top/components/fps_blocked.hpp').read_text())-len(block))
