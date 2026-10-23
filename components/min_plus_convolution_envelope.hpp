#ifndef MIN_PLUS_CONVOLUTION_ENVELOPE_HPP
#define MIN_PLUS_CONVOLUTION_ENVELOPE_HPP

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>
#include <cstddef>
#include <vector>

namespace min_plus_convolution_envelope_detail {
struct Envelope {
	struct Entry { int offset, end; std::uint32_t bias; };
	template <int step>
	static void sweep(const std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b, std::vector<std::uint32_t> &c,
		int first, int entering, int count, std::vector<Entry> &stack) {
		int n = int(a.size());
		stack.clear();
		auto value = [&](const Entry &e, int t) { return a[e.offset + step * t] + e.bias; };
		auto take = [&] { auto e = stack.back(); stack.pop_back(); return e; };
		Entry top{first - entering, n - 1, b[entering]};
		c[first] = std::min(c[first], value(top, 0));
		int t = 1;
		for (; t < count; ++t) {
			while (top.end < t) top = take();
			std::uint32_t best = value(top, t);
			int column = entering + step * t;
			Entry next{first - column, 0, b[column]};
			if (std::uint32_t v = value(next, t); v <= best) {
				int low = t;
				for (;;) {
					if (value(next, top.end) > value(top, top.end)) {
						int high = top.end;
						while (high - low > 1) {
							int mid = low + (high - low) / 2;
							(value(next, mid) <= value(top, mid) ? low : high) = mid;
						}
						next.end = low;
						stack.push_back(top);
						break;
					}
					low = top.end;
					if (stack.empty()) { next.end = n - 1; break; }
					top = take();
				}
				top = next;
				best = v;
			}
			int row = first + step * t;
			c[row] = std::min(c[row], best);
		}
		stack.push_back(top);
		while (t < n) {
			Entry e = take();
			if (e.end < t) continue;
			int last = e.end, lo = first + step * t, hi = first + step * last;
			if (hi < lo) std::swap(lo, hi);
			int column = first - e.offset;
			for (int row = lo; row <= hi; ++row) c[row] = std::min(c[row], a[row - column] + e.bias);
			t = last + 1;
		}
	}
	static std::vector<std::uint32_t> run(const std::vector<std::uint32_t> &a, const std::vector<std::uint32_t> &b) {
		int n = int(a.size()), m = int(b.size());
		std::vector<std::uint32_t> c(n + m - 1, std::numeric_limits<std::uint32_t>::max());
		std::vector<Entry> stack;
		stack.reserve(std::min(n, m));
		for (int j = 0; j < m; j += n) {
			int end = std::min(m, j + n);
			sweep<1>(a, b, c, j, j, end - j, stack);
			sweep<-1>(a, b, c, end + n - 2, end - 1, end - j, stack);
		}
		return c;
	}
};

template <class T, bool minimum>
std::optional<std::vector<T>> multiply(const std::vector<T> &a, const std::vector<T> &b) {
#if defined(__SIZEOF_INT128__)
	static_assert(std::is_integral_v<T> && sizeof(T) <= 8 && !std::is_same_v<T, bool>);
	using W = __int128_t;
	if (a.empty() || b.empty()) return std::vector<T>{};
	if (a.size() > std::size_t(std::numeric_limits<int>::max()) || b.size() > std::size_t(std::numeric_limits<int>::max()) - a.size()) return std::nullopt;
	int n = int(a.size()), m = int(b.size());
	int low = 0, high = n - 1;
	while (low < high) {
		int mid = low + (high - low) / 2;
		if (minimum ? a[mid] >= a[mid + 1] : a[mid] <= a[mid + 1]) high = mid;
		else low = mid + 1;
	}
	T extreme = a[low], endpoint = minimum ? std::min(a.front(), a.back()) : std::max(a.front(), a.back());
	W amin = minimum ? W(endpoint) : W(extreme), amax = minimum ? W(extreme) : W(endpoint);
	auto [bminptr, bmaxptr] = std::minmax_element(b.begin(), b.end());
	W bmin = W(*bminptr), bmax = W(*bmaxptr);
	if (amin + bmin < W(std::numeric_limits<T>::lowest()) || amax + bmax > W(std::numeric_limits<T>::max()) || amax - amin + bmax - bmin > W(std::numeric_limits<std::uint32_t>::max())) return std::nullopt;
	W abase = minimum ? amin : amax, bbase = minimum ? bmin : bmax, offset = abase + bbase;
	std::vector<std::uint32_t> x(n), y(m);
	for (int i = 0; i < n; ++i) x[i] = std::uint32_t(minimum ? W(a[i]) - abase : abase - W(a[i]));
	for (int i = 0; i < m; ++i) y[i] = std::uint32_t(minimum ? W(b[i]) - bbase : bbase - W(b[i]));
	auto c = Envelope::run(x, y);
	std::vector<T> result(c.size());
	for (std::size_t i = 0; i < c.size(); ++i) result[i] = T(minimum ? offset + c[i] : offset - c[i]);
	return result;
#else
	return std::nullopt;
#endif
}

}

#endif
