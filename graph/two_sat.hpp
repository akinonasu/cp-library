#ifndef TWO_SAT_HPP
#define TWO_SAT_HPP

#include "../components/scc_core.hpp"

class TwoSAT {
	int n_;
	std::vector<std::pair<int, int>> edges_;
	std::vector<bool> answer_;
	bool valid_ = false;
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), answer_(vertices(n) / 2) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		edges_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = false;
		if (i == j && f != g) return;
		assert(edges_.size() <= std::size_t(std::numeric_limits<int>::max() - 1 - int(i != j)));
		edges_.emplace_back(2 * i + !f, 2 * j + g);
		if (i != j) edges_.emplace_back(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		auto [count, id] = scc_detail::ids<true>(2 * n_, edges_);
		valid_ = false;
		if (count < 0) return false;
		for (int i = 0; i < n_; ++i) {
			if (id[2 * i] == id[2 * i + 1]) return false;
			answer_[i] = id[2 * i] < id[2 * i + 1];
		}
		return valid_ = true;
	}
	std::vector<bool> answer() const { assert(valid_); return answer_; }
};

#endif
