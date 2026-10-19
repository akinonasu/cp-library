#ifndef DIRECTED_CSR_HPP
#define DIRECTED_CSR_HPP

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <span>
#include <type_traits>
#include <utility>
#include <vector>

template<class T>
class DirectedCSR {
  public:
	struct Edge { int from, to; T weight; };
	using Arc = std::pair<int, T>;
	using weight_type = T;
	using distance_type = std::conditional_t<std::is_integral_v<T> && sizeof(T) < 8, std::int64_t, T>;

  private:
	std::vector<std::size_t> offset_;
	std::vector<Arc> arcs_;

  public:
	explicit DirectedCSR(int n = 0) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
	}
	DirectedCSR(int n, std::span<const Edge> edges) { build(n, edges); }
	void build(int n, std::span<const Edge> edges) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		for (const auto &edge : edges) {
			assert(0 <= edge.from && edge.from < n && 0 <= edge.to && edge.to < n);
			++offset_[std::size_t(edge.from) + 1];
		}
		for (int v = 0; v < n; ++v) offset_[v + 1] += offset_[v];
		arcs_.resize(edges.size());
		auto next = offset_;
		for (const auto &edge : edges) arcs_[next[edge.from]++] = {edge.to, edge.weight};
	}
	int size() const { return offset_.empty() ? 0 : int(offset_.size() - 1); }
	bool empty() const { return size() == 0; }
	std::size_t edge_count() const { return arcs_.size(); }
	std::span<const Arc> operator[](int v) const {
		assert(0 <= v && v < size());
		return std::span<const Arc>(arcs_).subspan(offset_[v], offset_[v + 1] - offset_[v]);
	}
};

#endif
