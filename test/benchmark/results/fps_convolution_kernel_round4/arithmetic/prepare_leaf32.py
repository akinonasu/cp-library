from pathlib import Path
import shutil

WORK = Path('/workspace/cp-fps-round4/kernel')
BASE = WORK / 'base'
TREE = WORK / 'leaf32'
assert not TREE.exists()
shutil.copytree(BASE, TREE)
path = TREE / 'components/fps_convolution.hpp'
source = path.read_text()
before, tail = source.rsplit('class Subtrees {', 1)
lines = ['''
[[gnu::target("avx512f,avx512vl,avx512dq")]]
[[gnu::noinline]] inline void bottom_four32(Vec* a, Vec* b, std::size_t nv, std::size_t first,
                                         const std::uint32_t* roots, const std::uint32_t* inverse_roots) {
\tusing namespace blocked_poly::detail;
\tconst Vec ni = broadcast(blocked_ntt::kernels::kNI), p = broadcast(kP), p2 = broadcast(2 * kP);
\tfor (std::size_t j = 0, g = first; j < nv; j += 4, ++g) {
\t\tVec af[4] = {a[j], a[j + 1], a[j + 2], a[j + 3]};
\t\tVec bf[4] = {b[j], b[j + 1], b[j + 2], b[j + 3]};
\t\tforward_h1(af, Group(roots, g));
\t\tforward_h1(bf, Group(roots, g));
\t\tconst Factor yw = entry(roots, 2 * g), zw = entry(roots, 2 * g + 1);
''']
for t in range(4):
    lines.append(f'\t\tconst Vec a{t} = canonical(af[{t}]), b{t} = canonical(bf[{t}]);')
    lines.append(f'\t\tconst Vec wa{t} = times(a{t}, {"yw" if t < 2 else "zw"});')
    lines.append(f'\t\tconst Vec w{t} = reduce({"_mm256_sub_epi32(p2, wa" + str(t) + ")" if t % 2 else "wa" + str(t)}, kP);')
    lines.append(f'\t\tVec prev{t} = _mm256_srli_epi64(a{t}, 32), even{t} = _mm256_setzero_si256(), odd{t} = _mm256_setzero_si256();')
for j in range(8):
    lines.append(f'\t\tconst Vec bi{j} = _mm256_set1_epi32({j});')
    if j:
        indices = ', '.join(str(8 - j + x) for x in range(8))
        lines.append(f'\t\tconst Vec index{j} = _mm256_setr_epi32({indices});')
    for t in range(4):
        lines.append(f'\t\tconst Vec bj{j}_{t} = _mm256_permutevar8x32_epi32(b{t}, bi{j});')
        window = f'_mm256_permutex2var_epi32(w{t}, index{j}, a{t})' if j else f'a{t}'
        lines.append(f'\t\tconst Vec v{j}_{t} = {window};')
    for t in range(4):
        lines.append(f'\t\teven{t} = _mm256_add_epi64(even{t}, _mm256_mul_epu32(v{j}_{t}, bj{j}_{t}));')
        lines.append(f'\t\todd{t} = _mm256_add_epi64(odd{t}, _mm256_mul_epu32(prev{t}, bj{j}_{t}));')
        lines.append(f'\t\tprev{t} = v{j}_{t};')
lines.append('\t\tVec product[4];')
for t in range(4):
    lines.append(f'\t\teven{t} = _mm256_add_epi64(even{t}, _mm256_mul_epu32(_mm256_mul_epu32(even{t}, ni), p));')
    lines.append(f'\t\todd{t} = _mm256_add_epi64(odd{t}, _mm256_mul_epu32(_mm256_mul_epu32(odd{t}, ni), p));')
    lines.append(f'\t\tproduct[{t}] = reduce(_mm256_blend_epi32(_mm256_srli_epi64(even{t}, 32), odd{t}, 0xaa), 2 * kP);')
lines.append('''\t\tinverse_h1(product, Group(inverse_roots, g));
\t\ta[j] = product[0];
\t\ta[j + 1] = product[1];
\t\ta[j + 2] = product[2];
\t\ta[j + 3] = product[3];
\t}
}

''')
function = '\n'.join(lines)
needle = '\tvoid bottom(Vec* a, Vec* b, std::size_t nv, std::size_t first) const {\n'
assert tail.count(needle) == 1
tail = tail.replace(needle, needle + '\t\tif (__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("avx512dq")) return bottom_four32(a, b, nv, first, r_, ir_);\n')
path.write_text(before + function + 'class Subtrees {' + tail)
