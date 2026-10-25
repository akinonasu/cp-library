#ifndef AHO_CORASICK_HPP
#define AHO_CORASICK_HPP

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>
#include <vector>

#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
#include <immintrin.h>
namespace aho_corasick_detail {
template<int Alphabet>
__attribute__((target("avx2"))) inline std::uint32_t complete(int *edges, const int *fallback) {
	std::uint32_t mask = 0;
	int i = 0;
	for (; i + 8 <= Alphabet; i += 8) {
		auto children = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(edges + i));
		auto inherited = _mm256_loadu_si256(reinterpret_cast<const __m256i *>(fallback + i));
		auto missing = _mm256_cmpeq_epi32(children, _mm256_setzero_si256());
		mask |= (std::uint32_t(~_mm256_movemask_ps(_mm256_castsi256_ps(missing))) & 255) << i;
		_mm256_storeu_si256(reinterpret_cast<__m256i *>(edges + i), _mm256_blendv_epi8(children, inherited, missing));
	}
	for (; i < Alphabet; ++i) {
		if (edges[i]) mask |= std::uint32_t(1) << i;
		else edges[i] = fallback[i];
	}
	return mask;
}
}
#endif

template <int Alphabet = 26, int First = 'a'>
class AhoCorasick {
	static_assert(0 < Alphabet && 0 <= First && First + Alphabet <= 256);
	struct alignas(Alphabet >= 16 ? 64 : alignof(int)) Node {
		std::array<int, Alphabet> next{};
		int matches = 0;
		int parent = 0, depth = 0, link = 0, output = -1, head = -1;
	};
	std::vector<Node> nodes_;
	std::vector<int> terminal_, next_pattern_, order_;
	bool built_ = false;
	static unsigned symbol_(char c) { return static_cast<unsigned>(static_cast<unsigned char>(c) - First); }
	void check_([[maybe_unused]] int v) const { assert(0 <= v && v < node_count()); }
	void link_(int u, int f) {
		nodes_[u].link = f;
		nodes_[u].output = nodes_[f].head != -1 ? f : nodes_[f].output;
		nodes_[u].matches += nodes_[f].matches;
		order_.push_back(u);
	}

