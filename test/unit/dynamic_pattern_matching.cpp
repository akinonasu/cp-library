#include "../../string/dynamic_pattern_matching.hpp"
#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>

std::vector<int> naive(std::string_view text, std::string_view pattern) {
	std::vector<int> result;
	for (std::size_t i = 0; i <= text.size(); ++i)
		if (pattern.size() <= text.size() - i && text.substr(i, pattern.size()) == pattern) result.push_back(static_cast<int>(i));
	return result;
}

void check(const DynamicPatternMatching &matching, const std::string &pattern) {
	assert(matching.size() == pattern.size());
	assert(matching.empty() == pattern.empty());
	assert(matching.str() == pattern);
	auto expected = naive(matching.text(), pattern), actual = matching.occurrences();
	assert(matching.count() == int(expected.size()));
	auto [l, r] = matching.occurrence_range();
	assert(r - l == matching.count());
	for (int i = 0; i < matching.count(); ++i) {
		assert(matching.occurrence(i) == actual[i]);
		assert(actual[i] == matching.suffix_array()[l + i]);
	}
	std::sort(actual.begin(), actual.end());
	assert(actual == expected);
	for (std::size_t i = 0; i < pattern.size(); ++i) assert(matching[i] == pattern[i]);
}

void ranges() {
	std::mt19937 rng(246);
	for (int n = 0; n <= 65; ++n) {
		std::string text(n, 'a');
		for (char &c : text) c = char(rng() % 5);
		SuffixRangeIndex index(text);
		auto missing = index.concat(index.character('x'), index.character('y'));
		assert(missing.left == missing.right && missing.length == 2);
		missing = index.concat(missing, index.character('a'));
		assert(missing.left == missing.right && missing.length == 3);
		for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
			auto range = index.empty_range();
			for (int i = l; i < r; ++i) range = index.concat(range, index.character(text[i]));
			assert(range.length == r - l);
			assert(range.right - range.left == int(naive(text, text.substr(l, r - l)).size()));
			for (int k : {0, (r - l) / 2, r - l}) {
				auto prefix = index.substring(range, 0, k), suffix = index.substring(range, k, r - l);
				assert(prefix.right - prefix.left == int(naive(text, text.substr(l, k)).size()));
				assert(suffix.right - suffix.left == int(naive(text, text.substr(l + k, r - l - k)).size()));
				auto joined = index.concat(prefix, suffix);
				assert(joined.left == range.left && joined.right == range.right && joined.length == range.length);
			}
		}
		for (int query = 0; query < 100; ++query) {
			std::string pattern(rng() % 70, '\0');
			for (char &c : pattern) c = char(rng() % 6);
			auto [range, length] = index.longest_prefix(pattern);
			int expected = 0;
			while (expected < int(pattern.size()) && !naive(text, pattern.substr(0, expected + 1)).empty()) ++expected;
			assert(length == expected && range.length == expected);
		}
	}
}

std::string binary(int mask) {
	std::string result;
	for (; mask > 1; mask >>= 1) result += char('a' + (mask & 1));
	return result;
}

void exhaustive() {
	for (int t = 1; t < 64; ++t) {
		DynamicPatternMatching base(binary(t));
		for (int p = 1; p < 64; ++p) {
			auto pattern = binary(p);
			base.assign(pattern);
			check(base, pattern);
			for (std::size_t i = 0; i <= pattern.size(); ++i) {
				for (char c : {'a', 'b', 'c'}) {
					auto changed = base;
					changed.insert(i, c);
					auto expected = pattern; expected.insert(expected.begin() + i, c);
					check(changed, expected);
					if (i < pattern.size()) {
						changed = base; changed.set(i, c);
						expected = pattern; expected[i] = c;
						check(changed, expected);
					}
				}
				for (std::size_t j = i; j <= pattern.size(); ++j) {
					auto changed = base;
					changed.erase(i, j);
					auto expected = pattern; expected.erase(i, j - i);
					check(changed, expected);
					for (std::size_t k = 0; k <= pattern.size(); ++k) {
						changed = base; changed.copy(i, j, k);
						expected = pattern; expected.insert(k, pattern.substr(i, j - i));
						check(changed, expected);
						if (k <= pattern.size() - (j - i)) {
							changed = base; changed.move(i, j, k);
							expected = pattern; expected.erase(i, j - i); expected.insert(k, pattern.substr(i, j - i));
							check(changed, expected);
						}
					}
				}
			}
			check(base, pattern);
			base.compact();
		}
	}
}

