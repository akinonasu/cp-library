#include <boost/multiprecision/cpp_int.hpp>
#include "data_structure/interval_set.hpp"
#include <array>
#include <cassert>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

using boost::multiprecision::cpp_int, boost::multiprecision::int256_t, boost::multiprecision::uint256_t;

template<class T, class Size = typename IntervalSet<T>::size_type>
void shifted(T origin) {
	std::mt19937 rng(731);
	IntervalSet<T, Size> s;
	std::array<bool, 65> present{};
	for (int step = 0; step < 2500; ++step) {
		int l = rng() % 66, r = rng() % 66;
		if (r < l) std::swap(l, r);
		bool insert = rng() & 1;
		unsigned changed = 0, size = 0;
		for (int x = l; x < r; ++x) {
			changed += present[x] != insert;
			present[x] = insert;
		}
		T left = origin + l, right = origin + r;
		assert((insert ? s.insert(left, right) : s.erase(left, right)) == changed);
		int x = rng() % 65;
		T point = origin + x;
		bool value = rng() & 1;
		assert((value ? s.insert(point) : s.erase(point)) == unsigned(present[x] != value));
		present[x] = value;
		std::vector<std::pair<T, T>> expected;
		for (int i = 0; i < 65;) {
			if (!present[i]) { ++i; continue; }
			int first = i;
			while (i < 65 && present[i]) { ++i; ++size; }
			expected.emplace_back(T(origin + first), T(origin + i));
		}
		assert((std::vector<std::pair<T, T>>(s.begin(), s.end()) == expected));
		assert((std::vector<std::pair<T, T>>(s.rbegin(), s.rend()) ==
			std::vector<std::pair<T, T>>(expected.rbegin(), expected.rend())));
		assert(s.size() == size && s.interval_count() == expected.size());
		assert(s.empty() == (size == 0));
		assert(s.min() == (size ? std::optional<T>(expected.front().first) : std::nullopt));
		assert(s.max() == (size ? std::optional<T>(T(expected.back().second - 1)) : std::nullopt));
		unsigned count = 0;
		for (int i = l; i < r; ++i) count += present[i];
		assert(s.count(left, right) == count);
		assert(s.contains(left, right) == (count == unsigned(r - l)));
		assert(s.intersects(left, right) == (count != 0));
		assert(s.contains(point) == present[x]);
		assert((s.find(point) != s.end()) == present[x]);
		std::optional<T> next, prev;
		for (int i = x; i < 65; ++i)
			if (present[i]) { next = T(origin + i); break; }
		for (int i = x; i >= 0; --i)
			if (present[i]) { prev = T(origin + i); break; }
		assert(s.next(point) == next && s.prev(point) == prev);
		auto it = s.lower_bound(point);
		assert((it != s.end()) == next.has_value());
		if (next) assert(it->first <= *next && *next < it->second);
		while (x < 65 && present[x]) ++x;
		assert(s.mex(point) == T(origin + x));
	}
	auto copy = s;
	IntervalSet<T, Size> moved(std::move(copy));
	assert(copy.empty() && copy.size() == 0 && moved.size() == s.size());
	copy = moved;
	moved.clear();
	assert(copy.size() == s.size() && moved.size() == 0);
	moved = std::move(copy);
	assert(copy.empty() && copy.size() == 0 && moved.size() == s.size());
	const auto &alias = moved;
	moved = alias;
	assert(moved.size() == s.size());
	swap(copy, moved);
	assert(moved.empty() && copy.size() == s.size());
	copy.swap(moved);
	assert(copy.empty() && moved.size() == s.size());
	moved.assign({{T(origin + 2), T(origin + 5)}, {origin, T(origin + 3)}, {T(origin + 5), T(origin + 5)}});
	assert(moved.interval_count() == 1 && moved.size() == 5);
	moved.assign({});
	assert(moved.empty() && moved.size() == 0);
}

