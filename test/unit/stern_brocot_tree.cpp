#include "math/number_theory/stern_brocot_tree.hpp"
#include "math/fraction.hpp"
#include <array>
#include <cassert>
#include <climits>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <vector>

using Tree = SternBrocotTree<>;
using U = std::uint64_t;
using W = __uint128_t;
using P = Tree::value_type;

Tree::path_type reference(U a, U b) {
	Tree::path_type path;
	bool right = true;
	while (b) {
		U q = a / b, r = a % b;
		if (!r) --q;
		if (q) path.push_back({right ? 'R' : 'L', q});
		a = b; b = r; right = !right;
	}
	return path;
}

Tree from_prefix(const Tree::path_type &path, U depth) {
	Tree result;
	for (auto [direction, count] : path) {
		U step = std::min(depth, count);
		bool ok = direction == 'R' ? result.move_right(step) : result.move_left(step);
		assert(ok); (void)ok;
		depth -= step;
		if (!depth) break;
	}
	assert(!depth);
	return result;
}

void check(U a, U b, std::mt19937_64 &rng) {
	Tree node(a, b);
	U g = std::gcd(a, b);
	assert(node.value() == P(a / g, b / g));
	assert(node.numerator() == a / g && node.denominator() == b / g);
	auto expected = reference(a, b);
	assert(node.path() == expected && Tree::encode(a, b) == expected);
	Tree::path_type callback;
	Tree::encode(a, b, [&](char direction, U count) { callback.push_back({direction, count}); });
	assert(callback == expected);
	assert(Tree::decode(expected) == node);
	U depth = 0;
	for (auto run : expected) depth += run.count;
	assert(node.depth() == depth && node.is_root() == (a == b));
	auto [l, r] = node.range();
	assert(l == node.lower_bound() && r == node.upper_bound());
	assert(l.first + r.first == node.numerator() && l.second + r.second == node.denominator());
	assert(W(r.first) * l.second - W(l.first) * r.second == 1);
	assert(node.contains(node.value()) && node.contains({a, b}) && node.is_ancestor(node));
	assert(!node.ancestor(depth + 1) && !Tree::ancestor({a, b}, depth + 1));
	for (U target : {U(0), depth, depth / 2, depth ? depth - 1 : 0, rng() % (depth + 1)}) {
		auto ancestor = from_prefix(expected, target);
		assert(node.ancestor(target) == ancestor && Tree::ancestor({a, b}, target) == ancestor);
		auto moved = node;
		assert(moved.move_up(depth - target) && moved == ancestor);
		assert(ancestor.is_ancestor(node) && ancestor.contains(node.value()));
		assert(node.distance(ancestor) == W(depth - target));
	}
	auto copy = node;
	assert(!copy.move_up(depth + 1) && copy == node && copy.move_up(0) && copy == node);
	assert(copy.move_left(0) && copy.move_right(0) && copy == node);
	if (depth) {
		auto parent = from_prefix(expected, depth - 1);
		assert(node.parent() == parent && node.lca(parent) == parent);
		assert(copy.move_up() && copy == parent);
	} else assert(!node.parent());
	for (bool right : {false, true}) {
		auto bound = right ? r : l;
		U count = rng() % 100;
		W n = W(node.numerator()) + W(count) * bound.first;
		W d = W(node.denominator()) + W(count) * bound.second;
		bool fits = n <= UINT64_MAX && d <= UINT64_MAX;
		auto moved = node;
		bool ok = right ? moved.move_right(count) : moved.move_left(count);
		assert(ok == fits);
		auto child = right ? node.right(count) : node.left(count);
		assert(bool(child) == fits);
		if (fits) {
			assert(moved.value() == P(U(n), U(d)) && *child == moved);
			assert(moved.depth() == depth + count && node.is_ancestor(moved));
			assert(moved.move_up(count) && moved == node);
		} else assert(moved == node);
	}
}

