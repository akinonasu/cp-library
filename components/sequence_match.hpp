#ifndef SEQUENCE_MATCH_HPP
#define SEQUENCE_MATCH_HPP

#include "sequence_compare.hpp"
#include "z_algorithm.hpp"
#include <algorithm>
#include <array>
#include <span>
#include <utility>
#include <vector>

namespace sequence_match_detail {
#if defined(__x86_64__)
using sequence_compare_detail::forward_avx2, sequence_compare_detail::backward_avx2;
#endif

template <class T, bool Vectorize = true>
class Matcher {
	const T *data;
	std::vector<std::array<int, 2>> runs;
public:
	explicit Matcher(std::span<const T> values) : data(values.data()) {}
	void index_runs(int n) {
		runs.resize(n);
		for (int first = 0, last; first < n; first = last) {
			last = first + 1;
			while (last < n && data[first] == data[last]) ++last;
			for (int i = first; i < last; ++i) runs[i] = {first, last};
		}
	}
	std::pair<int, int> forward_counted(int i, int j, int limit) const {
		if (runs.empty() || !limit || data[i] != data[j]) {
			int length = forward(i, j, limit);
			return {length, length};
		}
		int a = runs[i][1] - i, b = runs[j][1] - j, skip = std::min({limit, a, b});
		if (a != b || skip == limit) return {skip, 1};
		int length = forward(i + skip, j + skip, limit - skip);
		return {skip + length, length + 1};
	}
	std::pair<int, int> backward_counted(int i, int j, int limit) const {
		if (runs.empty() || !limit || data[i - 1] != data[j - 1]) {
			int length = backward(i, j, limit);
			return {length, length};
		}
		int a = i - runs[i - 1][0], b = j - runs[j - 1][0], skip = std::min({limit, a, b});
		if (a != b || skip == limit) return {skip, 1};
		int length = backward(i - skip, j - skip, limit - skip);
		return {skip + length, length + 1};
	}
	int forward(int i, int j, int limit) const {
		if (!limit) return 0;
		return sequence_compare_detail::prefix<Vectorize>(data + i, data + j, limit);
	}
	int backward(int i, int j, int limit) const {
		if (!limit) return 0;
		return sequence_compare_detail::suffix<Vectorize>(data + i, data + j, limit);
	}
};

}

#endif
