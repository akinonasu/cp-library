#include "components/binary_search.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <random>
#include <vector>

int main() {
	for (int n = 0; n <= 257; ++n) for (int boundary = 0; boundary <= n; ++boundary) {
		int calls = 0;
		int result = binary_search_detail::partition_point(n, [&](int p) {
			assert(0 <= p && p < n);
			++calls;
			return p < boundary;
		});
		assert(result == boundary && calls <= 9);
	}
	const int maximum = std::numeric_limits<int>::max();
	for (int n : {maximum / 2, maximum - 1, maximum})
		for (int boundary : {0, 1, n / 2, n - 1, n}) {
			int calls = 0;
			int result = binary_search_detail::partition_point(n, [&](int p) {
				assert(0 <= p && p < n);
				++calls;
				return p < boundary;
			});
			assert(result == boundary && calls <= 31);
		}
	std::mt19937 rng(20261019);
	for (int n : {0, 1, 2, 3, 31, 32, 33, 1000}) {
		std::vector<int> a(n);
		for (auto &x : a) x = rng() % 100;
		std::sort(a.begin(), a.end());
		for (int x = -1; x <= 100; ++x) {
			auto lower = binary_search_detail::partition_point(n, [&](int p) { return a[p] < x; });
			auto upper = binary_search_detail::partition_point(n, [&](int p) { return a[p] <= x; });
			assert(lower == std::lower_bound(a.begin(), a.end(), x) - a.begin());
			assert(upper == std::upper_bound(a.begin(), a.end(), x) - a.begin());
		}
	}
}