  public:
	static constexpr int root = 0;
	AhoCorasick() { nodes_.emplace_back(); }
	void clear() {
		nodes_.clear(), nodes_.emplace_back();
		terminal_.clear(), next_pattern_.clear(), order_.clear();
		built_ = false;
	}
	void reserve(int total_length, int patterns = 0) {
		assert(0 <= total_length && total_length < std::numeric_limits<int>::max() && 0 <= patterns);
		nodes_.reserve(std::size_t(total_length) + 1);
		terminal_.reserve(patterns), next_pattern_.reserve(patterns);
	}
	int add(std::string_view pattern) {
		assert(!built_ && size() < std::numeric_limits<int>::max());
		int v = root;
		for (char c : pattern) {
			unsigned x = symbol_(c);
			assert(x < Alphabet);
			int u = nodes_[v].next[x];
			if (!u) {
				assert(node_count() < std::numeric_limits<int>::max());
				u = node_count();
				nodes_[v].next[x] = u;
				nodes_.emplace_back();
				nodes_[u].parent = v, nodes_[u].depth = nodes_[v].depth + 1;
			}
			v = u;
		}
		int id = size();
		terminal_.push_back(v), next_pattern_.push_back(nodes_[v].head);
		nodes_[v].head = id, ++nodes_[v].matches;
		return id;
	}
	void build() {
		if (built_) return;
		order_.reserve(nodes_.size()), order_.push_back(root);
		for (std::size_t i = 0; i < order_.size(); ++i) {
			int v = order_[i];
			auto &node = nodes_[v];
			const auto &fallback = nodes_[node.link].next;
#if defined(__x86_64__) && (defined(__GNUC__) || defined(__clang__))
			if constexpr (8 <= Alphabet && Alphabet <= 32) {
				if (__builtin_cpu_supports("avx2")) {
					auto mask = aho_corasick_detail::complete<Alphabet>(node.next.data(), fallback.data());
					while (mask) {
						int c = std::countr_zero(mask), u = node.next[c], f = v == root ? root : fallback[c];
						mask &= mask - 1;
						link_(u, f);
					}
					continue;
				}
			}
#endif
			for (int c = 0; c < Alphabet; ++c) {
				if (int u = node.next[c]) {
					link_(u, v == root ? root : fallback[c]);
				} else {
					node.next[c] = fallback[c];
				}
			}
		}
		built_ = true;
	}
	int size() const { return static_cast<int>(terminal_.size()); }
	bool empty() const { return terminal_.empty(); }
	int node_count() const { return static_cast<int>(nodes_.size()); }
	bool built() const { return built_; }
	int terminal(int id) const {
		assert(0 <= id && id < size());
		return terminal_[id];
	}
	int length(int id) const { return nodes_[terminal(id)].depth; }
	int parent(int v) const { check_(v); return nodes_[v].parent; }
	int depth(int v) const { check_(v); return nodes_[v].depth; }
	int child(int v, char c) const {
		check_(v);
		unsigned x = symbol_(c);
		if (x >= Alphabet) return -1;
		int u = nodes_[v].next[x];
		return u && nodes_[u].parent == v ? u : -1;
	}
	int suffix_link(int v) const { check_(v); assert(built_); return nodes_[v].link; }
	int output_link(int v) const { check_(v); assert(built_); return nodes_[v].output; }
	int terminal_count(int v) const {
		check_(v);
		return built_ && v ? nodes_[v].matches - nodes_[nodes_[v].link].matches : nodes_[v].matches;
	}
	int match_count(int v) const { check_(v); assert(built_); return nodes_[v].matches; }
	int transition(int v, char c) const {
		check_(v), assert(built_);
		unsigned x = symbol_(c);
		return x < Alphabet ? nodes_[v].next[x] : root;
	}
	int walk(int v, std::string_view text) const {
		check_(v), assert(built_);
		for (char c : text) v = transition(v, c);
		return v;
	}
	const std::vector<int> &bfs_order() const { assert(built_); return order_; }
	template <class F>
	void for_each_match(int v, F &&visit) const {
		check_(v), assert(built_);
		if (nodes_[v].head == -1) v = nodes_[v].output;
		for (; v != -1; v = nodes_[v].output)
			for (int id = nodes_[v].head; id != -1; id = next_pattern_[id]) visit(id);
	}
	template <class F>
	void find_all(std::string_view text, F &&visit) const {
		assert(built_);
		int v = root;
		for_each_match(v, [&](int id) { visit(std::size_t(0), id); });
		for (std::size_t i = 0; i < text.size(); ++i) {
			v = transition(v, text[i]);
			for_each_match(v, [&](int id) { visit(i + 1, id); });
		}
	}
	std::uint64_t count_matches(std::string_view text) const {
		assert(built_);
		int v = root;
		std::uint64_t count = nodes_[root].matches;
		for (char c : text) {
			v = transition(v, c);
			count += nodes_[v].matches;
		}
		return count;
	}
	template <class T>
	void propagate(std::vector<T> &values) const {
		assert(built_ && values.size() == nodes_.size());
		for (std::size_t i = order_.size(); i-- > 1;) {
			int v = order_[i];
			values[nodes_[v].link] += values[v];
		}
	}
	std::vector<std::uint64_t> occurrences(std::string_view text) const {
		assert(built_);
		std::vector<std::uint64_t> hits(nodes_.size());
		int v = root;
		++hits[root];
		for (char c : text) v = transition(v, c), ++hits[v];
		propagate(hits);
		std::vector<std::uint64_t> count(size());
		for (int id = 0; id < size(); ++id) count[id] = hits[terminal_[id]];
		return count;
	}
};

#endif
