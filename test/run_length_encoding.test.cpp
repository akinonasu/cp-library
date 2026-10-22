#define PROBLEM "https://atcoder.jp/contests/abc019/tasks/abc019_2"
#include "sequence/run_length_encoding.hpp"
#include <iostream>
#include <string>

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(nullptr);
	std::string s;
	std::cin >> s;
	for (auto [value, count] : run_length_encode(s)) std::cout << value << count;
	std::cout << '\n';
}