void pair_check(P x, P y) {
	Tree a(x), b(y);
	auto p = reference(x.first, x.second), q = reference(y.first, y.second);
	Tree::path_type common;
	for (std::size_t i = 0; i < std::min(p.size(), q.size()) && p[i].direction == q[i].direction; ++i) {
		common.push_back({p[i].direction, std::min(p[i].count, q[i].count)});
		if (p[i].count != q[i].count) break;
	}
	auto expected = *Tree::decode(common);
	assert(a.lca(b) == expected && Tree::lca(x, y) == expected && b.lca(a) == expected);
	assert(a.distance(b) == W(a.depth()) + b.depth() - 2 * W(expected.depth()));
	assert(a.is_ancestor(b) == (expected == a));
}

int main() {
	std::mt19937_64 rng(519214);
	Tree root;
	assert(root.value() == P(1, 1) && root.range() == std::pair(P(0, 1), P(1, 0)));
	assert(Tree::decode(std::array<Tree::Run, 0>{}) == root);
	assert(!Tree::decode(std::array<Tree::Run, 1>{{{'X', 1}}}));
	assert(Tree::decode(std::array<Tree::Run, 4>{{{'L', 0}, {'R', 2}, {'R', 3}, {'L', 0}}})->value() == P(6, 1));
	assert(!Tree::decode(std::array<Tree::Run, 1>{{{'L', UINT64_MAX}}}));
	assert(!root.move_left(UINT64_MAX) && !root.move_right(UINT64_MAX) && root.is_root());
	for (U a = 1; a <= 65; ++a) for (U b = 1; b <= 65; ++b) check(a, b, rng);
	std::vector<std::pair<Tree, std::string>> nodes{{root, ""}};
	for (std::size_t i = 0; i < 2047; ++i) {
		auto [node, path] = nodes[i];
		if (path.size() == 10) continue;
		nodes.emplace_back(*node.left(), path + 'L');
		nodes.emplace_back(*node.right(), path + 'R');
	}
	for (const auto &[node, path] : nodes) {
		std::string expanded;
		for (auto run : node.path()) expanded.append(run.count, run.direction);
		assert(expanded == path && node.depth() == path.size());
		if (!node.is_root()) assert(node.parent()->depth() + 1 == node.depth());
	}
	for (int i = 0; i < 6000; ++i) {
		U a = rng() | 1, b = rng() | 1;
		check(a, b, rng);
		pair_check({a, b}, {rng() | 1, rng() | 1});
	}
	for (U a : {U(1), U(2), UINT64_MAX / 2, UINT64_MAX - 1, UINT64_MAX}) {
		for (U b : {U(1), U(2), UINT64_MAX - 1, UINT64_MAX}) check(a, b, rng);
	}
	U a = 1, b = 1;
	while (a <= UINT64_MAX - b) { pair_check({a, b}, {b, a}); check(a, b, rng); auto c = a + b; a = b; b = c; }
	Tree high(UINT64_MAX, 1), low(1, UINT64_MAX);
	assert(high.depth() == UINT64_MAX - 1 && high.distance(low) == W(2) * (UINT64_MAX - 1));
	assert(high.lca(low).is_root() && high.move_up(UINT64_MAX - 1) && high.is_root());
	SternBrocotTree<std::int64_t> signed_node(INT64_MAX, INT64_MAX - 1);
	assert(signed_node.depth() == U(INT64_MAX - 1));
	auto saved = signed_node;
	assert(!signed_node.move_right(UINT64_MAX) && signed_node == saved);
	assert(signed_node.move_up(signed_node.depth()) && signed_node.is_root());
	SternBrocotTree<std::uint32_t> small(UINT32_MAX, 1);
	assert(small.distance(SternBrocotTree<std::uint32_t>(1, UINT32_MAX)) == 2 * (std::uint64_t(UINT32_MAX) - 1));
	SternBrocotTree<std::int16_t> narrow(INT16_MAX, 1);
	assert(!narrow.move_right() && narrow.move_up(INT16_MAX - 1) && narrow.is_root());
	fraction rational(22, 14);
	assert(Tree(rational.numerator(), rational.denominator()).value() == P(11, 7));
}
