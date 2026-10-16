#include "data_structure/lazy_reversible_treap.hpp"
#include "standard.hpp"
#include "direct.hpp"
#include "priority64.hpp"
#include "split_reverse.hpp"
#include "leaf.hpp"
#include "packed.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <vector>

struct F { mint a, b; };
mint op(mint a, mint b) { return a + b; }
mint e() { return 0; }
mint mapping(F f, mint x, int n) { return f.a * x + f.b * n; }
F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
F id() { return {1, 0}; }
struct Query { int type, l, r; F f; };
using Clock = std::chrono::steady_clock;
double elapsed(Clock::time_point begin) { return std::chrono::duration<double, std::milli>(Clock::now() - begin).count(); }
struct Result { double build, run; std::size_t bytes; std::uint64_t checksum; };

template <template <class, auto, auto, class, auto, auto, auto, bool> class Tree, bool C>
Result run(const std::vector<mint> &a, const std::vector<Query> &queries) {
	auto begin = Clock::now();
	Tree<mint, op, e, F, mapping, composition, id, C> tree(a);
	tree.reserve(a.size() + queries.size());
	double build = elapsed(begin);
	begin = Clock::now();
	std::uint64_t checksum = 0;
	for (const auto &q : queries) {
		if (q.type == 0) tree.insert(q.l, q.f.b);
		else if (q.type == 1) checksum = checksum * 1000000007 + tree.erase(q.l).val();
		else if (q.type == 2) tree.reverse(q.l, q.r);
		else if (q.type == 3) tree.apply(q.l, q.r, q.f);
		else checksum = checksum * 1000000007 + tree.prod(q.l, q.r).val();
	}
	checksum = checksum * 1000000007 + tree.all_prod().val();
	return {build, elapsed(begin), tree.memory_usage(), checksum};
}

int main(int argc, char **argv) {
	int n = argc > 1 ? std::atoi(argv[1]) : 100000;
	int q = argc > 2 ? std::atoi(argv[2]) : 200000;
	int trials = argc > 3 ? std::atoi(argv[3]) : 5;
	std::mt19937_64 rng(769345);
	std::vector<mint> a(n);
	for (auto &x : a) x = rng() % mint::mod();
	using Fn = Result (*)(const std::vector<mint> &, const std::vector<Query> &);
	Fn functions[] = {run<Treap_standard, true>, run<Treap_direct, true>, run<Treap_priority64, true>, run<Treap_split_reverse, true>, run<Treap_leaf, true>, run<Treap_direct, false>, run<Treap_leaf, false>, run<LazyReversibleTreap, true>, run<Treap_packed, true>, run<Treap_packed, false>};
	const char *names[] = {"standard", "direct", "priority64", "split_reverse", "leaf", "direct_ordered", "leaf_ordered", "current", "packed", "packed_ordered"};
	std::puts("workload,variant,trial,build_ms,run_ms,bytes,checksum");
	for (int pattern = 0; pattern < 3; ++pattern) {
		std::vector<Query> queries;
		int size = n;
		for (int i = 0; i < q; ++i) {
			int type = rng() % 5;
			if (!size) type = 0;
			int l = rng() % (size + 1), r = rng() % (size + 1);
			if (l > r) std::swap(l, r);
			if (pattern == 1) r = std::min(size, l + int(rng() % 129));
			if (pattern == 2 && (rng() & 1)) l = 0, r = size;
			if (type == 1) l = rng() % size;
			queries.push_back({type, l, r, {rng() % mint::mod(), rng() % mint::mod()}});
			size += (type == 0) - (type == 1);
		}
		std::uint64_t expected = 0;
		for (int trial = 0; trial < trials; ++trial) {
			std::vector<int> order(std::size(functions));
			std::iota(order.begin(), order.end(), 0);
			std::shuffle(order.begin(), order.end(), rng);
			for (int index : order) {
				auto result = functions[index](a, queries);
				if (!trial && index == order[0]) expected = result.checksum;
				if (result.checksum != expected) std::abort();
				std::printf("%s,%s,%d,%.6f,%.6f,%zu,%llu\n", pattern == 0 ? "random" : pattern == 1 ? "local" : "whole", names[index], trial, result.build, result.run, result.bytes, (unsigned long long)result.checksum);
			}
		}
	}
}
