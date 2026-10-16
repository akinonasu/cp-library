#include "data_structure/dynamic_segtree.hpp"
#include "math/mint.hpp"
#include "base.hpp"
#include "branch.hpp"
#include "leaf.hpp"
#include "cover.hpp"
#include "layout.hpp"
#include "recursive.hpp"
#include "combo_branch.hpp"
#include "combo_leaf.hpp"
#include "dynamic_segtree_block.hpp"
#include "dynamic_segtree_bucket.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

using I = std::int64_t;
using U = std::uint64_t;
bool focus = false;
I add(I a, I b) { return a + b; }
I zero() { return 0; }
struct Odd { I alternating; unsigned parity; };
Odd odd_op(Odd a, Odd b) { return {a.alternating + (a.parity ? -b.alternating : b.alternating), a.parity ^ b.parity}; }
Odd odd_e() { return {0, 0}; }
using Affine = std::pair<mint, mint>;
Affine affine_op(Affine a, Affine b) { return {b.first * a.first, b.first * a.second + b.second}; }
Affine affine_e() { return {1, 0}; }
U hash(I x) { return U(x); }
U hash(Odd x) { return U(x.alternating) * 1000000007 + x.parity; }
U hash(Affine x) { return U(x.first.val()) * 1000000007 + x.second.val(); }
template <class S> S element(int x);
template <> I element<I>(int x) { return x; }
template <> Odd element<Odd>(int x) { return {x, 1}; }
template <> Affine element<Affine>(int x) { return {x + 1, x * 3 + 1}; }

struct Work {
	std::vector<I> keys;
	std::vector<int> changes;
	std::vector<std::pair<I, I>> ranges;
};

template <class Tree>
U exercise(const char *variant, const char *type, const std::string &kind, const Work &w, int trial) {
	using S = typename Tree::value_type;
	constexpr I size = I(1) << 62;
	auto start = std::chrono::steady_clock::now();
	Tree tree(size);
	tree.reserve(w.keys.size());
	for (int i = 0; i < int(w.keys.size()); ++i) tree.set(w.keys[i], element<S>(i % 1000));
	auto built = std::chrono::steady_clock::now();
	for (int i = 0; i < int(w.changes.size()); ++i) tree.set(w.keys[w.changes[i]], element<S>(i % 1000));
	auto updated = std::chrono::steady_clock::now();
	U checksum = hash(tree.all_prod());
	for (auto [l, r] : w.ranges) checksum = checksum * 1000000007 + hash(tree.prod(l, r));
	auto queried = std::chrono::steady_clock::now();
	for (int i : w.changes) checksum = checksum * 1000000007 + hash(tree.get(w.keys[i]));
	auto got = std::chrono::steady_clock::now();
	auto ms = [](auto a, auto b) { return std::chrono::duration<double, std::milli>(b - a).count(); };
	std::cout << kind << ',' << type << ',' << variant << ',' << trial << ',' << ms(start, built) << ',' << ms(built, updated) << ',' << ms(updated, queried) << ',' << ms(queried, got) << ',' << tree.node_count() << ',' << tree.memory_usage() << ',' << checksum << '\n';
	return checksum;
}

