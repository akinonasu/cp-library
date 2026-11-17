#include "string/data_dependent_tree.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
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

void byte_sequences() {
	DataDependentTree tree;
	std::vector<char> values{char(0), char(255), 'a', char(0)};
	int root = tree.build(values);
	std::string expected(values.begin(), values.end());
	check(tree, root, expected);
	assert(tree.build(std::span<char>(values)) == root);
	assert(tree.build(std::span<const char>(values)) == root);
	std::array<char, 2> extra{'b', char(128)};
	root = tree.insert(root, 2, extra);
	expected.insert(2, extra.data(), extra.size());
	check(tree, root, expected);
	assert(tree.build(std::array<char, 0>{}) == 0);
}

template <class T>
void integer_sequences(std::uint64_t seed) {
	using Tree = BasicDataDependentTree<T>;
	std::mt19937_64 rng(seed);
	Tree tree(seed);
	tree.reserve(1024);
	assert(tree.storage_nodes() == 0);
	std::vector<T> alphabet{std::numeric_limits<T>::min(), std::numeric_limits<T>::max(), T(0), T(1), T(257), T(-1)};
	for (int i = 0; i < 400; ++i) alphabet.push_back(T(rng()));
	std::array<T, 3> fixed{alphabet[0], alphabet[1], alphabet[2]};
	int fixed_root = tree.build(fixed);
	assert(tree.str(fixed_root) == std::vector<T>(fixed.begin(), fixed.end()));
	assert(tree.build(std::span<T, 3>(fixed)) == fixed_root);
	assert(tree.build(std::span<const T, 3>(fixed)) == fixed_root);
	std::vector<std::vector<T>> texts(1);
	std::vector<int> roots(1);
	auto verify = [&](int root, const std::vector<T> &text) {
		assert(tree.str(root) == text && tree.size(root) == text.size());
		assert(tree.empty(root) == text.empty());
		assert(tree.build(text) == root);
		for (std::size_t i = 0; i < text.size(); ++i) assert(tree.at(root, i) == text[i]);
	};
	for (int step = 0; step < 1800; ++step) {
		std::size_t version = rng() % roots.size();
		int root = roots[version];
		auto text = texts[version];
		std::size_t l = rng() % (text.size() + 1), r = rng() % (text.size() + 1);
		if (l > r) std::swap(l, r);
		std::vector<T> extra(rng() % 12);
		for (T &x : extra) x = alphabet[rng() % alphabet.size()];
		switch (rng() % 8) {
			case 0: root = tree.insert(root, l, extra); text.insert(text.begin() + l, extra.begin(), extra.end()); break;
			case 1: root = tree.erase(root, l, r); text.erase(text.begin() + l, text.begin() + r); break;
			case 2: root = tree.substr(root, l, r); text = std::vector<T>(text.begin() + l, text.begin() + r); break;
			case 3: {
				auto [a, b] = tree.split(root, l);
				verify(a, std::vector<T>(text.begin(), text.begin() + l));
				verify(b, std::vector<T>(text.begin() + l, text.end()));
				assert(tree.concat(a, b) == root);
				break;
			}
			case 4: root = tree.concat(root, tree.build(extra)); text.insert(text.end(), extra.begin(), extra.end()); break;
			case 5: {
				if (text.empty()) break;
				std::size_t p = rng() % text.size();
				text[p] = alphabet[rng() % alphabet.size()];
				root = tree.set(root, p, text[p]);
				break;
			}
			case 6: {
				unsigned k = text.size() > 100 ? 0 : rng() % 4;
				root = tree.repeat(root, k);
				auto original = text;
				text.clear();
				for (unsigned i = 0; i < k; ++i) text.insert(text.end(), original.begin(), original.end());
				break;
			}
			case 7: root = tree.insert(root, l, tree.build(extra)); text.insert(text.begin() + l, extra.begin(), extra.end()); break;
		}
		verify(root, text);
		std::size_t other = rng() % roots.size();
		verify(roots[other], texts[other]);
		std::size_t p = rng() % (text.size() + 1), q = rng() % (texts[other].size() + 1);
		Size prefix = 0;
		while (prefix < text.size() && prefix < texts[other].size() && text[prefix] == texts[other][prefix]) ++prefix;
		assert(tree.lcp(root, roots[other]) == prefix);
		Size suffix_prefix = 0;
		while (p + suffix_prefix < text.size() && q + suffix_prefix < texts[other].size() && text[p + suffix_prefix] == texts[other][q + suffix_prefix]) ++suffix_prefix;
		assert(tree.lcp(root, p, roots[other], q) == suffix_prefix);
		assert(tree.lcp(root, p, roots[other], q, 3) == std::min(Size(3), suffix_prefix));
		assert(tree.compare(root, roots[other]) == ((text > texts[other]) - (text < texts[other])));
		roots.push_back(root); texts.push_back(std::move(text));
	}
	int a = tree.build(alphabet);
	auto copied = tree;
	assert(copied.str(a) == alphabet);
	int changed = copied.set(a, 0, T(12345));
	assert(tree.str(a) == alphabet && copied.at(changed, 0) == T(12345));
	Tree moved(std::move(copied));
	assert(moved.at(changed, 0) == T(12345));
	assert(copied.storage_nodes() == 0 && copied.empty(0));
	int reused = copied.build(alphabet);
	assert(copied.str(reused) == alphabet);
	copied = moved;
	assert(copied.str(a) == alphabet);
	copied = std::move(moved);
	assert(copied.at(changed, 0) == T(12345) && moved.empty(0));
	tree.swap(copied);
	assert(tree.at(changed, 0) == T(12345) && copied.str(a) == alphabet);
	Tree huge(seed);
	int one = huge.build(std::span<const T>(alphabet).first(1));
	Size max = std::numeric_limits<Size>::max();
	int repeated = huge.repeat(one, max);
	assert(huge.at(repeated, max - 1) == alphabet[0]);
	int replaced = huge.set(repeated, max - 1, alphabet[1]);
	assert(huge.at(replaced, max - 1) == alphabet[1] && huge.lcp(repeated, replaced) == max - 1);
	assert(huge.compare(repeated, replaced) == ((alphabet[0] > alphabet[1]) - (alphabet[0] < alphabet[1])));
}

int main() {
	for (unsigned seed : {0, 1, 1729}) exhaustive(seed);
	for (unsigned seed : {0, 1, 713, 20261023}) randomized(seed);
	long_strings();
	huge_runs();
	byte_sequences();
	integer_sequences<std::int64_t>(713);
	integer_sequences<std::uint64_t>(129);
	integer_sequences<int>(17);
}
