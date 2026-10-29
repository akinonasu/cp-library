#ifndef BASIC_SUBSTRING_STRUCTURE_HPP
#define BASIC_SUBSTRING_STRUCTURE_HPP

#include "../components/suffix_locus.hpp"
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

class BasicSubstringStructure {
  public:
	struct Position {
		int row, column;
		friend bool operator==(const Position &, const Position &) = default;
	};
	struct Block {
		int left, right, row_begin, row_end, column_begin, column_end, occurrences;
	};

  private:
	std::string text_;
	SuffixLocusIndex index_;
	std::vector<Block> blocks_;
	std::vector<int> row_block_, column_block_, width_, height_, right_, down_, node_column_;
	std::uint64_t distinct_ = 0;
	void construct(std::string_view text) {
		assert(text.size() < std::size_t(std::numeric_limits<int>::max() / 2));
		text_ = text;
		if (text_.empty()) return;
		int n = size();
		SuffixTree forward(text_), backward(std::string(text_.rbegin(), text_.rend()));
		index_.build(forward);
		SuffixLocusIndex reverse_index(backward);
		int columns = forward.node_count() - 1, rows = backward.node_count() - 1;
		std::vector<int> order{0}, low(columns + 1, 257), high(columns + 1, -1);
		order.reserve(columns + 1);
		for (std::size_t i = 0; i < order.size(); ++i) {
			int v = order[i], p = forward.suffix_start(v);
			if (p >= 0) low[v] = high[v] = p ? static_cast<unsigned char>(text_[p - 1]) + 1 : 0;
			for (int u : forward.children(v)) order.push_back(u);
		}
		for (std::size_t i = order.size(); i-- > 1;) {
			int v = order[i], p = forward.parent(v);
			low[p] = std::min(low[p], low[v]);
			high[p] = std::max(high[p], high[v]);
		}
		std::vector<int> heads, counts(n + 1);
		for (int v = 1; v <= columns; ++v) {
			if (low[v] != high[v] || low[v] == 0) {
				heads.push_back(v);
				++counts[forward.occurrence_count(v)];
			}
		}
		int total = 0;
		for (int count = n; count > 0; --count) {
			int next = total + counts[count];
			counts[count] = total;
			total = next;
		}
		order.resize(heads.size());
		for (int v : heads) order[counts[forward.occurrence_count(v)]++] = v;
		blocks_.reserve(heads.size());
		row_block_.reserve(rows); width_.reserve(rows); right_.reserve(rows);
		column_block_.reserve(columns); height_.reserve(columns); down_.reserve(columns);
		node_column_.assign(columns + 1, -1);
		std::vector<int> node_row(rows + 1, -1);
		auto suffix_link = [](const SuffixTree &tree, const SuffixLocusIndex &index, int v) {
			int p = tree.suffix_start(v);
			if (p >= 0) return tree.suffix_node(p + 1);
			auto [l, r] = tree.label_range(v);
			return index.locate(l + 1, r).node;
		};
		for (int v : order) {
			auto [l, r] = forward.label_range(v);
			int u = reverse_index.locate(n - r, n - l).node;
			assert(backward.depth(u) == r - l);
			int h = forward.depth(v) - forward.depth(forward.parent(v));
			int w = backward.depth(u) - backward.depth(backward.parent(u));
			int row = static_cast<int>(row_block_.size()), column = static_cast<int>(column_block_.size());
			int id = block_count(), count = forward.occurrence_count(v);
			blocks_.push_back({l, r, row, row + h, column, column + w, count});
			for (int i = 0, a = u; i < h; ++i) {
				assert(a > 0 && node_row[a] == -1 && backward.occurrence_count(a) == count);
				node_row[a] = row + i;
				row_block_.push_back(id);
				width_.push_back(backward.depth(a) - backward.depth(backward.parent(a)));
				right_.push_back(backward.parent(a));
				distinct_ += width_.back();
				if (i + 1 < h) a = suffix_link(backward, reverse_index, a);
			}
			for (int i = 0, a = v; i < w; ++i) {
				assert(a > 0 && node_column_[a] == -1 && forward.occurrence_count(a) == count);
				node_column_[a] = column + i;
				column_block_.push_back(id);
				height_.push_back(forward.depth(a) - forward.depth(forward.parent(a)));
				down_.push_back(forward.parent(a));
				if (i + 1 < w) a = suffix_link(forward, index_, a);
			}
		}
		assert(row_block_.size() == std::size_t(rows) && column_block_.size() == std::size_t(columns));
		for (int &v : right_) v = node_row[v];
		for (int &v : down_) v = node_column_[v];
	}

