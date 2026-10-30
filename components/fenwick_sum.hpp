#ifndef FENWICK_SUM_HPP
#define FENWICK_SUM_HPP

#include <cassert>
#include <bit>
#include <limits>
#include <vector>

template <class T>
class FenwickSum {
	std::vector<T> data;

  public:
	FenwickSum() = default;
	explicit FenwickSum(int n) { assert(n >= 0); data.resize(n); }
	explicit FenwickSum(const std::vector<T> &a) { build(a); }
	void build(const std::vector<T> &a) {
		assert(a.size() <= static_cast<unsigned>(std::numeric_limits<int>::max()));
		data = a;
		for (unsigned i = 2; i <= data.size(); i += 2) {
			data[i - 1] += data[i - 2];
			unsigned p = i + (i & -i);
			if (p <= data.size()) data[p - 1] += data[i - 1];
		}
	}
	int size() const { return static_cast<int>(data.size()); }
	void add(int p, const T &x) {
		assert(0 <= p && p < size());
		for (unsigned i = unsigned(p) + 1; i <= data.size(); i += i & -i) data[i - 1] += x;
	}
	void add_pair(int p, const T &x, int q, const T &y) {
		assert(0 <= p && p < size() && 0 <= q && q < size());
		unsigned a = unsigned(p) + 1, b = unsigned(q) + 1;
		while (a != b && (a <= data.size() || b <= data.size())) {
			if (a < b) data[a - 1] += x, a += a & -a;
			else data[b - 1] += y, b += b & -b;
		}
		T both = x;
		both += y;
		for (; a <= data.size(); a += a & -a) data[a - 1] += both;
	}
	T prefix(int r) const {
		assert(0 <= r && r <= size());
		T result{};
		for (; r; r &= r - 1) result += data[r - 1];
		return result;
	}
	template <class Accum>
	Accum prod_as(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		Accum result{};
		if (l == r) return result;
		unsigned shared = unsigned(l) & (std::numeric_limits<unsigned>::max() << std::bit_width(unsigned(l ^ r)));
		for (; unsigned(l) != shared; l &= l - 1) result -= data[l - 1];
		for (; unsigned(r) != shared; r &= r - 1) result += data[r - 1];
		return result;
	}
	T prod(int l, int r) const { return prod_as<T>(l, r); }
};

#endif
