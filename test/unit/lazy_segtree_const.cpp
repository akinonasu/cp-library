#include "data_structure/lazy_segtree.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

using U = std::uint64_t;
struct Sum { U value; int length; };
Sum op(Sum a, Sum b) { return {a.value + b.value, a.length + b.length}; }
Sum zero() { return {0, 0}; }
struct Affine { U a, b; };
Sum mapping(Affine f, Sum x) { return {f.a * x.value + f.b * U(x.length), x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine identity() { return {1, 0}; }
using Tree = LazySegtree<Sum, op, zero, Affine, mapping, composition, identity>;
static_assert(std::is_same_v<decltype(std::declval<const Tree &>().get(0)), Sum>);

int main() {
	std::mt19937_64 random(721293);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 63, 64, 65, 129, 257}) {
		std::vector<Sum> initial(n);
		std::vector<U> values(n);
		for (int i = 0; i < n; ++i) initial[i] = {values[i] = random(), 1};
		Tree tree(initial);
		for (int trial = 0; trial < 1000; ++trial) {
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			Affine f{random() % 5, random()};
			tree.apply(l, r, f);
			for (int i = l; i < r; ++i) values[i] = f.a * values[i] + f.b;
			if (n && trial % 3 == 0) {
				int p = random() % n;
				values[p] = random();
				tree.set(p, {values[p], 1});
			}
			const Tree &constant = tree;
			U total = 0;
			for (int i = 0; i < n; ++i) {
				total += values[i];
				if (trial % 20 == 0 || i == 0 || i == n - 1) {
					auto value = constant.get(i);
					assert(value.value == values[i] && value.length == 1);
				}
			}
			assert(constant.all_prod().value == total && constant.all_prod().length == n);
			if (n) {
				int p = random() % n;
				assert(constant.get(p).value == values[p]);
				assert(tree.get(p).value == values[p]);
			}
			if (trial == 500) tree = Tree(tree);
		}
	}
}
