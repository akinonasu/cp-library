#include "math/floor_sum/prefix_min_of_mod_of_linear.hpp"
#include "math/floor_sum/min_of_mod_of_linear.hpp"
#include "baseline.hpp"
#include "reference.hpp"
#include "inlined.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using U = std::uint64_t;
using Segment = ModLinearPrefixMinSegment;
using Input = std::array<U, 4>;

U fingerprint(const auto &s) { return s.x + s.y * 3 + s.dx * 5 + s.dy * 7 + s.count * 11; }

std::vector<Segment> reference_parts(U n, U m, U a, U b) {
	if (!n) return {};
	a %= m, b %= m;
	auto [x, dx] = reference_article::min_of_linear_segments(a, b, m);
	std::vector<Segment> result;
	for (std::size_t i = 0; i < dx.size(); ++i) {
		U start = x[i], step = dx[i], end = std::min<U>(x[i + 1], n - 1);
		if (start >= n || start + step >= n) break;
		U count = (end - start) / step, dy = m - a * step % m;
		if (i == 0) result.push_back({0, b, step, dy, count + 1});
		else result.push_back({start + step, (a * (start + step) + b) % m, step, dy, count});
		if (U(x[i + 1]) >= n) break;
	}
	if (result.empty()) result.push_back({0, b, 0, 0, 1});
	return result;
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string variant = argv[1], kind = argv[2];
	int count = std::stoi(argv[3]);
	std::mt19937_64 random(std::stoull(argv[4]));
	std::vector<Input> inputs;
	bool wide = kind == "wide" || kind == "wide_short" || kind == "fibonacci64" || kind == "descending64";
	for (int i = 0; i < count; ++i) {
		U m = wide ? random() | 1 : random() % 1000000000 + 1;
		U a = random() % m, b = random() % m, n = random() % m + 1;
		if (kind == "full") n = m;
		if (kind == "short" || kind == "wide_short") n = random() % 64 + 1;
		if (kind == "fibonacci") m = 701408733, a = 433494437, b %= m, n = m;
		if (kind == "fibonacci64") m = 12200160415121876738ULL, a = 7540113804746346429ULL, b %= m, n = m;
		if (kind == "descending64") a = m - 1, b = m - 1, n = m;
		if (kind == "degenerate") {
			if (i % 4 == 0) a = 0;
			if (i % 4 == 1) b = 0;
			if (i % 4 == 2) n = 0;
			if (i % 4 == 3) n = 1;
		}
		inputs.push_back({n, m, a, b});
	}
	if (variant == "check") {
		for (auto [n, m, a, b] : inputs) {
			auto actual = prefix_min_of_mod_of_linear(n, m, a, b);
			auto before = prefix_min_baseline(n, m, a, b);
			auto inlined = prefix_min_inlined(n, m, a, b);
			if (actual.size() != before.size()) return 2;
			if (actual.size() != inlined.size()) return 8;
			for (std::size_t j = 0; j < actual.size(); ++j) {
				auto s = before[j];
				if (actual[j] != Segment{s.x, s.y, s.dx, s.dy, s.count}) return 3;
				auto v = inlined[j];
				if (actual[j] != Segment{v.x, v.y, v.dx, v.dy, v.count}) return 9;
			}
			std::vector<Segment> streamed;
			prefix_min_of_mod_of_linear(n, m, a, b, [&](Segment s) { streamed.push_back(s); });
			if (actual != streamed) return 4;
			if (!wide) {
				if (actual != reference_parts(n, m, a, b)) return 5;
				if (n && reference_fastest::LinearModMin(n, m, a, b) !=
					actual.back().y - actual.back().dy * (actual.back().count - 1)) return 6;
			}
		}
		std::cout << "verified " << kind << ' ' << count << '\n';
		return 0;
	}
	U digest = 0, blocks = 0;
	std::vector<Segment> reusable;
	reusable.reserve(64);
	auto begin = std::chrono::steady_clock::now();
	for (auto [n, m, a, b] : inputs) {
		if (variant == "baseline") {
			auto result = prefix_min_baseline(n, m, a, b);
			blocks += result.size();
			for (const auto &s : result) digest += fingerprint(s);
		} else if (variant == "vector") {
			auto result = prefix_min_of_mod_of_linear(n, m, a, b);
			blocks += result.size();
			for (const auto &s : result) digest += fingerprint(s);
		} else if (variant == "reserved" || variant == "reuse") {
			std::vector<Segment> local;
			if (variant == "reserved") local.reserve(8);
			auto &result = variant == "reuse" ? reusable : local;
			result.clear();
			prefix_min_of_mod_of_linear(n, m, a, b, [&](Segment s) { result.push_back(s); });
			blocks += result.size();
			for (const auto &s : result) digest += fingerprint(s);
		} else if (variant == "callback") {
			prefix_min_of_mod_of_linear(n, m, a, b, [&](Segment s) { ++blocks; digest += fingerprint(s); });
		} else if (variant == "inlined") {
			auto result = prefix_min_inlined(n, m, a, b);
			blocks += result.size();
			for (const auto &s : result) digest += fingerprint(s);
		} else if (variant == "inlined_callback") {
			prefix_min_inlined(n, m, a, b, [&](InlinedSegment s) { ++blocks; digest += fingerprint(s); });
		} else if (variant == "article") {
			if (!n) continue;
			auto [x, dx] = reference_article::min_of_linear_segments(a, b, m);
			blocks += x.size() + dx.size();
			for (int v : x) digest += v;
			for (int v : dx) digest += U(v) * 5;
		} else if (variant == "min_only") {
			if (n) digest += reference_fastest::LinearModMin(n, m, a, b);
		} else return 7;
	}
	double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
	std::cout << "{\"variant\":\"" << variant << "\",\"kind\":\"" << kind << "\",\"count\":" << count
		<< ",\"seconds\":" << elapsed << ",\"blocks\":" << blocks << ",\"digest\":\"" << digest << "\"}\n";
}
