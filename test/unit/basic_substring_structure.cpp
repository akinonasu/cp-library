#include "string/basic_substring_structure.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

using Structure = BasicSubstringStructure;
using Position = Structure::Position;

std::string label(const Structure &structure, Position position) {
	auto [l, r] = structure.substring_range(position);
	assert(0 <= l && l < r && r <= structure.size());
	return std::string(structure.text().substr(l, r - l));
}

void check(const std::string &s) {
	int n = int(s.size());
	Structure structure(s);
	assert(structure.size() == n && structure.empty() == s.empty() && structure.text() == s);
	auto [rows, columns] = structure.shape();
	assert(rows >= 0 && columns >= 0);
	assert(rows <= (n ? 2 * n - 1 : 0) && columns <= (n ? 2 * n - 1 : 0));
	assert(structure.block_count() == int(structure.blocks().size()));
	assert(!structure.valid({-1, 0}) && !structure.valid({0, -1}));
	assert(!structure.valid({rows, 0}) && !structure.valid({0, columns}));
	assert(!structure.valid({std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}));
	std::map<std::string, int> frequency;
	for (int l = 0; l < n; ++l)
		for (int r = l + 1; r <= n; ++r) ++frequency[s.substr(l, r - l)];
	assert(structure.distinct_substrings() == frequency.size());
	std::map<std::string, Position> locations;
	for (int l = 0; l <= n; ++l) {
		assert(!structure.locate(l, l));
		for (int r = l + 1; r <= n; ++r) {
			auto position = structure.locate(l, r);
			assert(position && structure.valid(*position));
			std::string value = s.substr(l, r - l);
			assert(label(structure, *position) == value);
			auto [it, inserted] = locations.emplace(value, *position);
			assert(inserted || it->second == *position);
			assert(structure.occurrence_count(*position) == frequency.at(value));
		}
	}
	int row_end = 0, column_end = 0, last_occurrences = n + 1;
	std::set<std::string> represented;
	std::uint64_t occurrence_sum = 0, width_sum = 0, height_sum = 0;
	for (int b = 0; b < structure.block_count(); ++b) {
		const auto &block = structure.block(b);
		const auto &listed = structure.blocks()[b];
		assert(&block == &listed);
		assert(0 <= block.left && block.left < block.right && block.right <= n);
		assert(block.row_begin == row_end && block.row_begin < block.row_end);
		assert(block.column_begin == column_end && block.column_begin < block.column_end);
		assert(0 < block.occurrences && block.occurrences <= last_occurrences);
		row_end = block.row_end;
		column_end = block.column_end;
		last_occurrences = block.occurrences;
		std::string representative = s.substr(block.left, block.right - block.left);
		assert(frequency.at(representative) == block.occurrences);
		std::set<int> preceding, following;
		for (int l = 0; l + int(representative.size()) <= n; ++l) {
			int r = l + int(representative.size());
			if (s.compare(l, representative.size(), representative) != 0) continue;
			preceding.insert(l ? int(static_cast<unsigned char>(s[l - 1])) : -1);
			following.insert(r < n ? int(static_cast<unsigned char>(s[r])) : -1);
		}
		assert(preceding.size() > 1 || preceding.contains(-1));
		assert(following.size() > 1 || following.contains(-1));
		assert(structure.width(block.row_begin) == block.column_end - block.column_begin);
		assert(structure.height(block.column_begin) == block.row_end - block.row_begin);
		for (int x = block.row_begin; x < block.row_end; ++x) {
			assert(structure.row_block(x) == b);
			int width = structure.width(x);
			assert(0 < width && width <= block.column_end - block.column_begin);
			if (x > block.row_begin) assert(width <= structure.width(x - 1));
			width_sum += width;
			std::string first = label(structure, {x, block.column_begin});
			std::string after = first.substr(width);
			int next = structure.right(x);
			if (after.empty()) assert(next == -1);
			else {
				assert(0 <= next && next < rows);
				int target = structure.row_block(next);
				assert(target < b && structure.block(target).occurrences > block.occurrences);
				assert((locations.at(after) == Position{next, structure.block(target).column_begin}));
			}
			for (int y = block.column_begin; y < block.column_end; ++y) {
				Position position{x, y};
				bool valid = y - block.column_begin < width;
				assert(structure.valid(position) == valid);
				assert(valid == (x - block.row_begin < structure.height(y)));
				if (!valid) continue;
				auto [l, r] = structure.substring_range(position);
				assert(l == block.left + y - block.column_begin);
				assert(r == block.right - x + block.row_begin);
				std::string value = s.substr(l, r - l);
				assert(represented.insert(value).second);
				assert(locations.at(value) == position);
				assert(frequency.at(value) == block.occurrences);
				assert(structure.occurrence_count(position) == block.occurrences);
				occurrence_sum += block.occurrences;
				auto first_removed = structure.erase_first(position);
				auto last_removed = structure.erase_last(position);
				if (value.size() == 1) assert(!first_removed && !last_removed);
				else {
					assert(first_removed && *first_removed == locations.at(value.substr(1)));
					assert(last_removed && *last_removed == locations.at(value.substr(0, value.size() - 1)));
				}
			}
		}
		for (int y = block.column_begin; y < block.column_end; ++y) {
			assert(structure.column_block(y) == b);
			int height = structure.height(y);
			assert(0 < height && height <= block.row_end - block.row_begin);
			if (y > block.column_begin) assert(height <= structure.height(y - 1));
			height_sum += height;
			std::string first = label(structure, {block.row_begin, y});
			std::string after = first.substr(0, first.size() - height);
			int next = structure.down(y);
			if (after.empty()) assert(next == -1);
			else {
				assert(0 <= next && next < columns);
				int target = structure.column_block(next);
				assert(target < b && structure.block(target).occurrences > block.occurrences);
				assert((locations.at(after) == Position{structure.block(target).row_begin, next}));
			}
		}
	}
	assert(row_end == rows && column_end == columns);
	assert(represented.size() == frequency.size());
	assert(width_sum == frequency.size() && height_sum == frequency.size());
	assert(occurrence_sum == std::uint64_t(n) * (n + 1) / 2);
	for (int x = 0; x < rows; ++x) for (int y = 0; y < columns; ++y)
		if (structure.row_block(x) != structure.column_block(y)) assert(!structure.valid({x, y}));
	if (n) {
		const auto &last = structure.block(structure.block_count() - 1);
		assert(last.left == 0 && last.right == n && last.occurrences == 1);
	} else assert(structure.block_count() == 0);
}

