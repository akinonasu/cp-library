#ifndef UNION_FIND_HPP
#define UNION_FIND_HPP

#include <algorithm>
#include <cassert>
#include <vector>

class UnionFind {
	std::vector<int> parent_;
	int count_ = 0;

  public:
	explicit UnionFind(int n = 0) : count_(n) {
		assert(n >= 0);
		parent_.assign(n, -1);
	}
	int size() const { return static_cast<int>(parent_.size()); }
	int count() const { return count_; }
	int leader(int v) {
		assert(0 <= v && v < size());
		while (parent_[v] >= 0) {
			int p = parent_[v];
			if (parent_[p] < 0) return p;
			v = parent_[v] = parent_[p];
		}
		return v;
	}
	int merge(int a, int b) {
		a = leader(a), b = leader(b);
		if (a == b) return a;
		if (parent_[a] > parent_[b]) std::swap(a, b);
		parent_[a] += parent_[b];
		parent_[b] = a;
		--count_;
		return a;
	}
	bool same(int a, int b) { return leader(a) == leader(b); }
	int size(int v) { return -parent_[leader(v)]; }
	std::vector<std::vector<int>> groups() {
		std::vector<int> index(size(), -1);
		std::vector<std::vector<int>> result(count_);
		int next = 0;
		for (int v = 0; v < size(); ++v) if (parent_[v] < 0) {
			index[v] = next;
			result[next++].reserve(-parent_[v]);
		}
		for (int v = 0; v < size(); ++v) result[index[leader(v)]].push_back(v);
		return result;
	}
};

#endif
