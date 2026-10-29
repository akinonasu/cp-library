#include "components/suffix_locus.hpp"
#include <random>
#include <type_traits>

static_assert(std::is_nothrow_move_constructible_v<SuffixLocusIndex>);
static_assert(std::is_nothrow_move_assignable_v<SuffixLocusIndex>);

void check(const std::string &s) {
	SuffixTree tree(s);
	SuffixLocusIndex index(tree);
	assert(index.size() == int(s.size()));
	for (int l = 0; l <= index.size(); ++l) for (int r = l; r <= index.size(); ++r) {
		auto expected = tree.find(std::string_view(s).substr(l, r - l));
		auto actual = index.locate(l, r);
		assert(expected && actual.node == expected->node && actual.length == expected->length);
		assert(tree.depth(actual.node) >= actual.length);
		assert(!actual.node || tree.depth(tree.parent(actual.node)) < actual.length);
	}
	auto full = index.locate(0, index.size());
	auto copy = index;
	SuffixLocusIndex moved(std::move(copy));
	assert(copy.size() == 0 && copy.locate(0, 0).node == 0);
	copy = std::move(moved);
	assert(moved.size() == 0 && moved.locate(0, 0).length == 0);
	assert(copy.locate(0, copy.size()).node == full.node);
	tree.clear();
	assert(copy.size() == int(s.size()) && copy.locate(0, copy.size()).node == full.node);
	copy.build(tree);
	assert(copy.size() == 0 && copy.locate(0, 0).node == 0);
	copy = index;
	assert(copy.size() == index.size() && copy.locate(0, copy.size()).node == full.node);
	copy.swap(index);
	assert(index.locate(0, index.size()).node == full.node);
}

int main() {
	SuffixLocusIndex empty;
	assert(empty.size() == 0 && empty.locate(0, 0).node == 0 && empty.locate(0, 0).length == 0);
	for (int n = 0; n <= 8; ++n) for (int mask = 0; mask < (1 << n); ++mask) {
		std::string s(n, 'a');
		for (int i = 0; i < n; ++i) s[i] += (mask >> i) & 1;
		check(s);
	}
	std::mt19937 random(91723);
	for (int test = 0; test < 150; ++test) {
		int n = random() % 65, alphabet = test % 2 ? 256 : 5;
		std::string s(n, 0);
		for (char &c : s) c = char(random() % alphabet);
		check(s);
	}
	std::string bytes;
	for (int c = 0; c < 256; ++c) bytes += char(c);
	check(bytes);
	check("mississippi");
	check("abababababababababa");
	for (int period : {1, 7}) {
		std::string s(200000, 0);
		for (int i = 0; i < int(s.size()); ++i) s[i] = char(i % period);
		SuffixTree tree(s);
		SuffixLocusIndex index(tree);
		for (int test = 0; test < 1000; ++test) {
			int l = random() % (s.size() + 1), r = l + random() % (s.size() - l + 1);
			auto expected = tree.find(std::string_view(s).substr(l, r - l));
			auto actual = index.locate(l, r);
			assert(expected && actual.node == expected->node && actual.length == expected->length);
		}
	}
}
