#include "geometry/convex.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using P = geometry::IPoint;

std::vector<P> make_points(const std::string &kind, int n, std::uint64_t seed) {
	std::mt19937_64 random(seed);
	std::vector<P> points(n);
	int width = std::max(1, int(std::sqrt(n)));
	for (int i = 0; i < n; ++i) {
		if (kind == "random") points[i] = {std::int64_t(random() % 2000000001) - 1000000000, std::int64_t(random() % 2000000001) - 1000000000};
		else if (kind == "collinear") points[i] = {i, 3LL * i};
		else if (kind == "grid") points[i] = {i % width, i / width};
		else if (kind == "duplicates") points[i] = {std::int64_t(random() % 16), std::int64_t(random() % 16)};
		else if (kind == "parabola") { std::int64_t x = i - n / 2; points[i] = {x, x * x}; }
		else std::abort();
	}
	std::shuffle(points.begin(), points.end(), random);
	return points;
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string operation = argv[1], kind = argv[2];
	int n = std::stoi(argv[3]);
	std::uint64_t seed = std::stoull(argv[4]);
	if (n < 2) return 1;
	auto points = make_points(kind, n, seed);
	std::uint64_t digest = 0;
	std::size_t count = 0;
	auto begin = std::chrono::steady_clock::now();
	if (operation == "hull") {
		auto hull = geometry::convex_hull(points);
		count = hull.size();
		for (auto p : hull) digest = digest * 1000000007 + std::uint64_t(p.x) * 1009 + std::uint64_t(p.y);
	} else {
		auto pair = operation == "closest" ? geometry::closest_pair(points) : geometry::furthest_pair(points);
		if (!pair || pair->first == pair->second) std::abort();
		if (geometry::norm2(points[pair->first] - points[pair->second]) != pair->squared_distance) std::abort();
		count = 2;
		digest = std::uint64_t(pair->squared_distance) ^ (std::uint64_t(pair->first) << 32) ^ std::uint64_t(pair->second);
	}
	auto end = std::chrono::steady_clock::now();
	double milliseconds = std::chrono::duration<double, std::milli>(end - begin).count();
	std::cout << "{\"operation\":\"" << operation << "\",\"kind\":\"" << kind << "\",\"n\":" << n
		<< ",\"seed\":" << seed << ",\"milliseconds\":" << milliseconds << ",\"count\":" << count << ",\"digest\":\"" << digest << "\"}\n";
}
