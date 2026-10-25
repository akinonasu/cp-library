#include "data_structure/lazy_reversible_treap.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct Affine { mint a, b; };
mint add(mint a, mint b) { return a + b; }
mint zero() { return 0; }
mint map_affine(Affine f, mint x, int n) { return f.a * x + f.b * n; }
Affine compose(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine identity() { return {1, 0}; }
template <bool C> using Tree = LazyReversibleTreap<mint, add, zero, Affine, map_affine, compose, identity, C>;

std::string concat(const std::string &a, const std::string &b) { return a + b; }
std::string empty_string() { return {}; }
struct Shift { int k; };
std::string shift(Shift f, std::string s, int) { for (auto &c : s) c = 'a' + (c - 'a' + f.k) % 26; return s; }
Shift compose_shift(Shift f, Shift g) { return {(f.k + g.k) % 26}; }
Shift no_shift() { return {0}; }
using Text = LazyReversibleTreap<std::string, concat, empty_string, Shift, shift, compose_shift, no_shift>;

struct Box {
	int x;
	Box() = delete;
	explicit Box(int x) : x(x) {}
};
Box box_add(Box a, Box b) { return Box(a.x + b.x); }
Box box_zero() { return Box(0); }
Box box_map(Box f, Box a, int n) { return Box(a.x + f.x * n); }
Box box_id() { return Box(0); }

template <bool C>
void random_test(std::mt19937_64 &rng) {
	for (int initial : {0, 1, 2, 31, 200}) {
		std::vector<mint> a(initial);
		for (auto &x : a) x = rng() % 10000;
		Tree<C> tree(a, rng());
		tree.reserve(256);
		for (int t = 0; t < 6000; ++t) {
			int n = a.size(), cmd = rng() % 11;
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			if ((!n || cmd == 0) && n < 300) {
				mint value = rng() % 100000;
				a.insert(a.begin() + l, value);
				tree.insert(l, value);
			} else if (cmd == 1 && n) {
				int p = rng() % n;
				assert(tree.erase(p) == a[p]);
				a.erase(a.begin() + p);
			} else if (cmd == 2) {
				tree.reverse(l, r);
				std::reverse(a.begin() + l, a.begin() + r);
			} else if (cmd == 3) {
				Affine f{rng() % 100, rng() % 10000};
				tree.apply(l, r, f);
				for (int i = l; i < r; ++i) a[i] = map_affine(f, a[i], 1);
			} else if (cmd == 4 && n) {
				int p = rng() % n;
				mint value = rng() % 100000;
				tree.set(p, a[p] = value);
			} else if (cmd == 5) {
				assert(tree.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, mint(0)));
			} else if (cmd == 6) {
				int m = l + rng() % (r - l + 1);
				tree.rotate(l, m, r);
				std::rotate(a.begin() + l, a.begin() + m, a.begin() + r);
			} else if (cmd == 7) {
				int p = rng() % (n - (r - l) + 1);
				tree.move(l, r, p);
				std::vector<mint> cut(a.begin() + l, a.begin() + r);
				a.erase(a.begin() + l, a.begin() + r);
				a.insert(a.begin() + p, cut.begin(), cut.end());
			} else if (cmd == 8 && n) {
				int p = rng() % n;
				Affine f{rng() % 100, rng() % 100};
				tree.apply(p, f);
				a[p] = map_affine(f, a[p], 1);
			} else if (cmd == 9 && t % 10 == 0) {
				auto copy = tree;
				assert(copy.to_vector() == a);
				Tree<C> moved(std::move(copy));
				assert(copy.empty() && moved.to_vector() == a);
				copy.insert(0, 7);
				assert(copy.get(0) == mint(7));
				copy = std::move(moved);
				assert(moved.empty() && copy.to_vector() == a);
				moved = copy;
				assert(moved.to_vector() == a);
			}
			assert(tree.size() == int(a.size()) && tree.empty() == a.empty());
			assert(tree.all_prod() == std::accumulate(a.begin(), a.end(), mint(0)));
			if (!a.empty()) { int p = rng() % a.size(); assert(tree.get(p) == a[p]); }
			if (t % 137 == 0) assert(tree.to_vector() == a);
		}
		tree.clear();
		assert(tree.empty() && tree.prod(0, 0) == mint(0) && tree.to_vector().empty());
		tree.assign(a);
		assert(tree.to_vector() == a);
	}
}

