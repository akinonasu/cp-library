#line 1 "test/hld_subtree_sum.test.cpp"
#define PROBLEM "https://judge.yosupo.jp/problem/vertex_add_subtree_sum"
#line 1 "data_structure/segment_tree/dynamic_segtree.hpp"


#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

template <class S, auto op, auto e, class Coord = std::int64_t>
class DynamicSegtree {
	static_assert(std::is_integral_v<Coord> && !std::is_same_v<Coord, bool> && sizeof(Coord) <= 8);
	static constexpr unsigned B = 8, limit = (1u << 26) - 1;
	using U = std::make_unsigned_t<Coord>;
	static constexpr unsigned tag = 1u << 31;
	static constexpr U sign = std::is_signed_v<Coord> ? U(1) << (std::numeric_limits<U>::digits - 1) : U(0);
	template <std::size_t... I>
	static auto units(std::index_sequence<I...>) { return std::array<S, sizeof...(I)>{(void(I), e())...}; }
	struct Leaf {
		std::array<U, B> keys{};
		std::array<S, B> values = units(std::make_index_sequence<B>{});
		S product = e();
		unsigned count = 0;
	};
	struct Node {
		U middle = 0;
		unsigned left = 0, right = 0;
		S product = e();
	};
	std::vector<Leaf> leaves;
	std::vector<Node> nodes{Node{}};
	unsigned root = 0, points = 0;
	Coord first, last;
	static U encode(Coord x) { return U(x) ^ sign; }
	static Coord decode(U x) { return Coord(x ^ sign); }
	Leaf &leaf(unsigned id) { return leaves[id & ~tag]; }
	const Leaf &leaf(unsigned id) const { return leaves[id & ~tag]; }
	const S &product(unsigned id) const { return id & tag ? leaf(id).product : nodes[id].product; }
	unsigned make_leaf() {
		leaves.emplace_back();
		return unsigned(leaves.size() - 1) | tag;
	}
	unsigned make_node(U middle, unsigned left, unsigned right) {
		S value = op(product(left), product(right));
		nodes.push_back({middle, left, right, std::move(value)});
		return unsigned(nodes.size() - 1);
	}
	template <unsigned N>
	[[gnu::always_inline]] static inline S fold(const S *values, unsigned count) {
		if constexpr (N == 1) return values[0];
		else {
			if (count <= N / 2) return fold<N / 2>(values, count);
			return op(fold<N / 2>(values, N / 2), fold<N / 2>(values + N / 2, count - N / 2));
		}
	}
	static void pull(Leaf &node) { node.product = fold<B>(node.values.data(), node.count); }

