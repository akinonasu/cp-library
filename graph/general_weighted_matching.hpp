#ifndef GENERAL_WEIGHTED_MATCHING_HPP
#define GENERAL_WEIGHTED_MATCHING_HPP

#include "../components/weighted_blossom.hpp"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

class GeneralWeightedMatching {
	int n;
	std::vector<std::int64_t> weights;
	std::int64_t maximum = 0;
	__int128_t total = 0;
	int cardinality = 0;
	bool built = true;
	static int checked_size(int n) { assert(0 <= n && n <= (std::numeric_limits<int>::max() - 1) / 2); return n; }
	void vertex([[maybe_unused]] int v) const { assert(0 <= v && v < n); }
public:
	using weight_type = std::int64_t;
	using total_type = __int128_t;
	std::vector<int> mate;
	explicit GeneralWeightedMatching(int n = 0) : n(checked_size(n)), weights(std::size_t(this->n) * this->n), mate(this->n, -1) {}
	GeneralWeightedMatching(const GeneralWeightedMatching &) = default;
	GeneralWeightedMatching &operator=(const GeneralWeightedMatching &) = default;
	GeneralWeightedMatching(GeneralWeightedMatching &&other) noexcept : GeneralWeightedMatching() { swap(other); }
	GeneralWeightedMatching &operator=(GeneralWeightedMatching &&other) noexcept {
		if (this != &other) { GeneralWeightedMatching moved(std::move(other)); swap(moved); }
		return *this;
	}
	int size() const { return n; }
	void add_edge(int u, int v, weight_type weight) {
		vertex(u); vertex(v);
		if (u == v || weight <= 0) return;
		auto &current = weights[std::size_t(u) * n + v];
		if (weight <= current) return;
		current = weights[std::size_t(v) * n + u] = weight;
		maximum = std::max(maximum, weight); built = false;
	}
	total_type build() {
		if (built) return total;
		std::int64_t bound = 4 * (std::int64_t(n) + 1);
		if (maximum <= std::numeric_limits<int>::max() / bound)
			weighted_matching_detail::Blossom<int>(n, weights, int(maximum)).solve(mate);
		else if (maximum <= std::numeric_limits<std::int64_t>::max() / bound)
			weighted_matching_detail::Blossom<std::int64_t>(n, weights, maximum).solve(mate);
		else weighted_matching_detail::Blossom<__int128_t>(n, weights, maximum).solve(mate);
		total = 0; cardinality = 0;
		for (int u = 0; u < n; ++u) if (u < mate[u]) {
			total += weights[std::size_t(u) * n + mate[u]]; ++cardinality;
		}
		built = true; return total;
	}
	total_type weight() const { assert(built); return total; }
	int matching_size() const { assert(built); return cardinality; }
	std::vector<std::pair<int, int>> edges() const {
		assert(built);
		std::vector<std::pair<int, int>> result; result.reserve(cardinality);
		for (int u = 0; u < n; ++u) if (u < mate[u]) result.emplace_back(u, mate[u]);
		return result;
	}
	void clear() {
		std::fill(weights.begin(), weights.end(), 0); std::fill(mate.begin(), mate.end(), -1);
		maximum = 0; total = 0; cardinality = 0; built = true;
	}
	void swap(GeneralWeightedMatching &other) noexcept {
		using std::swap;
		swap(n, other.n); weights.swap(other.weights); swap(maximum, other.maximum);
		swap(total, other.total); swap(cardinality, other.cardinality); swap(built, other.built); mate.swap(other.mate);
	}
};

#endif
