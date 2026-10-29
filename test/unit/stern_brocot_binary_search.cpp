#include "math/stern_brocot_tree.hpp"
#include <numeric>
#include <random>

using Tree = SternBrocotTree<>;
using U = std::uint64_t;
using W = __uint128_t;
using P = Tree::value_type;

int main() {
	for (U n = 1; n <= 12; ++n) for (U d = 1; d <= 12; ++d) {
		std::vector<P> fractions;
		for (U a = 1; a <= n; ++a) for (U b = 1; b <= d; ++b)
			if (std::gcd(a, b) == 1) fractions.emplace_back(a, b);
		std::sort(fractions.begin(), fractions.end(), [](P a, P b) { return a.first * b.second < b.first * a.second; });
		for (std::size_t cut = 0; cut <= fractions.size(); ++cut) {
			P lower = cut ? fractions[cut - 1] : P{0, 1};
			P upper = cut < fractions.size() ? fractions[cut] : P{1, 0};
			auto predicate = [&](U a, U b) {
				assert(a > 0 && a <= n && b > 0 && b <= d && std::gcd(a, b) == 1);
				return a * lower.second <= lower.first * b;
			};
			assert(Tree::binary_search(n, d, predicate) == std::pair(lower, upper));
			if (n == d) assert(Tree::binary_search(n, predicate) == std::pair(lower, upper));
		}
	}
	std::mt19937_64 rng(517311);
	for (int i = 0; i < 10000; ++i) {
		U n = rng() | 1, d = rng() | 1, x = rng() | 1, y = rng() | 1;
		for (bool strict : {false, true}) {
			int calls = 0;
			auto predicate = [&](U a, U b) {
				++calls;
				assert(a && a <= n && b && b <= d);
				return strict ? W(a) * y < W(x) * b : W(a) * y <= W(x) * b;
			};
			auto [l, r] = Tree::binary_search(n, d, predicate);
			assert(calls < 512);
			assert(!l.first || predicate(l.first, l.second));
			assert(!r.second || !predicate(r.first, r.second));
			assert(W(r.first) * l.second - W(l.first) * r.second == 1);
			assert(W(l.first) + r.first > n || W(l.second) + r.second > d);
			auto expected = Tree::approximate(x, y, n, d);
			if (expected.first != expected.second) assert(std::pair(l, r) == expected);
			else assert((strict ? r : l) == expected.first);
		}
	}
	auto extremes = []<class Int>() {
		using T = SternBrocotTree<Int>;
		using V = typename T::value_type;
		Int bound = std::numeric_limits<Int>::max();
		for (bool side : {false, true}) {
			int calls = 0;
			auto [l, r] = T::binary_search(bound, [&](Int a, Int b) {
				assert(a > 0 && b > 0); ++calls; return side;
			});
			assert(calls <= 66);
			assert(l == (side ? V{bound, 1} : V{0, 1}));
			assert(r == (side ? V{1, 0} : V{1, bound}));
		}
	};
	extremes.operator()<std::uint8_t>(); extremes.operator()<std::int8_t>();
	extremes.operator()<std::uint16_t>(); extremes.operator()<std::int16_t>();
	extremes.operator()<std::uint32_t>(); extremes.operator()<std::int32_t>();
	extremes.operator()<std::uint64_t>(); extremes.operator()<std::int64_t>();
	auto [l, r] = Tree::binary_search(100, [](U a, U b) { return a * a <= 2 * b * b; });
	assert(l == P(41, 29) && r == P(99, 70));
	struct Noncopyable {
		Noncopyable() = default;
		Noncopyable(const Noncopyable &) = delete;
		bool operator()(U a, U b) { return a <= b; }
	} predicate;
	assert(Tree::binary_search(10, predicate).first == P(1, 1));
	assert(Tree::binary_search(10, Noncopyable{}).first == P(1, 1));
}
