#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <string_view>
#include <vector>
extern "C" int baseline_call(std::string_view, std::string_view);
extern "C" int candidate_call(std::string_view, std::string_view);
using Call = int (*)(std::string_view, std::string_view);
struct Case {
	std::string a, b;
	std::size_t oa, ob, na, nb;
	int expected;
	std::string_view s() const { return {a.data() + oa, na}; }
	std::string_view t() const { return {b.data() + ob, nb}; }
};
struct Family { std::string name; int repeats; std::vector<Case> cases; };
int naive(std::string_view s, std::string_view t) {
	std::vector<int> row(t.size() + 1);
	std::iota(row.begin(), row.end(), 0);
	for (char c : s) {
		int diagonal = row[0]++;
		for (std::size_t j = 0; j < t.size(); ++j) {
			int old = row[j + 1];
			row[j + 1] = std::min({old + 1, row[j] + 1, diagonal + (c != t[j])});
			diagonal = old;
		}
	}
	return row.back();
}
Case make_case(std::string a, std::string b, int expected = -1, std::size_t oa = 0, std::size_t ob = 0) {
	if (expected < 0) expected = naive(a, b);
	auto na = a.size(), nb = b.size();
	return {std::string(oa, '\x55') + a + std::string(8, '\x33'), std::string(ob, '\x66') + b + std::string(8, '\x44'), oa, ob, na, nb, expected};
}
int main(int argc, char **argv) {
	const unsigned seed = argc > 1 ? unsigned(std::stoul(argv[1])) : 1234;
	std::mt19937_64 rng(seed);
	auto random = [&](int n, int alphabet) { std::string s(n, 0); for (char &c : s) c = char(rng() % alphabet); return s; };
	const auto original = random(200000, 256);
	std::vector<Family> families;
	families.push_back({"identical200k", 2000, {make_case(original, original, 0)}});
	for (auto [name, pos] : std::vector<std::pair<std::string, int>>{{"last_sub200k", 199999}, {"first_sub200k", 0}, {"middle_sub200k", 100000}}) {
		auto changed = original;
		changed[pos] = char(static_cast<unsigned char>(changed[pos]) ^ 0x80);
		families.push_back({name, 2000, {make_case(original, changed, 1)}});
	}
	families.push_back({"prefix199999_200000", 2000, {make_case(original.substr(0, 199999), original, 1)}});
	auto r1 = random(4096, 26), r2 = random(4096, 26);
	if (r2.front() == r1.front()) r2.front() = char((r2.front() + 1) % 26);
	if (r2.back() == r1.back()) r2.back() = char((r2.back() + 1) % 26);
	families.push_back({"random26_4096", 200, {make_case(r1, r2)}});
	families.push_back({"disjoint200k", 1000, {make_case(std::string(200000, 'a'), std::string(200000, 'b'), 200000)}});
	Family short_cases{"short_corpus", 1000, {}};
	std::vector<int> lengths;
	for (int n = 0; n <= 17; ++n) lengths.push_back(n);
	for (int n : {63, 64, 65}) lengths.push_back(n);
	for (int n : lengths) for (int off = 0; off < 8; ++off) for (int variant = 0; variant < 8; ++variant) {
		auto a = random(n, 256), b = a;
		if (variant >= 1 && variant <= 3 && n) b[variant == 1 ? 0 : variant == 2 ? n - 1 : n / 2] ^= char(0x80);
		if (variant == 4 && n) b.pop_back();
		if (variant == 5) b.insert(b.begin() + n / 2, char(rng() % 256));
		if (variant == 6) b = random(n, 256);
		if (variant == 7) { a.assign(n, '\0'); b.assign(n, char(0xff)); }
		short_cases.cases.push_back(make_case(a, b, -1, off, 7 - off));
	}
	families.push_back(std::move(short_cases));
	std::uint64_t corpus_hash = 1469598103934665603ULL;
	for (auto &f : families) for (auto &c : f.cases) {
		for (auto s : {c.s(), c.t()}) for (unsigned char x : s) corpus_hash = (corpus_hash ^ x) * 1099511628211ULL;
		for (Call fn : {baseline_call, candidate_call}) {
			int got = fn(c.s(), c.t());
			if (got != c.expected) { std::cerr << "oracle mismatch " << f.name << " got=" << got << " expected=" << c.expected << '\n'; return 2; }
		}
	}
	std::cerr << "seed=" << seed << " corpus_hash=" << corpus_hash << " short_cases=" << families.back().cases.size() << '\n';
	std::cout << "family,pair,order,implementation,repeats,cases,calls,elapsed_ns,ns_per_call,checksum,expected_checksum\n" << std::setprecision(12);
	for (auto &f : families) for (int pair = 0; pair < 4; ++pair) for (int slot = 0; slot < 2; ++slot) {
		const bool candidate = (slot ^ (pair & 1));
		Call fn = candidate ? candidate_call : baseline_call;
		std::uint64_t checksum = 0, expected = 0;
		for (auto &c : f.cases) expected += std::uint64_t(c.expected) + 1;
		expected *= f.repeats;
		auto start = std::chrono::steady_clock::now();
		for (int rep = 0; rep < f.repeats; ++rep) for (auto &c : f.cases) checksum += std::uint64_t(fn(c.s(), c.t())) + 1;
		auto stop = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
		auto calls = std::uint64_t(f.repeats) * f.cases.size();
		std::cout << f.name << ',' << pair << ',' << (pair & 1 ? "BA" : "AB") << ',' << (candidate ? "B" : "A") << ',' << f.repeats << ',' << f.cases.size() << ',' << calls << ',' << elapsed << ',' << double(elapsed) / calls << ',' << checksum << ',' << expected << '\n' << std::flush;
		if (checksum != expected) { std::cerr << "checksum mismatch\n"; return 3; }
	}
}
