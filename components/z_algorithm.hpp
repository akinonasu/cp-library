#ifndef Z_ALGORITHM_HPP
#define Z_ALGORITHM_HPP

namespace sequence_match_detail {
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
