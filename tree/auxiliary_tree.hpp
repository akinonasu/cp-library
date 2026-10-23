#ifndef AUXILIARY_TREE_HPP
#define AUXILIARY_TREE_HPP

#include "../components/radix_sort.hpp"
#include <algorithm>
#include <cassert>
#include <span>
#include <utility>
#include <vector>

template <class Tree> class AuxiliaryTree {
  public:
	struct Node {
		int vertex, parent, length;
		bool selected;
	};

  private:
	struct Frame {
		int vertex, child;
		bool selected;
	};
	const Tree *tree_;
	std::vector<std::pair<int, int>> keys_;
	std::vector<Frame> stack_;
	std::vector<Node> nodes_;
	int finish() {
		Frame current = stack_.back();
		stack_.pop_back();
		int id = size();
		for (int child = current.child; child != -1;) {
			int next = nodes_[child].parent;
			nodes_[child].parent = id;
			nodes_[child].length = tree_->depth[nodes_[child].vertex] - tree_->depth[current.vertex];
			child = next;
		}
#ifdef __cpp_aggregate_paren_init
		nodes_.emplace_back(current.vertex, -1, 0, current.selected);
#else
		nodes_.push_back({current.vertex, -1, 0, current.selected});
#endif
		return id;
	}
	void attach(int child) {
		nodes_[child].parent = stack_.back().child;
		stack_.back().child = child;
	}
	const std::vector<Node> &construct(std::span<const int> vertices, bool sorted) {
		keys_.clear();
		for (int v : vertices) keys_.emplace_back(tree_->subtree(v).first, v);
		if (!sorted) {
			if (keys_.size() < 256) std::sort(keys_.begin(), keys_.end());
			else stable_radix_sort(keys_, [](const auto &x) { return static_cast<unsigned>(x.first); });
		}
		else assert(std::is_sorted(keys_.begin(), keys_.end()));
		nodes_.clear();
		stack_.clear();
		int previous = -1;
		for (auto [position, v] : keys_) {
			(void)position;
			if (v == previous) continue;
			previous = v;
			if (!stack_.empty()) {
				int w = tree_->lca(stack_.back().vertex, v);
				while (tree_->depth[stack_.back().vertex] > tree_->depth[w]) {
					int child = finish();
					if (stack_.empty() || tree_->depth[stack_.back().vertex] < tree_->depth[w])
#ifdef __cpp_aggregate_paren_init
						stack_.emplace_back(w, -1, false);
#else
						stack_.push_back({w, -1, false});
#endif
					attach(child);
				}
			}
#ifdef __cpp_aggregate_paren_init
			stack_.emplace_back(v, -1, true);
#else
			stack_.push_back({v, -1, true});
#endif
		}
		while (stack_.size() > 1) attach(finish());
		if (!stack_.empty()) finish();
		return nodes_;
	}

  public:
	explicit AuxiliaryTree(const Tree &tree) : tree_(&tree) {}
	AuxiliaryTree(const Tree &&) = delete;
	int size() const { return static_cast<int>(nodes_.size()); }
	bool empty() const { return nodes_.empty(); }
	int root() const { return size() - 1; }
	const Node &operator[](int i) const {
		assert(0 <= i && i < size());
		return nodes_[i];
	}
	const std::vector<Node> &nodes() const { return nodes_; }
	std::vector<std::vector<int>> to_graph() const {
		std::vector<std::vector<int>> graph(size());
		std::vector<int> degree(size());
		for (int i = 0; i < size(); ++i) if (nodes_[i].parent >= 0) {
			++degree[i];
			++degree[nodes_[i].parent];
		}
		for (int i = 0; i < size(); ++i) graph[i].reserve(degree[i]);
		for (int i = 0; i < size(); ++i) if (nodes_[i].parent >= 0) {
			int parent = nodes_[i].parent;
			graph[i].push_back(parent);
			graph[parent].push_back(i);
		}
		return graph;
	}
	const std::vector<Node> &build(std::span<const int> vertices) { return construct(vertices, false); }
	const std::vector<Node> &build_sorted(std::span<const int> vertices) { return construct(vertices, true); }
};

#endif
