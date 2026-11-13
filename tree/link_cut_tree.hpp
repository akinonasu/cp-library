#ifndef LINK_CUT_TREE_HPP
#define LINK_CUT_TREE_HPP

#include "../components/link_cut_tree_core.hpp"

template <class S, auto op, auto e, bool commutative = false>
class LinkCutTree : public link_cut_tree_detail::Tree<S, op, e, commutative, false> {
	using Base = link_cut_tree_detail::Tree<S, op, e, commutative, false>;

  public:
	explicit LinkCutTree(int n = 0) : Base(n) {}
	explicit LinkCutTree(const std::vector<S> &values) : Base(values) {}
};

#endif
