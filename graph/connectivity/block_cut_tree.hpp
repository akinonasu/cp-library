#ifndef BLOCK_CUT_TREE_HPP
#define BLOCK_CUT_TREE_HPP

#include "../../components/undirected_csr.hpp"
#include <algorithm>

class BlockCutTree {
	struct Frame { int v, parent, next; };
	bool built_ = false;
	std::vector<int> vertices_, boundaries_;
	void check_([[maybe_unused]] int v) const { assert(built_ && 0 <= v && v < n); }

  public:
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> node_id, articulation;
	std::vector<std::vector<int>> tree;

	explicit BlockCutTree(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		assert(edges.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		built_ = false;
		int id = static_cast<int>(edges.size());
		edges.emplace_back(u, v);
		return id;
	}
	void build() {
		built_ = false;
		UndirectedCSR graph(n, edges);
		std::vector<int> ord(n, -1), low(n), active;
		std::vector<Frame> stack;
		active.reserve(n), stack.reserve(n);
		vertices_.clear(), boundaries_.assign(1, 0);
		vertices_.reserve(n);
		node_id.assign(n, -1), articulation.clear(), tree.clear();
		int timer = 0;
		auto append = [&](int v, int block) {
			vertices_.push_back(v);
			node_id[v] = node_id[v] == -1 ? block : -2;
		};
		for (int root = 0; root < n; ++root) if (ord[root] == -1) {
			ord[root] = low[root] = timer++;
			active.push_back(root);
			stack.push_back({root, -1, graph.start[root]});
			while (!stack.empty()) {
				auto& frame = stack.back();
				int v = frame.v;
				if (frame.next == graph.start[v + 1]) {
					stack.pop_back();
					if (stack.empty()) {
						active.pop_back();
						if (node_id[v] == -1) {
							append(v, static_cast<int>(boundaries_.size()) - 1);
							boundaries_.push_back(static_cast<int>(vertices_.size()));
						}
					} else {
						int p = stack.back().v;
						low[p] = std::min(low[p], low[v]);
						if (low[v] >= ord[p]) {
							int block = static_cast<int>(boundaries_.size()) - 1, u;
							append(p, block);
							do {
								u = active.back();
								active.pop_back();
								append(u, block);
							} while (u != v);
							boundaries_.push_back(static_cast<int>(vertices_.size()));
						}
					}
					continue;
				}
				auto [to, id] = graph.adj[frame.next++];
				if (id == frame.parent) continue;
				if (ord[to] != -1) low[v] = std::min(low[v], ord[to]);
				else {
					ord[to] = low[to] = timer++;
					active.push_back(to);
					stack.push_back({to, id, graph.start[to]});
				}
			}
		}
		int blocks = static_cast<int>(boundaries_.size()) - 1;
		for (int v = 0; v < n; ++v) if (node_id[v] == -2) {
			node_id[v] = blocks + static_cast<int>(articulation.size());
			articulation.push_back(v);
		}
		tree.resize(std::size_t(blocks) + articulation.size());
		std::vector<int> degree(tree.size());
		for (int c = 0; c < blocks; ++c)
			for (int i = boundaries_[c]; i < boundaries_[c + 1]; ++i) {
				int v = node_id[vertices_[i]];
				if (v >= blocks) ++degree[c], ++degree[v];
			}
		for (std::size_t v = 0; v < tree.size(); ++v) tree[v].reserve(degree[v]);
		for (int c = 0; c < blocks; ++c)
			for (int i = boundaries_[c]; i < boundaries_[c + 1]; ++i) {
				int v = node_id[vertices_[i]];
				if (v >= blocks) tree[c].push_back(v), tree[v].push_back(c);
			}
		built_ = true;
	}
	int block_count() const { assert(built_); return static_cast<int>(boundaries_.size()) - 1; }
	int node_count() const { assert(built_); return static_cast<int>(tree.size()); }
	bool is_articulation(int v) const { check_(v); return node_id[v] >= block_count(); }
	bool is_block(int node) const { assert(built_ && 0 <= node && node < node_count()); return node < block_count(); }
	int vertex(int node) const {
		assert(built_ && 0 <= node && node < node_count());
		return node < block_count() ? -1 : articulation[node - block_count()];
	}
	std::span<const int> group(int block) const {
		assert(built_ && 0 <= block && block < block_count());
		return {vertices_.data() + boundaries_[block], std::size_t(boundaries_[block + 1] - boundaries_[block])};
	}
	std::vector<std::vector<int>> groups() const {
		std::vector<std::vector<int>> result(block_count());
		for (int c = 0; c < block_count(); ++c) {
			auto members = group(c);
			result[c].assign(members.begin(), members.end());
		}
		return result;
	}
};

#endif
