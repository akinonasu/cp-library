#include "data_structure/dynamic_li_chao_tree.hpp"
#include <bit>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>

using T = std::int64_t;
using R = __int128_t;

template<class V> class Baseline {
	struct Line {
		T a = 0, b = 0;
		V eval(T x) const { return V(R(a) * x + b); }
	};
	struct Node {
		Line line;
		std::unique_ptr<Node> left, right;
		bool present = false;
	};
	std::unique_ptr<Node> root;
	T low, high;
	std::size_t count = 0;
	void insert(std::unique_ptr<Node>& node, T l, T r, Line f) {
		if (!node) node = std::make_unique<Node>(), ++count;
		if (!node->present) { node->line = f, node->present = true; return; }
		auto& g = node->line;
		V fl = f.eval(l), fr = f.eval(r), gl = g.eval(l), gr = g.eval(r);
		if (gl <= fl && gr <= fr) return;
		if (fl <= gl && fr <= gr) { g = f; return; }
		T m = std::midpoint(l, r);
		if (f.eval(m) < g.eval(m)) std::swap(f, g);
		if (f.eval(l) < g.eval(l)) insert(node->left, l, m, f);
		else insert(node->right, m + 1, r, f);
	}
	void segment(std::unique_ptr<Node>& node, T l, T r, T ql, T qr, Line f) {
		if (node && node->present) {
			T a = std::max(l, ql), b = std::min(r, qr);
			if (node->line.eval(a) <= f.eval(a) && node->line.eval(b) <= f.eval(b)) return;
		}
		if (ql <= l && r <= qr) { insert(node, l, r, f); return; }
		if (!node) node = std::make_unique<Node>(), ++count;
		T m = std::midpoint(l, r);
		if (ql <= m) segment(node->left, l, m, ql, qr, f);
		if (m < qr) segment(node->right, m + 1, r, ql, qr, f);
	}
	std::optional<R> get(const Node* node, T l, T r, T x) const {
		if (!node) return std::nullopt;
		std::optional<R> result;
		if (node->present) result = node->line.eval(x);
		if (l == r) return result;
		T m = std::midpoint(l, r);
		auto child = x <= m ? get(node->left.get(), l, m, x) : get(node->right.get(), m + 1, r, x);
		return !result ? child : child ? std::min(*result, *child) : result;
	}

  public:
	Baseline(R l, R r) : low(T(l)), high(T(r - 1)) {}
	void reserve(std::size_t) {}
	std::size_t node_count() const { return count; }
	void add_line(T a, T b) { insert(root, low, high, {a, b}); }
	void add_segment(T l, T r, T a, T b) { if (l < r) segment(root, low, high, l, r - 1, {a, b}); }
	std::optional<R> query(T x) const { return get(root.get(), low, high, x); }
};

struct Operation { T l, r, a, b, x; };

template<class Tree> void run(const std::string& shape, const std::vector<Operation>& operations, R low, R high) {
	auto start = std::chrono::steady_clock::now();
	Tree tree(low, high);
	tree.reserve(operations.size());
	if (shape == "dominated") tree.add_line(0, -8000000000000000000LL);
	std::uint64_t hash = 0;
	for (const auto& f : operations) {
		if (shape == "segments" || shape == "points" || shape == "dominated") tree.add_segment(f.l, f.r, f.a, f.b);
		else tree.add_line(f.a, f.b);
		auto result = tree.query(f.x);
		__uint128_t value = result ? __uint128_t(*result) : (__uint128_t(1) << 127);
		hash = (hash ^ std::uint64_t(value) ^ std::uint64_t(value >> 64)) * 0x9e3779b97f4a7c15ULL;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::setprecision(12) << std::chrono::duration<double>(end - start).count() << '\n'
		<< hash << '\n' << tree.node_count() << '\n';
}

int main(int argc, char** argv) {
	if (argc != 4) return 1;
	std::string variant = argv[1], shape = argv[2];
	int count = std::atoi(argv[3]);
	std::mt19937_64 rng(731);
	std::vector<Operation> operations(count);
	R low = -1000000000, high = 1000000001;
	if (shape == "hull") low = 0, high = count;
	if (shape == "wide") low = std::numeric_limits<T>::lowest(), high = R(std::numeric_limits<T>::max()) + 1;
	for (int i = 0; i < count; ++i) {
		auto& f = operations[i];
		f.a = T(rng() % 2000000001) - 1000000000;
		f.b = T(rng() % 2000000000000000001ULL) - 1000000000000000000LL;
		f.l = T(rng() % 2000000001) - 1000000000, f.r = T(rng() % 2000000001) - 1000000000;
		if (f.r < f.l) std::swap(f.l, f.r);
		f.x = T(rng() % 2000000001) - 1000000000;
		if (shape == "points") f.r = f.l + 1, f.x = i & 1 ? f.l : f.x;
		if (shape == "same_slope") f.a = 7;
		if (shape == "hull") f.a = -2LL * i, f.b = 1LL * i * i, f.x = rng() % count;
		if (shape == "wide") f.a = std::bit_cast<T>(rng()), f.b = std::bit_cast<T>(rng()), f.x = std::bit_cast<T>(rng());
	}
	if (variant == "library") run<DynamicLiChaoTree<>>(shape, operations, low, high);
	else if (variant == "library64") run<DynamicLiChaoTree<T, false, T>>(shape, operations, low, high);
	else if (shape == "wide") run<Baseline<R>>(shape, operations, low, high);
	else run<Baseline<T>>(shape, operations, low, high);
}
