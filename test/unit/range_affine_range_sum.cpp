#include "data_structure/segment_tree/range_affine_range_sum.hpp"
#include <cassert>
#include <cstdint>
#include <numeric>
#include <random>
#include <type_traits>

template<unsigned Mod, bool Dynamic = false>
struct Field {
	unsigned value = 0;
	inline static unsigned dynamic_modulus = Mod;
	static constexpr unsigned mod() { if constexpr (Dynamic) return dynamic_modulus; else return Mod; }
	Field() = default;
	Field(long long x) {
		long long r = x % mod();
		value = unsigned(r < 0 ? r + mod() : r);
	}
	unsigned val() const { return value; }
	friend Field operator+(Field a, Field b) {
		unsigned sum = a.value + b.value;
		return Field(sum < mod() ? sum : sum - mod());
	}
	friend Field operator*(Field a, Field b) { return Field(std::uint64_t(a.value) * b.value % mod()); }
	friend bool operator==(Field a, Field b) { return a.value == b.value; }
};

template<class Mint>
void randomized(const std::vector<int> &sizes, int queries) {
	std::mt19937 random(20261023);
	for (int n : sizes) {
		std::vector<Mint> a(n);
		for (auto &x : a) x = Mint(random());
		RangeAffineRangeSum<Mint> tree(a);
		assert(tree.size() == n);
		for (int step = 0; step < queries; ++step) {
			int l = random() % (n + 1), r = random() % (n + 1);
			if (l > r) std::swap(l, r);
			Mint scale = step % 17 ? Mint(random()) : Mint(0);
			Mint shift = step % 19 ? Mint(random()) : Mint(0);
			if (step % 23 == 0) scale = Mint(1), shift = Mint(0);
			if (step % 31 == 0) l = 0, r = n;
			tree.apply(l, r, scale, shift);
			for (int i = l; i < r; ++i) a[i] = scale * a[i] + shift;
			if (n && step % 7 == 0) {
				int p = random() % n;
				tree.set(p, a[p] = Mint(random()));
			}
			if (n && step % 11 == 0) {
				int p = random() % n;
				tree.apply(p, scale, shift);
				a[p] = scale * a[p] + shift;
			}
			Mint sum = 0, total = 0;
			for (int i = 0; i < n; ++i) {
				total = total + a[i];
				if (l <= i && i < r) sum = sum + a[i];
			}
			const auto &read = tree;
			assert(read.prod(l, r) == sum && read.sum(l, r) == sum);
			assert(read.all_prod() == total && read.all_sum() == total);
			assert(read.prod(0, n) == total && read.sum(l, l) == Mint(0));
			if (n) {
				int p = random() % n;
				assert(read.get(p) == a[p] && read[p] == a[p]);
			}
			if (step % 53 == 0) {
				auto copy = tree;
				auto moved = std::move(copy);
				assert(copy.size() == 0 && copy.all_prod() == Mint(0));
				assert(moved.size() == n && moved.all_prod() == total);
				copy = moved;
				copy = std::move(moved);
				assert(moved.size() == 0 && copy.size() == n && copy.all_sum() == total);
				auto &same = copy;
				copy = std::move(same);
				assert(copy.size() == n && copy.all_sum() == total);
				if (n) copy.set(0, Mint(7));
				assert(tree.all_sum() == total);
			}
		}
	}
}

int main() {
	static_assert(std::is_same_v<RangeAffineRangeSum<>::value_type, mint>);
	RangeAffineRangeSum<> empty;
	assert(empty.size() == 0 && empty.prod(0, 0) == mint(0));
	empty.apply(0, 0, mint(2), mint(3));
	RangeAffineRangeSum<> repeated(35, mint(4));
	assert(repeated.all_sum() == mint(140) && repeated.prod(1, 34) == mint(132));
	const std::vector<int> sizes{0, 1, 2, 7, 15, 16, 17, 31, 32, 33, 255, 256, 257, 4095, 4096, 4097};
	randomized<mint>(sizes, 350);
	randomized<Field<3>>(sizes, 350);
	randomized<Field<17>>(sizes, 350);
	randomized<Field<1000000007>>(sizes, 350);
	randomized<Field<1073741789>>(sizes, 350);
	randomized<Field<2>>(sizes, 200);
	randomized<Field<1073741827>>(sizes, 200);
	randomized<Field<2147483647>>(sizes, 200);
	randomized<Field<998244353, true>>(sizes, 350);
	Field<998244353, true>::dynamic_modulus = 1000000007;
	randomized<Field<998244353, true>>(sizes, 200);
	randomized<mint>({65535, 65536, 65537, 262143, 262144, 262145}, 40);
	randomized<Field<3>>({65537, 262145}, 40);
}
