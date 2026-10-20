import argparse
import concurrent.futures
import json
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def replace(source, old, new):
    if old not in source:
        raise ValueError(f'Missing replacement: {old[:70]!r}')
    return source.replace(old, new)


def point_set(source):
    return replace(source, 'access<!scalar>(x);', 'splay<false>(x);')


def fused(source):
    begin = source.index('\ttemplate <bool update = true>\n\tint access(')
    end = source.index('\n\tvoid reroot(', begin)
    return source[:begin] + '''	template <bool update = true>
	int access(int x) {
		splay<false>(x);
		t[x].ch[1] = 0;
		int last = x;
		while (int p = t[x].parent) {
			splay<false>(p);
			t[p].ch[1] = x;
			rotate(x);
			last = p;
		}
		if constexpr (update) pull(x);
		return last;
	}''' + source[end:]


def signed_parent(source):
    begin = source.index('\tbool is_root(')
    end = source.index('\n\tvoid pull(', begin)
    source = source[:begin] + '''	bool is_root(int x) const { return t[x].parent <= 0; }''' + source[end:]
    source = replace(source, 'if (!is_root(p)) t[g].ch[t[g].ch[1] == p] = x;',
                     'if (g > 0) t[g].ch[t[g].ch[1] == p] = x;')
    source = replace(source, 'for (int y = x; y; y = t[y].parent)',
                     'for (int y = x; y; y = t[y].parent < 0 ? -t[y].parent : t[y].parent)')
    source = replace(source, 't[y].ch[1] = last;',
                     'if (t[y].ch[1]) t[t[y].ch[1]].parent = -y;\n\t\t\tt[y].ch[1] = last;\n\t\t\tif (last) t[last].parent = y;')
    source = replace(source, 't[u].parent = v;', 't[u].parent = -v;')
    return source


def signed_fused(source):
    source = signed_parent(source)
    begin = source.index('\ttemplate <bool update = true>\n\tint access(')
    end = source.index('\n\tvoid reroot(', begin)
    return source[:begin] + '''	template <bool update = true>
	int access(int x) {
		splay<false>(x);
		if (t[x].ch[1]) t[t[x].ch[1]].parent = -x;
		t[x].ch[1] = 0;
		int last = x;
		while (t[x].parent) {
			int p = -t[x].parent;
			splay<false>(p);
			if (t[p].ch[1]) t[t[p].ch[1]].parent = -p;
			t[p].ch[1] = x;
			t[x].parent = p;
			rotate(x);
			last = p;
		}
		if constexpr (update) pull(x);
		return last;
	}''' + source[end:]


def directions(source):
    source = replace(source, 'S value, prod;', 'std::int8_t side = -1;\n\t\tS value, prod;')
    begin = source.index('\tbool is_root(')
    end = source.index('\n\tvoid pull(', begin)
    source = source[:begin] + '''	bool is_root(int x) const { return t[x].side < 0; }''' + source[end:]
    source = replace(source, 'std::swap(t[x].ch[0], t[x].ch[1]);',
                     'std::swap(t[x].ch[0], t[x].ch[1]);\n\t\tif (t[x].ch[0]) t[t[x].ch[0]].side = 0;\n\t\tif (t[x].ch[1]) t[t[x].ch[1]].side = 1;')
    source = replace(source, 'int d = t[p].ch[1] == x, c = t[x].ch[d ^ 1];',
                     'int d = t[x].side, c = t[x].ch[d ^ 1], side = t[p].side;')
    source = replace(source, 'if (!is_root(p)) t[g].ch[t[g].ch[1] == p] = x;',
                     'if (side >= 0) t[g].ch[side] = x;\n\t\tt[x].side = side;\n\t\tt[p].side = d ^ 1;\n\t\tif (c) t[c].side = d;')
    source = replace(source, '(t[p].ch[0] == x) == (t[g].ch[0] == p)', 't[x].side == t[p].side')
    source = replace(source, 't[y].ch[1] = last;',
                     'if (t[y].ch[1]) t[t[y].ch[1]].side = -1;\n\t\t\tt[y].ch[1] = last;\n\t\t\tif (last) t[last].side = 1;')
    source = replace(source, 't[v].ch[0] = t[u].parent = 0;',
                     't[v].ch[0] = t[u].parent = 0;\n\t\tt[u].side = -1;')
    return source


