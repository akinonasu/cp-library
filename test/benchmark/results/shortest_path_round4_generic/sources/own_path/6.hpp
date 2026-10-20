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

namespace directed_csr_detail { struct Access; }

template<class T>
class DirectedCSR {
  public:
	struct Edge { int from, to; T weight; };
	using Arc = std::pair<int, T>;
	using weight_type = T;
	using distance_type = std::conditional_t<std::is_integral_v<T> && sizeof(T) < 8, std::int64_t, T>;

  private:
	friend struct directed_csr_detail::Access;
	std::vector<std::size_t> offset_;
	std::vector<Arc> arcs_;
	int single_gap_ = -2;
	bool unique_incoming_ = true;
	void count_edge(const Edge &edge) {
		assert(0 <= edge.from && edge.from < size() && 0 <= edge.to && edge.to < size());
		++offset_[edge.from];
	}
	void check_incoming(std::span<const Edge> edges) {
		if (!unique_incoming_) return;
		std::vector<std::uint64_t> seen((std::size_t(size()) + 63) / 64);
		for (const auto &edge : edges) {
			auto mask = std::uint64_t(1) << (edge.to % 64);
			auto &word = seen[edge.to / 64];
			if (word & mask) { unique_incoming_ = false; return; }
			word |= mask;
		}
	}
	void finish(std::span<const Edge> edges) {
		check_incoming(edges);
		const auto n = offset_.size() - 1;
		single_gap_ = -2;
		if (edges.size() >= n - (n != 0) && edges.size() <= n &&
			std::all_of(offset_.begin(), offset_.end() - 1, [](std::size_t x) { return x <= 1; })) {
			single_gap_ = -1;
			if (edges.size() != n) single_gap_ = int(std::find(offset_.begin(), offset_.end() - 1, 0) - offset_.begin());
			for (std::size_t v = 0; v <= n; ++v) offset_[v] = v - (single_gap_ >= 0 && v > std::size_t(single_gap_));
			arcs_.resize(edges.size());
			for (const auto &edge : edges) arcs_[edge.from - (single_gap_ >= 0 && edge.from > single_gap_)] = {edge.to, edge.weight};
			return;
		}
		for (std::size_t v = 1; v < offset_.size(); ++v) offset_[v] += offset_[v - 1];
		arcs_.resize(edges.size());
		std::size_t i = edges.size();
#if defined(__GNUC__) || defined(__clang__)
		if (n >= 8192) {
			for (; i > 32; --i) {
				const auto from = edges[i - 33].from;
				__builtin_prefetch(&offset_[from], 1, 3);
				__builtin_prefetch(&arcs_[offset_[from] - 1], 1, 3);
				const auto &edge = edges[i - 1];
				arcs_[--offset_[edge.from]] = {edge.to, edge.weight};
			}
		}
#endif
		for (; i; --i) {
			const auto &edge = edges[i - 1];
			arcs_[--offset_[edge.from]] = {edge.to, edge.weight};
		}
	}
	void clear_edges() {
		single_gap_ = -2;
		unique_incoming_ = true;
		std::fill(offset_.begin(), offset_.end(), 0);
		arcs_.clear();
	}

  public:
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
		single_gap_ = -2;
		unique_incoming_ = edges.size() <= std::size_t(n);
		try {
			for (const auto &edge : edges) count_edge(edge);
			finish(edges);
		}
		catch (...) { clear_edges(); throw; }
	}
	template<class Producer>
	void build(int n, std::size_t m, Producer &&produce) {
		assert(n >= 0);
		offset_.assign(std::size_t(n) + 1, 0);
		single_gap_ = -2;
		unique_incoming_ = m <= std::size_t(n);
		try {
			auto edges = std::make_unique_for_overwrite<Edge[]>(m);
			std::size_t count = 0;
			auto emit = [&](Edge edge) {
				if (count == m) throw std::length_error("DirectedCSR edge count");
				count_edge(edge);
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

namespace directed_csr_detail {
template<class T>
struct SingleEdgeView {
	std::span<const typename DirectedCSR<T>::Arc> arcs;
	int n, gap;
	int size() const { return n; }
	std::span<const typename DirectedCSR<T>::Arc> operator[](int v) const {
		if (v == gap) return {};
		return arcs.subspan(v - (gap >= 0 && v > gap), 1);
	}
};

struct Access {
	template<class T>
	static bool unique_incoming(const DirectedCSR<T> &g) { return g.offset_.empty() || g.unique_incoming_; }
	template<class T>
	static bool has_single_edge_layout(const DirectedCSR<T> &g) { return !g.offset_.empty() && g.single_gap_ >= -1; }
	template<class T>
	static SingleEdgeView<T> single_edge_view(const DirectedCSR<T> &g) { return {g.arcs_, g.size(), g.single_gap_}; }
};
}

#endif
