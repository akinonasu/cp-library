#include "data_structure/segment_tree/dual_segtree.hpp"
#include "math/mint.hpp"
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>

struct Affine { mint a, b; };
mint mapping(Affine f, mint x) { return f.a * x + f.b; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

class BasicDual {
	int base_ = 1, log_ = 0;
	std::vector<mint> data_;
	std::vector<Affine> lazy_;
	std::vector<unsigned char> dirty_;
	void apply_node(int k, Affine f) {
		if (k >= base_) data_[k - base_] = mapping(f, data_[k - base_]);
		else { lazy_[k] = dirty_[k] ? composition(f, lazy_[k]) : f; dirty_[k] = 1; }
	}
	void push(int k) {
		if (!dirty_[k]) return;
		apply_node(2 * k, lazy_[k]);
		apply_node(2 * k + 1, lazy_[k]);
		dirty_[k] = 0;
	}
	void push_path(int k) { for (int h = log_; h; --h) push(k >> h); }

  public:
	explicit BasicDual(std::vector<mint> a) : data_(std::move(a)) {
		while (base_ < int(data_.size())) base_ *= 2, ++log_;
		lazy_.assign(base_, id()); dirty_.assign(base_, 0);
	}
	void apply(int l, int r, Affine f) {
		if (l == r) return;
		l += base_; r += base_;
		push_path(l); push_path(r - 1);
		while (l < r) {
			if (l & 1) apply_node(l++, f);
			if (r & 1) apply_node(--r, f);
			l >>= 1; r >>= 1;
		}
	}
	mint get(int p) { push_path(p + base_); return data_[p]; }
};

struct Query { int type, l, r; Affine f; };

template<class Tree>
void measure(const std::vector<mint> &a, const std::vector<Query> &queries) {
	Tree seg(a);
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (const auto &q : queries) {
		if (!q.type) seg.apply(q.l, q.r, q.f);
		else checksum = checksum * 1000000007 + seg.get(q.l).val();
	}
	auto end = std::chrono::steady_clock::now();
	for (int i = 0; i < int(a.size()); ++i) checksum = checksum * 1000000007 + seg.get(i).val();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 5) return 2;
	std::string variant = argv[1], kind = argv[2];
	int n = std::atoi(argv[3]), q = std::atoi(argv[4]);
	if (n <= 0 || q <= 0) return 2;
	std::mt19937 rng(20261019);
	std::vector<mint> a(n);
	for (auto &x : a) x = rng();
	std::vector<Query> queries;
	for (int i = 0; i < q; ++i) {
		int l = rng() % n, r = rng() % (n + 1);
		if (l > r) std::swap(l, r);
		int type = rng() & 1;
		if (kind == "full") l = 0, r = n;
		if (kind == "point") r = l + 1;
		if (kind == "prefix") l = 0;
		if (kind == "query_heavy") type = rng() % 10 != 0;
		if (kind == "hot") {
			type = i % 32 != 0;
			if (type) l = n / 2;
			else l = 0, r = n;
		}
		if (type) l = rng() % n;
		if (kind == "hot" && type) l = n / 2;
		Affine f{mint(rng()), mint(rng())};
		if (kind == "add") f.a = 1;
		queries.push_back({type, l, r, f});
	}
	if (variant == "baseline") measure<BasicDual>(a, queries);
	else if (variant == "library") measure<DualSegtree<mint, Affine, mapping, composition, id>>(a, queries);
	else if (variant == "commutative" && kind == "add") measure<DualSegtree<mint, Affine, mapping, composition, id, true>>(a, queries);
	else return 2;
}
