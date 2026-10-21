#ifndef TWO_SAT_HPP
#define TWO_SAT_HPP

#include "connectivity/scc.hpp"

class TwoSAT {
	int n_;
	SCCGraph graph_;
	std::vector<bool> answer_;
	bool valid_ = false;
	static int vertices(int n) {
		assert(0 <= n && n <= std::numeric_limits<int>::max() / 2);
		return n * 2;
	}

  public:
	explicit TwoSAT(int n = 0) : n_(n), graph_(vertices(n)), answer_(n) {}
	int size() const { return n_; }
	void reserve(std::size_t clauses) {
		assert(clauses <= std::size_t(std::numeric_limits<int>::max() / 2));
		graph_.reserve(clauses * 2);
	}
	void add_clause(int i, bool f, int j, bool g) {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		valid_ = false;
		if (i == j && f != g) return;
		graph_.add_edge(2 * i + !f, 2 * j + g);
		if (i != j) graph_.add_edge(2 * j + !g, 2 * i + f);
	}
	bool satisfiable() {
		auto id = graph_.scc_ids().second;
		valid_ = false;
		for (int i = 0; i < n_; ++i) {
			if (id[2 * i] == id[2 * i + 1]) return false;
			answer_[i] = id[2 * i] < id[2 * i + 1];
		}
		return valid_ = true;
	}
	std::vector<bool> answer() const { assert(valid_); return answer_; }
};

#endif
