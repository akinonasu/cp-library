#ifndef TOP_TREE_HPP
#define TOP_TREE_HPP

#include <cassert>
#include <utility>
#include <vector>

template <class DP>
class TopTree {
  public:
	using Vertex = typename DP::Vertex;
	using Path = typename DP::Path;
	using Point = typename DP::Point;

  private:
	struct Node {
		Vertex value;
		Path forward, backward;
		int child[2] = {}, parent = 0, light = 0, belong = 0;
		bool reversed = false;
	};
	struct Rake {
		Point value, sum;
		int child[2] = {}, parent = 0;
	};
	std::vector<Node> nodes_;
	std::vector<Rake> rake_;
	int free_ = 0;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < size()); }
	bool path_root(int v) const {
		int p = nodes_[v].parent;
		return !p || (nodes_[p].child[0] != v && nodes_[p].child[1] != v);
	}
	void rake_pull(int v) {
		auto& x = rake_[v];
		x.sum = x.value;
		for (int c : x.child) if (c) x.sum = DP::rake(x.sum, rake_[c].sum);
	}
	void rake_rotate(int v) {
		int p = rake_[v].parent, g = rake_[p].parent, d = rake_[p].child[1] == v;
		int c = rake_[v].child[d ^ 1];
		rake_[p].child[d] = c;
		if (c) rake_[c].parent = p;
		rake_[v].child[d ^ 1] = p, rake_[p].parent = v, rake_[v].parent = g;
		if (g) rake_[g].child[rake_[g].child[1] == p] = v;
		rake_pull(p), rake_pull(v);
	}
	void rake_splay(int v) {
		while (int p = rake_[v].parent) {
			int g = rake_[p].parent;
			if (g) rake_rotate((rake_[p].child[1] == v) == (rake_[g].child[1] == p) ? p : v);
			rake_rotate(v);
		}
	}
	int rake_insert(int root, const Point& value) {
		assert(free_);
		int v = free_;
		free_ = rake_[v].parent;
		rake_[v] = {value, value, {root, 0}, 0};
		if (root) rake_[root].parent = v;
		rake_pull(v);
		return v;
	}
	int rake_erase(int v) {
		rake_splay(v);
		int l = rake_[v].child[0], r = rake_[v].child[1];
		rake_[v].parent = free_, free_ = v;
		if (!l) {
			if (r) rake_[r].parent = 0;
			return r;
		}
		rake_[l].parent = 0;
		int root = l;
		while (rake_[root].child[1]) root = rake_[root].child[1];
		rake_splay(root);
		rake_[root].child[1] = r;
		if (r) rake_[r].parent = root;
		rake_pull(root);
		return root;
	}
	Path vertex_path(int v) const {
		const auto& x = nodes_[v];
		return DP::add_vertex(x.light ? rake_[x.light].sum : DP::point_identity(), x.value);
	}
	void pull(int v) {
		auto& x = nodes_[v];
		x.forward = x.backward = vertex_path(v);
		if (int l = x.child[0]) {
			x.forward = DP::compress(nodes_[l].forward, x.forward);
			x.backward = DP::compress(x.backward, nodes_[l].backward);
		}
		if (int r = x.child[1]) {
			x.forward = DP::compress(x.forward, nodes_[r].forward);
			x.backward = DP::compress(nodes_[r].backward, x.backward);
		}
	}
	void reverse(int v) {
		auto& x = nodes_[v];
		using std::swap;
		std::swap(x.child[0], x.child[1]);
		swap(x.forward, x.backward);
		x.reversed = !x.reversed;
	}
	void push(int v) {
		auto& x = nodes_[v];
		if (!x.reversed) return;
		for (int c : x.child) if (c) reverse(c);
		x.reversed = false;
	}
	void rotate(int v) {
		int p = nodes_[v].parent, g = nodes_[p].parent, d = nodes_[p].child[1] == v;
		int c = nodes_[v].child[d ^ 1];
		if (path_root(p)) {
			nodes_[v].belong = nodes_[p].belong;
			nodes_[p].belong = 0;
		} else nodes_[g].child[nodes_[g].child[1] == p] = v;
		nodes_[p].child[d] = c;
		if (c) nodes_[c].parent = p;
		nodes_[v].child[d ^ 1] = p, nodes_[p].parent = v, nodes_[v].parent = g;
		pull(p), pull(v);
	}
	void splay(int v) {
		push(v);
		while (!path_root(v)) {
			int p = nodes_[v].parent;
			if (!path_root(p)) {
				int g = nodes_[p].parent;
				push(g), push(p), push(v);
				rotate((nodes_[p].child[1] == v) == (nodes_[g].child[1] == p) ? p : v);
			} else push(p), push(v);
			rotate(v);
		}
	}
	void expose(int v) {
		int last = 0;
		for (int x = v; x; x = nodes_[x].parent) {
			splay(x);
			int r = nodes_[x].child[1];
			if (r) {
				nodes_[x].light = rake_insert(nodes_[x].light, DP::add_edge(nodes_[r].forward));
				nodes_[r].belong = nodes_[x].light;
			}
			if (last) {
				nodes_[x].light = rake_erase(nodes_[last].belong);
				nodes_[last].belong = 0;
			}
			nodes_[x].child[1] = last;
			pull(x), last = x;
		}
		splay(v);
	}
	void evert(int v) { expose(v); reverse(v); }

  public:
	TopTree() = default;
	explicit TopTree(std::vector<Vertex> values) {
		int n = int(values.size());
		nodes_.reserve(n + 1), rake_.reserve(n + 1);
		if (n) {
			Path path = DP::add_vertex(DP::point_identity(), values.front());
			nodes_.push_back({values.front(), path, path});
		}
		for (auto& value : values) {
			Path path = DP::add_vertex(DP::point_identity(), value);
			nodes_.push_back({std::move(value), path, path});
		}
		for (int i = 0; i <= n; ++i) rake_.push_back({DP::point_identity(), DP::point_identity(), {}, i < n ? i + 1 : 0});
		free_ = n ? 1 : 0;
	}
	int size() const { return nodes_.empty() ? 0 : int(nodes_.size()) - 1; }
	bool empty() const { return size() == 0; }
	const Vertex& get(int v) const { check(v); return nodes_[v + 1].value; }
	void set(int v, const Vertex& value) {
		check(v), ++v;
		expose(v), nodes_[v].value = value, pull(v);
	}
	bool same(int u, int v) {
		check(u), check(v), ++u, ++v;
		expose(u), expose(v);
		return u == v || nodes_[u].parent;
	}
	bool link(int u, int v) {
		check(u), check(v), ++u, ++v;
		evert(u), expose(v);
		if (u == v || nodes_[u].parent) return false;
		nodes_[u].parent = v, nodes_[v].child[1] = u;
		pull(v);
		return true;
	}
	bool cut(int u, int v) {
		check(u), check(v), ++u, ++v;
		evert(u), expose(v), push(u);
		if (nodes_[v].child[0] != u || nodes_[u].child[1]) return false;
		nodes_[v].child[0] = nodes_[u].parent = 0;
		pull(v);
		return true;
	}
	Path prod(int u, int v) {
		check(u), check(v), ++u, ++v;
		evert(u), expose(v);
		assert(u == v || nodes_[u].parent);
		return nodes_[v].forward;
	}
	Path all_prod(int root) { return subtree_prod(root, root); }
	Path subtree_prod(int root, int v) {
		check(root), check(v), ++root, ++v;
		evert(root), expose(v);
		assert(root == v || nodes_[root].parent);
		return vertex_path(v);
	}
};

#endif
