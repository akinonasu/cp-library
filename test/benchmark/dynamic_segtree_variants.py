import sys
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
source = (root / 'test/benchmark/dynamic_segtree_base.hpp').read_text()
pull = '''		nodes[k].product = op(op(nodes[left(k)].product, nodes[k].value), nodes[nodes[k].right].product);'''
branch = source.replace(pull, '''		auto l = left(k), r = nodes[k].right;
		if (!l) nodes[k].product = r ? op(nodes[k].value, nodes[r].product) : nodes[k].value;
		else if (!r) nodes[k].product = op(nodes[l].product, nodes[k].value);
		else nodes[k].product = op(op(nodes[l].product, nodes[k].value), nodes[r].product);''')
leaf = source.replace(pull, '''		if (!(left(k) | nodes[k].right)) nodes[k].product = nodes[k].value;
		else nodes[k].product = op(op(nodes[left(k)].product, nodes[k].value), nodes[nodes[k].right].product);''')
cover = source.replace('''			else {
				S part = op(nodes[k].value, nodes[nodes[k].right].product);''', '''			else {
				U bits = (U(~U(0)) << (nodes[k].left >> 26)) << 1;
				if (l <= (nodes[k].key & bits)) return op(nodes[k].product, result);
				S part = op(nodes[k].value, nodes[nodes[k].right].product);''')
cover = cover.replace('''			else {
				S part = op(nodes[left(k)].product, nodes[k].value);''', '''			else {
				U bits = (U(~U(0)) << (nodes[k].left >> 26)) << 1;
				if ((nodes[k].key | U(~bits)) < r) return op(result, nodes[k].product);
				S part = op(nodes[left(k)].product, nodes[k].value);''')
layout = source.replace('''		U key;
		S value, product;
		std::uint32_t left = 0, right = 0;''', '''		U key;
		std::uint32_t left = 0, right = 0;
		S value, product;''')
recursion = source
start=source.index('\ttemplate <bool append>\n\tstd::uint32_t update(')
end=source.index('\n\tS suffix(',start)
recursive='''	template <bool append>
	std::uint32_t update(std::uint32_t k, U p, S value) {
		if (!k) return make(p, std::move(value));
		if (p == nodes[k].key) {
			if constexpr (append) nodes[k].value = op(nodes[k].value, value);
			else nodes[k].value = std::move(value);
		} else {
			unsigned h = nodes[k].left >> 26;
			U difference = p ^ nodes[k].key;
			if (!(left(k) | nodes[k].right)) {
				h = std::bit_width(difference) - 1;
				nodes[k].left = h << 26;
			} else if ((difference >> h) > 1) {
				h = std::bit_width(difference) - 1;
				auto parent = make(p, std::move(value));
				nodes[parent].left = h << 26;
				if (nodes[k].key < p) nodes[parent].left |= k;
				else nodes[parent].right = k;
				pull(parent);
				return parent;
			}
			bool right = (p >> h) & 1;
			bool swapped = right ? p < nodes[k].key : nodes[k].key < p;
			if (swapped) {
				std::swap(p, nodes[k].key);
				std::swap(value, nodes[k].value);
			}
			auto child = right ? nodes[k].right : left(k);
			if (swapped) child = update<false>(child, p, std::move(value));
			else child = update<append>(child, p, std::move(value));
			if (right) nodes[k].right = child;
			else nodes[k].left = (nodes[k].left & ~mask) | child;
		}
		pull(k);
		return k;
	}
'''
recursion=source[:start]+recursive+source[end:]
combo_branch=branch.replace('\t\tU key;\n\t\tS value, product;\n\t\tstd::uint32_t left = 0, right = 0;', '\t\tU key;\n\t\tstd::uint32_t left = 0, right = 0;\n\t\tS value, product;')
combo_leaf=leaf.replace('\t\tU key;\n\t\tS value, product;\n\t\tstd::uint32_t left = 0, right = 0;', '\t\tU key;\n\t\tstd::uint32_t left = 0, right = 0;\n\t\tS value, product;')
for name, text in [('combo_branch',combo_branch),('combo_leaf',combo_leaf),('branch',branch),('leaf',leaf),('cover',cover),('layout',layout),('recursive',recursion),('base',source)]:
    text=text.replace('DYNAMIC_SEGTREE_BASE_HPP','DYNAMIC_SEGTREE_'+name.upper()+'_HPP').replace('DynamicSegtreeBase','DynamicSegtree_'+name)
    (out/(name+'.hpp')).write_text(text)
    unit=(root/'test/unit/dynamic_segtree.cpp').read_text().replace('"data_structure/segment_tree/dynamic_segtree.hpp"',f'"{name}.hpp"').replace('DynamicSegtree','DynamicSegtree_'+name)
    (out/(name+'_test.cpp')).write_text(unit)

block_unit=(root/'test/unit/dynamic_segtree.cpp').read_text().replace('"data_structure/segment_tree/dynamic_segtree.hpp"','"test/benchmark/dynamic_segtree_block.hpp"').replace('DynamicSegtree','DynamicSegtreeBlock')
(out/'block_test.cpp').write_text(block_unit)

unit=(root/'test/unit/dynamic_segtree.cpp').read_text().replace('"data_structure/segment_tree/dynamic_segtree.hpp"','"test/benchmark/dynamic_segtree_block.hpp"').replace('DynamicSegtree','TestTree')
unit=unit.replace('using namespace std;', 'template <class S, auto op, auto e, class Coord = std::int64_t> using TestTree = DynamicSegtreeBlock<S, op, e, Coord, 16, 4, true>;\nusing namespace std;')
(out/'balanced_test.cpp').write_text(unit)

for cap in [4, 8, 16]:
    unit=(root/'test/unit/dynamic_segtree.cpp').read_text().replace('"data_structure/segment_tree/dynamic_segtree.hpp"','"test/benchmark/dynamic_segtree_bucket.hpp"').replace('DynamicSegtree','TestTree')
    unit=unit.replace('using namespace std;', f'template <class S, auto op, auto e, class Coord = std::int64_t> using TestTree = DynamicSegtreeBucket<S, op, e, Coord, {cap}, true>;\nusing namespace std;')
    (out/f'bucket{cap}_test.cpp').write_text(unit)