  public:
	BasicSubstringStructure() = default;
	explicit BasicSubstringStructure(std::string_view text) { construct(text); }
	BasicSubstringStructure(const BasicSubstringStructure &) = default;
	BasicSubstringStructure &operator=(const BasicSubstringStructure &) = default;
	BasicSubstringStructure(BasicSubstringStructure &&other) noexcept { swap(other); }
	BasicSubstringStructure &operator=(BasicSubstringStructure &&other) noexcept {
		if (this != &other) { BasicSubstringStructure moved(std::move(other)); swap(moved); }
		return *this;
	}
	void swap(BasicSubstringStructure &other) noexcept {
		text_.swap(other.text_);
		index_.swap(other.index_);
		blocks_.swap(other.blocks_);
		row_block_.swap(other.row_block_); column_block_.swap(other.column_block_);
		width_.swap(other.width_); height_.swap(other.height_);
		right_.swap(other.right_); down_.swap(other.down_); node_column_.swap(other.node_column_);
		std::swap(distinct_, other.distinct_);
	}
	void build(std::string_view text) { BasicSubstringStructure next(text); swap(next); }
	void clear() { BasicSubstringStructure next; swap(next); }
	int size() const { return static_cast<int>(text_.size()); }
	bool empty() const { return text_.empty(); }
	std::string_view text() const { return text_; }
	int block_count() const { return static_cast<int>(blocks_.size()); }
	std::span<const Block> blocks() const { return blocks_; }
	const Block &block(int id) const { assert(0 <= id && id < block_count()); return blocks_[id]; }
	std::pair<int, int> shape() const { return {static_cast<int>(row_block_.size()), static_cast<int>(column_block_.size())}; }
	int row_block(int row) const { assert(0 <= row && row < shape().first); return row_block_[row]; }
	int column_block(int column) const { assert(0 <= column && column < shape().second); return column_block_[column]; }
	int width(int row) const { assert(0 <= row && row < shape().first); return width_[row]; }
	int height(int column) const { assert(0 <= column && column < shape().second); return height_[column]; }
	int right(int row) const { assert(0 <= row && row < shape().first); return right_[row]; }
	int down(int column) const { assert(0 <= column && column < shape().second); return down_[column]; }
	bool valid(Position p) const {
		if (p.row < 0 || p.row >= shape().first || p.column < 0 || p.column >= shape().second) return false;
		int id = row_block_[p.row];
		return column_block_[p.column] == id && p.column - blocks_[id].column_begin < width_[p.row];
	}
	std::optional<Position> locate(int l, int r) const {
		assert(0 <= l && l <= r && r <= size());
		if (l == r) return std::nullopt;
		int column = node_column_[index_.locate(l, r).node];
		const Block &b = blocks_[column_block_[column]];
		int row = b.row_begin + ((b.right - b.left) - (r - l) - (column - b.column_begin));
		return Position{row, column};
	}
	std::pair<int, int> substring_range(Position p) const {
		assert(valid(p));
		const Block &b = blocks_[row_block_[p.row]];
		return {b.left + (p.column - b.column_begin), b.right - (p.row - b.row_begin)};
	}
	int occurrence_count(Position p) const { assert(valid(p)); return blocks_[row_block_[p.row]].occurrences; }
	std::optional<Position> erase_first(Position p) const {
		assert(valid(p));
		if (p.column + 1 < blocks_[row_block_[p.row]].column_begin + width_[p.row]) return Position{p.row, p.column + 1};
		int row = right_[p.row];
		if (row == -1) return std::nullopt;
		return Position{row, blocks_[row_block_[row]].column_begin};
	}
	std::optional<Position> erase_last(Position p) const {
		assert(valid(p));
		if (p.row + 1 < blocks_[column_block_[p.column]].row_begin + height_[p.column]) return Position{p.row + 1, p.column};
		int column = down_[p.column];
		if (column == -1) return std::nullopt;
		return Position{blocks_[column_block_[column]].row_begin, column};
	}
	std::uint64_t distinct_substrings() const { return distinct_; }
};

#endif
