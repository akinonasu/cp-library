from pathlib import Path
import shutil

WORK = Path('/workspace/cp-fps-round4/kernel')
TREE = WORK / 'leaf32asm'
assert not TREE.exists()
shutil.copytree(WORK / 'leaf32', TREE)
path = TREE / 'components/fps_convolution.hpp'
source = path.read_text()
index = ',\n'.join('\t{' + ', '.join(str(8 - j + x) for x in range(8)) + '}' for j in range(1, 8))
helper = '''
alignas(32) inline constexpr std::uint32_t kLeafIndices32[7][8] = {
''' + index + '''
};
alignas(32) inline constexpr std::uint32_t kLeafBroadcast32[8] = {0, 1, 2, 3, 4, 5, 6, 7};

[[gnu::target("avx512f,avx512vl,avx512dq"), gnu::always_inline]]
inline void leaf_four32_regs(const Vec* a, const Vec* w, const Vec* b, Vec* output) {
\tasm volatile(
'''
instructions = []
for t in range(4):
    instructions += [f'vmovdqa32 {32*t}(%[a]), %%ymm{2*t}',
                     f'vmovdqa32 {32*t}(%[w]), %%ymm{2*t+1}',
                     f'vmovdqa32 {32*t}(%[b]), %%ymm{8+t}',
                     f'vpxord %%ymm{12+2*t}, %%ymm{12+2*t}, %%ymm{12+2*t}',
                     f'vpxord %%ymm{13+2*t}, %%ymm{13+2*t}, %%ymm{13+2*t}',
                     f'vpsrlq $32, %%ymm{2*t}, %%ymm{20+t}']
for j in range(8):
    instructions.append(f'vpbroadcastd {4*j}(%[broadcast]), %%ymm28')
    for t in range(4):
        instructions.append(f'vpermd %%ymm{8+t}, %%ymm28, %%ymm{24+t}')
    for t in range(4):
        instructions += [f'vpmuludq %%ymm{20+t}, %%ymm{24+t}, %%ymm29',
                         f'vpaddq %%ymm29, %%ymm{13+2*t}, %%ymm{13+2*t}']
    if j:
        instructions.append(f'vmovdqa32 {32*(j-1)}(%[index]), %%ymm28')
    for t in range(4):
        if j:
            instructions += [f'vmovdqa32 %%ymm{2*t+1}, %%ymm{20+t}',
                             f'vpermt2d %%ymm{2*t}, %%ymm28, %%ymm{20+t}']
        else:
            instructions.append(f'vmovdqa32 %%ymm{2*t}, %%ymm{20+t}')
        instructions += [f'vpmuludq %%ymm{20+t}, %%ymm{24+t}, %%ymm{24+t}',
                         f'vpaddq %%ymm{24+t}, %%ymm{12+2*t}, %%ymm{12+2*t}']
instructions += ['vpbroadcastd %[P], %%ymm29', 'vpbroadcastd %[NI], %%ymm30',
                 'vpbroadcastd %[P2], %%ymm31', 'kmovw %[mask], %%k1']
for t in range(4):
    even, odd = 12 + 2*t, 13 + 2*t
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
\t\t  [index] "r"(kLeafIndices32), [broadcast] "r"(kLeafBroadcast32),
\t\t  [P] "m"(kP), [P2] "m"(ntt::kernels::k2P), [NI] "m"(ntt::kernels::kNI), [mask] "r"(0xaa)
\t\t: ''' + ', '.join('"xmm' + str(i) + '"' for i in range(32)) + ''', "k1", "memory", "cc");
}

'''
marker = '[[gnu::target("avx512f,avx512vl,avx512dq")]]\n[[gnu::noinline]] inline void bottom_four32'
assert source.count(marker) == 1
source = source.replace(marker, helper + marker)
start = source.index('\t\tconst Vec a0 = canonical(af[0])')
stop = source.index('\t\tinverse_h1(product, Group(inverse_roots, g));', start)
source = source[:start] + '''\t\talignas(32) Vec av[4], bv[4], wv[4], product[4];
#pragma GCC unroll 4
\t\tfor (int t = 0; t < 4; ++t) {
\t\t\tav[t] = canonical(af[t]);
\t\t\tbv[t] = canonical(bf[t]);
\t\t\tconst Vec wa = times(av[t], t < 2 ? yw : zw);
\t\t\twv[t] = reduce(t % 2 ? _mm256_sub_epi32(p2, wa) : wa, kP);
\t\t}
\t\tleaf_four32_regs(av, wv, bv, product);
''' + source[stop:]
path.write_text(source)
