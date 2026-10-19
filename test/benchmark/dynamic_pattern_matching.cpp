#include "string/dynamic_pattern_matching.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

#ifdef AUTHOR_REFERENCE
#include "author/src/alg/matching.cpp"
#endif

struct PlainMatching {
	std::string text, pattern;
	PlainMatching(const std::string &text, const std::string &pattern) : text(text), pattern(pattern) {}
	void insert(std::uint64_t p, char c) { pattern.insert(pattern.begin() + p, c); }
	void erase(std::uint64_t l, std::uint64_t r) { pattern.erase(l, r - l); }
	void copy(std::uint64_t l, std::uint64_t r, std::uint64_t p) { pattern.insert(p, pattern.substr(l, r - l)); }
	void move(std::uint64_t l, std::uint64_t r, std::uint64_t p) {
		auto middle = pattern.substr(l, r - l); pattern.erase(l, r - l); pattern.insert(p, middle);
	}
	std::uint64_t size() const { return pattern.size(); }
	int count() const {
		if (pattern.empty()) return static_cast<int>(text.size()) + 1;
		if (pattern.size() > text.size()) return 0;
		std::vector<int> failure(pattern.size());
		for (int i = 1, j = 0; i < int(pattern.size()); ++i) {
			while (j && pattern[i] != pattern[j]) j = failure[j - 1];
			if (pattern[i] == pattern[j]) ++j;
			failure[i] = j;
		}
		int result = 0, j = 0;
		for (char c : text) {
			while (j && c != pattern[j]) j = failure[j - 1];
			if (c == pattern[j]) ++j;
			if (j == int(pattern.size())) { ++result; j = failure[j - 1]; }
		}
		return result;
	}
};

template<class Matching>
std::pair<std::uint64_t, std::uint64_t> run(Matching &matching, const std::string &operation, const std::string &kind, int queries) {
	std::mt19937_64 rng(20261020);
	std::uint64_t checksum = 0;
	auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < queries; ++i) {
		auto p = rng() % (matching.size() + 1);
		if (operation == "edit") {
			matching.insert(p, kind == "equal" ? 'a' : 'z');
			checksum += matching.count();
			matching.erase(p, p + 1);
		} else if (operation == "splice") {
			auto l = rng() % (matching.size() / 2), r = l + matching.size() / 4;
			matching.copy(l, r, p);
			checksum += matching.count();
			matching.erase(p, p + (r - l));
		} else {
			auto l = rng() % (matching.size() / 2), r = l + matching.size() / 4;
			auto destination = rng() % (matching.size() - (r - l) + 1);
			matching.move(l, r, destination);
			checksum += matching.count();
		}
		checksum += matching.count();
	}
	return {std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count(), checksum};
}

int main(int argc, char **argv) {
	if (argc != 6) return 1;
	std::string variant = argv[1], operation = argv[2], kind = argv[3];
	int n = std::stoi(argv[4]), queries = std::stoi(argv[5]);
	std::mt19937_64 rng(54789);
	std::string text(n, 'a');
	if (kind == "binary" || kind == "fragmented") for (char &c : text) c += rng() % 2;
	if (kind == "letters") for (char &c : text) c += rng() % 26;
	if (kind == "periodic") for (int i = 0; i < n; ++i) text[i] += i % 7;
	std::string pattern = text.substr(n / 3, n / 5);
	if (kind == "fragmented") for (char &c : pattern) c = char('a' + rng() % 3);
	if (operation == "build") {
		auto start = std::chrono::steady_clock::now();
		DynamicPatternMatching matching(text);
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
		std::cout << elapsed << ' ' << matching.count() << ' ' << matching.storage_nodes() << '\n';
		return 0;
	}
#ifdef AUTHOR_REFERENCE
	if (variant == "author") {
		dyn_pattern::matching matching(text);
		matching.insert(0, pattern);
		std::mt19937_64 positions(20261020);
		std::uint64_t checksum = 0;
		auto start = std::chrono::steady_clock::now();
		for (int i = 0; i < queries; ++i) {
			int p = positions() % (pattern.size() + 1);
			matching.insert(p, 'a'); checksum += matching.matches();
			matching.erase(p); checksum += matching.matches();
		}
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - start).count();
		std::cout << elapsed << ' ' << checksum << " 0\n";
		return 0;
	}
#endif
	if (variant == "kmp") {
		PlainMatching matching(text, pattern);
		auto [elapsed, checksum] = run(matching, operation, kind, queries);
		std::cout << elapsed << ' ' << checksum << " 0\n";
	} else {
		DynamicPatternMatching matching(text, pattern);
		auto [elapsed, checksum] = run(matching, operation, kind, queries);
		std::cout << elapsed << ' ' << checksum << ' ' << matching.storage_nodes() << '\n';
	}
}