template<class T, class Size>
void boundaries() {
	const T lo = (std::numeric_limits<T>::lowest)(), hi = (std::numeric_limits<T>::max)();
	const cpp_int expected = cpp_int(hi) - cpp_int(lo);
	IntervalSet<T, Size> s;
	assert(cpp_int(s.insert(lo, hi)) == expected && cpp_int(s.size()) == expected);
	assert(s.contains(lo, hi) && s.count(lo, hi) == s.size());
	assert(s.erase(hi) == 0 && !s.contains(hi) && s.mex(lo) == hi);
	assert(s.erase(lo) == 1 && s.erase(T(hi - 1)) == 1);
	assert(!s.prev(lo) && !s.next(T(hi - 1)));
	assert(s.insert(lo) == 1 && s.insert(T(hi - 1)) == 1);
	assert(cpp_int(s.size()) == expected && s.interval_count() == 1);
	assert(cpp_int(s.erase(lo, hi)) == expected && s.empty());
}

template<class T>
void large_counts() {
	const T big = T(1) << 200;
	IntervalSet<T> s(std::vector<std::pair<T, T>>{{0, big}, {T(2 * big), T(3 * big)}});
	assert(s.size() == 2 * big && s.count(0, T(3 * big)) == 2 * big);
	assert(s.insert(big, T(2 * big)) == big && s.size() == 3 * big);
	assert(s.erase(big, T(2 * big)) == big && s.size() == 2 * big);
	if constexpr (std::numeric_limits<T>::is_signed) {
		assert(s.insert(T(-big), 0) == big && s.size() == 3 * big);
		assert(s.count(T(-big), big) == 2 * big);
	}
	s.assign({{big, T(2 * big)}, {0, big}});
	assert(s.size() == 2 * big && s.interval_count() == 1);
}

int main() {
	static_assert(std::is_same_v<IntervalSet<cpp_int>::size_type, cpp_int>);
	static_assert(std::is_same_v<IntervalSet<int256_t>::size_type, int256_t>);
	static_assert(std::is_same_v<IntervalSet<uint256_t>::size_type, uint256_t>);
	static_assert(!std::numeric_limits<cpp_int>::is_bounded);
	shifted<__int128_t>(-(__int128_t(1) << 110));
	shifted<__uint128_t>(__uint128_t(1) << 120);
	shifted<cpp_int>(cpp_int(1) << 1024);
	shifted<cpp_int>(-(cpp_int(1) << 1024));
	shifted<int256_t>(-(int256_t(1) << 240));
	shifted<uint256_t>(uint256_t(1) << 240);
	shifted<int256_t, cpp_int>(int256_t(1) << 240);
	shifted<__int128_t, cpp_int>(-(__int128_t(1) << 110));
	boundaries<__int128_t, cpp_int>();
	boundaries<__uint128_t, cpp_int>();
	boundaries<int256_t, cpp_int>();
	boundaries<uint256_t, uint256_t>();
	boundaries<boost::multiprecision::checked_int256_t, cpp_int>();
	boundaries<boost::multiprecision::checked_uint256_t, boost::multiprecision::checked_uint256_t>();
	large_counts<cpp_int>();
	large_counts<int256_t>();
	large_counts<uint256_t>();
	large_counts<boost::multiprecision::checked_int256_t>();
	large_counts<boost::multiprecision::checked_uint256_t>();
	const cpp_int big = cpp_int(1) << 4096;
	IntervalSet<cpp_int> s;
	assert(s.insert(-big, big) == 2 * big);
	assert(s.erase(-big / 2, big / 2) == big && s.size() == big);
	assert(s.insert(-big / 2, big / 2) == big && s.size() == 2 * big);
	assert(s.count(-big, big) == 2 * big && s.mex(-big) == big);
	assert(s.erase(-big) == 1 && s.erase(cpp_int(big - 1)) == 1);
	assert(s.insert(-big) == 1 && s.insert(cpp_int(big - 1)) == 1);
	assert(s.erase(-big, big) == 2 * big && s.empty());
	assert(s.insert(0) == 1 && s.insert(-1) == 1 && s.mex(-1) == 1);
	assert(s.erase(0) == 1 && s.erase(-1) == 1 && s.empty());
}
