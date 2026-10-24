#ifndef TREE_PATH_PRODUCT_HPP
#define TREE_PATH_PRODUCT_HPP

#include "../data_structure/advanced/hld.hpp"
#include <bit>
#include <cstdint>
#include <type_traits>

template<class S, auto op, auto e, auto reverse = nullptr> class TreePathProduct {
	static constexpr bool has_reverse = !std::is_same_v<decltype(reverse), std::nullptr_t>;
	struct Empty {};
	struct Back { S prod, prefix; };
	using BackStorage = std::conditional_t<has_reverse, Empty, Back>;
	struct alignas(64) Node {
		int left = 0, right = 0, parent = 0, head = 0, up = 0;
		unsigned char priority = 0;
		S value, prod, prefix;
		[[no_unique_address]] BackStorage back;
		static BackStorage make_back(const S& value) {
			if constexpr (has_reverse) return {};
			else return {value, value};
		}
		explicit Node(const S& v) : value(v), prod(v), prefix(v), back(make_back(v)) {}
	};
	std::vector<Node> nodes_;
	std::vector<int> index_;
	std::vector<S> dense_;
	std::vector<int> dense_heads_, dense_up_;
	unsigned base_ = 1;
	bool flat_ = false;
	int index(int v) const {
		assert(0 <= v && v < size());
		return index_[v];
	}
	void pull(int v) {
		auto& node = nodes_[v];
		node.prefix = node.left ? op(nodes_[node.left].prod, node.value) : node.value;
		node.prod = node.right ? op(node.prefix, nodes_[node.right].prod) : node.prefix;
		if constexpr (!has_reverse) {
			node.back.prefix = node.left ? op(node.value, nodes_[node.left].back.prod) : node.value;
			node.back.prod = node.right ? op(nodes_[node.right].back.prod, node.back.prefix) : node.back.prefix;
		}
	}
	S back_prod(int v) const {
		if constexpr (has_reverse) return reverse(nodes_[v].prod);
		else return nodes_[v].back.prod;
	}
	S back_prefix(int v) const {
		if constexpr (has_reverse) return reverse(nodes_[v].prefix);
		else return nodes_[v].back.prefix;
	}
	template<bool Reverse> S dense_interval(int l, int r) const {
		unsigned a = base_ + unsigned(l - 1), b = base_ + unsigned(r);
		S left = e(), right = e();
		while (a < b) {
			if (a & 1) left = op(left, dense_[a++]);
			if (b & 1) right = op(dense_[--b], right);
			a >>= 1, b >>= 1;
		}
		S result = op(left, right);
		if constexpr (Reverse && has_reverse) return reverse(result);
		else return result;
	}
	template<bool Reverse> S prefix(int v) const {
		S result = e();
		int last = v;
		for (; v; v = nodes_[v].parent) if (v <= last) {
			if constexpr (Reverse) result = op(result, back_prefix(v));
			else result = op(nodes_[v].prefix, result);
		}
		return result;
	}
	template<bool Reverse> S interval(int l, int r) const {
		int a = l, b = r;
		S left = e(), right = e();
		while (a != b) {
			if (nodes_[a].priority < nodes_[b].priority) {
				if (l <= a) {
					const auto& node = nodes_[a];
					if constexpr (Reverse) left = op(op(back_prod(node.right), node.value), left);
					else left = op(left, op(node.value, nodes_[node.right].prod));
				}
				a = nodes_[a].parent;
			} else {
				if (b <= r) {
					if constexpr (Reverse) right = op(right, back_prefix(b));
					else right = op(nodes_[b].prefix, right);
				}
				b = nodes_[b].parent;
			}
		}
		if constexpr (Reverse) return op(right, op(nodes_[a].value, left));
		else return op(left, op(nodes_[a].value, right));
	}

	template<bool Reverse, class X, class Apply> X apply_interval(int l, int r, X value, Apply& mapping) const {
		int a = l, b = r;
		S outside = e();
		while (a != b) {
			if (nodes_[a].priority < nodes_[b].priority) {
				if (l <= a) {
					const auto& node = nodes_[a];
					if constexpr (Reverse) outside = op(op(back_prod(node.right), node.value), outside);
					else value = mapping(nodes_[node.right].prod, mapping(node.value, std::move(value)));
				}
				a = nodes_[a].parent;
			} else {
				if (b <= r) {
					if constexpr (Reverse) value = mapping(back_prefix(b), std::move(value));
					else outside = op(nodes_[b].prefix, outside);
				}
				b = nodes_[b].parent;
			}
		}
		return mapping(outside, mapping(nodes_[a].value, std::move(value)));
	}

	template<class X, class Apply> X dense_apply(int u, int v, X value, Apply& mapping) const {
		std::array<std::pair<int, int>, 4> right;
		int count = 0;
		while (dense_heads_[u] != dense_heads_[v]) {
			if (dense_heads_[u] > dense_heads_[v]) {
				value = mapping(dense_interval<true>(dense_heads_[u], u), std::move(value));
				u = dense_up_[u];
			} else {
				assert(count < int(right.size()));
				right[count++] = {dense_heads_[v], v};
				v = dense_up_[v];
			}
		}
		if (u <= v) value = mapping(dense_interval<false>(u, v), std::move(value));
		else value = mapping(dense_interval<true>(v, u), std::move(value));
		while (count) {
			auto [l, r] = right[--count];
			value = mapping(dense_interval<false>(l, r), std::move(value));
		}
		return value;
	}

  public:
	TreePathProduct() = default;
	TreePathProduct(const HLD& tree, const std::vector<S>& values) { build(tree, values); }
	void build(const HLD& tree, const std::vector<S>& values) {
		assert(values.size() == std::size_t(tree.n) && tree.order.size() == values.size());
		index_.resize(values.size());
		nodes_.clear();
		for (int v : tree.order) index_[v] = tree.in[v] + 1;
		flat_ = false;
		dense_.clear();
		if constexpr (has_reverse) {
			std::vector<unsigned char> levels(values.size());
			unsigned maximum = 0;
			for (int v : tree.order) {
				if (tree.parent[v] != -1) levels[v] = levels[tree.parent[v]] + (tree.head[v] == v);
				maximum = std::max(maximum, unsigned(levels[v]));
			}
			flat_ = maximum <= 4;
		}
		if (flat_) {
			base_ = std::bit_ceil(unsigned(size()) ? unsigned(size()) : 1U);
			dense_.assign(std::size_t(base_) * 2, e());
			dense_heads_.assign(values.size() + 1, 0), dense_up_.assign(values.size() + 1, 0);
			for (int v : tree.order) {
				int id = index_[v], h = tree.head[v];
				dense_heads_[id] = index_[h];
				dense_up_[id] = tree.parent[h] == -1 ? 0 : index_[tree.parent[h]];
				dense_[base_ + unsigned(id - 1)] = values[v];
			}
			for (unsigned i = base_ - 1; i; --i) dense_[i] = op(dense_[i * 2], dense_[i * 2 + 1]);
			return;
		}
		dense_heads_.clear(), dense_up_.clear();
		nodes_.reserve(values.size() + 1);
		nodes_.emplace_back(e());
		for (int v : tree.order) nodes_.emplace_back(values[v]);
		std::vector<int> stack, order;
		order.reserve(values.size());
		for (int v : tree.order) {
			if (tree.head[v] != v) continue;
			stack.clear();
			unsigned position = 0;
			int head = index_[v], up = tree.parent[v] == -1 ? 0 : index_[tree.parent[v]];
			for (int u = v; u != -1; u = tree.heavy[u]) {
				int id = index_[u], weight = tree.size[u] - (tree.heavy[u] == -1 ? 0 : tree.size[tree.heavy[u]]);
				auto& node = nodes_[id];
				node.head = head, node.up = up;
				node.priority = std::bit_width(position ^ (position + unsigned(weight)));
				position += unsigned(weight);
				int last = 0;
				while (!stack.empty() && nodes_[stack.back()].priority < node.priority) {
					last = stack.back();
					stack.pop_back();
				}
				node.left = last;
				if (last) nodes_[last].parent = id;
				if (!stack.empty()) nodes_[stack.back()].right = id, node.parent = stack.back();
				stack.push_back(id);
			}
			order.push_back(stack[0]);
		}
		for (std::size_t i = 0; i < order.size(); ++i) {
			const auto& node = nodes_[order[i]];
			if (node.left) order.push_back(node.left);
			if (node.right) order.push_back(node.right);
		}
		for (auto it = order.rbegin(); it != order.rend(); ++it) pull(*it);
	}
	int size() const { return int(index_.size()); }
	bool empty() const { return index_.empty(); }
	const S& get(int v) const {
		int id = index(v);
		return flat_ ? dense_[base_ + unsigned(id - 1)] : nodes_[id].value;
	}
	void set(int v, const S& value) {
		int id = index(v);
		if (flat_) {
			unsigned k = base_ + unsigned(id - 1);
			dense_[k] = value;
			for (k >>= 1; k; k >>= 1) dense_[k] = op(dense_[k * 2], dense_[k * 2 + 1]);
			return;
		}
		nodes_[id].value = value;
		for (; id; id = nodes_[id].parent) pull(id);
	}
	S prod(int u, int v) const {
		u = index(u), v = index(v);
		if (flat_) {
			auto append = [](const S& part, S value) { return op(std::move(value), part); };
			return dense_apply(u, v, e(), append);
		}
		S left = e(), right = e();
		while (nodes_[u].head != nodes_[v].head) {
			if (nodes_[u].head > nodes_[v].head) {
				left = op(left, prefix<true>(u));
				u = nodes_[u].up;
			} else {
				right = op(prefix<false>(v), right);
				v = nodes_[v].up;
			}
		}
		S middle = u <= v ? interval<false>(u, v) : interval<true>(v, u);
		return op(left, op(middle, right));
	}
	template<class X, class Apply> X apply(int u, int v, X value, Apply mapping) const {
		u = index(u), v = index(v);
		if (flat_) return dense_apply(u, v, std::move(value), mapping);
		S right = e();
		while (nodes_[u].head != nodes_[v].head) {
			if (nodes_[u].head > nodes_[v].head) {
				for (int id = u; id; id = nodes_[id].parent)
					if (id <= u) value = mapping(back_prefix(id), std::move(value));
				u = nodes_[u].up;
			} else {
				right = op(prefix<false>(v), right);
				v = nodes_[v].up;
			}
		}
		if (u <= v) value = apply_interval<false>(u, v, std::move(value), mapping);
		else value = apply_interval<true>(v, u, std::move(value), mapping);
		return mapping(right, std::move(value));
	}

};

#endif