	template <bool Append>
	static void write(Leaf &node, unsigned p, U key, S value) {
		if (p < node.count && node.keys[p] == key) {
			if constexpr (Append) node.values[p] = op(node.values[p], value);
			else node.values[p] = std::move(value);
		} else {
			std::move_backward(node.keys.begin() + p, node.keys.begin() + node.count, node.keys.begin() + node.count + 1);
			std::move_backward(node.values.begin() + p, node.values.begin() + node.count, node.values.begin() + node.count + 1);
			node.keys[p] = key;
			node.values[p] = std::move(value);
			++node.count;
		}
		pull(node);
	}
	template <bool Append>
	void update(U key, S value) {
		if (!root) {
			root = make_leaf();
			write<Append>(leaf(root), 0, key, std::move(value));
			++points;
			return;
		}
		std::array<unsigned, 64> path;
		unsigned depth = 0, id = root;
		bool outside = false;
		while (!(id & tag)) {
			unsigned h = std::countr_zero(nodes[id].middle);
			if (((key ^ nodes[id].middle) >> h) > 1) { outside = true; break; }
			assert(depth < path.size());
			path[depth++] = id;
			id = key < nodes[id].middle ? nodes[id].left : nodes[id].right;
		}
		unsigned replacement = id;
		if (outside) {
			assert(points < limit);
			unsigned added = make_leaf();
			write<Append>(leaf(added), 0, key, std::move(value));
			unsigned h = std::bit_width(U(key ^ nodes[id].middle)) - 1;
			U middle = ((key | nodes[id].middle) >> h) << h;
			replacement = key < middle ? make_node(middle, added, id) : make_node(middle, id, added);
			++points;
		} else {
			auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
			bool exists = p < node.count && node.keys[p] == key;
			assert(exists || points < limit);
			if (exists || node.count < B) write<Append>(node, p, key, std::move(value));
			else {
				U low = std::min(key, node.keys[0]), high = std::max(key, node.keys[B - 1]);
				unsigned h = std::bit_width(U(low ^ high)) - 1;
				U middle = ((low | high) >> h) << h;
				unsigned cut = std::lower_bound(node.keys.begin(), node.keys.end(), middle) - node.keys.begin();
				unsigned added = make_leaf();
				auto &a = leaf(id), &b = leaf(added);
				b.count = B - cut;
				for (unsigned i = 0; i < b.count; ++i) b.keys[i] = a.keys[cut + i], b.values[i] = std::move(a.values[cut + i]);
				a.count = cut;
				if (key < middle) write<Append>(a, p, key, std::move(value));
				else write<Append>(b, p - cut, key, std::move(value));
				if (key < middle) pull(b);
				else pull(a);
				replacement = make_node(middle, id, added);
			}
			points += !exists;
		}
		if (depth) {
			auto parent = path[depth - 1];
			if (key < nodes[parent].middle) nodes[parent].left = replacement;
			else nodes[parent].right = replacement;
		} else root = replacement;
		while (depth) {
			unsigned k = path[--depth];
			nodes[k].product = op(product(nodes[k].left), product(nodes[k].right));
		}
	}
	S prefix(unsigned id, U r) const {
		S result = e();
		while (!(id & tag)) {
			if (r <= nodes[id].middle) id = nodes[id].left;
			else { result = op(result, product(nodes[id].left)); id = nodes[id].right; }
		}
		const auto &node = leaf(id);
		for (unsigned i = 0; i < node.count && node.keys[i] < r; ++i) result = op(result, node.values[i]);
		return result;
	}
	S suffix(unsigned id, U l) const {
		S result = e();
		while (!(id & tag)) {
			if (nodes[id].middle <= l) id = nodes[id].right;
			else { result = op(product(nodes[id].right), result); id = nodes[id].left; }
		}
		const auto &node = leaf(id);
		for (unsigned i = node.count; i && l <= node.keys[i - 1];) result = op(node.values[--i], result);
		return result;
	}
	template <class Pred>
	bool search_right(unsigned id, U lower, U l, S &value, Pred &f, Coord &answer) const {
		if (l <= lower) {
			S next = op(value, product(id));
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, l) - node.keys.begin();
			for (; p < node.count; ++p) {
				S next = op(value, node.values[p]);
				if (!f(next)) { answer = decode(node.keys[p]); return true; }
				value = std::move(next);
			}
			return false;
		}
		U middle = nodes[id].middle;
		if (l < middle && search_right(nodes[id].left, lower, l, value, f, answer)) return true;
		return search_right(nodes[id].right, middle, l, value, f, answer);
	}
	template <class Pred>
	bool search_left(unsigned id, U upper, U r, S &value, Pred &f, Coord &answer) const {
		if (upper <= r) {
			S next = op(product(id), value);
			if (f(next)) { value = std::move(next); return false; }
		}
		if (id & tag) {
			const auto &node = leaf(id);
			unsigned p = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, r) - node.keys.begin();
			while (p) {
				S next = op(node.values[--p], value);
				if (!f(next)) { answer = decode(node.keys[p]) + 1; return true; }
				value = std::move(next);
			}
			return false;
		}
		U middle = nodes[id].middle;
		if (middle < r && search_left(nodes[id].right, upper, r, value, f, answer)) return true;
		return search_left(nodes[id].left, middle, r, value, f, answer);
	}

  public:
	using value_type = S;
	using coordinate_type = Coord;
	explicit DynamicSegtree(Coord n = 0) : DynamicSegtree(0, n) {}
	DynamicSegtree(Coord l, Coord r) : first(l), last(r) { assert(l <= r); }
	DynamicSegtree(const DynamicSegtree &) = default;
	DynamicSegtree &operator=(const DynamicSegtree &) = default;
	DynamicSegtree(DynamicSegtree &&other) : DynamicSegtree(other.first, other.last) { swap(other); }
	DynamicSegtree &operator=(DynamicSegtree &&other) {
		if (this != &other) { DynamicSegtree moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(DynamicSegtree &other) noexcept {
		leaves.swap(other.leaves);
		nodes.swap(other.nodes);
		std::swap(root, other.root);
		std::swap(points, other.points);
		std::swap(first, other.first);
		std::swap(last, other.last);
	}
	void reserve(std::size_t n) {
		assert(n <= limit);
		std::size_t capacity = (3 * n + 2 * B - 1) / (2 * B);
		leaves.reserve(capacity);
		nodes.reserve(capacity + 1);
	}
	std::size_t node_count() const { return points; }
	std::size_t memory_usage() const { return leaves.capacity() * sizeof(Leaf) + nodes.capacity() * sizeof(Node); }
	U size() const { return U(last) - U(first); }
	Coord left_bound() const { return first; }
	Coord right_bound() const { return last; }
	void clear() { leaves.clear(); nodes.erase(nodes.begin() + 1, nodes.end()); root = points = 0; }
	void set(Coord p, S value) { assert(first <= p && p < last); update<false>(encode(p), std::move(value)); }
	void multiply(Coord p, S value) { assert(first <= p && p < last); update<true>(encode(p), std::move(value)); }
	S get(Coord p) const {
		assert(first <= p && p < last);
		if (!root) return e();
		U key = encode(p);
		unsigned id = root;
		while (!(id & tag)) id = key < nodes[id].middle ? nodes[id].left : nodes[id].right;
		const auto &node = leaf(id);
		unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, key) - node.keys.begin();
		return i < node.count && node.keys[i] == key ? node.values[i] : e();
	}
	S prod(Coord l, Coord r) const {
		assert(first <= l && l <= r && r <= last);
		if (!root || l == r) return e();
		if (l == first && r == last) return product(root);
		U a = encode(l), b = encode(r);
		unsigned id = root;
		while (!(id & tag)) {
			U middle = nodes[id].middle;
			if (b <= middle) id = nodes[id].left;
			else if (middle <= a) id = nodes[id].right;
			else return op(suffix(nodes[id].left, a), prefix(nodes[id].right, b));
		}
		const auto &node = leaf(id);
		unsigned i = std::lower_bound(node.keys.begin(), node.keys.begin() + node.count, a) - node.keys.begin();
		S result = e();
		for (; i < node.count && node.keys[i] < b; ++i) result = op(result, node.values[i]);
		return result;
	}
	S all_prod() const { return root ? product(root) : e(); }
	template <class Pred>
	Coord max_right(Coord l, Pred f) const {
		assert(first <= l && l <= last && f(e()));
		Coord answer = last;
		if (root && l != last) { S value = e(); search_right(root, encode(first), encode(l), value, f, answer); }
		return answer;
	}
	template <class Pred>
	Coord min_left(Coord r, Pred f) const {
		assert(first <= r && r <= last && f(e()));
		Coord answer = first;
		if (root && r != first) { S value = e(); search_left(root, encode(last), encode(r), value, f, answer); }
		return answer;
	}
};

