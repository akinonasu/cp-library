#ifndef SUFFIX_LOCUS_HPP
#define SUFFIX_LOCUS_HPP

#include "../string/suffix_tree.hpp"

class SuffixLocusIndex {
	struct Node { int parent, depth, head, vertex; };
	std::vector<Node> nodes_;
	std::vector<int> terminal_;

  public:
	SuffixLocusIndex() = default;
	explicit SuffixLocusIndex(const SuffixTree &tree) { build(tree); }
	SuffixLocusIndex(const SuffixLocusIndex &) = default;
	SuffixLocusIndex &operator=(const SuffixLocusIndex &) = default;
	SuffixLocusIndex(SuffixLocusIndex &&other) noexcept { swap(other); }
	SuffixLocusIndex &operator=(SuffixLocusIndex &&other) noexcept {
		if (this != &other) { SuffixLocusIndex moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(SuffixLocusIndex &other) noexcept {
		nodes_.swap(other.nodes_);
		terminal_.swap(other.terminal_);
	}
	void build(const SuffixTree &tree) {
		SuffixLocusIndex result;
		int count = tree.node_count();
		std::vector<int> heavy(count, -1), position(count);
		for (int v = 0; v < count; ++v)
			for (int u : tree.children(v))
				if (heavy[v] == -1 || tree.occurrence_count(u) > tree.occurrence_count(heavy[v])) heavy[v] = u;
		result.nodes_.clear();
		result.nodes_.reserve(count);
		std::vector<std::pair<int, int>> stack{{0, -1}};
		while (!stack.empty()) {
			auto [v, parent] = stack.back();
			stack.pop_back();
			int head = int(result.nodes_.size());
			while (v != -1) {
				int i = int(result.nodes_.size());
				result.nodes_.push_back({parent, tree.depth(v), head, v});
				position[v] = i;
				for (int u : tree.children(v)) if (u != heavy[v]) stack.emplace_back(u, i);
				parent = i;
				v = heavy[v];
			}
		}
		result.terminal_.resize(std::size_t(tree.size()) + 1);
		for (int p = 0; p <= tree.size(); ++p) result.terminal_[p] = position[tree.suffix_node(p)];
		swap(result);
	}
	int size() const { return terminal_.empty() ? 0 : int(terminal_.size()) - 1; }
	SuffixTree::Locus locate(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		int length = r - l;
		if (!length) return {0, 0};
		int v = terminal_[l];
		for (;;) {
			int p = nodes_[nodes_[v].head].parent;
			if (p == -1 || nodes_[p].depth < length) break;
			v = p;
		}
		int a = nodes_[v].head, b = v;
		while (a < b) {
			int m = a + (b - a) / 2;
			if (nodes_[m].depth < length) a = m + 1;
			else b = m;
		}
		return {nodes_[a].vertex, length};
	}
};

#endif
