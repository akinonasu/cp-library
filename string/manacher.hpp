#ifndef MANACHER_HPP
#define MANACHER_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

class Manacher {
  public:
	std::vector<int> odd, even;

	Manacher() = default;
	explicit Manacher(std::string_view s) { build(s); }
	template <class Sequence> requires requires(const Sequence &s) { s.size(); s[0]; }
	explicit Manacher(const Sequence &s) { build(s); }
	void build(std::string_view s) { build<std::string_view>(s); }
	template <class Sequence> requires requires(const Sequence &s) { s.size(); s[0]; }
	void build(const Sequence &s) {
		assert(s.size() <= std::size_t(std::numeric_limits<int>::max()));
		int n = int(s.size());
		odd.resize(n), even.resize(n);
		int first = std::min(n, 1);
		while (first < n && s[first] == s[0]) ++first;
		for (int i = 0; i < first; ++i) {
			odd[i] = std::min(i + 1, first - i);
			even[i] = std::min(i, first - i);
		}
		for (int i = first, k = 1; i < n;) {
			int limit = std::min(i + 1, n - i);
			while (k < limit && s[i - k] == s[i + k]) ++k;
			odd[i] = k;
			int j = 1;
			while (j < k && odd[i - j] != k - j) {
				odd[i + j] = std::min(odd[i - j], k - j);
				++j;
			}
			i += j;
			k = std::max(k - j, 1);
		}
		for (int i = first, k = 0; i < n;) {
			int limit = std::min(i, n - i);
			while (k < limit && s[i - k - 1] == s[i + k]) ++k;
			even[i] = k;
			int j = 1;
			while (j < k && even[i - j] != k - j) {
				even[i + j] = std::min(even[i - j], k - j);
				++j;
			}
			i += j;
			k = std::max(k - j, 0);
		}
	}
	int size() const { return int(odd.size()); }
	bool empty() const { return odd.empty(); }
	bool is_palindrome(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		int length = r - l, center = l + length / 2;
		return !length || (length & 1 ? odd[center] > length / 2 : even[center] >= length / 2);
	}
	std::pair<int, int> longest_palindrome() const {
		int start = 0, length = 0;
		for (int i = 0; i < size(); ++i) {
			int a = 2 * (odd[i] - 1) + 1, b = 2 * even[i];
			if (a > length) start = i - odd[i] + 1, length = a;
			if (b > length) start = i - even[i], length = b;
		}
		return {start, start + length};
	}
	std::int64_t palindrome_count() const {
		std::int64_t result = 0;
		for (int i = 0; i < size(); ++i) result += std::int64_t(odd[i]) + even[i];
		return result;
	}
	std::vector<int> lengths() const {
		if (empty()) return {};
		std::vector<int> result(2 * odd.size() - 1);
		for (int i = 0; i < size(); ++i) {
			result[2 * std::size_t(i)] = 2 * (odd[i] - 1) + 1;
			if (i) result[2 * std::size_t(i) - 1] = 2 * even[i];
		}
		return result;
	}
};

#endif