template <class S, auto op, auto e>
void run(const char *type, const std::string &kind, const Work &w, int trials, std::mt19937_64 &rng) {
	std::array<int, 20> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	for (int trial = 0; trial < trials; ++trial) {
		std::shuffle(order.begin(), order.end(), rng);
		U expected = 0;
		bool first = true;
		for (int id : order) {
			if (focus && id != 0 && id != 6 && id < 11) continue;
			U checksum = 0;
			if (id == 0) checksum = exercise<DynamicSegtree_base<S, op, e>>("base", type, kind, w, trial);
			if (id == 1) checksum = exercise<DynamicSegtree_branch<S, op, e>>("branch", type, kind, w, trial);
			if (id == 2) checksum = exercise<DynamicSegtree_leaf<S, op, e>>("leaf", type, kind, w, trial);
			if (id == 3) checksum = exercise<DynamicSegtree_cover<S, op, e>>("cover", type, kind, w, trial);
			if (id == 4) checksum = exercise<DynamicSegtree_layout<S, op, e>>("layout", type, kind, w, trial);
			if (id == 5) checksum = exercise<DynamicSegtree_recursive<S, op, e>>("recursive", type, kind, w, trial);
			if (id == 6) checksum = exercise<DynamicSegtree<S, op, e>>("current", type, kind, w, trial);
			if (id == 7) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 8, 4>>("block8x4", type, kind, w, trial);
			if (id == 8) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 16, 4>>("block16x4", type, kind, w, trial);
			if (id == 9) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 16, 8>>("block16x8", type, kind, w, trial);
			if (id == 10) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 32, 8>>("block32x8", type, kind, w, trial);
			if (id == 11) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 8, 4, true>>("balanced8x4", type, kind, w, trial);
			if (id == 12) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 16, 4, true>>("balanced16x4", type, kind, w, trial);
			if (id == 13) checksum = exercise<DynamicSegtreeBlock<S, op, e, I, 32, 8, true>>("balanced32x8", type, kind, w, trial);
			if (id == 14) checksum = exercise<DynamicSegtree_combo_branch<S, op, e>>("combo_branch", type, kind, w, trial);
			if (id == 15) checksum = exercise<DynamicSegtree_combo_leaf<S, op, e>>("combo_leaf", type, kind, w, trial);
			if (id == 16) checksum = exercise<DynamicSegtreeBucket<S, op, e, I, 4, true>>("bucket4", type, kind, w, trial);
			if (id == 17) checksum = exercise<DynamicSegtreeBucket<S, op, e, I, 8, true>>("bucket8", type, kind, w, trial);
			if (id == 18) checksum = exercise<DynamicSegtreeBucket<S, op, e, I, 16, true>>("bucket16", type, kind, w, trial);
			if (id == 19) checksum = exercise<DynamicSegtreeBucket<S, op, e, I, 8, false>>("bucket_linear", type, kind, w, trial);
			if (first) expected = checksum, first = false;
			else if (expected != checksum) std::abort();
		}
	}
}

