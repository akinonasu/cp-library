#ifndef SCC_HPP
#define SCC_HPP

#include "../../components/scc_core.hpp"

class SCCGraph {
	int n_;
	std::vector<std::pair<int, int>> edges_;

  public:
	explicit SCCGraph(int n = 0) : n_(n) { assert(n >= 0); }
	int size() const { return n_; }
	std::size_t edge_count() const { return edges_.size(); }
	void reserve(std::size_t m) {
		assert(m <= std::size_t(std::numeric_limits<int>::max()));
		edges_.reserve(m);
	}
	void add_edge(int from, int to) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		assert(edges_.size() < std::size_t(std::numeric_limits<int>::max()));
		edges_.emplace_back(from, to);
	}
	std::pair<int, std::vector<int>> scc_ids() const { return scc_detail::ids(n_, edges_); }
	std::vector<std::vector<int>> scc() const {
		auto [count, id] = scc_ids();
		std::vector<int> sizes(count);
		for (int x : id) ++sizes[x];
		std::vector<std::vector<int>> groups(count);
		for (int i = 0; i < count; ++i) groups[i].reserve(sizes[i]);
		for (int v = 0; v < n_; ++v) groups[id[v]].push_back(v);
		return groups;
	}
};

#endif
