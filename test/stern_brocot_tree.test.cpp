#define PROBLEM "https://judge.yosupo.jp/problem/stern_brocot_tree"
#include "../math/number_theory/stern_brocot_tree.hpp"
#include "../components/fast_io.hpp"
#include <array>
#include <cstdint>

int main() {
#if defined(BENCH_STERN_BROCOT_64)
	using Tree = SternBrocotTree<std::uint64_t>;
#else
	using Tree = SternBrocotTree<std::uint32_t>;
#endif
	FastInput in;
	FastOutput out;
	int q = 0;
	if (!in.read(q)) return 0;
	auto read = [&] { Tree::count_type x = 0; in.read(x); return x; };
	auto print = [&](Tree node) { auto [a, b] = node.value(); out.write(a); out.write(' '); out.println(b); };
	while (q--) {
		char command = in.token()[0];
		if (command == 'E') {
			auto a = read(), b = read();
			std::array<Tree::Run, 128> path;
			int n = 0;
			Tree::encode(a, b, [&](char direction, auto count) { path[n++] = {direction, count}; });
			out.write(n);
			for (int i = 0; i < n; ++i) { out.write(' '); out.write(path[i].direction); out.write(' '); out.write(path[i].count); }
			out.write('\n');
		} else if (command == 'D') {
			auto n = read();
			Tree node;
			while (n--) {
				bool right = in.token()[0] == 'R';
				auto count = read();
				[[maybe_unused]] bool ok = right ? node.move_right(count) : node.move_left(count);
				assert(ok);
			}
			print(node);
		} else if (command == 'L') {
			auto a = read(), b = read(), c = read(), d = read();
			print(Tree::lca({a, b}, {c, d}));
		} else if (command == 'A') {
			auto depth = read(), a = read(), b = read();
			auto node = Tree::ancestor({a, b}, depth);
			if (node) print(*node);
			else out.println(-1);
		} else {
			auto a = read(), b = read();
			auto [lower, upper] = Tree(a, b).range();
			out.write(lower.first); out.write(' '); out.write(lower.second); out.write(' ');
			out.write(upper.first); out.write(' '); out.println(upper.second);
		}
	}
}
