#include "string/data_dependent_tree.hpp"
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Size = DataDependentTree::size_type;

Size naive_lcp(const std::string &a, Size p, const std::string &b, Size q) {
	Size k = 0;
	while (p + k < a.size() && q + k < b.size() && a[p + k] == b[q + k]) ++k;
	return k;
}

void check(DataDependentTree &tree, int root, const std::string &s) {
	assert(tree.size(root) == s.size());
	assert(tree.empty(root) == s.empty());
	assert(tree.str(root) == s);
	assert(tree.equal(root, tree.build(s)));
	assert(tree.lcp(root, root) == s.size());
	for (std::size_t i = 0; i < s.size(); ++i) assert(tree.at(root, i) == s[i]);
}

void exhaustive(std::uint64_t seed) {
	DataDependentTree tree(seed);
	std::vector<std::string> texts;
	std::vector<int> roots;
	for (int n = 0; n <= 7; ++n) {
		for (int mask = 0; mask < (1 << n); ++mask) {
			std::string s(n, 'a');
			for (int i = 0; i < n; ++i) s[i] += (mask >> i) & 1;
			int root = tree.build(s);
			check(tree, root, s);
			texts.push_back(s), roots.push_back(root);
			for (int p = 0; p <= n; ++p) {
				auto [a, b] = tree.split(root, p);
				assert(a == tree.build(std::string_view(s).substr(0, p)));
				assert(b == tree.build(std::string_view(s).substr(p)));
				assert(tree.concat(a, b) == root);
				for (int r = p; r <= n; ++r)
					assert(tree.substr(root, p, r) == tree.build(std::string_view(s).substr(p, r - p)));
			}
		}
	}
	for (std::size_t i = 0; i < texts.size(); ++i) {
		for (std::size_t j = 0; j < texts.size(); ++j) {
			assert(tree.lcp(roots[i], roots[j]) == naive_lcp(texts[i], 0, texts[j], 0));
			assert(tree.compare(roots[i], roots[j]) == ((texts[i] > texts[j]) - (texts[i] < texts[j])));
			if (texts[i].size() <= 4 && texts[j].size() <= 4)
				assert(tree.concat(roots[i], roots[j]) == tree.build(texts[i] + texts[j]));
		}
	}
}

void randomized(std::uint64_t seed) {
	DataDependentTree tree(seed);
	tree.reserve(1024);
	std::mt19937_64 rng(seed);
	std::vector<std::string> texts(1);
	std::vector<int> roots(1);
	for (int t = 0; t < 1800; ++t) {
		int i = rng() % roots.size(), v = roots[i];
		std::string s = texts[i];
		Size l = rng() % (s.size() + 1), r = rng() % (s.size() + 1);
		if (l > r) std::swap(l, r);
		std::string extra(rng() % 15, '\0');
		for (char &c : extra) c = char(rng() % (t & 1 ? 4 : 256));
		switch (rng() % 8) {
			case 0: s.insert(l, extra); v = tree.insert(v, l, extra); break;
			case 1: s.erase(l, r - l); v = tree.erase(v, l, r); break;
			case 2: s = s.substr(l, r - l); v = tree.substr(v, l, r); break;
			case 3: {
				auto [a, b] = tree.split(v, l);
				check(tree, a, s.substr(0, l)); check(tree, b, s.substr(l));
				v = tree.concat(a, b);
				break;
			}
			case 4: s += extra; v = tree.concat(v, tree.build(extra)); break;
			case 5: {
				if (s.empty()) break;
				Size p = rng() % s.size();
				char c = char(rng());
				s[p] = c, v = tree.set(v, p, c);
				break;
			}
			case 6: {
				unsigned k = rng() % 5;
				if (s.size() * k > 600) k = 0;
				std::string a = s;
				s.clear();
				for (unsigned j = 0; j < k; ++j) s += a;
				v = tree.repeat(v, k);
				break;
			}
			case 7: s.insert(l, extra); v = tree.insert(v, l, tree.build(extra)); break;
		}
		check(tree, v, s);
		int j = rng() % roots.size();
		check(tree, roots[j], texts[j]);
		Size p = rng() % (s.size() + 1), q = rng() % (texts[j].size() + 1), limit = rng() % 100;
		Size expected = naive_lcp(s, p, texts[j], q);
		std::size_t before = tree.storage_nodes();
		assert(tree.lcp(v, p, roots[j], q) == expected);
		assert(tree.lcp(v, p, roots[j], q, limit) == std::min(expected, limit));
		assert(tree.lcp(v, roots[j], limit) == std::min(naive_lcp(s, 0, texts[j], 0), limit));
		int cmp = s.compare(texts[j]);
		assert(tree.compare(v, roots[j]) == ((cmp > 0) - (cmp < 0)));
		assert(tree.storage_nodes() == before);
		texts.push_back(s), roots.push_back(v);
	}
	int a = tree.build("persistent"), b = tree.build(" trees");
	auto copy = tree;
	int joined = copy.concat(a, b);
	assert(copy.str(joined) == "persistent trees");
	assert(tree.str(a) == "persistent");
	DataDependentTree assigned;
	assigned = copy;
	assert(assigned.str(joined) == copy.str(joined));
	const auto *same = &assigned;
	assigned = *same;
	auto moved = std::move(assigned);
	assert(moved.str(joined) == "persistent trees");
	assert(assigned.build("reused") == assigned.build("reused"));
	assigned = std::move(moved);
	assert(assigned.str(joined) == "persistent trees");
	assert(moved.empty(0));
	int old = moved.build("old");
	assigned.swap(moved);
	assert(assigned.str(old) == "old");
	assert(moved.str(joined) == "persistent trees");
}

