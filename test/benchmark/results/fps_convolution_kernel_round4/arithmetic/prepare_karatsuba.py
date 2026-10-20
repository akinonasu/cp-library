from pathlib import Path
import shutil

WORK = Path('/workspace/cp-fps-round4/kernel')
TREE = WORK / 'karatsuba4'
assert not TREE.exists()
shutil.copytree(WORK / 'base', TREE)
path = TREE / 'components/fps_convolution.hpp'
source = path.read_text()
before, tail = source.rsplit('class Subtrees {', 1)
rotation = [[(4 - j + x) if x < j else (8 + x - j) for x in range(4)] for j in range(1, 4)]
rotation = [r + [x + 4 for x in r] for r in rotation]
literal = lambda rows: ',\n'.join('\t{' + ', '.join(map(str, row)) + '}' for row in rows)
helper = '''
alignas(32) inline constexpr std::uint32_t kRotate4[3][8] = {
''' + literal(rotation) + '''
};
alignas(32) inline constexpr std::uint32_t kBroadcast4[4][8] = {
''' + literal([[j] * 4 + [j + 4] * 4 for j in range(4)]) + '''
};

[[gnu::target("avx512f,avx512vl,avx512dq"), gnu::always_inline]]
inline void leaf_three4_regs(const Vec* a, const Vec* w, const Vec* b, Vec* output) {
\tasm volatile(
'''
instructions = []
for t in range(3):
    instructions += [f'vmovdqa32 {32*t}(%[a]), %%ymm{2*t}',
                     f'vmovdqa32 {32*t}(%[w]), %%ymm{2*t+1}',
                     f'vmovdqa32 {32*t}(%[b]), %%ymm{6+t}',
                     f'vpxord %%ymm{9+2*t}, %%ymm{9+2*t}, %%ymm{9+2*t}',
                     f'vpxord %%ymm{10+2*t}, %%ymm{10+2*t}, %%ymm{10+2*t}',
                     f'vpsrlq $32, %%ymm{2*t}, %%ymm{15+t}']
for j in range(4):
    instructions.append(f'vmovdqa32 {32*j}(%[broadcast]), %%ymm21')
    for t in range(3):
        instructions.append(f'vpermd %%ymm{6+t}, %%ymm21, %%ymm{18+t}')
    for t in range(3):
        instructions += [f'vpmuludq %%ymm{15+t}, %%ymm{18+t}, %%ymm22',
                         f'vpaddq %%ymm22, %%ymm{10+2*t}, %%ymm{10+2*t}']
    if j:
        instructions.append(f'vmovdqa32 {32*(j-1)}(%[index]), %%ymm21')
    for t in range(3):
        if j:
            instructions += [f'vmovdqa32 %%ymm{2*t+1}, %%ymm{15+t}',
                             f'vpermt2d %%ymm{2*t}, %%ymm21, %%ymm{15+t}']
        else:
            instructions.append(f'vmovdqa32 %%ymm{2*t}, %%ymm{15+t}')
        instructions += [f'vpmuludq %%ymm{15+t}, %%ymm{18+t}, %%ymm{18+t}',
                         f'vpaddq %%ymm{18+t}, %%ymm{9+2*t}, %%ymm{9+2*t}']
instructions += ['vpbroadcastd %[P], %%ymm29', 'vpbroadcastd %[NI], %%ymm30',
                 'vpbroadcastd %[P2], %%ymm31', 'kmovw %[mask], %%k1']
for t in range(3):
    even, odd = 9 + 2*t, 10 + 2*t
    instructions += [f'vpmuludq %%ymm30, %%ymm{even}, %%ymm0',
                     f'vpmuludq %%ymm30, %%ymm{odd}, %%ymm1',
                     'vpmuludq %%ymm29, %%ymm0, %%ymm0',
                     'vpmuludq %%ymm29, %%ymm1, %%ymm1',
                     f'vpaddq %%ymm0, %%ymm{even}, %%ymm{even}',
                     f'vpaddq %%ymm1, %%ymm{odd}, %%ymm{odd}',
                     f'vpsrlq $32, %%ymm{even}, %%ymm{even}',
                     f'vpblendmd %%ymm{odd}, %%ymm{even}, %%ymm{even}%{{%%k1%}}',
                     f'vpsubd %%ymm31, %%ymm{even}, %%ymm0',
                     f'vpminud %%ymm0, %%ymm{even}, %%ymm{even}',
                     f'vmovdqa32 %%ymm{even}, {32*t}(%[output])']
