#include "data_structure/segment_tree/dynamic_lazy_segtree.hpp"
#include <array>
#include <random>
#include <string>

struct Sum {
	long long sum, length;
	Sum() = delete;
	Sum(long long s, long long n) : sum(s), length(n) {}
	bool operator==(const Sum &) const = default;
};
struct Affine {
	long long a, b;
	Affine() = delete;
	Affine(long long a_, long long b_) : a(a_), b(b_) {}
};
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }
Sum initial(std::int64_t l, std::int64_t r) { return {0, r - l}; }
Sum ones(std::int64_t l, std::int64_t r) { return {r - l, r - l}; }
using Tree = DynamicLazySegtree<Sum, op, e, Affine, mapping, composition, id, initial>;

void random_test() {
	std::mt19937 rng(20261020);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 32, 33, 100}) {
		for (int origin : {-127, 0, 231}) {
			Tree seg(origin, origin + n);
			std::vector<long long> a(n);
			for (int step = 0; step < 2500; ++step) {
				int l = rng() % (n + 1), r = rng() % (n + 1);
				if (l > r) std::swap(l, r);
				switch (rng() % 7) {
				case 0: {
					Affine f(rng() % 2, rng() % 6);
					seg.apply(origin + l, origin + r, f);
					for (int i = l; i < r; ++i) a[i] = f.a * a[i] + f.b;
					break;
				}
				case 1:
					if (l < n) { a[l] = rng() % 100; seg.set(origin + l, Sum{a[l], 1}); }
					break;
				case 2:
					if (l < n) { seg.apply(origin + l, Affine{1, 3}); a[l] += 3; }
					break;
				case 3:
					if (step % 17 == 0) { seg.clear(); std::fill(a.begin(), a.end(), 0); assert(seg.node_count() == 0); }
					break;
				default: break;
				}
				const Tree &read = seg;
				auto count = read.node_count(), memory = read.memory_usage();
				long long total = 0, part = 0;
				for (int i = 0; i < n; ++i) {
					total += a[i];
					if (l <= i && i < r) part += a[i];
				}
				assert(read.all_prod() == Sum(total, n));
				assert(read.prod(origin + l, origin + r) == Sum(part, r - l));
				if (l < n) assert(read.get(origin + l) == Sum(a[l], 1));
				long long limit = rng() % 100;
				int right = l, left = r;
				long long sum = 0;
				while (right < n && sum + a[right] <= limit) sum += a[right++];
				sum = 0;
				while (left && sum + a[left - 1] <= limit) sum += a[--left];
				auto pred = [&](Sum x) { return x.sum <= limit; };
				assert(read.max_right(origin + l, pred) == origin + right);
				assert(read.min_left(origin + r, pred) == origin + left);
				assert(read.node_count() == count && read.memory_usage() == memory);
			}
			std::vector<Sum> values;
			for (auto x : a) values.emplace_back(x, 1);
			Tree built(values);
			assert(built.node_count() == (n ? std::size_t(2 * n - 1) : 0));
			for (int i = 0; i < n; ++i) assert(built.get(i) == values[i]);
			built.apply(0, n, Affine{2, 7});
			auto copy = built;
			Tree moved(std::move(copy));
			assert(moved.all_prod() == built.all_prod());
			assert(copy.node_count() == 0 && copy.all_prod() == Sum(0, n));
			copy = std::move(moved);
			assert(copy.all_prod() == built.all_prod() && moved.node_count() == 0);
			moved = built;
			moved.swap(copy);
			copy.reserve(1000);
			copy.clear();
			assert(copy.all_prod() == Sum(0, n));
			copy.build(values);
			assert(copy.all_prod() == seg.all_prod());
		}
	}
}

std::string text_op(std::string a, const std::string &b) { return a + b; }
std::string text_e() { return {}; }
using Perm = std::array<int, 3>;
std::string text_mapping(Perm f, std::string a) { for (char &c : a) c = char('a' + f[c - 'a']); return a; }
Perm text_composition(Perm f, Perm g) { return {f[g[0]], f[g[1]], f[g[2]]}; }
Perm text_id() { return {0, 1, 2}; }
std::string text_initial(std::int64_t l, std::int64_t r) {
	std::string s;
	for (; l < r; ++l) s += char('a' + (l % 3 + 3) % 3);
	return s;
}
void noncommutative_test() {
	using TextTree = DynamicLazySegtree<std::string, text_op, text_e, Perm,
		text_mapping, text_composition, text_id, text_initial>;
	TextTree seg(-20, 21);
	std::string a = text_initial(-20, 21);
	std::mt19937 rng(713);
	for (int step = 0; step < 1500; ++step) {
		int l = rng() % 42, r = rng() % 42;
		if (l > r) std::swap(l, r);
		Perm f = text_id();
		std::shuffle(f.begin(), f.end(), rng);
		seg.apply(l - 20, r - 20, f);
		for (int i = l; i < r; ++i) a[i] = char('a' + f[a[i] - 'a']);
		if (l < 41 && step % 5 == 0) { a[l] = char('a' + rng() % 3); seg.set(l - 20, std::string(1, a[l])); }
		assert(seg.prod(l - 20, r - 20) == a.substr(l, r - l));
		assert(seg.all_prod() == a);
		std::string needle = step % 2 ? "ab" : "bc";
		auto pred = [&](const std::string &s) { return s.find(needle) == std::string::npos; };
		int right = l, left = r;
		while (right < 41 && pred(a.substr(l, right - l + 1))) ++right;
		while (left && pred(a.substr(left - 1, r - left + 1))) --left;
		auto count = seg.node_count();
		assert(seg.max_right(l - 20, pred) == right - 20);
		assert(seg.min_left(r - 20, pred) == left - 20);
		assert(seg.node_count() == count);
	}
}

