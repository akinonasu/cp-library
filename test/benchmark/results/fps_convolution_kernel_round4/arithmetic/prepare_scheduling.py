from pathlib import Path
import shutil

WORK = Path('/workspace/cp-fps-round4/kernel')
BASE = WORK / 'base'
original = (BASE / 'components/fps_convolution.hpp').read_text()
before, tail = original.rsplit('class Subtrees {', 1)

for variant in ['pairall', 'staticlevels', 'top32']:
    tree = WORK / variant
    assert not tree.exists()
    shutil.copytree(BASE, tree)
    changed = tail
    if variant == 'pairall':
        changed = changed.replace('if (h == 4) return ntt::kernels::forward_pair(a, b, h, x, y);', 'return ntt::kernels::forward_pair(a, b, h, x, y);')
    elif variant == 'staticlevels':
        start = changed.index('\ttemplate <std::size_t NV>\n')
        stop = changed.index('\n\tvoid forward(', start)
        changed = changed[:start] + '''\ttemplate <std::size_t NV, std::size_t H>
\t[[gnu::always_inline]] void forward_levels(Vec* a, Vec* b, std::size_t k) const {
\t\tfor (std::size_t j = 0, g = k * (NV / (4 * H)); j < NV; j += 4 * H, ++g) forward(a + j, b + j, H, g);
\t\tif constexpr (H > 4) forward_levels<NV, H / 4>(a, b, k);
\t}

\ttemplate <std::size_t NV, std::size_t H>
\t[[gnu::always_inline]] void inverse_levels(Vec* a, std::size_t k) const {
\t\tfor (std::size_t j = 0, g = k * (NV / (4 * H)); j < NV; j += 4 * H, ++g) inverse(a + j, H, g);
\t\tif constexpr (H * 4 < NV) inverse_levels<NV, H * 4>(a, k);
\t}

\ttemplate <std::size_t NV>
\t[[gnu::noinline]] void tile(Vec* a, Vec* b, std::size_t k) const {
\t\tforward_levels<NV, NV / 4>(a, b, k);
\t\tbottom(a, b, NV, k * (NV / 4));
\t\tinverse_levels<NV, 4>(a, k);
\t}
''' + changed[stop:]
    else:
        a = before.rindex('inline void forward_radix8(')
        body = before[a:]
        body = body[body.index('\tfor (std::size_t j = 0;'):]
        body = body[:body.index('\n\t}\n}')]
        body = body[body.index('\n') + 1:]
        lines = []
        import re
        for line in body.splitlines():
            if not line.strip():
                continue
            for factor in ['a', 'b']:
                line1 = re.sub(r'\bf\[', factor + '[', line)
                line1 = re.sub(r'\b(f[0-3]|g[01]|h[01]|ih1|if[23]|u0|v0|yu1|zv1)\b', lambda m: factor + '_' + m[1], line1)
                lines.append(line1)
        function = '''\n[[gnu::target("avx512f,avx512vl,avx512dq")]]
inline void forward_radix8_pair32(Vec* __restrict a, Vec* __restrict b, std::size_t q, const std::uint32_t* roots) {
\tconst Factor i(roots[1], roots[9]), y(roots[2], roots[10]), z(roots[3], roots[11]);
\tfor (std::size_t j = 0; j < q; ++j) {
''' + '\n'.join(lines) + '''
\t}
}
\n'''
        before = before + function
        changed = changed.replace('\t\tforward_radix8(a, q, roots_);\n\t\tforward_radix8(b, q, roots_);', '''\t\tif (__builtin_cpu_supports("avx512f") && __builtin_cpu_supports("avx512vl") && __builtin_cpu_supports("avx512dq")) forward_radix8_pair32(a, b, q, roots_);
\t\telse {
\t\t\tforward_radix8(a, q, roots_);
\t\t\tforward_radix8(b, q, roots_);
\t\t}''')
    (tree / 'components/fps_convolution.hpp').write_text(before + 'class Subtrees {' + changed)
