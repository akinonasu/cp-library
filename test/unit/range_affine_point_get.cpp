#include "data_structure/range_affine_point_get.hpp"
#include <cassert>
#include <cstdint>
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

struct Scalar {
	unsigned value = 0;
	Scalar() = default;
	Scalar(unsigned x) : value(x % 17) {}
	friend Scalar operator+(Scalar a, Scalar b) { return Scalar(a.value + b.value); }
	friend Scalar operator*(Scalar a, Scalar b) { return Scalar(a.value * b.value); }
	friend bool operator==(Scalar a, Scalar b) { return a.value == b.value; }
};

struct Restricted {
	int value = 0;
	static constexpr unsigned mod() { return 17; }
	unsigned val() const { return unsigned(value); }
	Restricted() = default;
	Restricted(int x) : value((x % 17 + 17) % 17) {}
	Restricted(unsigned) = delete;
	friend Restricted operator+(Restricted a, Restricted b) { return Restricted(a.value + b.value); }
	friend Restricted operator*(Restricted a, Restricted b) { return Restricted(a.value * b.value); }
};

template<class Mint>
void randomized(const std::vector<int> &sizes, int queries) {
	std::mt19937 random(20261112);
	for (int n : sizes) {
		std::vector<Mint> a(n);
		for (auto &x : a) x = Mint(random());
		RangeAffinePointGet<Mint> tree(a);
		assert(tree.size() == n && tree.empty() == (n == 0));
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
			const auto &read = tree;
			if (step % 19 == 0) for (int i = 0; i < n; ++i) assert(read.get(i) == a[i] && read[i] == a[i]);
			if (n) {
				int p = random() % n;
				assert(read.get(p) == a[p] && read[p] == a[p]);
				if (l < n) assert(read.get(l) == a[l]);
				if (r) assert(read.get(r - 1) == a[r - 1]);
			}
			if (step % 53 == 0) {
				auto copy = tree;
				auto moved = std::move(copy);
				assert(copy.empty() && moved.size() == n);
				copy = moved;
				copy = std::move(moved);
				assert(moved.empty() && copy.size() == n);
				auto &same = copy;
				copy = std::move(same);
				assert(copy.size() == n);
				for (int i = 0; i < n; ++i) assert(copy.get(i) == a[i]);
				if (n) copy.set(0, Mint(7));
				for (int i = 0; i < n; ++i) assert(tree.get(i) == a[i]);
				swap(copy, moved);
				assert(copy.empty() && moved.size() == n);
				copy = RangeAffinePointGet<Mint>(3, Mint(8));
				assert(copy.size() == 3 && copy.get(2) == Mint(8));
			}
		}
	}
}

int main() {
	static_assert(std::is_same_v<RangeAffinePointGet<>::value_type, mint>);
	RangeAffinePointGet<> empty;
	assert(empty.size() == 0 && empty.empty());
	empty.apply(0, 0, mint(2), mint(3));
	RangeAffinePointGet<Restricted> restricted(3, Restricted(1));
	restricted.apply(0, 2, Restricted(3), Restricted(2));
	restricted.apply(1, 3, Restricted(2), Restricted(1));
	assert(restricted[0].value == 5 && restricted[1].value == 11 && restricted[2].value == 3);
	RangeAffinePointGet<> repeated(35, mint(-4));
	assert(repeated.get(1) == mint(-4) && repeated.get(34) == mint(-4));
	RangeAffinePointGet<> chronological(std::vector<mint>{1, 2, 3, 4});
	chronological.apply(0, 4, mint(2), mint(3));
	chronological.apply(1, 3, mint(0), mint(7));
	chronological.apply(0, 4, mint(3), mint(1));
	chronological.set(2, mint(-5));
	assert(chronological[0] == mint(16) && chronological[1] == mint(22));
	assert(chronological[2] == mint(-5) && chronological[3] == mint(34));
	const std::vector<int> sizes{0, 1, 2, 7, 15, 16, 17, 31, 32, 33, 255, 256, 257, 1023, 1024, 1025, 4095, 4096, 4097};
	randomized<mint>(sizes, 500);
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
	randomized<Scalar>(sizes, 200);
	randomized<mint>({32767, 32768, 32769, 65535, 65536, 65537, 262143, 262144, 262145}, 80);
	randomized<Field<3>>({65537, 262145}, 40);
	randomized<mint>({1048575, 1048576, 1048577}, 20);
}
