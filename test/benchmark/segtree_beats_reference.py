import hashlib
import json
import sys
import urllib.request
from pathlib import Path

out = Path(sys.argv[1])
out.mkdir(parents=True, exist_ok=True)
url = 'https://v3.api.judge.yosupo.jp/submissions/204482'
source = json.load(urllib.request.urlopen(url, timeout=60))['source']
(out / 'reference-source.cpp').write_text(source)
(out / 'reference-source.sha256').write_text(hashlib.sha256(source.encode()).hexdigest() + '\n')
source = '#include <algorithm>\n#include <bit>\n#include <cstdint>\n#include <limits>\n#include <type_traits>\n#include <vector>\n' + source[source.index('#ifndef __OY_SEGMENTBEAT__'):source.index('int main() {')]
source += '''
#undef cin
#undef cout
#undef endl
struct Beats_reference {
	using Tree = OY::ChminChmaxAddTree<int64_t, int32_t, int64_t, true, true, true>;
	using Node = Tree::node;
	Tree tree;
	explicit Beats_reference(const std::vector<long long> &a) : tree(a.size(), [&](auto i) { return a[i]; }) {}
	void chmin(int l, int r, long long x) { tree.add(l, r - 1, Node::Chmin{x}); }
	void chmax(int l, int r, long long x) { tree.add(l, r - 1, Node::Chmax{x}); }
	void add(int l, int r, long long x) { tree.add(l, r - 1, Node::Add{x}); }
	void assign(int l, int r, long long x) { tree.add(l, r - 1, Node::Assign{x}); }
	long long sum(int l, int r) { return tree.query<Node::SumGetter>(l, r - 1); }
	long long min(int l, int r) { return tree.query<Node::MinGetter>(l, r - 1); }
	long long max(int l, int r) { return tree.query<Node::MaxGetter>(l, r - 1); }
};
'''
(out / 'reference.hpp').write_text(source)
