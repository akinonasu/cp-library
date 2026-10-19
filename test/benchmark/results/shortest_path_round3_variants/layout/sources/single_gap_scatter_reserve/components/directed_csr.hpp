#ifndef DIRECTED_CSR_HPP
#define DIRECTED_CSR_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <stdexcept>
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
	int single_gap_ = -2;
	void finish(std::span<const Edge> edges) {
		const auto n = offset_.size() - 1;
		single_gap_ = -2;
		if (edges.size() >= n - (n != 0) && edges.size() <= n && std::all_of(offset_.begin(), offset_.end() - 1, [](std::size_t x) { return x <= 1; })) {
			single_gap_ = -1;
			if (edges.size() != n) single_gap_ = int(std::find(offset_.begin(), offset_.end() - 1, 0) - offset_.begin());
		}
		if (single_gap_ >= -1) {
			for (std::size_t v = 0; v <= n; ++v) offset_[v] = v - (single_gap_ >= 0 && v > std::size_t(single_gap_));
			arcs_.resize(edges.size());
			for (const auto &edge : edges) arcs_[edge.from - (single_gap_ >= 0 && edge.from > single_gap_)] = {edge.to, edge.weight};
			return;
		}
		for (std::size_t v = 1; v < offset_.size(); ++v) offset_[v] += offset_[v - 1];
		arcs_.resize(edges.size());
		for (std::size_t i = edges.size(); i; --i) {
			const auto &edge = edges[i - 1];
			arcs_[--offset_[edge.from]] = {edge.to, edge.weight};
		}
	}
	void clear_edges() {
		single_gap_ = -2;
		std::fill(offset_.begin(), offset_.end(), 0);
		arcs_.clear();
	}

  public:
	struct SingleEdgeView {
		std::span<const Arc> arcs;
		int n, gap;
		int size() const { return n; }
		std::span<const Arc> operator[](int v) const {
			if (v == gap) return {};
			return arcs.subspan(v - (gap >= 0 && v > gap), 1);
		}
	};
	bool has_single_edge_layout() const { return single_gap_ >= -1; }
	SingleEdgeView single_edge_view() const { return {arcs_, size(), single_gap_}; }
	explicit DirectedCSR(int n = 0) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
	}
	DirectedCSR(int n, std::span<const Edge> edges) { build(n, edges); }
	template<class Producer>
	DirectedCSR(int n, std::size_t m, Producer &&produce) { build(n, m, std::forward<Producer>(produce)); }
	void build(int n, std::span<const Edge> edges) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		for (const auto &edge : edges) {
			assert(0 <= edge.from && edge.from < n && 0 <= edge.to && edge.to < n);
			++offset_[edge.from];
		}
		try { finish(edges); }
		catch (...) { clear_edges(); throw; }
	}
	template<class Producer>
	void build(int n, std::size_t m, Producer &&produce) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		try {
			auto edges = std::make_unique_for_overwrite<Edge[]>(m);
			std::size_t count = 0;
			auto emit = [&](Edge edge) {
				if (count == m) throw std::length_error("DirectedCSR edge count");
				assert(0 <= edge.from && edge.from < n && 0 <= edge.to && edge.to < n);
				++offset_[edge.from];
				edges[count++] = std::move(edge);
			};
			std::forward<Producer>(produce)(emit);
			if (count != m) throw std::length_error("DirectedCSR edge count");
			finish(std::span<const Edge>(edges.get(), m));
		} catch (...) { clear_edges(); throw; }
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