def lazy_root(source):
    source = replace(source, '\t\tif constexpr (update) pull(x);\n', '')
    source = replace(source, '\t\tt[x].value = std::move(value);\n\t\tpull(x);',
                     '\t\tt[x].value = std::move(value);')
    source = replace(source, '\t\t\tif constexpr (scalar) pull(v);\n', '')
    source = replace(source, '\t\tpull(v);\n', '')
    source = replace(source, 'S prod(int u, int v) { return t[expose(u, v)].prod; }',
                     'S prod(int u, int v) {\n\t\tint x = expose(u, v), l = t[x].ch[0];\n\t\treturn l ? op(t[l].prod, t[x].value) : t[x].value;\n\t}')
    source = replace(source, 'int path_size(int u, int v) { return t[expose(u, v)].count; }',
                     'int path_size(int u, int v) {\n\t\tint x = expose(u, v);\n\t\treturn t[t[x].ch[0]].count + 1;\n\t}')
    source = replace(source, 'k >= int(t[x].count)', 'k > int(t[t[x].ch[0]].count)')
    return source


def variants():
    baseline = (ROOT / 'test/benchmark/link_cut_tree_baseline.hpp').read_text()
    result = {'baseline': baseline, 'set': point_set(baseline), 'fused': fused(baseline),
              'set_fused': fused(point_set(baseline)), 'signed': signed_parent(baseline),
              'set_signed': signed_parent(point_set(baseline)),
              'set_signed_fused': signed_fused(point_set(baseline)),
              'directions': directions(baseline), 'set_directions': directions(point_set(baseline))}
    result['lazy_root'] = lazy_root(result['set_fused'])
    result['signed_lazy_root'] = lazy_root(result['set_signed_fused'])
    result['current'] = (ROOT / 'tree/link_cut_tree.hpp').read_text()
    for name in ['baseline', 'set_fused', 'set_signed_fused', 'set_directions']:
        source = result[name]
        source = source.replace('template <class S, auto op, auto e, bool commutative = false>',
            '#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))\n'
            '#pragma GCC push_options\n#pragma GCC optimize("O3")\n#endif\n\n'
            'template <class S, auto op, auto e, bool commutative = false>', 1)
        source += '\n#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))\n#pragma GCC pop_options\n#endif\n'
        result[name + '_o3'] = source
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('directory', type=Path)
    parser.add_argument('--check', action='store_true')
    parser.add_argument('--check-variants', nargs='+')
    args = parser.parse_args()
    args.directory.mkdir(parents=True, exist_ok=True)
    test = (ROOT / 'test/unit/link_cut_tree.cpp').read_text()
    names = []
    for name, source in variants().items():
        class_name = 'LinkCutTree_' + name
        source = source.replace('LINK_CUT_TREE_HPP', 'LINK_CUT_TREE_' + name.upper() + '_HPP')
        source = source.replace('LinkCutTree', class_name)
        (args.directory / f'{name}.hpp').write_text(source)
        unit = test.replace('#include "tree/link_cut_tree.hpp"', f'#include "{name}.hpp"')
        (args.directory / f'{name}_test.cpp').write_text(unit.replace('LinkCutTree', class_name))
        names.append(name)
    (args.directory / 'variants.hpp').write_text(''.join(f'#include "{name}.hpp"\n' for name in names))
    (args.directory / 'dispatch.inc').write_text(''.join(
        f'if (name == "{name}") return run<LinkCutTree_{name}>();\n' for name in names))
    if args.check:
        def check(name):
            binary = args.directory / (name + '_test')
            command = ['g++', '-std=c++23', '-O2', '-Wall', '-Wextra', '-Werror',
                       '-I', str(ROOT), str(binary) + '.cpp', '-o', str(binary)]
            subprocess.run(command, check=True, capture_output=True)
            result = subprocess.run([str(binary)], capture_output=True, text=True)
            print(json.dumps({'variant': name, 'returncode': result.returncode,
                              'output': result.stdout + result.stderr}), flush=True)
            return result.returncode == 0
        with concurrent.futures.ThreadPoolExecutor(max_workers=3) as executor:
            passed = list(executor.map(check, args.check_variants or names))
        if not all(passed):
            raise SystemExit(1)


if __name__ == '__main__':
    main()