int main() {
	for (int n = 0; n <= 10; ++n) for (int seed = 0; seed < 4; ++seed) {
		std::vector<mint> values(n);
		std::iota(values.begin(), values.end(), mint(1));
		std::string letters;
		for (int i = 0; i < n; ++i) letters += char('a' + i);
		std::vector<std::string> symbols;
		for (char c : letters) symbols.emplace_back(1, c);
		for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
			Tree<true> sum(values, seed);
			Tree<false> ordered_sum(values, seed);
			Text text(symbols, seed), plain(symbols, seed);
			sum.apply(0, n, {2, 3});
			ordered_sum.apply(0, n, {2, 3});
			text.apply(0, n, {7});
			sum.reverse(l, r);
			ordered_sum.reverse(l, r);
			text.reverse(l, r);
			plain.reverse(l, r);
			auto expected = values;
			for (auto &x : expected) x = x * 2 + 3;
			std::reverse(expected.begin() + l, expected.begin() + r);
			auto unshifted = letters;
			std::reverse(unshifted.begin() + l, unshifted.begin() + r);
			auto word = shift({7}, letters, n);
			std::reverse(word.begin() + l, word.begin() + r);
			assert(sum.all_prod() == std::accumulate(expected.begin(), expected.end(), mint(0)));
			assert(ordered_sum.all_prod() == sum.all_prod() && text.all_prod() == word);
			for (int a = 0; a <= n; ++a) for (int b = a; b <= n; ++b) {
				auto total = std::accumulate(expected.begin() + a, expected.begin() + b, mint(0));
				assert(sum.prod(a, b) == total && ordered_sum.prod(a, b) == total);
				assert(text.prod(a, b) == word.substr(a, b - a));
				assert(plain.prod(a, b) == unshifted.substr(a, b - a));
			}
			assert(sum.to_vector() == expected && ordered_sum.to_vector() == expected);
		}
	}
	std::mt19937_64 rng(3495682);
	random_test<true>(rng);
	random_test<false>(rng);
	Text text;
	std::string a;
	for (int t = 0; t < 12000; ++t) {
		int n = a.size(), cmd = rng() % 7;
		int l = rng() % (n + 1), r = rng() % (n + 1);
		if (l > r) std::swap(l, r);
		if ((!n || cmd == 0) && n < 100) {
			char c = 'a' + rng() % 26;
			text.insert(l, std::string(1, c));
			a.insert(a.begin() + l, c);
		} else if (cmd == 1 && n) {
			int p = rng() % n;
			assert(text.erase(p) == a.substr(p, 1));
			a.erase(p, 1);
		} else if (cmd == 2) {
			text.reverse(l, r);
			std::reverse(a.begin() + l, a.begin() + r);
		} else if (cmd == 3) {
			Shift f{int(rng() % 26)};
			text.apply(l, r, f);
			a.replace(l, r - l, shift(f, a.substr(l, r - l), r - l));
		} else if (cmd == 4) {
			int m = l + rng() % (r - l + 1);
			text.rotate(l, m, r);
			std::rotate(a.begin() + l, a.begin() + m, a.begin() + r);
		} else if (cmd == 5) {
			int p = rng() % (n - (r - l) + 1);
			text.move(l, r, p);
			auto cut = a.substr(l, r - l);
			a.erase(l, r - l);
			a.insert(p, cut);
		} else assert(text.prod(l, r) == a.substr(l, r - l));
		assert(text.all_prod() == a);
		int ql = rng() % (text.size() + 1), qr = rng() % (text.size() + 1);
		if (ql > qr) std::swap(ql, qr);
		assert(text.prod(ql, qr) == a.substr(ql, qr - ql));
	}
	LazyReversibleTreap<Box, box_add, box_zero, Box, box_map, box_add, box_id, true> boxes;
	boxes.insert(0, Box(2)); boxes.insert(1, Box(3));
	boxes.apply(0, 2, Box(7)); boxes.reverse(0, 2);
	assert(boxes.get(0).x == 10 && boxes.all_prod().x == 19 && boxes.erase(1).x == 9);
	Tree<true> recycled;
	std::size_t initial_bytes = 0;
	for (int i = 0; i < 10000; ++i) {
		recycled.insert(0, i);
		auto bytes = recycled.memory_usage();
		assert(recycled.erase(0) == mint(i));
		assert(recycled.memory_usage() == bytes);
		if (!i) initial_bytes = bytes;
		else assert(bytes == initial_bytes);
	}
	constexpr auto bit_op = [](bool a, bool b) { return a != b; };
	constexpr auto bit_id = [] { return false; };
	constexpr auto bit_map = [](bool f, bool a, int n) { return a != (f && (n & 1)); };
	LazyReversibleTreap<bool, bit_op, bit_id, bool, bit_map, bit_op, bit_id, true> bits(std::vector<bool>{true, false, true});
	bits.apply(0, 2, true);
	bits.reverse(0, 3);
	assert(bits.to_vector() == std::vector<bool>({true, true, false}));
	assert(!bits.all_prod() && bits.get(0) && bits.prod(1, 3));
	std::vector<mint> values(200000, 1);
	Tree<true> large(values);
	large.apply(0, large.size(), {3, 7});
	large.reverse(0, large.size());
	large.reverse(1, large.size() - 1);
	assert(large.all_prod() == mint(2000000));
	assert(large.get(0) == mint(10) && large.get(large.size() - 1) == mint(10));
	assert(large.prod(12345, 13456) == mint(11110));
}
