#ifndef TWO_EDGE_CONNECTED_COMPONENTS_BASELINE_HPP
#define TWO_EDGE_CONNECTED_COMPONENTS_BASELINE_HPP

#include "graph/connectivity/low_link.hpp"
#include <numeric>
#include <span>

class LowLinkComponents : public LowLink {
	std::vector<int> parent_, sizes_, start_, members_;
	int find_(int v) {
		while (parent_[v] != v) v = parent_[v] = parent_[parent_[v]];
		return v;
	}

  public:
	explicit LowLinkComponents(int n) : LowLink(n) {}
	void build() {
		LowLink::build();
		parent_.resize(n), sizes_.assign(n, 1);
		std::iota(parent_.begin(), parent_.end(), 0);
		for (int id = 0; id < int(edges.size()); ++id) if (!is_bridge(id)) {
			auto [u, v] = edges[id];
			int a = find_(u), b = find_(v);
			if (a == b) continue;
			if (sizes_[a] < sizes_[b]) std::swap(a, b);
			parent_[b] = a, sizes_[a] += sizes_[b];
		}
		std::vector<int> root_id(n, -1), component(n);
		int count = 0;
		for (int v = 0; v < n; ++v) {
			int root = find_(v);
			if (root_id[root] < 0) root_id[root] = count++;
			component[v] = root_id[root];
		}
		start_.assign(count + 1, 0), members_.resize(n);
		for (int v = 0; v < n; ++v) ++start_[component[v] + 1];
		std::partial_sum(start_.begin(), start_.end(), start_.begin());
		auto cursor = start_;
		for (int v = 0; v < n; ++v) members_[cursor[component[v]]++] = v;
	}
	int component_count() const { return int(start_.size()) - 1; }
	std::span<const int> group(int c) const { return {members_.data() + start_[c], std::size_t(start_[c + 1] - start_[c])}; }
};

#endif
