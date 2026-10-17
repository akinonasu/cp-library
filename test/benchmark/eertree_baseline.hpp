#ifndef EERTREE_BASELINE_HPP
#define EERTREE_BASELINE_HPP

#include <array>
#include <string_view>
#include <vector>

class DenseEertree {
	struct Node {
		std::array<int, 26> next{};
		int length = 0, link = 0, parent = 0, first_end = 0, suffix_count = 0, hits = 0;
	};
	std::vector<Node> nodes_;
	std::vector<int> text_, prefix_;
	int last_ = 1, longest_ = 1;
	std::uint64_t total_ = 0;
	int extendable_(int v, int position, int c) const {
		while (text_[position - nodes_[v].length - 1] != c) v = nodes_[v].link;
		return v;
	}

  public:
	explicit DenseEertree(std::string_view s) {
		nodes_.reserve(s.size() + 2), text_.reserve(s.size() + 1), prefix_.reserve(s.size());
		nodes_.resize(2), nodes_[0].length = -1, text_.push_back(-1);
		for (char symbol : s) {
			int c = symbol - 'a', position = int(text_.size());
			text_.push_back(c);
			int parent = extendable_(last_, position, c), v = nodes_[parent].next[c];
			if (!v) {
				int link = parent == 0 ? 1 : nodes_[extendable_(nodes_[parent].link, position, c)].next[c];
				v = int(nodes_.size());
				nodes_.push_back({{}, nodes_[parent].length + 2, link, parent, position, nodes_[link].suffix_count + 1, 0});
				nodes_[parent].next[c] = v;
				if (nodes_[v].length > nodes_[longest_].length) longest_ = v;
			}
			last_ = v, ++nodes_[v].hits, total_ += nodes_[v].suffix_count;
			prefix_.push_back(v);
		}
	}
	int size() const { return int(prefix_.size()); }
	int node_count() const { return int(nodes_.size()); }
	int distinct_count() const { return node_count() - 2; }
	int parent(int v) const { return nodes_[v].parent; }
	int suffix_link(int v) const { return nodes_[v].link; }
	const std::vector<int> &prefix_nodes() const { return prefix_; }
};

#endif