void randomized() {
	std::mt19937_64 rng(985738);
	for (int trial = 0; trial < 100; ++trial) {
		std::string text(rng() % 150, '\0');
		int alphabet = trial % 3 == 0 ? 256 : trial % 3 + 1;
		for (char &c : text) c = char(rng() % alphabet);
		DynamicPatternMatching matching(text);
		std::string pattern;
		std::vector<std::pair<DynamicPatternMatching, std::string>> versions;
		for (int step = 0; step < 2000; ++step) {
			auto l = rng() % (pattern.size() + 1), r = rng() % (pattern.size() + 1);
			if (l > r) std::swap(l, r);
			char c = char(rng() % (alphabet + (alphabet < 256)));
			int op = rng() % 12;
			if (pattern.size() > 160) op = 2;
			if (op == 0) { matching.insert(l, c); pattern.insert(pattern.begin() + l, c); }
			if (op == 1 && l < pattern.size()) { matching.set(l, c); pattern[l] = c; }
			if (op == 2) { matching.erase(l, r); pattern.erase(l, r - l); }
			if (op == 3) {
				auto p = rng() % (pattern.size() + 1);
				matching.copy(l, r, p); pattern.insert(p, pattern.substr(l, r - l));
			}
			if (op == 4) {
				auto p = rng() % (pattern.size() - (r - l) + 1);
				matching.move(l, r, p);
				auto middle = pattern.substr(l, r - l); pattern.erase(l, r - l); pattern.insert(p, middle);
			}
			if (op == 5) {
				auto input = text.substr(rng() % (text.size() + 1), rng() % 40);
				matching.insert(l, input); pattern.insert(l, input);
			}
			if (op == 6) { pattern = text.substr(rng() % (text.size() + 1), rng() % 40); matching.assign(pattern); }
			if (op == 7) { versions.emplace_back(matching, pattern); if (versions.size() > 8) versions.erase(versions.begin()); }
			if (op == 8 && !versions.empty()) { auto &version = versions[rng() % versions.size()]; matching = version.first; pattern = version.second; }
			if (op == 9) { matching.compact(); }
			if (op == 10 && l < pattern.size()) { matching.erase(l); pattern.erase(l, 1); }
			if (op == 11 && rng() % 10 == 0) { matching.clear(); pattern.clear(); }
			check(matching, pattern);
			if (step % 50 == 0) for (auto &version : versions) check(version.first, version.second);
		}
	}
}

void limits() {
	DynamicPatternMatching matching("abc", "z");
	for (int i = 0; i < 63; ++i) matching.copy(0, matching.size(), matching.size());
	assert(matching.size() == (std::uint64_t(1) << 63));
	matching.copy(1, matching.size(), 0);
	assert(matching.size() == std::numeric_limits<std::uint64_t>::max());
	assert(matching[0] == 'z' && matching[matching.size() - 1] == 'z');
	auto snapshot = matching;
	matching.move(0, 1000000000000000000ULL, matching.size() - 1000000000000000000ULL);
	matching.erase(1, matching.size() - 1);
	check(matching, "zz");
	snapshot.compact();
	assert(snapshot.storage_nodes() < 10000);
	snapshot.erase(0, snapshot.size()); check(snapshot, "");
	matching.build("banana", "ana"); check(matching, "ana");
	auto moved = std::move(matching); check(moved, "ana"); check(matching, "");
	matching = std::move(moved); check(matching, "ana"); check(moved, "");
	matching.swap(moved); check(moved, "ana"); check(matching, "");
	std::string bytes;
	for (int i = 0; i < 256; ++i) bytes += char(i);
	matching.build(bytes, bytes); check(matching, bytes);
	matching.erase(10, 250); check(matching, bytes.substr(0, 10) + bytes.substr(250));
	matching.insert(10, std::string_view(bytes).substr(10, 240)); check(matching, bytes);
	DynamicPatternMatching empty;
	empty.insert(0, bytes); check(empty, bytes);
	empty.erase(0, 256); check(empty, "");
	DynamicPatternMatching repeated(std::string(200000, 'a'), std::string(100000, 'a'));
	assert(repeated.count() == 100001);
	for (int i = 0; i < 5000; ++i) {
		repeated.insert(50000, 'b'); assert(repeated.count() == 0);
		repeated.erase(50000); assert(repeated.count() == 100001);
	}
	repeated.compact(); assert(repeated.storage_nodes() == 1);
}

int main() {
	ranges(); exhaustive(); randomized(); limits();
	std::cout << "Dynamic pattern matching passed\n";
}
