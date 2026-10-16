import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/lazy_reversible_treap_base.hpp').read_text().replace('TreapBase', 'LazyReversibleTreap').replace('LAZY_REVERSIBLE_TREAP_BASE_HPP', 'LAZY_REVERSIBLE_TREAP_HPP').replace('"../components/splitmix64.hpp"', '"components/splitmix64.hpp"')

def replace_body(text, start, replacement):
    first = text.index(start)
    begin = text.index('{', first)
    depth = 1
    end = begin + 1
    while depth:
        depth += (text[end] == '{') - (text[end] == '}')
        end += 1
    return text[:first] + replacement + text[end:]

standard = source
for start, body in [
    ('\tvoid insert(int p, S value)', '\tvoid insert(int p, S value) { bounds(p, p); int x = make(std::move(value)); auto [a, b] = split(root, p); root = merge(a, merge(x, b)); }'),
    ('\tS prod(int l, int r)', '\tS prod(int l, int r) { bounds(l, r); if (l == r) return e(); auto [a, b] = split(root, l); auto [c, d] = split(b, r - l); S value = t[c].product; root = merge(a, merge(c, d)); return value; }'),
    ('\tvoid apply(int l, int r, const F &f)', '\tvoid apply(int l, int r, const F &f) { bounds(l, r); if (l == r) return; auto [a, b] = split(root, l); auto [c, d] = split(b, r - l); act(c, f); root = merge(a, merge(c, d)); }'),
    ('\tvoid reverse(int l, int r)', '\tvoid reverse(int l, int r) { bounds(l, r); if (r - l < 2) return; auto [a, b] = split(root, l); auto [c, d] = split(b, r - l); flip(c); root = merge(a, merge(c, d)); }'),
]:
    standard = replace_body(standard, start, body)
split_reverse = replace_body(source, '\tvoid reverse(int l, int r)', '\tvoid reverse(int l, int r) { bounds(l, r); if (r - l < 2) return; auto [a, b] = split(root, l); auto [c, d] = split(b, r - l); flip(c); root = merge(a, merge(c, d)); }')
leaf = source.replace('if (!x) return;\n\t\tauto &v = t[x];\n\t\tstd::swap(v.left, v.right);', 'if (count(x) < 2) return;\n\t\tauto &v = t[x];\n\t\tstd::swap(v.left, v.right);')
leaf = leaf.replace('v.value = mapping(f, v.value, 1);', 'v.value = mapping(f, v.value, 1);\n\t\tif (v.count == 1) {\n\t\t\tv.product = v.value;\n\t\t\tif constexpr (!commutative) v.back = v.value;\n\t\t\treturn;\n\t\t}')
priority64 = source.replace('std::uint32_t priority', 'std::uint64_t priority').replace('std::uint32_t(rng())', 'rng()')
packed = source.replace('std::uint32_t count : 31 = 1;', 'std::uint32_t count : 30 = 1;\n\t\tstd::uint32_t pending : 1 = 0;').replace('\n\t\tbool pending = false;', '').replace('t.size() < std::size_t(std::numeric_limits<int>::max())', 't.size() <= (std::size_t(1) << 30) - 1').replace('n < std::size_t(std::numeric_limits<int>::max())', 'n < (std::size_t(1) << 30)')
for name, text in [('standard', standard), ('direct', source), ('priority64', priority64), ('split_reverse', split_reverse), ('leaf', leaf), ('packed', packed)]:
    text = text.replace('LAZY_REVERSIBLE_TREAP_HPP', 'LAZY_REVERSIBLE_TREAP_' + name.upper() + '_HPP').replace('LazyReversibleTreap', 'Treap_' + name)
    (out / (name + '.hpp')).write_text(text)
    unit = (root / 'test/unit/lazy_reversible_treap.cpp').read_text().replace('"data_structure/lazy_reversible_treap.hpp"', '"' + name + '.hpp"').replace('LazyReversibleTreap', 'Treap_' + name)
    (out / (name + '_test.cpp')).write_text(unit)
