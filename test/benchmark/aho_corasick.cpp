#ifndef AHO_HEADER
#define AHO_HEADER "string/aho_corasick.hpp"
#endif
#include AHO_HEADER
#include "components/fast_io.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

int main() {
	FastInput input;
	int n, total = 0;
	input.read(n);
	std::vector<std::string> patterns(n);
	for (auto &s : patterns) input.read(s), total += static_cast<int>(s.size());
	std::string text;
	input.read(text);
	auto start = std::chrono::steady_clock::now();
	AhoCorasick<> ac;
	ac.reserve(total, n);
	for (const auto &s : patterns) ac.add(s);
	ac.build();
	auto built = std::chrono::steady_clock::now();
	auto count = ac.count_matches(text);
	auto scanned = std::chrono::steady_clock::now();
	std::uint64_t checksum = ac.node_count();
	for (int v = 1; v < ac.node_count(); ++v) checksum = (checksum ^ ac.suffix_link(v)) * 1099511628211ULL;
	std::cout << std::chrono::duration<double, std::milli>(built - start).count() << ' '
		<< std::chrono::duration<double, std::milli>(scanned - built).count() << ' ' << count << ' ' << checksum << '\n';
}
