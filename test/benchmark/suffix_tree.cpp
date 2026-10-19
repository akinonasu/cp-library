#include "string/suffix_tree.hpp"
#include "suffix_tree_base.hpp"
#include "acl_string.hpp"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>

[[gnu::noinline]] std::vector<int> construct(const std::string &s, const std::string &variant) {
	if (variant == "acl") {
		std::vector<int> values;
		values.reserve(s.size());
		for (unsigned char c : s) values.push_back(c);
		return atcoder::suffix_array(values, 255);
	}
	return suffix_array_detail::bytes(s);
}

int main(int argc, char **argv) {
	if (argc != 4) return 2;
	std::string variant = argv[1], kind = argv[2];
	int n = std::atoi(argv[3]);
	std::mt19937 rng(20261020);
	std::string s(n, 0);
	for (int i = 0; i < n; ++i) {
		if (kind == "equal") s[i] = 'a';
		else if (kind == "periodic") s[i] = "abracadabra"[i % 11];
		else s[i] = char(rng() % (kind == "binary" ? 2 : kind == "letters" ? 26 : 256));
	}
	std::uint64_t checksum = 0;
	auto begin = std::chrono::steady_clock::now();
	if (variant == "tree") {
		SuffixTree tree(s);
		auto end = std::chrono::steady_clock::now();
		for (int x : tree.suffix_array()) checksum = checksum * 1000000007 + x;
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << ' ' << checksum << '\n';
		return 0;
	}
	if (variant == "base") {
		BaseSuffixTree tree(s);
		auto end = std::chrono::steady_clock::now();
		for (int x : tree.suffix_array()) checksum = checksum * 1000000007 + x;
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << ' ' << checksum << '\n';
		return 0;
	}
	if (variant == "acl" || variant == "induced") {
		auto sa = construct(s, variant);
		auto end = std::chrono::steady_clock::now();
		checksum = n;
		for (int x : sa) checksum = checksum * 1000000007 + x;
		std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << ' ' << checksum << '\n';
		return 0;
	}
	SuffixTree tree(s);
	BaseSuffixTree base(s);
	std::vector<std::string> patterns;
	for (int i = 0; i < 20000; ++i) {
		int p = rng() % n;
		auto pattern = s.substr(p, std::min(n - p, i % 2 ? 32 : 128));
		if (i % 3 == 0) pattern.back() ^= 64;
		patterns.push_back(std::move(pattern));
	}
	begin = std::chrono::steady_clock::now();
	for (const auto &pattern : patterns) {
		int count;
		if (variant == "find") count = tree.count(pattern);
		else if (variant == "base_find") count = base.count(pattern);
		else {
			const auto &sa = tree.suffix_array();
			auto lower = std::partition_point(sa.begin(), sa.end(), [&](int p) { return std::string_view(s).substr(p).compare(0, pattern.size(), pattern) < 0; });
			auto upper = std::partition_point(lower, sa.end(), [&](int p) { return std::string_view(s).substr(p).compare(0, pattern.size(), pattern) <= 0; });
			count = upper - lower;
		}
		checksum = checksum * 1000000007 + count;
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << ' ' << checksum << '\n';
}
