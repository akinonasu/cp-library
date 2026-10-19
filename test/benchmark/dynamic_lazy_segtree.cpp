#include "data_structure/segment_tree/dynamic_lazy_segtree.hpp"
#include "math/mint.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }
Sum initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }
using Tree = DynamicLazySegtree<Sum, op, e, Affine, mapping, composition, id, initial>;

class PushTree {
	struct Node { Sum value; Affine lazy; int left = 0, right = 0; bool dirty = false; };
	std::vector<Node> nodes{Node{e(), id()}};
	int root = 0;
	std::int64_t size;
	int create(std::int64_t l, std::int64_t r) {
		nodes.push_back(Node{initial(l, r), id()});
		return int(nodes.size() - 1);
	}
	void act(int k, Affine f, bool leaf) {
		nodes[k].value = mapping(f, nodes[k].value);
		if (!leaf) { nodes[k].lazy = nodes[k].dirty ? composition(f, nodes[k].lazy) : f; nodes[k].dirty = true; }
	}
	void push(int k, std::int64_t l, std::int64_t m, std::int64_t r) {
		if (!nodes[k].dirty) return;
		Affine f = nodes[k].lazy;
		if (!nodes[k].left) { int child = create(l, m); nodes[k].left = child; }
		if (!nodes[k].right) { int child = create(m, r); nodes[k].right = child; }
		act(nodes[k].left, f, m - l == 1);
		act(nodes[k].right, f, r - m == 1);
		nodes[k].dirty = false;
	}
	Sum value(int k, std::int64_t l, std::int64_t r) { return k ? nodes[k].value : initial(l, r); }
	int update(int k, std::int64_t l, std::int64_t r, std::int64_t a, std::int64_t b, Affine f) {
		if (!k) k = create(l, r);
		if (a == l && b == r) { act(k, f, r - l == 1); return k; }
		auto m = (l + r) / 2;
		push(k, l, m, r);
		if (a < m) { int child = update(nodes[k].left, l, m, a, std::min(b, m), f); nodes[k].left = child; }
		if (m < b) { int child = update(nodes[k].right, m, r, std::max(a, m), b, f); nodes[k].right = child; }
		nodes[k].value = op(value(nodes[k].left, l, m), value(nodes[k].right, m, r));
		return k;
	}
	Sum product(int k, std::int64_t l, std::int64_t r, std::int64_t a, std::int64_t b) {
		if (!k) return initial(a, b);
		if (a == l && b == r) return nodes[k].value;
		auto m = (l + r) / 2;
		push(k, l, m, r);
		if (b <= m) return product(nodes[k].left, l, m, a, b);
		if (m <= a) return product(nodes[k].right, m, r, a, b);
		return op(product(nodes[k].left, l, m, a, m), product(nodes[k].right, m, r, m, b));
	}
  public:
	explicit PushTree(std::int64_t n) : size(n) {}
	void apply(std::int64_t l, std::int64_t r, Affine f) { root = update(root, 0, size, l, r, f); }
	Sum prod(std::int64_t l, std::int64_t r) { return product(root, 0, size, l, r); }
	Sum all_prod() { return value(root, 0, size); }
	std::size_t node_count() { return nodes.size() - 1; }
};

struct Query { std::int64_t l, r; bool update; Affine f; };
template<class Seg>
void run(std::int64_t n, const std::vector<Query> &queries, bool full) {
	Seg seg(n);
	if (full) seg.apply(0, n, Affine{2, 3});
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (const auto &q : queries) {
		if (q.update) seg.apply(q.l, q.r, q.f);
		else checksum = checksum * 1000000007 + seg.prod(q.l, q.r).sum.val();
	}
	auto end = std::chrono::steady_clock::now();
	checksum = checksum * 1000000007 + seg.all_prod().sum.val();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
		<< ' ' << checksum << ' ' << seg.node_count() << '\n';
}

int main(int argc, char **argv) {
	if (argc != 4) return 2;
	std::string variant = argv[1], kind = argv[2];
	int count = std::atoi(argv[3]);
	std::int64_t n = kind == "dense" ? 1 << 17 : 1LL << 40;
	std::mt19937_64 rng(20261020);
	std::vector<Query> queries;
	for (int i = 0; i < count; ++i) {
		std::int64_t l = rng() % n, r = rng() % n;
		if (l > r) std::swap(l, r);
		if (kind == "sparse") r = std::min(n - 1, l + std::int64_t(rng() % 1024));
		bool update = kind != "read_heavy" && rng() % 2;
		queries.push_back({l, r + 1, update, Affine{mint(rng() % 998244353), mint(rng() % 998244353)}});
	}
	if (variant == "push") run<PushTree>(n, queries, kind == "read_heavy");
	else if (variant == "library") run<Tree>(n, queries, kind == "read_heavy");
	else return 2;
}
