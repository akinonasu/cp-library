from pathlib import Path
import shutil

WORK = Path('/workspace/cp-fps-round4/kernel')
base = (WORK / 'base/components/fps_convolution.hpp').read_text()
old_target = '_Pragma("GCC target(\\"avx2\\")")'
new_target = '_Pragma("GCC target(\\"avx2,avx512f,avx512vl\\")")'
assert base.count(old_target) == 1
context = base.replace(old_target, new_target)
for variant in ['vlcontext', 'vlregs']:
    tree = WORK / variant
    assert not tree.exists()
    shutil.copytree(WORK / 'base', tree)
    source = context
    if variant == 'vlregs':
        start = source.index('inline void forward_pair(')
        stop = source.index('\ninline void bottom_first(', start)
        body = source[start:stop]
        assert body.count('alignas(32) Vec w[3];') == 1
        body = body.replace('\talignas(32) Vec w[3];\n', '')
        for offset, operand, reg in [(0, '(%[x])', 16), (32, '(%[y])', 17), (64, '4(%[y])', 18)]:
            store = '(%[w])' if offset == 0 else str(offset) + '(%[w])'
            original = '\t\t"vbroadcastss ' + operand + ', %%ymm0\\n\\t"\n\t\t"vmovdqa %%ymm0, ' + store + '\\n\\t"'
            replacement = '\t\t"vbroadcastss ' + operand + ', %%ymm' + str(reg) + '\\n\\t"'
            assert body.count(original) == 1, original
            body = body.replace(original, replacement)
        for memory, reg in [('64(%[w])', 18), ('32(%[w])', 17), ('(%[w])', 16)]:
            body = body.replace(memory, '%%ymm' + str(reg))
        assert '[w] "r"(w), ' in body
        body = body.replace('[w] "r"(w), ', '')
        body = body.replace('"xmm15", "memory", "cc"', '"xmm15", "xmm16", "xmm17", "xmm18", "memory", "cc"')
        assert '%[w]' not in body
        source = source[:start] + body + source[stop:]
    (tree / 'components/fps_convolution.hpp').write_text(source)
