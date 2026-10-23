#include "sequence/permutation_rank.hpp"
#include "math/mint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <random>
#include <type_traits>

using Big = boost::multiprecision::cpp_int;
static_assert(std::is_same_v<decltype(permutation_rank(std::vector<int>{})), std::uint64_t>);

template <unsigned Bits, boost::multiprecision::cpp_integer_type Sign, boost::multiprecision::cpp_int_check_type Check>
void bounded_ranks() {
	using Rank = boost::multiprecision::number<boost::multiprecision::cpp_int_backend<Bits, Bits, Sign, Check, void>>;
	for (int n : {0, 1, 2, 3, 5, 6, 7, 64, 65, 255, 256, 257, 1000}) {
		for (unsigned rank = 0; rank < (1u << Bits); ++rank) {
			auto expected = permutation_unrank(n, std::uint64_t(rank));
			assert(permutation_unrank(n, Rank(rank)) == expected);
			if (expected) assert(permutation_rank<Rank>(*expected) == Rank(rank));
		}
		if constexpr (std::numeric_limits<Rank>::is_signed) assert(!permutation_unrank(n, Rank(-1)));
	}
	std::vector<int> reverse{5, 4, 3, 2, 1, 0};
	bool caught = false;
	try { (void)permutation_rank<Rank>(reverse); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
}

int main() {
	using namespace boost::multiprecision;
	bounded_ranks<1, unsigned_magnitude, checked>();
	bounded_ranks<8, unsigned_magnitude, checked>();
	bounded_ranks<8, unsigned_magnitude, unchecked>();
	bounded_ranks<8, signed_magnitude, checked>();
	bounded_ranks<8, signed_magnitude, unchecked>();
	for (int n = 0; n <= 8; ++n) {
		std::vector<int> p(n);
		std::iota(p.begin(), p.end(), 0);
		std::uint64_t rank = 0;
		do {
			assert(permutation_rank(p) == rank);
			assert(permutation_unrank(n, rank) == p);
			assert(permutation_rank<mint>(p) == mint(rank));
			++rank;
		} while (std::next_permutation(p.begin(), p.end()));
		assert(!permutation_unrank(n, rank));
		assert(!permutation_unrank(n, -1));
	}
	std::array<int, 4> a{2, 0, 3, 1};
	assert(permutation_rank(a) == 13);
	int b[]{1, 0};
	assert(permutation_rank(b) == 1);
	std::mt19937 random(208371);
	for (int n : {20, 21, 33, 34, 63, 64, 65, 100}) {
		std::vector<int> p(n);
		std::iota(p.begin(), p.end(), 0);
		assert(permutation_rank(p) == 0);
		for (int t = 0; t < 20; ++t) {
			std::shuffle(p.begin(), p.end(), random);
			Big rank = permutation_rank<Big>(p);
			assert(permutation_unrank(n, rank) == p);
			assert(permutation_rank<mint>(p) == mint((rank % mint::mod()).convert_to<unsigned>()));
			if (rank <= std::numeric_limits<std::uint64_t>::max()) assert(permutation_rank(p) == rank);
			else {
				bool caught = false;
				try { (void)permutation_rank(p); } catch (const std::overflow_error &) { caught = true; }
				assert(caught);
			}
		}
		Big factorial = 1;
		for (int i = 2; i <= n; ++i) factorial *= i;
		assert(!permutation_unrank(n, factorial));
		std::sort(p.rbegin(), p.rend());
		assert(permutation_rank<Big>(p) == factorial - 1);
		assert(permutation_unrank(n, Big(factorial - 1)) == p);
	}
	for (int n : {6, 33}) {
		std::vector<int> p(n);
		std::iota(p.rbegin(), p.rend(), 0);
		__uint128_t rank = permutation_rank<__uint128_t>(p);
		assert(permutation_unrank(n, rank) == p);
	}
	std::vector<int> large(100003);
	std::iota(large.begin(), large.end(), 0);
	std::swap(large[large.size() - 1], large[large.size() - 2]);
	assert(permutation_rank(large) == 1);
	assert(permutation_unrank(int(large.size()), std::uint8_t(1)) == large);
	std::vector<int> tiny{4, 3, 2, 1, 0};
	assert(permutation_rank<std::int8_t>(tiny) == 119);
	assert(permutation_unrank(5, std::int8_t(119)) == tiny);
	for (bool signed_type : {false, true}) {
		bool caught = false;
		try {
			std::vector<int> p{5, 4, 3, 2, 1, 0};
			if (signed_type) (void)permutation_rank<std::int8_t>(p);
			else (void)permutation_rank<std::uint8_t>(p);
		} catch (const std::overflow_error &) { caught = true; }
		assert(caught);
	}
	std::vector<std::vector<int>> invalids{ {0, 0}, {-1}, {1}, {0, 2} };
	for (int n : {22, 65}) {
		std::vector<int> p(n);
		std::iota(p.rbegin(), p.rend(), 0);
		p.back() = p.front();
		invalids.push_back(p);
	}
	for (const auto &p : invalids) {
		bool caught = false;
		try { (void)permutation_rank(p); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	bool caught = false;
	try { (void)permutation_unrank(-1, 0); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
}