template <class Tree>
U online(const char *variant, const std::vector<unsigned> &input, int trial) {
	auto start = std::chrono::steady_clock::now();
	Tree tree(1000000001);
	tree.reserve(input.size());
	I total = 0, answer = 0;
	U checksum = 0;
	for (unsigned y : input) {
		I x = (y + answer) % 1000000000 + 1;
		total += x;
		tree.multiply(x, Odd{x, 1});
		answer = (total + tree.all_prod().alternating) / 2;
		checksum = checksum * 1000000007 + U(answer);
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "abc403g,odd," << variant << ',' << trial << ',' << std::chrono::duration<double, std::milli>(end - start).count() << ",0,0,0," << tree.node_count() << ',' << tree.memory_usage() << ',' << checksum << '\n';
	return checksum;
}

U packed_op(U a, U b) {
	U value = b & ~U(1);
	if (a & 1) value = -value;
	return (a & ~U(1)) + value + ((a ^ b) & 1);
}
U packed_e() { return 0; }
template <class Tree>
U packed_online(const char *variant, const std::vector<unsigned> &input, int trial) {
	auto start = std::chrono::steady_clock::now();
	Tree tree(1000000001);
	tree.reserve(input.size());
	I total = 0, answer = 0;
	U checksum = 0;
	for (unsigned y : input) {
		I x = (y + answer) % 1000000000 + 1;
		total += x;
		tree.multiply(x, (U(x) << 1) | 1);
		answer = (total + (std::bit_cast<I>(tree.all_prod()) >> 1)) / 2;
		checksum = checksum * 1000000007 + U(answer);
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << "abc403g,packed64," << variant << ',' << trial << ',' << std::chrono::duration<double, std::milli>(end - start).count() << ",0,0,0," << tree.node_count() << ',' << tree.memory_usage() << ',' << checksum << '\n';
	return checksum;
}

int main(int argc, char **argv) {
	int n = argc > 1 ? std::stoi(argv[1]) : 100000, q = argc > 2 ? std::stoi(argv[2]) : 100000, trials = argc > 3 ? std::stoi(argv[3]) : 5;
	focus = argc > 4 && std::string(argv[4]) == "new";
	std::mt19937_64 rng(896724253);
	std::cout << "kind,type,variant,trial,build_ms,update_ms,prod_ms,get_ms,nodes,bytes,checksum\n";
	for (std::string kind : {"random", "clustered", "ascending", "descending"}) {
		Work w;
		w.keys.resize(n);
		w.changes.resize(q);
		w.ranges.resize(q);
		constexpr I size = I(1) << 62;
		for (int i = 0; i < n; ++i) w.keys[i] = kind == "random" ? I(rng() % size) : kind == "clustered" ? (I(1) << 60) + i : I(__int128(size) * i / n);
		if (kind == "random" || kind == "clustered") std::shuffle(w.keys.begin(), w.keys.end(), rng);
		if (kind == "descending") std::reverse(w.keys.begin(), w.keys.end());
		for (int &i : w.changes) i = rng() % n;
		for (auto &[l, r] : w.ranges) {
			l = kind == "clustered" ? (I(1) << 60) + rng() % n : I(rng() % size);
			r = kind == "clustered" ? (I(1) << 60) + rng() % n : I(rng() % size);
			if (l > r) std::swap(l, r);
			++r;
		}
		run<I, add, zero>("sum", kind, w, trials, rng);
		run<Odd, odd_op, odd_e>("odd", kind, w, trials, rng);
		run<Affine, affine_op, affine_e>("affine", kind, w, trials, rng);
	}
	std::vector<unsigned> input(300000);
	for (auto &x : input) x = rng() % 1000000000;
	for (int trial = 0; trial < trials; ++trial) {
		std::array<int, 20> order{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
		std::shuffle(order.begin(), order.end(), rng);
		U expected = 0;
		bool first = true;
		for (int id : order) {
			if (focus && id != 0 && id != 6 && id < 11) continue;
			U checksum = 0;
			if (id == 0) checksum = online<DynamicSegtree_base<Odd, odd_op, odd_e>>("base", input, trial);
			if (id == 1) checksum = online<DynamicSegtree_branch<Odd, odd_op, odd_e>>("branch", input, trial);
			if (id == 2) checksum = online<DynamicSegtree_leaf<Odd, odd_op, odd_e>>("leaf", input, trial);
			if (id == 3) checksum = online<DynamicSegtree_cover<Odd, odd_op, odd_e>>("cover", input, trial);
			if (id == 4) checksum = online<DynamicSegtree_layout<Odd, odd_op, odd_e>>("layout", input, trial);
			if (id == 5) checksum = online<DynamicSegtree_recursive<Odd, odd_op, odd_e>>("recursive", input, trial);
			if (id == 6) checksum = online<DynamicSegtree<Odd, odd_op, odd_e>>("current", input, trial);
			if (id == 7) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 8, 4>>("block8x4", input, trial);
			if (id == 8) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 16, 4>>("block16x4", input, trial);
			if (id == 9) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 16, 8>>("block16x8", input, trial);
			if (id == 10) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 32, 8>>("block32x8", input, trial);
			if (id == 11) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 8, 4, true>>("balanced8x4", input, trial);
			if (id == 12) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 16, 4, true>>("balanced16x4", input, trial);
			if (id == 13) checksum = online<DynamicSegtreeBlock<Odd, odd_op, odd_e, I, 32, 8, true>>("balanced32x8", input, trial);
			if (id == 14) checksum = online<DynamicSegtree_combo_branch<Odd, odd_op, odd_e>>("combo_branch", input, trial);
			if (id == 15) checksum = online<DynamicSegtree_combo_leaf<Odd, odd_op, odd_e>>("combo_leaf", input, trial);
			if (id == 16) checksum = online<DynamicSegtreeBucket<Odd, odd_op, odd_e, I, 4, true>>("bucket4", input, trial);
			if (id == 17) checksum = online<DynamicSegtreeBucket<Odd, odd_op, odd_e, I, 8, true>>("bucket8", input, trial);
			if (id == 18) checksum = online<DynamicSegtreeBucket<Odd, odd_op, odd_e, I, 16, true>>("bucket16", input, trial);
			if (id == 19) checksum = online<DynamicSegtreeBucket<Odd, odd_op, odd_e, I, 8, false>>("bucket_linear", input, trial);
			if (first) expected = checksum, first = false;
			else if (expected != checksum) std::abort();
		}
		if (packed_online<DynamicSegtree_base<U, packed_op, packed_e>>("base", input, trial) != expected) std::abort();
		if (packed_online<DynamicSegtree<U, packed_op, packed_e>>("current", input, trial) != expected) std::abort();
		if (packed_online<DynamicSegtreeBlock<U, packed_op, packed_e, I, 16, 4, true>>("balanced16x4", input, trial) != expected) std::abort();
		if (packed_online<DynamicSegtreeBucket<U, packed_op, packed_e, I, 8, true>>("bucket8", input, trial) != expected) std::abort();

	}
}
