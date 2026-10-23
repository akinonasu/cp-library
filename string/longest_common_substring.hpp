#ifndef LONGEST_COMMON_SUBSTRING_HPP
#define LONGEST_COMMON_SUBSTRING_HPP

#include "../components/sequence_compare.hpp"
#include "../components/suffix_array.hpp"
#include "../components/substring_seed.hpp"
#include <algorithm>
#include <cassert>
#include <limits>
#include <string_view>
#include <vector>

struct LongestCommonSubstringResult {
	int s_pos = 0, t_pos = 0, length = 0;
};

inline LongestCommonSubstringResult longest_common_substring(std::string_view s, std::string_view t) {
	[[maybe_unused]] constexpr std::size_t maximum = std::numeric_limits<int>::max() - 1;
	assert(s.size() <= maximum && t.size() <= maximum - s.size());
	if (s.empty() || t.empty()) return {};
	int n = int(s.size()), m = int(t.size()), bound = std::min(n, m);
	LongestCommonSubstringResult best{0, 0, sequence_compare_detail::prefix(s.data(), t.data(), bound)};
	if (best.length == bound) return best;
	if (auto match = substring_seed_detail::search(s, t, {best.s_pos, best.t_pos, best.length}))
		return {match->first, match->second, match->length};
	struct Joined {
		std::string_view s, t;
		int split;
		std::size_t size() const { return s.size() + t.size() + 1; }
		int operator[](int p) const {
			if (p < split) return static_cast<unsigned char>(s[p]) + 1;
			return p == split ? 0 : static_cast<unsigned char>(t[p - split - 1]) + 1;
		}
	};
	auto sa = suffix_array_detail::induced_sort(Joined{s, t, n}, 256);
	int size = int(sa.size());
	std::vector<int> rank(size);
	for (int i = 0; i < size; ++i) rank[sa[i]] = i;
	for (int p = 0, h = 0; p < size; ++p) {
		if (h) --h;
		if (!rank[p]) { h = 0; continue; }
		int q = sa[rank[p] - 1];
		if (q == n) { h = 0; continue; }
		bool first = p < n, other = q < n;
		int a = first ? p : p - n - 1, b = other ? q : q - n - 1;
		auto left = first ? s : t, right = other ? s : t;
		int limit = std::min(int(left.size()) - a, int(right.size()) - b);
		h += sequence_compare_detail::prefix(left.data() + a + h, right.data() + b + h, limit - h);
		if (first != other && h > best.length) {
			best = first ? LongestCommonSubstringResult{a, b, h} : LongestCommonSubstringResult{b, a, h};
			if (h == bound) return best;
		}
	}
	return best;
}

#endif
