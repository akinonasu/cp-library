from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]


def pack(source):
    source = source.replace('static constexpr bool aggregate = !std::is_void_v<S>;',
                            'static constexpr bool aggregate = !std::is_void_v<S>;\n\tstatic constexpr bool packed = aggregate && std::is_arithmetic_v<S>;')
    source = source.replace('std::uint8_t own = 0, flags = 0;',
                            'std::uint8_t own = 0, flags = 0;\n\t\t[[no_unique_address]] std::conditional_t<packed, Value, Empty> sum{};')
    source = source.replace('std::vector<Value> values, sums;', '''std::vector<Value> values, sums;
	Value &total(int x) {
		if constexpr (packed) return t[x].sum;
		else return sums[x];
	}''')
    import re
    source = re.sub(r'sums\[([^\]]+)\]', r'total(\1)', source)
    source = source.replace('else return total(x);', 'else return sums[x];')
    source = source.replace('if constexpr (aggregate) sums.push_back(e());',
                            'if constexpr (packed) total(x) = e();\n\t\t\telse if constexpr (aggregate) sums.push_back(e());')
    source = source.replace('if constexpr (aggregate) sums.insert(sums.end(), 2, e());',
                            'if constexpr (packed) total(x) = total(x + 1) = e();\n\t\t\telse if constexpr (aggregate) sums.insert(sums.end(), 2, e());')
    source = source.replace('sums.reserve(std::size_t(n) * 3 + 1);\n\t\t\tsums.assign(std::size_t(n) + 1, e());',
                            'if constexpr (packed) for (int i = 0; i <= n; ++i) total(i) = e();\n\t\t\telse { sums.reserve(std::size_t(n) * 3 + 1); sums.assign(std::size_t(n) + 1, e()); }')
    return source


def rightmost(source):
    source = source.replace('key = 0, aux = 0;', 'key = 0, aux = 0, last = 0;')
    source = source.replace('v.flags = t[l].flags | t[r].flags | (v.own & 3);',
                            'v.flags = t[l].flags | t[r].flags | (v.own & 3);\n\t\tv.last = r ? t[r].last : x;')
    source = source.replace('int x = a;\n\t\twhile (t[x].ch[1]) x = t[x].ch[1];', 'int x = t[a].last;')
    source = source.replace('t[x].size = 1;', 't[x].size = 1;\n\t\t\tt[x].last = x;')
    source = source.replace('t[v + 1].size = 1;', 't[v + 1].size = 1;\n\t\t\tt[v + 1].last = v + 1;')
    source = source.replace('t[a].flags = t[u].flags | t[v].flags | exact;',
                            't[a].flags = t[u].flags | t[v].flags | exact;\n\t\tt[a].last = t[v].last;')
    source = source.replace('t[b].flags = t[a].flags;', 't[b].flags = t[a].flags;\n\t\tt[b].last = b;')
    return source


def root_cache(source):
    begin = source.index('void splay(')
    end = source.index('\n\tbool same_nodes', begin)
    body = source[begin:end]
    body = body.replace('int p = t[x].parent, g = t[p].parent;', '''int p = t[x].parent, g = t[p].parent;
			if constexpr (update) {
				if (!g || !t[g].parent) {
					int root = g ? g : p;
					t[x].size = t[root].size;
					t[x].flags = t[root].flags;
					if constexpr (aggregate) if (t[x].own & 4) sums[x] = sums[root];
				}
			}''')
    body = body.replace('\t\tif constexpr (update) pull(x);\n', '')
    return source[:begin] + body + source[end:]


def sum_first(source):
    return source.replace('if (v.own & 4) sums[x] = op(op(sums[l], v.key > 0 ? values[v.key - 1] : sums[0]), sums[r]);',
                          'if (v.own & 4) { sums[x] = op(sums[l], sums[r]); if (v.key > 0) sums[x] = op(sums[x], values[v.key - 1]); }')


