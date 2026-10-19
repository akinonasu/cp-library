#include "data_structure/segment_tree/kinetic_tournament.hpp"
#include "ekzhang.cpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

struct Reference {
	kinetic_tournament<std::int64_t> tree;
	int n;
	explicit Reference(int size) : tree(size), n(size) {}
	void set(int i, std::int64_t a, std::int64_t b) { tree.update(i, a, b); }
	void advance(std::int64_t t) { tree.heaten(t); }
	std::int64_t prod(int l, int r) { return tree.query(l, r - 1); }
	std::int64_t all() { return prod(0, n); }
};

template <class R>
struct Library {
	KineticTournament<std::int64_t, false, R> tree;
	explicit Library(int n) : tree(n) {}
	void set(int i, std::int64_t a, std::int64_t b) { tree.set(i, a, b); }
	void advance(std::int64_t t) { tree.advance(t); }
	std::int64_t prod(int l, int r) { return std::int64_t(tree.prod(l, r)->value); }
	std::int64_t all() { return std::int64_t(tree.all_prod()->value); }
};

struct Operation { int type, l, r; std::int64_t a, b; };

template <class Tree>
void run(const std::string &kind) {
	constexpr int n = 100000, q = 100000;
	std::mt19937_64 rng(20261020);
	Tree tree(n);
	for (int i = 0; i < n; ++i) {
		std::int64_t a, b;
		if (kind == "envelope" || kind == "jump") a = n - i - 1, b = std::int64_t(i) * (i + 1) / 2;
		else if (kind == "concurrent") a = i, b = std::int64_t(n - i - 1) * n;
		else a = kind == "same_slope" ? 7 : rng() % 1000000, b = rng() % 1000000000000ULL;
		tree.set(i, a, b);
	}
	std::vector<Operation> operations;
	if (kind == "jump" || kind == "concurrent") operations.push_back({1, 0, 0, n, 0});
	else for (int i = 0; i < q; ++i) {
		if (kind == "envelope") { operations.push_back({1, 0, 0, i, 0}); operations.push_back({3, 0, 0, 0, 0}); }
		else if (kind == "same_slope") {
			operations.push_back({1, 0, 0, i, 0});
			int l = rng() % n, r = l + 1 + rng() % (n - l);
			operations.push_back({2, l, r, 0, 0});
		} else if (i % 3 == 0) operations.push_back({0, int(rng() % n), 0, std::int64_t(rng() % 1000000), std::int64_t(rng() % 1000000000000ULL)});
		else if (i % 3 == 1) operations.push_back({1, 0, 0, i, 0});
		else {
			int l = rng() % n, r = l + 1 + rng() % (n - l);
			operations.push_back({2, l, r, 0, 0});
		}
	}
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	for (auto op : operations) {
		if (!op.type) tree.set(op.l, op.a, op.b);
		else if (op.type == 1) tree.advance(op.a);
		else checksum = checksum * 1000000007 + std::uint64_t(op.type == 2 ? tree.prod(op.l, op.r) : tree.all());
	}
	checksum = checksum * 1000000007 + std::uint64_t(tree.all());
	auto elapsed = std::chrono::steady_clock::now() - begin;
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 3) return 2;
	std::string mode = argv[1];
	if (mode == "reference") run<Reference>(argv[2]);
	else if (mode == "library64") run<Library<std::int64_t>>(argv[2]);
	else if (mode == "library") run<Library<__int128_t>>(argv[2]);
	else return 2;
}
