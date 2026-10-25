#include "data_structure/dual_segtree.hpp"
#include "math/mint.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <type_traits>

struct Affine { mint a, b; };
mint mapping(Affine f, mint x) { return f.a * x + f.b; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }
using Tree = DualSegtree<mint, Affine, mapping, composition, id>;

template<class Seg, class T>
void check(Seg &seg, const std::vector<T> &a) {
	assert(seg.size() == int(a.size()) && seg.empty() == a.empty());
	const auto &view = seg;
	for (int i = 0; i < seg.size(); ++i) assert(view.get(i) == a[i] && view.get(i) == a[i]);
	assert(seg.values() == a);
	for (int i = 0; i < seg.size(); ++i) assert(view.get(i) == a[i]);
}

void affine() {
	std::mt19937 rng(20261019);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257}) {
		std::vector<mint> a(n);
		for (auto &x : a) x = rng();
		Tree seg(a);
		for (int step = 0; step < 10000; ++step) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			if (step % 17 == 0) l = 0, r = n;
			Affine f{rng() % 5, rng()};
			switch (rng() % 8) {
			case 0: case 1: case 2:
				seg.apply(l, r, f);
				for (int i = l; i < r; ++i) a[i] = f.a * a[i] + f.b;
				break;
			case 3:
				if (n) { int p = rng() % n; seg.set(p, f.b); a[p] = f.b; }
				break;
			case 4:
				if (n) { int p = rng() % n; seg.apply(p, f); a[p] = f.a * a[p] + f.b; }
				break;
			case 5:
				seg.apply(l, r, id());
				break;
			case 6:
				if (n) { int p = rng() % n; const Tree &view = seg; assert(view.get(p) == a[p]); }
				break;
			case 7:
				if (step % 19 == 0) { Tree copy = seg; check(copy, a); }
				break;
			}
			if (step % 43 == 0) check(seg, a);
			if (step % 4093 == 4092) seg.build(a);
		}
		check(seg, a);
		Tree copy = seg;
		Tree moved(std::move(copy));
		check(moved, a);
		copy.build(a);
		seg = copy;
		check(seg, a);
		seg.build({});
		assert(seg.empty() && seg.values().empty());
	}
}

long long add(long long f, long long x) { return f + x; }
long long combine(long long f, long long g) { return f + g; }
long long zero() { return 0; }

void commutative() {
	using General = DualSegtree<long long, long long, add, combine, zero>;
	using Fast = DualSegtree<long long, long long, add, combine, zero, true>;
	std::mt19937 rng(731);
	for (int n : {0, 1, 2, 3, 8, 9, 64, 65, 257}) {
		General general(n, -7);
		Fast fast(n, -7);
		std::vector<long long> a(n, -7);
		for (int step = 0; step < 10000; ++step) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			long long value = int(rng() % 2001) - 1000;
			if (step % 5 == 0 && n) {
				int p = rng() % n;
				general.set(p, value); fast.set(p, value); a[p] = value;
			} else if (step % 5 == 1 && n) {
				int p = rng() % n;
				general.apply(p, value); fast.apply(p, value); a[p] += value;
			} else {
				general.apply(l, r, value); fast.apply(l, r, value);
				for (int i = l; i < r; ++i) a[i] += value;
			}
			if (n) { int p = rng() % n; assert(general.get(p) == a[p] && fast.get(p) == a[p]); }
			if (step % 47 == 0) { check(general, a); check(fast, a); }
		}
		check(general, a); check(fast, a);
	}
}

struct Value {
	long long x;
	explicit Value(long long x) : x(x) {}
};
struct Update {
	long long a, b;
	Update(long long a, long long b) : a(a), b(b) {}
};
Value map_value(const Update &f, const Value &x) { return Value(f.a * x.x + f.b); }
Update compose_value(const Update &f, const Update &g) { return {f.a * g.a, f.a * g.b + f.b}; }
Update identity_value() { return {1, 0}; }

int main() {
	static_assert(std::is_same_v<Tree::value_type, mint> && std::is_same_v<Tree::update_type, Affine>);
	Tree empty;
	assert(empty.empty() && empty.size() == 0 && empty.values().empty());
	Tree zeros(5);
	check(zeros, std::vector<mint>(5));
	empty.apply(0, 0, {2, 3});
	empty.build({1, 2, 3, 4});
	empty.apply(0, 4, {2, 3});
	empty.apply(1, 3, {5, 7});
	empty.apply(0, 4, {11, 13});
	assert(empty.get(1) == mint(11 * (5 * (2 * 2 + 3) + 7) + 13));
	empty.set(1, 100);
	empty.apply(0, 4, {0, 9});
	empty.apply(1, {3, 4});
	assert(empty.get(1) == mint(31) && empty.get(0) == mint(9));
	DualSegtree<Value, Update, map_value, compose_value, identity_value> nondefault;
	nondefault.build({Value(1), Value(2), Value(3)});
	nondefault.apply(0, 3, Update(2, 3));
	nondefault.apply(1, 3, Update(3, 4));
	assert(nondefault.get(0).x == 5 && nondefault.get(1).x == 25 && nondefault.get(2).x == 31);
	nondefault.set(2, Value(8));
	assert(nondefault.get(2).x == 8);
	assert(nondefault.values()[1].x == 25);
	affine();
	commutative();
}
