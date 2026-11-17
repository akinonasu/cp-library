#define PROBLEM "https://judge.u-aizu.ac.jp/onlinejudge/description.jsp?id=CGL_4_B"
#define ERROR "1e-8"
#include "integer_geometry/convex.hpp"
#include <iomanip>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

std::int64_t read_coordinate() {
	std::string token;
	std::cin >> token;
	std::int64_t value = 0;
	int fractional = -1;
	bool negative = false;
	for (char c : token) {
		if (c == '-') negative = true;
		else if (c == '.') fractional = 0;
		else {
			value = value * 10 + c - '0';
			if (fractional >= 0) ++fractional;
		}
	}
	for (int i = fractional < 0 ? 0 : fractional; i < 6; ++i) value *= 10;
	return negative ? -value : value;
}

int main() {
	std::cin.tie(nullptr)->sync_with_stdio(false);
	std::cout << std::fixed << std::setprecision(15);
	int n = 0;
	std::cin >> n;
	std::vector<integer_geometry::Point> polygon(n);
	for (auto& point : polygon) point = {read_coordinate(), read_coordinate()};
	std::vector<integer_geometry::Point> corners;
	for (int i = 0; i < n; ++i)
		if (integer_geometry::orientation(polygon[(i + n - 1) % n], polygon[i], polygon[(i + 1) % n]) != 0)
			corners.push_back(polygon[i]);
	if (corners.empty()) {
		auto [first, last] = std::minmax_element(polygon.begin(), polygon.end());
		corners = {*first, *last};
	}
	std::cout << std::sqrt(static_cast<long double>(integer_geometry::diameter_squared(corners))) / 1000000 << '\n';
}