def scratch(source):
    source = source.replace('std::vector<Value> values, sums;', 'std::vector<Value> values, sums;\n\tstd::array<std::pair<int, int>, 32> cut_roots;')
    source = source.replace('\t\t\tstd::array<std::pair<int, int>, 32> roots;\n', '')
    source = source.replace('roots[i--] = cut_tree(a);', 'cut_roots[i--] = cut_tree(a);')
    return source.replace('reconnect(roots.data(), level)', 'reconnect(cut_roots.data(), level)')

FUSED = '''		while (t[x].parent) {
			int p = t[x].parent, g = t[p].parent;
			if (!g) { rotate(x); break; }
			int gg = t[g].parent, d = t[p].ch[1] == x, side = t[g].ch[1] == p;
			if (d == side) {
				int a = t[p].ch[d ^ 1], b = t[x].ch[d ^ 1];
				t[g].ch[d] = a;
				if (a) t[a].parent = g;
				t[p].ch[d] = b;
				if (b) t[b].parent = p;
				t[p].ch[d ^ 1] = g;
				t[g].parent = p;
				t[x].ch[d ^ 1] = p;
				t[p].parent = x;
				pull(g);
				pull(p);
			} else {
				int a = t[x].ch[d], b = t[x].ch[d ^ 1];
				t[p].ch[d] = b;
				if (b) t[b].parent = p;
				t[g].ch[side] = a;
				if (a) t[a].parent = g;
				t[x].ch[d] = g;
				t[x].ch[d ^ 1] = p;
				t[p].parent = t[g].parent = x;
				pull(p);
				pull(g);
			}
			t[x].parent = gg;
			if (gg) t[gg].ch[t[gg].ch[1] == g] = x;
		}
'''


def variants():
    baseline = (ROOT / 'test/benchmark/dynamic_connectivity_baseline.hpp').read_text()
    baseline = baseline.replace('../components/', 'components/')
    baseline = baseline.replace('std::chrono::steady_clock::now().time_since_epoch().count()', 'dynamic_connectivity_benchmark_seed')
    begin = baseline.index('\t\twhile (t[x].parent) {', baseline.index('void splay('))
    end = baseline.index('\t\tif constexpr (update)', begin)
    fused = baseline[:begin] + FUSED + baseline[end:]
    return {
        'baseline': baseline,
        'current': (ROOT / 'data_structure/dynamic_connectivity.hpp').read_text().replace('../components/', 'components/').replace('std::chrono::steady_clock::now().time_since_epoch().count()', 'dynamic_connectivity_benchmark_seed'),
        'pad32': baseline.replace('struct Node {', 'struct alignas(32) Node {'),
        'fused': fused,
        'fused_pad32': fused.replace('struct Node {', 'struct alignas(32) Node {'),
        'packed': pack(baseline),
        'fused_packed': pack(fused),
        'fused_packed64': pack(fused).replace('struct Node {', 'struct alignas(64) Node {'),
        'rightmost': rightmost(fused).replace('struct Node {', 'struct alignas(32) Node {'),
        'rightmost_packed': rightmost(pack(fused)),
        'root_cache': root_cache(fused).replace('struct Node {', 'struct alignas(32) Node {'),
        'root_cache_packed': pack(root_cache(fused)),
        'sum_first': sum_first(baseline),
        'fused_sum_first': sum_first(fused).replace('struct Node {', 'struct alignas(32) Node {'),
        'scratch': scratch(fused).replace('struct Node {', 'struct alignas(32) Node {'),
        'o3': '#pragma GCC push_options\n#pragma GCC optimize("O3")\n' + baseline + '\n#pragma GCC pop_options\n',
        'fused_o3': '#pragma GCC push_options\n#pragma GCC optimize("O3")\n' + fused + '\n#pragma GCC pop_options\n',
    }


def generate(directory):
    directory.mkdir(parents=True, exist_ok=True)
    for name, source in variants().items():
        source = source.replace('DYNAMIC_CONNECTIVITY_HPP', f'DYNAMIC_CONNECTIVITY_{name.upper()}_HPP')
        source = source.replace('DynamicConnectivity', f'DynamicConnectivity_{name}')
        (directory / f'{name}.hpp').write_text(source)


if __name__ == '__main__':
    generate(Path(sys.argv[1]))
