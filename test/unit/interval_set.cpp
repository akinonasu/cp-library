#include "data_structure/interval_set.hpp"
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <vector>

template<class T>
void boundaries() {
	using Set = IntervalSet<T>;
	const T lo = (std::numeric_limits<T>::min)(), hi = (std::numeric_limits<T>::max)();
	using Size = typename Set::size_type;
	const Size length = static_cast<Size>(hi) - static_cast<Size>(lo);
	Set s;
	assert(s.insert(lo, hi) == length);
	assert(s.size() == length && s.interval_count() == 1);
	assert(s.contains(lo, hi) && !s.contains(hi));
	assert(s.mex(lo) == hi && s.mex(hi) == hi);
	assert(s.next(lo) == lo && !s.next(hi));
	assert(s.prev(hi) == static_cast<T>(hi - 1) && s.prev(lo) == lo);
	assert(s.min() == lo && s.max() == static_cast<T>(hi - 1));
	assert(s.count(lo, hi) == length && s.erase(hi) == 0);
	assert(s.erase(lo) == 1 && s.erase(static_cast<T>(hi - 1)) == 1);
	assert(!s.prev(lo) && !s.next(static_cast<T>(hi - 1)));
	assert(s.insert(lo) == 1 && s.insert(static_cast<T>(hi - 1)) == 1);
	assert(s.size() == length && s.interval_count() == 1);
	assert(s.erase(lo, hi) == length && s.empty());
}

void randomized() {
	std::mt19937 rng(731);
	IntervalSet<int> s;
	std::array<bool, 65> present{};
	for (int step = 0; step < 25000; ++step) {
		int l = static_cast<int>(rng() % 66) - 32, r = static_cast<int>(rng() % 66) - 32;
		if (l > r) std::swap(l, r);
		bool insert = rng() & 1;
		std::uint64_t changed = 0;
		for (int x = l; x < r; ++x) {
			changed += present[x + 32] != insert;
			present[x + 32] = insert;
		}
		assert((insert ? s.insert(l, r) : s.erase(l, r)) == changed);
		if (step % 13 == 0) {
			int x = static_cast<int>(rng() % 65) - 32;
			bool value = rng() & 1;
			auto delta = value ? s.insert(x) : s.erase(x);
			assert(delta == static_cast<unsigned>(present[x + 32] != value));
			present[x + 32] = value;
		}
		std::vector<std::pair<int, int>> expected;
		std::uint64_t size = 0;
		for (int x = -32; x < 33;) {
			if (!present[x + 32]) { ++x; continue; }
			int first = x;
			while (x < 33 && present[x + 32]) { ++x; ++size; }
			expected.emplace_back(first, x);
		}
		assert((std::vector<std::pair<int, int>>(s.begin(), s.end()) == expected));
		assert((std::vector<std::pair<int, int>>(s.rbegin(), s.rend()) ==
			std::vector<std::pair<int, int>>(expected.rbegin(), expected.rend())));
		assert(s.size() == size && s.interval_count() == expected.size() && s.empty() == !size);
		assert(s.min() == (size ? std::optional<int>(expected.front().first) : std::nullopt));
		assert(s.max() == (size ? std::optional<int>(expected.back().second - 1) : std::nullopt));
		for (int x = -33; x <= 34; ++x) {
			bool in = -32 <= x && x < 33 && present[x + 32];
			assert(s.contains(x) == in);
			auto it = s.find(x);
			assert((it != s.end()) == in);
			if (in) assert(it->first <= x && x < it->second);
			std::optional<int> next, prev;
			for (int y = (std::max)(x, -32); y < 33; ++y)
				if (present[y + 32]) { next = y; break; }
			for (int y = (std::min)(x, 32); y >= -32; --y)
				if (present[y + 32]) { prev = y; break; }
			assert(s.next(x) == next && s.prev(x) == prev);
			auto bound = s.lower_bound(x);
			assert((bound != s.end()) == next.has_value());
			if (next) assert(bound->first <= *next && *next < bound->second);
			int mex = x;
			while (-32 <= mex && mex < 33 && present[mex + 32]) ++mex;
			assert(s.mex(x) == mex);
		}
		for (int j = 0; j < 8; ++j) {
			l = static_cast<int>(rng() % 66) - 32; r = static_cast<int>(rng() % 66) - 32;
			if (l > r) std::swap(l, r);
			std::uint64_t count = 0;
			for (int x = l; x < r; ++x) count += present[x + 32];
			assert(s.count(l, r) == count);
			assert(s.contains(l, r) == (count == static_cast<unsigned>(r - l)));
			assert(s.intersects(l, r) == (count != 0));
		}
	}
}

int main() {
	boundaries<std::int8_t>(); boundaries<std::uint8_t>();
	boundaries<int>(); boundaries<std::uint32_t>();
	boundaries<std::int64_t>(); boundaries<std::uint64_t>();
	boundaries<__int128_t>(); boundaries<__uint128_t>();
	static_assert(std::is_same_v<IntervalSet<>::size_type, std::uint64_t>);
	static_assert(std::is_same_v<IntervalSet<__int128_t>::size_type, __uint128_t>);
	static_assert(std::is_same_v<IntervalSet<__uint128_t>::size_type, __uint128_t>);
	IntervalSet<> s;
	assert(s.empty() && !s.min() && !s.max() && s.mex() == 0);
	assert(s.contains(3, 3) && !s.intersects(3, 3) && s.count(3, 3) == 0);
	assert(s.insert(3, 3) == 0 && s.erase(3, 3) == 0);
	s.assign({{8, 10}, {2, 5}, {5, 8}, {0, 0}, {3, 4}, {-2, -1}});
	assert(s.interval_count() == 2 && s.size() == 9);
	assert(s.erase(4, 6) == 2 && s.interval_count() == 3);
	assert(s.insert(-1, 7) == 5 && s.interval_count() == 1);
	auto copy = s;
	IntervalSet<> moved(std::move(copy));
	assert(copy.empty() && copy.size() == 0 && moved.size() == s.size());
	copy = moved;
	assert(copy.size() == s.size());
	copy = std::move(moved);
	assert(moved.empty() && moved.size() == 0 && copy.size() == s.size());
	const auto &alias = copy;
	copy = alias;
	assert(copy.size() == s.size());
	IntervalSet<> other(std::vector<std::pair<std::int64_t, std::int64_t>>{{20, 30}});
	swap(other, copy);
	assert(copy.min() == 20 && other.min() == -2);
	copy.swap(other);
	assert(other.min() == 20 && copy.min() == -2);
	static_assert(std::is_const_v<std::remove_reference_t<decltype((copy.begin()->second))>>);
	copy.clear(); copy.assign({});
	assert(copy.empty() && copy.size() == 0);
	copy.insert(0); assert(copy.mex() == 1);
	randomized();
}
