#ifndef SEQUENCE_MATCH_HPP
#define SEQUENCE_MATCH_HPP

#include "sequence_compare.hpp"
#include <algorithm>
#include <array>
#include <span>
#include <utility>
#include <vector>

namespace sequence_match_detail {
#if defined(__x86_64__)
using sequence_compare_detail::forward_avx2;
using sequence_compare_detail::backward_avx2;
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

template <class F>
void z_table(int n, int *z, F match) {
	if (!n) return;
	z[0] = n;
	for (int i = 1, base = 0, end = 0; i < n; ++i) {
		int length = 0;
		if (i < end) {
			length = z[i - base];
			if (length < end - i) {
				z[i] = length;
				continue;
			}
			length = end - i;
		}
		length += match(length, i + length, n - i - length);
		z[i] = length;
		if (i + length > end) base = i, end = i + length;
	}
}

template <class F, class Emit>
void prefix_matches(int n, int m, const int *z, F match, Emit emit) {
	for (int i = 0, base = 0, end = 0; i < n; ++i) {
		int length = 0;
		if (i < end) {
			length = z[i - base];
			if (length < end - i) {
				emit(i, length);
				continue;
			}
			length = end - i;
		}
		length += match(length, i + length, m - length);
		if (i + length > end) base = i, end = i + length;
		emit(i, length);
	}
}
}

#endif