#line 1 "data_structure/advanced/hld.hpp"



#line 11 "data_structure/advanced/hld.hpp"

class HLD {
	int shape_ = 0, center_ = 0;
	struct Node { int position, head, parent, offset; };
	std::vector<Node> nodes;
	void check([[maybe_unused]] int v) const { assert(0 <= v && v < n && int(in.size()) == n); }

  public:
	struct Segment {
		int l, r;
		bool reversed;
	};
	int n;
	std::vector<std::pair<int, int>> edges;
	std::vector<int> parent, depth, size, heavy, head, in, out, order;
	explicit HLD(int n = 0) : n(n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		edges.reserve(std::max(0, n - 1));
	}
	int add_edge(int u, int v) {
		assert(0 <= u && u < n && 0 <= v && v < n);
		edges.emplace_back(u, v);
		return int(edges.size()) - 1;
	}
	void build(int root = 0) {
		assert(n >= 0 && edges.size() == std::size_t(std::max(0, n - 1)));
		assert(!n || (0 <= root && root < n));
		parent.assign(n, -1);
		depth.assign(n, 0);
		size.assign(n, 1);
		heavy.assign(n, -1);
		head.resize(n), in.resize(n), out.resize(n), order.resize(n);
		nodes.resize(n);
		if (!n) return;
		std::vector<int> degree(n), neighbors(n);
		for (auto [u, v] : edges) {
			++degree[u], ++degree[v];
			neighbors[u] ^= v, neighbors[v] ^= u;
		}
		center_ = int(std::max_element(degree.begin(), degree.end()) - degree.begin());
		shape_ = degree[center_] <= 2 ? 1 : degree[center_] == n - 1 ? 2 : 0;
		degree[root] += 2;
		int count = 0;
		for (int v = 0; v < n; ++v) if (degree[v] == 1) order[count++] = v;
		for (int i = 0; i < count; ++i) {
			int v = order[i], p = neighbors[v];
			parent[v] = p;
			size[p] += size[v];
			if (heavy[p] == -1 || size[heavy[p]] < size[v]) heavy[p] = v;
			neighbors[p] ^= v;
			if (--degree[p] == 1) order[count++] = p;
		}
		assert(count == n - 1);
		in[root] = 0, out[root] = n, head[root] = root;
		for (int i = n - 2; i >= 0; --i) {
			int v = order[i], p = parent[v];
			depth[v] = depth[p] + 1;
			if (heavy[p] == v) in[v] = in[p] + 1, head[v] = head[p];
			else in[v] = out[p] -= size[v], head[v] = v;
			out[v] = in[v] + size[v];
		}
		for (int v = 0; v < n; ++v) out[v] = in[v] + size[v], order[in[v]] = v;
		parent[root] = -1;
		for (int v = 0; v < n; ++v) nodes[v] = {in[v], in[head[v]], parent[head[v]], depth[v] - in[v]};
	}
	int lca(int u, int v) const {
		check(u), check(v);
		if (shape_ == 1) return head[u] != head[v] ? order[0] : in[u] < in[v] ? u : v;
		if (shape_ == 2) return u == v ? u : u == order[0] || v == order[0] ? order[0] : center_;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) u = nodes[u].parent;
			else v = nodes[v].parent;
		}
		return nodes[u].position < nodes[v].position ? u : v;
	}
	int dist(int u, int v) const {
		int w = lca(u, v);
		return depth[u] + depth[v] - 2 * depth[w];
	}
	bool is_ancestor(int u, int v) const {
		check(u), check(v);
		return in[u] <= in[v] && in[v] < out[u];
	}
	int ancestor(int v, int k) const {
		check(v);
		if (shape_) {
			if (k < 0 || k > depth[v]) return -1;
			if (!k) return v;
			if (k == depth[v]) return order[0];
			return shape_ == 2 ? parent[v] : order[in[v] - k];
		}
		if (k < 0) return -1;
		int target = nodes[v].position + nodes[v].offset - k;
		if (target < 0) return -1;
		while (nodes[v].head + nodes[v].offset > target) v = nodes[v].parent;
		return order[target - nodes[v].offset];
	}
	int jump(int u, int v, int k) const {
		check(u), check(v);
		if (k < 0) return -1;
		if (!k) return u;
		if (shape_ == 1) {
			if (head[u] == head[v]) {
				int delta = in[v] - in[u];
				if (k > (delta < 0 ? -delta : delta)) return -1;
				return order[in[u] + (delta < 0 ? -k : k)];
			}
			int a = depth[u], b = depth[v];
			if (k > a + b) return -1;
			if (k == a) return order[0];
			return k < a ? order[in[u] - k] : order[in[v] - (a + b - k)];
		}
		if (shape_ == 2) {
			int d = (u != center_) + (v != center_);
			if (u == v || k > d) return -1;
			return k == d ? v : center_;
		}
		int from = u, to = v, target = nodes[u].position + nodes[u].offset - k;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				if (nodes[u].head + nodes[u].offset <= target) return order[target - nodes[u].offset];
				u = nodes[u].parent;
			} else v = nodes[v].parent;
		}
		int w = nodes[u].position < nodes[v].position ? u : v;
		int d = nodes[w].position + nodes[w].offset;
		int length = nodes[from].position + nodes[from].offset + nodes[to].position + nodes[to].offset - 2 * d;
		if (k > length) return -1;
		if (target >= d) return order[target - nodes[u].offset];
		return ancestor(to, length - k);
	}
	std::pair<int, int> subtree(int v, bool edge = false) const {
		check(v);
		return {in[v] + edge, out[v]};
	}
	int edge_vertex(int id) const {
		assert(0 <= id && id < int(edges.size()));
		auto [u, v] = edges[id];
		check(u);
		return parent[u] == v ? u : v;
	}
	template <class F>
	void for_each_path(int u, int v, F &&f, bool edge = false) const {
		check(u), check(v);
		if (shape_ == 1) {
			int x = in[u], y = in[v];
			if (head[u] == head[v]) {
				int l = std::min(x, y) + edge, r = std::max(x, y) + 1;
				if (l < r) f(l, r, x > y);
			} else if (head[u] == order[0]) {
				if (edge < x + 1) f(int(edge), x + 1, true);
				f(in[head[v]], y + 1, false);
			} else {
				f(in[head[u]], x + 1, true);
				if (edge < y + 1) f(int(edge), y + 1, false);
			}
			return;
		}
		std::array<std::array<int, 2>, 32> right;
		int count = 0;
		while (nodes[u].head != nodes[v].head) {
			if (nodes[u].position > nodes[v].position) {
				f(nodes[u].head, nodes[u].position + 1, true);
				u = nodes[u].parent;
			} else {
				assert(count < int(right.size()));
				right[count++] = {nodes[v].head, nodes[v].position + 1};
				v = nodes[v].parent;
			}
		}
		bool reversed = nodes[u].position > nodes[v].position;
		int l = std::min(nodes[u].position, nodes[v].position) + edge, r = std::max(nodes[u].position, nodes[v].position) + 1;
		if (l < r) f(l, r, reversed);
		while (count) {
			auto [a, b] = right[--count];
			f(a, b, false);
		}
	}
	std::vector<Segment> path(int u, int v, bool edge = false) const {
		std::vector<Segment> result;
		for_each_path(u, v, [&](int l, int r, bool reversed) { result.push_back({l, r, reversed}); }, edge);
		return result;
	}
};


#line 4 "test/hld_subtree_sum.test.cpp"
#include <iostream>
#line 6 "test/hld_subtree_sum.test.cpp"

long long op(long long a, long long b) { return a + b; }
long long e() { return 0; }

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	int n, q;
	std::cin >> n >> q;
	std::vector<long long> a(n);
	for (auto &x : a) std::cin >> x;
	HLD tree(n);
	for (int v = 1, p; v < n; ++v) {
		std::cin >> p;
		tree.add_edge(p, v);
	}
	tree.build();
	DynamicSegtree<long long, op, e> seg(n);
	seg.reserve(n);
	for (int v = 0; v < n; ++v) seg.set(tree.in[v], a[v]);
	while (q--) {
		int type, v;
		std::cin >> type >> v;
		if (type == 0) {
			long long x;
			std::cin >> x;
			seg.multiply(tree.in[v], x);
		} else {
			auto [l, r] = tree.subtree(v);
			std::cout << seg.prod(l, r) << '\n';
		}
	}
}
