#ifndef BINARY_SEARCH_HPP
#define BINARY_SEARCH_HPP

#include <cassert>

namespace binary_search_detail {
template<class Predicate>
int partition_point(int n, Predicate predicate) {
	assert(n >= 0);
	unsigned first = 0;
	for (unsigned width = static_cast<unsigned>(n) + 1; width > 1;) {
		unsigned half = width / 2;
		width -= half;
		first += static_cast<unsigned>(bool(predicate(static_cast<int>(first + half - 1)))) * half;
	}
	return static_cast<int>(first);
}
}

#endif
