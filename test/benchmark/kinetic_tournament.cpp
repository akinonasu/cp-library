#include "data_structure/advanced/kinetic_tournament.hpp"
#include "reference.hpp"
#include <chrono>
#include <iostream>
#include <numeric>
#include <random>

#define KINETIC_TOURNAMENT_JSC_TEST
#include "test/benchmark/kinetic_tournament_jsc.cpp"

struct Reference {
	reference::KineticRangeLinearAddMinTree tree;
	explicit Reference(const std::vector<std::int64_t> &values) : tree(values) {}
	void apply(int l, int r, std::int64_t a, std::int64_t b) { tree.add(l, r, a, b); }
	std::int64_t prod(int l, int r) { return tree.minimum(l, r); }
};
struct Library {
	KineticTournament<std::int64_t, false> tree;
	static std::vector<std::int64_t> slopes(std::size_t n) { std::vector<std::int64_t> v(n); std::iota(v.begin(), v.end(), 0); return v; }
	explicit Library(const std::vector<std::int64_t> &values) : tree(slopes(values.size()), values) {}
	void apply(int l, int r, std::int64_t a, std::int64_t b) { tree.apply(l, r, a, b); }
	std::int64_t prod(int l, int r) { return tree.prod(l, r)->value; }
};
struct Operation { int type, l, r; std::int64_t a, b; };

template<class Tree> void run(const std::string &kind) {
	constexpr int n = 100000, q = 100000;
	std::mt19937_64 rng(20261020);
	std::vector<std::int64_t> values(n);
	for (int i = 0; i < n; ++i) values[i] = kind == "random" || kind == "partial" ? std::int64_t(rng() % 2000000000001ULL) - 1000000000000LL : std::int64_t(n - i - 1) * (n - i) / 2;
	Tree tree(values);
	std::vector<Operation> ops;
	if (kind == "jump") { ops.push_back({0, 0, n, n, 0}); ops.push_back({1, 0, n, 0, 0}); }
	else for (int i = 0; i < q; ++i) {
		if (kind == "envelope") { ops.push_back({0, 0, n, 1, 0}); ops.push_back({1, 0, n, 0, 0}); }
		else {
			int l = rng() % n, r = l + 1 + rng() % (n - l);
			if (i % 3 == 2) ops.push_back({1, l, r, 0, 0});
			else ops.push_back({0, l, r, std::int64_t(rng() % 11), std::int64_t(rng() % 2000001) - 1000000});
			if (kind == "partial" && i % 3 == 1) ops.push_back({0, 0, n / 2, 0, -1000000000});
		}
	}
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (auto op : ops) {
		if (!op.type) tree.apply(op.l, op.r, op.a, op.b);
		else checksum = checksum * 1000000007 + std::uint64_t(tree.prod(op.l, op.r));
	}
	auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
	std::cout << elapsed << ' ' << checksum << '\n';
}

int main(int argc, char **argv) {
	if (argc != 3) return 1;
	std::string mode = argv[1], kind = argv[2];
	if (mode == "reference") run<Reference>(kind);
	else if (mode == "library") run<Library>(kind);
	else {
		std::mt19937_64 rng(478321);
		std::vector<std::int64_t> a(500000);
		for (int i = 0; i < int(a.size()); ++i) {
			if (kind == "equal") a[i] = 1000000000;
			else if (kind == "ascending") a[i] = i + 1;
			else if (kind == "descending") a[i] = a.size() - i;
			else if (kind == "alternating") a[i] = i % 2 ? i + 1 : 1000000000 - i;
			else a[i] = 1 + rng() % 1000000000;
		}
		auto start = std::chrono::steady_clock::now();
		auto answer = solve(a);
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
		std::cout << elapsed << ' ' << answer << '\n';
	}
}