void lifetime() {
	Structure empty;
	assert(empty.empty() && empty.size() == 0 && empty.block_count() == 0);
	assert((empty.shape() == std::pair<int, int>{0, 0}));
	assert(!empty.locate(0, 0) && empty.distinct_substrings() == 0);
	std::string s = "banana";
	Structure structure(s);
	s.assign(s.size(), 'x');
	assert(structure.text() == "banana");
	Structure copy(structure);
	copy.build("abracadabra");
	assert(structure.text() == "banana" && copy.text() == "abracadabra");
	copy = structure;
	Structure &self = copy;
	copy = self;
	assert(copy.text() == "banana");
	Structure moved(std::move(copy));
	assert(moved.text() == "banana");
	assert(copy.empty() && copy.block_count() == 0 && !copy.locate(0, 0));
	assert((copy.shape() == std::pair<int, int>{0, 0}));
	copy.build("abba");
	assert(copy.text() == "abba");
	copy = std::move(moved);
	assert(copy.text() == "banana");
	assert(moved.empty() && moved.block_count() == 0 && !moved.locate(0, 0));
	moved.clear();
	assert(moved.empty());
	copy.build(copy.text());
	assert(copy.text() == "banana" && copy.distinct_substrings() == 15);
	copy.build(copy.text().substr(1, 3));
	assert(copy.text() == "ana" && copy.distinct_substrings() == 5);
	for (std::string value : {"", "a", "abababa", "", "banana", "a", ""}) {
		copy.build(value);
		assert(copy.text() == value);
		std::set<std::string> unique;
		for (int l = 0; l < int(value.size()); ++l)
			for (int r = l + 1; r <= int(value.size()); ++r) unique.insert(value.substr(l, r - l));
		assert(copy.distinct_substrings() == unique.size());
	}
	copy.clear();
	copy.clear();
	assert(copy.empty() && !copy.locate(0, 0));
	copy.swap(structure);
	assert(copy.text() == "banana" && structure.empty());
}

void long_cases() {
	constexpr int n = 100000;
	Structure uniform(std::string(n, 'a'));
	assert(uniform.distinct_substrings() == n);
	assert(uniform.block_count() == n);
	for (int length : {1, 2, 31, 32, n / 2, n - 1, n}) {
		auto first = uniform.locate(0, length);
		auto last = uniform.locate(n - length, n);
		assert(first && first == last && uniform.occurrence_count(*first) == n - length + 1);
	}
	std::string periodic;
	for (int i = 0; i < n; ++i) periodic += char(i % 3);
	Structure structure(periodic);
	assert(structure.distinct_substrings() == std::uint64_t(3) * n - 3);
	for (int l : {0, 1, 2, 1000, n / 2}) for (int length : {1, 7, 80, 1000}) {
		auto position = structure.locate(l, l + length);
		assert(position && structure.occurrence_count(*position) == (n - length - l % 3) / 3 + 1);
		assert(structure.locate(l + 3, l + 3 + length) == position);
	}
	std::mt19937 rng(987525);
	std::string random(n, '\0');
	for (char &c : random) c = char(rng());
	std::vector<int> order(n);
	std::iota(order.begin(), order.end(), 0);
	std::string_view view(random);
	std::sort(order.begin(), order.end(), [&](int a, int b) { return view.substr(a) < view.substr(b); });
	std::uint64_t expected = std::uint64_t(n) * (n + 1) / 2;
	for (int i = 1; i < n; ++i) {
		int a = order[i - 1], b = order[i], common = 0;
		while (a + common < n && b + common < n && random[a + common] == random[b + common]) ++common;
		expected -= common;
	}
	assert(expected > std::numeric_limits<std::uint32_t>::max());
	structure.build(random);
	assert(structure.distinct_substrings() == expected);
}

int main() {
	for (int n = 0; n <= 9; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::string s(n, 'a');
		for (int i = 0; i < n; ++i) s[i] += (mask >> i) & 1;
		check(s);
	}
	for (int n = 1, choices = 3; n <= 6; ++n, choices *= 3) for (int code = 0; code < choices; ++code) {
		std::string s(n, 'a');
		for (int i = 0, value = code; i < n; ++i, value /= 3) s[i] += value % 3;
		check(s);
	}
	for (const char *s : {"banana", "mississippi", "abracadabra", "abcabcababcabc", "aaaaabaaaaabaaaaa"}) check(s);
	for (int byte = 0; byte < 256; ++byte) check(std::string(1, char(byte)));
	check(std::string("\0\xff\x80\0\xff\0", 6));
	std::mt19937 rng(71298465);
	for (int trial = 0; trial < 300; ++trial) {
		int n = rng() % 35;
		std::string s(n, '\0');
		for (char &c : s) c = char(rng() % (trial % 2 ? 256 : 5));
		check(s);
	}
	lifetime();
	long_cases();
}