#if defined(__SIZEOF_INT128__)
struct Wide { unsigned __int128 sum, length; };
Wide wide_op(Wide a, Wide b) { return {a.sum + b.sum, a.length + b.length}; }
Wide wide_e() { return {0, 0}; }
Wide wide_mapping(unsigned f, Wide x) { return {x.sum + f * x.length, x.length}; }
unsigned wide_composition(unsigned f, unsigned g) { return f + g; }
unsigned wide_id() { return 0; }
template<class C>
Wide wide_initial(C l, C r) { return {0, std::make_unsigned_t<C>(std::make_unsigned_t<C>(r) - std::make_unsigned_t<C>(l))}; }
template<class C>
void extremes() {
	using WideTree = DynamicLazySegtree<Wide, wide_op, wide_e, unsigned, wide_mapping,
		wide_composition, wide_id, wide_initial<C>, C>;
	C low = std::numeric_limits<C>::min(), high = std::numeric_limits<C>::max();
	WideTree seg(low, high);
	auto length = static_cast<unsigned __int128>(std::make_unsigned_t<C>(std::make_unsigned_t<C>(high) - std::make_unsigned_t<C>(low)));
	assert(seg.size() == length && seg.node_count() == 0);
	seg.apply(low, high, 2);
	assert(seg.node_count() == 1 && seg.all_prod().sum == 2 * length);
	assert(seg.get(low).sum == 2 && seg.get(C(high - 1)).sum == 2);
	assert(seg.max_right(low, [](Wide x) { return x.sum <= 12; }) == C(low + 6));
	assert(seg.min_left(high, [](Wide x) { return x.sum <= 12; }) == C(high - 6));
	assert(seg.node_count() == 1);
	seg.set(low, Wide{9, 1});
	seg.apply(C(high - 1), 3);
	assert(seg.all_prod().sum == 2 * length + 10);
	assert(seg.prod(low, C(low + 1)).sum == 9);
	assert(seg.get(C(high - 1)).sum == 5);
	assert(seg.node_count() < 260);
	if constexpr (std::is_signed_v<C>) {
		seg.set(C(-1), Wide{17, 1});
		assert(seg.get(C(-1)).sum == 17);
		assert(seg.prod(C(-1), C(0)).sum == 17);
	}
}
#endif

long long max_op(long long a, long long b) { return std::max(a, b); }
long long max_e() { return std::numeric_limits<long long>::lowest(); }
long long assign(long long f, long long x) { return f == max_e() ? x : f; }
long long newest(long long f, long long g) { return f == max_e() ? g : f; }
long long assign_id() { return max_e(); }

int main() {
	random_test();
	noncommutative_test();
#if defined(__SIZEOF_INT128__)
	extremes<std::int64_t>();
	extremes<std::uint64_t>();
	extremes<std::int32_t>();
	extremes<std::uint32_t>();
	extremes<std::int16_t>();
	extremes<std::uint16_t>();
	extremes<std::int8_t>();
	extremes<std::uint8_t>();
#endif
	DynamicLazySegtree<Sum, op, e, Affine, mapping, composition, id, ones> seg(-9, 14);
	assert(seg.prod(-2, 5) == Sum(7, 7));
	assert(seg.max_right(-2, [](Sum x) { return x.sum <= 4; }) == 2);
	assert(seg.min_left(5, [](Sum x) { return x.sum <= 4; }) == 1);
	assert(seg.node_count() == 0);
	seg.apply(-9, 14, Affine{3, 2});
	seg.clear();
	assert(seg.all_prod() == Sum(23, 23));
	DynamicLazySegtree<long long, max_op, max_e, long long, assign, newest, assign_id> maximum(1000000000000LL);
	maximum.apply(0, 1000000000000LL, assign_id());
	maximum.apply(2, 8, 7);
	maximum.apply(4, 5, 3);
	assert(maximum.prod(0, 10) == 7 && maximum.get(4) == 3 && maximum.get(9) == max_e());
	assert(maximum.max_right(0, [](long long x) { return x < 7; }) == 2);
	assert(maximum.min_left(10, [](long long x) { return x < 7; }) == 8);
}
