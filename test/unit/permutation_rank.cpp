#include "sequence/permutation_rank.hpp"
#include "math/mint.hpp"
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <deque>
#include <memory_resource>
#include <span>
#include <string>
#include <utility>
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

template <class T>
void label_permutations(const std::vector<T> &alphabet) {
	std::vector<int> indices(alphabet.size());
	std::iota(indices.begin(), indices.end(), 0);
	std::uint64_t rank = 0;
	do {
		std::vector<T> values;
		for (int i : indices) values.push_back(alphabet[i]);
		assert(permutation_rank(values, alphabet) == rank);
		assert(permutation_rank<mint>(values, alphabet) == mint(rank));
		assert(permutation_unrank(alphabet, rank) == values);
		assert(permutation_rank(std::span<const T>(values), std::span<const T>(alphabet)) == rank);
		++rank;
	} while (std::next_permutation(indices.begin(), indices.end()));
	assert(!permutation_unrank(alphabet, rank));
	assert(!permutation_unrank(alphabet, -1));
}

void labels() {
	label_permutations(std::vector<std::string>{});
	label_permutations(std::vector<std::string>{"z", "a", "abc", "", "aaa", "zz"});
	label_permutations(std::vector<std::uint64_t>{UINT64_MAX, 0, 257, UINT64_C(1) << 63, 1, UINT64_MAX - 1, 42});
	label_permutations(std::vector<std::int64_t>{INT64_MIN, INT64_MAX, -1, 0, 257, 1});
	label_permutations(std::vector<std::pair<int, std::string>>{ {2, "z"}, {-1, "x"}, {2, "a"}, {0, ""}, {1, "b"} });
	std::array<std::string, 3> alphabet{"red", "blue", "green"};
	std::deque<std::string> values{"green", "red", "blue"};
	assert(permutation_rank(values, alphabet) == 4);
	assert((permutation_unrank(alphabet, 4) == std::vector<std::string>{"green", "red", "blue"}));
	std::pmr::vector<std::string> pooled(values.begin(), values.end());
	assert(permutation_rank(pooled, std::span(alphabet)) == 4);
	assert(permutation_rank(std::string("BAC"), std::string("CAB")) == 5);
	assert((permutation_unrank(std::string("CAB"), 5) == std::vector<char>{'B', 'A', 'C'}));
	std::vector<std::uint64_t> large(300);
	for (int i = 0; i < int(large.size()); ++i) large[i] = UINT64_MAX - std::uint64_t(i) * 65537;
	auto swapped = large;
	std::swap(swapped[298], swapped[299]);
	assert(permutation_rank<std::uint8_t>(swapped, large) == 1);
	assert(permutation_unrank(large, std::uint8_t(1)) == swapped);
	std::vector<std::string> reverse{"c", "b", "a", "f", "e", "d"}, order = reverse;
	std::reverse(reverse.begin(), reverse.end());
	assert(permutation_rank<Big>(reverse, order) == 719);
	bool caught = false;
	try { (void)permutation_rank<std::uint8_t>(reverse, order); } catch (const std::overflow_error &) { caught = true; }
	assert(caught);
	for (const std::vector<std::string> &bad : std::vector<std::vector<std::string>>{ {"red", "red", "green"}, {"red", "blue", "unknown"}, {"red", "blue"} }) {
		caught = false;
		try { (void)permutation_rank(bad, alphabet); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	std::vector<std::string> duplicates{"x", "x"};
	caught = false;
	try { (void)permutation_rank(duplicates, duplicates); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
	caught = false;
	try { (void)permutation_unrank(duplicates, 0); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
	caught = false;
	try { (void)permutation_rank(std::vector<int>{1000}); } catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
	assert(permutation_rank(std::vector<int>{1000}, std::vector<int>{1000}) == 0);
}

int main() {
	labels();
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
	for (int n : {0, 1, 2, 19, 20, 21, 22, 63, 64, 65, 1000}) {
		for (std::uint64_t rank : std::array<std::uint64_t, 5>{0, 1, 3628800, 2432902008176640000ULL, UINT64_MAX})
			assert(permutation_unrank(n, rank) == permutation_unrank(n, Big(rank)));
		assert(permutation_unrank(n, UINT32_MAX) == permutation_unrank(n, Big(UINT32_MAX)));
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
