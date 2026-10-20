#include "data_structure/segment_tree/persistent_segtree.hpp"
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }
using Candidate = PersistentSegtree<long long, op, e>;

class Baseline {
	struct Node { long long value; unsigned left, right; };
	int n;
	std::vector<Node> nodes{{0, 0, 0}};
	unsigned set_impl(unsigned root, int l, int r, int p, long long value) {
		unsigned next = nodes.size();
		nodes.push_back(nodes[root]);
		if (r - l == 1) nodes[next].value = value;
		else {
			int m = l + (r - l) / 2;
			if (p < m) nodes[next].left = set_impl(nodes[next].left, l, m, p, value);
			else nodes[next].right = set_impl(nodes[next].right, m, r, p, value);
			nodes[next].value = nodes[nodes[next].left].value + nodes[nodes[next].right].value;
		}
		return next;
	}
	long long prod_impl(unsigned root, int l, int r, int ql, int qr) const {
		if (!root || qr <= l || r <= ql) return 0;
		if (ql <= l && r <= qr) return nodes[root].value;
		int m = l + (r - l) / 2;
		return prod_impl(nodes[root].left, l, m, ql, qr) + prod_impl(nodes[root].right, m, r, ql, qr);
	}
	unsigned copy_impl(unsigned root, unsigned source, int l, int r, int ql, int qr) {
		if (root == source || qr <= l || r <= ql) return root;
		if (ql <= l && r <= qr) return source;
		int m = l + (r - l) / 2;
		unsigned a = nodes[root].left, b = nodes[root].right;
		unsigned c = nodes[source].left, d = nodes[source].right;
		unsigned left = copy_impl(a, c, l, m, ql, qr), right = copy_impl(b, d, m, r, ql, qr);
		if (left == a && right == b) return root;
		if (!left && !right) return 0;
		unsigned next = nodes.size();
		nodes.push_back({nodes[left].value + nodes[right].value, left, right});
		return next;
	}
  public:
	using root_type = unsigned;
	explicit Baseline(int n) : n(n) {}
	root_type initial_root() const { return 0; }
	void reserve(std::size_t count) { nodes.reserve(count + 1); }
	std::size_t node_count() const { return nodes.size() - 1; }
	std::size_t memory_usage() const { return nodes.capacity() * sizeof(Node); }
	root_type set(root_type root, int p, long long value) { return set_impl(root, 0, n, p, value); }
	root_type copy_range(root_type root, root_type source, int l, int r) {
		return l == r ? root : copy_impl(root, source, 0, n, l, r);
	}
	long long prod(root_type root, int l, int r) const { return l == r ? 0 : prod_impl(root, 0, n, l, r); }
};

struct Query { int type, version, other, l, r; long long value; };
struct Result { double ms; std::size_t nodes, bytes; std::vector<long long> answers; };

template <class Tree>
Result run(int n, const std::vector<Query> &queries, int warmup) {
	Tree tree(n);
	tree.reserve(queries.size() * (std::bit_width(unsigned(n - 1)) + 1));
	std::vector<typename Tree::root_type> versions{tree.initial_root()};
	versions.reserve(queries.size() + 1);
	std::vector<long long> answers;
	answers.reserve(queries.size());
	auto execute = [&](const Query &q) {
		if (q.type == 0) versions.push_back(tree.set(versions[q.version], q.l, q.value));
		else if (q.type == 1) answers.push_back(tree.prod(versions[q.version], q.l, q.r));
		else versions.push_back(tree.copy_range(versions[q.version], versions[q.other], q.l, q.r));
	};
	for (int i = 0; i < warmup; ++i) execute(queries[i]);
	auto start = std::chrono::steady_clock::now();
	for (std::size_t i = warmup; i < queries.size(); ++i) execute(queries[i]);
	double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count();
	return {ms, tree.node_count(), tree.memory_usage(), std::move(answers)};
}

int main() {
	constexpr int n = 131073, count = 160000;
	for (const std::string name : {"latest", "branching", "copy_mix", "read_heavy"}) {
		std::mt19937 random(20261021);
		int versions = 1, warmup = name == "read_heavy" ? 20000 : 0;
		std::vector<Query> queries;
		for (int i = 0; i < count + warmup; ++i) {
			int type = random() % 2;
			if (name == "copy_mix") type = random() % 4 == 0 ? 2 : type;
			if (name == "read_heavy") type = i < warmup ? 0 : 1;
			int version = name == "latest" || i < warmup ? versions - 1 : int(random() % versions);
			int other = random() % versions, l = random() % n, r = random() % (n + 1);
			if (type && r < l) std::swap(l, r);
			queries.push_back({type, version, other, l, r, static_cast<long long>(random() % 1000)});
			if (type != 1) ++versions;
		}
		for (int trial = 0; trial < 5; ++trial) {
			Result a, b;
			if (trial % 2) { b = run<Candidate>(n, queries, warmup); a = run<Baseline>(n, queries, warmup); }
			else { a = run<Baseline>(n, queries, warmup); b = run<Candidate>(n, queries, warmup); }
			if (a.answers != b.answers || a.nodes != b.nodes) {
				std::cerr << name << ": answers=" << (a.answers == b.answers) << ", nodes=" << a.nodes << '/' << b.nodes << '\n';
				return 1;
			}
			std::uint64_t checksum = 0;
			for (auto x : a.answers) checksum = checksum * 1000000007 + static_cast<std::uint64_t>(x);
			std::cout << "{\"case\":\"" << name << "\",\"trial\":" << trial
				<< ",\"baseline_ms\":" << a.ms << ",\"candidate_ms\":" << b.ms
				<< ",\"nodes\":" << b.nodes << ",\"bytes\":" << b.bytes
				<< ",\"answers\":" << b.answers.size() << ",\"checksum\":" << checksum << "}\n";
		}
	}
}
