#include <boost/multiprecision/cpp_int.hpp>
#include "data_structure/interval_map.hpp"
#include <cassert>
#include <limits>
#include <vector>

using boost::multiprecision::cpp_int;
using boost::multiprecision::int256_t;
using boost::multiprecision::uint256_t;

template<class T> void boundaries() {
	const T lo = (std::numeric_limits<T>::lowest)(), hi = (std::numeric_limits<T>::max)();
	const cpp_int length = cpp_int(hi) - cpp_int(lo);
	IntervalMap<T, int, cpp_int> map(lo, hi, 1);
	assert(map.size() == length && map.count(lo, hi) == length);
	map.assign(lo, T(1), 2);
	assert(map.interval_count() == 2 && map.contains(lo, hi));
	assert(map.get(lo) == 2 && map.at(T(1)) == 1 && !map.get(hi));
	assert(map.erase(lo) == 1 && map.size() == length - 1);
	map.set(lo, 2);
	assert(map.size() == length && map.interval_count() == 2);
	assert(map.erase(hi) == 0 && map.erase(lo, hi) == length && map.empty());
}

int main() {
	const cpp_int large = cpp_int(1) << 1000;
	IntervalMap<cpp_int, int> map(-large, large, 1);
	assert(map.size() == 2 * large);
	map.assign(-7, 9, 2);
	assert(map.interval_count() == 3 && map.contains(-large, large));
	assert(map.count(-7, 9) == 16 && map.contains(-7, 9, 2));
	assert(map.erase(-3, 2) == 5 && map.size() == 2 * large - 5);
	assert(map.interval_count() == 4 && !map.get(0));
	assert(map.lower_bound(0)->first == 2 && !map.contains(-4, 4));
	std::vector<int> values;
	map.for_each(-4, 4, [&](const cpp_int &l, const cpp_int &r, const int &v) {
		assert((-4 <= l && r <= -3) || (2 <= l && r <= 4));
		values.push_back(v);
	});
	assert((values == std::vector<int>{2, 2}));
	map.assign(-large, large, map.at(3));
	assert(map.interval_count() == 1 && map.size() == 2 * large);
	auto moved = std::move(map);
	assert(map.empty() && map.size() == 0 && moved.size() == 2 * large);
	map = std::move(moved);
	assert(moved.empty() && moved.size() == 0 && map.size() == 2 * large);
	map.set(large, 3);
	assert(map.get(large) == 3 && map.size() == 2 * large + 1);
	boundaries<int256_t>();
	boundaries<uint256_t>();
}