void long_strings() {
	DataDependentTree tree(129);
	std::mt19937_64 rng(14);
	for (int kind = 0; kind < 4; ++kind) {
		std::string s(12000, 'a');
		for (std::size_t i = 0; i < s.size(); ++i) {
			if (kind == 1) s[i] = "abacaba"[i % 7];
			if (kind >= 2) s[i] = char(rng() % (kind == 2 ? 2 : 256));
		}
		int root = tree.build(s);
		for (int i = 0; i < 80; ++i) {
			Size p = rng() % s.size();
			s[p] ^= char(1);
			int next = tree.set(root, p, s[p]);
			assert(tree.lcp(root, next) == p);
			root = next;
			Size q = rng() % s.size();
			assert(tree.lcp(root, p, root, q) == naive_lcp(s, p, s, q));
		}
		check(tree, root, s);
		int assembled = 0;
		for (std::size_t p = 0; p < s.size(); p += 137)
			assembled = tree.concat(assembled, tree.build(std::string_view(s).substr(p, 137)));
		assert(assembled == root);
	}
}

void huge_runs() {
	DataDependentTree tree(0);
	Size max = std::numeric_limits<Size>::max();
	int a = tree.build("a"), huge = tree.repeat(a, max);
	assert(tree.size(huge) == max && tree.at(huge, max - 1) == 'a');
	assert(tree.lcp(huge, 0, huge, 7) == max - 7);
	for (Size p : {Size(0), Size(1), max / 2, max - 1, max}) {
		auto [l, r] = tree.split(huge, p);
		assert(tree.size(l) == p && tree.size(r) == max - p);
		assert(tree.concat(l, r) == huge);
	}
	int changed = tree.set(huge, max - 1, 'b');
	assert(tree.lcp(huge, changed) == max - 1);
	assert(tree.compare(huge, changed) < 0);
	assert(tree.at(changed, max - 1) == 'b');
	assert(tree.erase(changed, max - 1, max) == tree.repeat(a, max - 1));
	int pattern = tree.repeat(tree.build("abcabcab"), Size(100000000000000000));
	assert(tree.lcp(pattern, 0, pattern, 8) == tree.size(pattern) - 8);
	assert(tree.lcp(pattern, 0, pattern, 3) == 5);
	assert(tree.repeat(0, max) == 0 && tree.repeat(huge, 0) == 0);
	bool overflow = false;
	try { tree.concat(huge, a); } catch (const std::length_error &) { overflow = true; }
	assert(overflow);
	overflow = false;
	try { tree.repeat(huge, 2); } catch (const std::length_error &) { overflow = true; }
	assert(overflow);
	overflow = false;
	try { tree.str(huge); } catch (const std::length_error &) { overflow = true; }
	assert(overflow);
	overflow = false;
	try { tree.reserve(std::numeric_limits<int>::max()); } catch (const std::length_error &) { overflow = true; }
	assert(overflow);
	assert(tree.lcp(huge, changed, 100) == 100);
	assert(tree.height(0) == 0 && tree.height(a) == 0);
}

int main() {
	for (unsigned seed : {0, 1, 1729}) exhaustive(seed);
	for (unsigned seed : {0, 1, 713, 20261023}) randomized(seed);
	long_strings();
	huge_runs();
}
