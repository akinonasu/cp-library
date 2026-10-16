#include "../../string/run_enumerate.hpp"
#include <cassert>
#include <cstdint>
#include <random>
#include <string>
#include <vector>

template <class T>
std::vector<Run> brute(const T &s) {
	std::vector<Run> result;
	int n = static_cast<int>(s.size());
	for (int l = 0; l < n; ++l) {
		for (int r = l + 2; r <= n; ++r) {
			int p = 1;
			for (; p <= (r - l) / 2; ++p) {
				int i = l + p;
				while (i < r && s[i] == s[i - p]) ++i;
				if (i == r) break;
			}
			if (p > (r - l) / 2) continue;
			if (l && s[l - 1] == s[l + p - 1]) continue;
			if (r < n && s[r] == s[r - p]) continue;
			result.push_back({p, l, r});
		}
	}
	std::sort(result.begin(), result.end());
	return result;
}

template <class T>
void check(const T &s) {
	auto expected = brute(s);
	assert(run_enumerate(s) == expected);
	using Value = typename T::value_type;
	auto span = std::span<const Value>(s.data(), s.size());
	std::vector<Run> divided;
	run_enumerate_detail::divide(span, divided);
	run_enumerate_detail::sort_runs(divided, static_cast<int>(s.size()));
	assert(divided == expected);
	if (s.size() > 1) {
		std::vector<Run> direct;
		assert(run_enumerate_detail::lyndon(span, direct, std::numeric_limits<std::uint64_t>::max()));
		run_enumerate_detail::sort_runs(direct, static_cast<int>(s.size()));
		assert(direct == expected);
	}
}

struct Symbol {
	int value;
	friend auto operator<=>(const Symbol &, const Symbol &) = default;
};

int main() {
	assert(run_enumerate("aaaa") == std::vector<Run>({{1, 0, 4}}));
	for (const std::string text : {"", "a", "aa", "ab", "ababa", "mississippi", "abcabcabcxababab", "aabaaabaaab"}) check(text);
	for (int alphabet : {2, 3}) {
		for (int n = 0, total = 1; n <= (alphabet == 2 ? 13 : 8); ++n, total *= alphabet) {
			for (int code = 0; code < total; ++code) {
				std::string text(n, 'a');
				for (int i = 0, x = code; i < n; ++i, x /= alphabet) text[i] += x % alphabet;
				check(text);
			}
		}
	}
	std::mt19937_64 random(713);
	for (int trial = 0; trial < 4000; ++trial) {
		int n = random() % 85, alphabet = 1 + random() % 16;
		std::string text(n, 'a');
		std::vector<std::int64_t> integers(n);
		std::vector<Symbol> symbols(n);
		for (int i = 0; i < n; ++i) {
			int value = random() % alphabet;
			text[i] = static_cast<char>(value * 17);
			integers[i] = std::numeric_limits<std::int64_t>::min() + std::int64_t(value) * 1000000007;
			symbols[i] = {value};
		}
		check(text);
		assert(run_enumerate(integers) == run_enumerate(text));
		assert(run_enumerate(symbols) == run_enumerate(text));
		if (trial < 100) {
			check(integers);
			check(symbols);
		}
	}
	for (int n : {511, 512, 513, 1023, 1024, 4097, 200000}) {
		for (int kind = 0; kind < 5; ++kind) {
			std::string text(n, 'a');
			if (kind == 1) for (int i = 0; i < n; ++i) text[i] += i % 7;
			if (kind == 2) for (int i = 0; i < n; ++i) text[i] += std::popcount(unsigned(i)) % 2;
			if (kind == 3) {
				std::string a = "a", b = "ab";
				while (int(b.size()) < n) { auto next = b + a; a = std::move(b); b = std::move(next); }
				text = b.substr(0, n);
			}
			if (kind == 4) for (char &c : text) c += random() % 2;
			auto expected = run_enumerate(text);
			std::vector<Run> divided;
			run_enumerate_detail::divide(std::span<const char>(text.data(), text.size()), divided);
			run_enumerate_detail::sort_runs(divided, n);
			assert(expected == divided);
			assert(expected.size() < text.size());
		}
	}
	for (int trial = 0; trial < 30; ++trial) {
		std::string text(128 + random() % 80, 'a');
		for (int i = 0; i < int(text.size()); i += 1 + random() % 100) text[i] = 'b' + random() % 2;
		check(text);
		std::vector<Run> runs;
		auto span = std::span<const char>(text.data(), text.size());
		if (!run_enumerate_detail::lyndon(span, runs, 0)) {
			runs.clear();
			run_enumerate_detail::divide(span, runs);
		}
		run_enumerate_detail::sort_runs(runs, static_cast<int>(text.size()));
		assert(runs == brute(text));
	}
}