helper += ''.join('\t\t"' + instruction + '\\n\\t"\n' for instruction in instructions)
helper += '''\t\t:
\t\t: [a] "r"(a), [w] "r"(w), [b] "r"(b), [output] "r"(output),
\t\t  [index] "r"(kRotate4), [broadcast] "r"(kBroadcast4),
\t\t  [P] "m"(kP), [P2] "m"(ntt::kernels::k2P), [NI] "m"(ntt::kernels::kNI), [mask] "r"(0xaa)
\t\t: ''' + ', '.join('"xmm' + str(i) + '"' for i in range(32)) + ''', "k1", "memory", "cc");
}

[[gnu::target("avx512f,avx512vl,avx512dq")]]
[[gnu::noinline]] inline void bottom_karatsuba4(Vec* a, Vec* b, std::size_t nv, std::size_t first,
                                             const std::uint32_t* roots, const std::uint32_t* inverse_roots) {
\tusing namespace blocked_poly::detail;
\tconst Vec p = broadcast(kP), p2 = broadcast(2 * kP);
\tfor (std::size_t j = 0, g = first; j < nv; j += 4, ++g) {
\t\tVec af[4] = {a[j], a[j + 1], a[j + 2], a[j + 3]};
\t\tVec bf[4] = {b[j], b[j + 1], b[j + 2], b[j + 3]};
\t\tforward_h1(af, Group(roots, g));
\t\tforward_h1(bf, Group(roots, g));
\t\tconst Factor yw = entry(roots, 2 * g), zw = entry(roots, 2 * g + 1);
\t\tVec aw[4];
#pragma GCC unroll 4
\t\tfor (int t = 0; t < 4; ++t) {
\t\t\taf[t] = canonical(af[t]);
\t\t\tbf[t] = canonical(bf[t]);
\t\t\tconst Vec wa = times(af[t], t < 2 ? yw : zw);
\t\t\taw[t] = reduce(t % 2 ? _mm256_sub_epi32(p2, wa) : wa, kP);
\t\t}
\t\tfor (int t = 0; t < 4; t += 2) {
\t\t\tconst auto separate = [](Vec x) __attribute__((target("avx512f,avx512vl,avx512dq"))) {
\t\t\t\treturn _mm256_permute4x64_epi64(_mm256_shuffle_epi32(x, 0xd8), 0xd8);
\t\t\t};
\t\t\tconst Vec ax = separate(af[t]), ay = separate(af[t + 1]);
\t\t\tconst Vec bx = separate(bf[t]), by = separate(bf[t + 1]);
\t\t\tconst Vec wx = separate(aw[t]), wy = separate(aw[t + 1]);
\t\t\talignas(32) Vec av[3], bv[3], wv[3], product[3];
\t\t\tav[0] = _mm256_permute2x128_si256(ax, ay, 0x20);
\t\t\tav[1] = _mm256_permute2x128_si256(ax, ay, 0x31);
\t\t\tbv[0] = _mm256_permute2x128_si256(bx, by, 0x20);
\t\t\tbv[1] = _mm256_permute2x128_si256(bx, by, 0x31);
\t\t\twv[0] = _mm256_permute2x128_si256(wx, wy, 0x20);
\t\t\twv[1] = _mm256_permute2x128_si256(wx, wy, 0x31);
\t\t\tav[2] = reduce(add(av[0], av[1]), kP);
\t\t\tbv[2] = reduce(add(bv[0], bv[1]), kP);
\t\t\twv[2] = reduce(add(wv[0], wv[1]), kP);
\t\t\tleaf_three4_regs(av, wv, bv, product);
\t\t\tconst Vec c0 = canonical(product[0]), c2 = canonical(product[1]), cs = canonical(product[2]);
\t\t\tconst Factor base = t < 2 ? yw : zw;
\t\t\tFactor weight(0);
\t\t\tweight.w = _mm256_blend_epi32(base.w, _mm256_sub_epi32(p, base.w), 0xf0);
\t\t\tweight.q = _mm256_blend_epi32(base.q, _mm256_xor_si256(base.q, _mm256_set1_epi32(-1)), 0xf0);
\t\t\tconst Vec wc2 = reduce(times(c2, weight), kP);
\t\t\tconst Vec yc2 = _mm256_blend_epi32(_mm256_shuffle_epi32(c2, 0x93), _mm256_shuffle_epi32(wc2, 0x93), 0x11);
\t\t\tconst Vec ce = reduce(add(c0, yc2), kP), co = difference(difference(cs, c0), c2);
\t\t\tconst Vec lo = _mm256_unpacklo_epi32(ce, co), hi = _mm256_unpackhi_epi32(ce, co);
\t\t\taf[t] = _mm256_permute2x128_si256(lo, hi, 0x20);
\t\t\taf[t + 1] = _mm256_permute2x128_si256(lo, hi, 0x31);
\t\t}
\t\tinverse_h1(af, Group(inverse_roots, g));
\t\ta[j] = af[0];
\t\ta[j + 1] = af[1];
\t\ta[j + 2] = af[2];
\t\ta[j + 3] = af[3];
\t}
}

'''
needle = '\tvoid bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {\n'
assert tail.count(needle) == 1
tail = tail.replace(needle, needle + '\t\tif (__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("avx512dq")) return bottom_karatsuba4(a, b, nv, first, r_, ir_);\n')
path.write_text(before + helper + 'class Subtrees {' + tail)
