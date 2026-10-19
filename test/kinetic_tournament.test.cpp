#define PROBLEM "https://judge.yosupo.jp/problem/line_add_get_min"
#include "../data_structure/segment_tree/kinetic_tournament.hpp"
#include "../components/fast_io.hpp"

int main() {
	using Tree = KineticTournament<>;
	FastInput in;
	FastOutput out;
	int n = 0, q = 0;
	in.read(n); in.read(q);
	std::vector<Tree::Line> lines(n);
	for (auto &[a, b] : lines) { in.read(a); in.read(b); }
	struct Query { std::int64_t x; int count, index; };
	std::vector<Query> queries;
	for (int i = 0; i < q; ++i) {
		int type = 0;
		std::int64_t a = 0, b = 0;
		in.read(type); in.read(a);
		if (!type) { in.read(b); lines.push_back({a, b}); }
		else queries.push_back({a, int(lines.size()), int(queries.size())});
	}
	if (queries.empty()) return 0;
	std::sort(queries.begin(), queries.end(), [](auto a, auto b) { return a.x < b.x; });
	Tree tree(lines, queries.front().x);
	std::vector<Tree::value_type> answer(queries.size());
	for (auto query : queries) {
		tree.advance(query.x);
		answer[query.index] = tree.prod(0, query.count)->value;
	}
	for (auto value : answer) out.println(value);
}
