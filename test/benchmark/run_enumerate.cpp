#include "reference.hpp"
#include "string/run_enumerate.hpp"
#include "o3.hpp"
#include "zero_initialized.hpp"
#include "word.hpp"
#include "no_index.hpp"
#include "vectorized.hpp"
#include <chrono>
#include <iostream>
#include <random>
#include <string>

struct Symbol {
	unsigned char value;
	friend auto operator<=>(const Symbol &, const Symbol &) = default;
};

std::string input(std::string kind, int n) {
	std::mt19937_64 random(713);
	std::string s(n, 'a');
	if (kind == "random" || kind == "binary") for (char &c : s) c += random() % (kind == "random" ? 26 : 2);
	else if (kind == "periodic") for (int i = 0; i < n; ++i) s[i] += i % 7;
	else if (kind == "thue") for (int i = 0; i < n; ++i) s[i] += std::popcount(unsigned(i)) % 2;
	else if (kind == "fibonacci") {
		std::string a = "a", b = "ab";
		while (int(b.size()) < n) { auto next = b + a; a = std::move(b); b = std::move(next); }
		s = b.substr(0, n);
	} else if (kind == "blocks") {
		for (int i = 0; i < n; i += 257) s[i] = 'b';
	} else if (kind == "near_periodic") {
		std::string block(1024, 'a');
		for (char &c : block) c += random() % 2;
		for (int i = 0; i < n; ++i) s[i] = block[i % 1024];
		for (int i = 1023; i < n; i += 8191) s[i] = 'c';
	}
	return s;
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string name = argv[1], kind = argv[2];
	int n = std::stoi(argv[3]), repetitions = std::stoi(argv[4]);
	auto s = input(kind, n);
	std::vector<Symbol> symbols;
	for (unsigned char c : s) symbols.push_back({c});
	std::uint64_t digest = 0;
	std::size_t count = 0;
	int fallbacks = 0;
	auto record = [&](const auto &runs) {
		count = runs.size();
		for (auto run : runs) {
			if constexpr (requires { run.l; }) digest = digest * 1000000007 + std::uint64_t(run.period) * 1000003 + std::uint64_t(run.l) * 1009 + run.r;
			else if constexpr (requires { run.first; }) digest = digest * 1000000007 + std::uint64_t(run.period) * 1000003 + std::uint64_t(run.first) * 1009 + run.last;
			else digest = digest * 1000000007 + std::uint64_t(run[0]) * 1000003 + std::uint64_t(run[1]) * 1009 + run[2] + 1;
		}
	};
	auto begin = std::chrono::steady_clock::now();
	for (int rep = 0; rep < repetitions; ++rep) {
		if (name == "current") record(run_enumerate(s));
		else if (name == "o3") record(run_enumerate_o3(s));
		else if (name == "zero_initialized") record(run_enumerate_zero_initialized(s));
		else if (name == "word") record(run_enumerate_word(s));
		else if (name == "no_index") record(run_enumerate_no_index(s));
		else if (name == "vectorized") record(run_enumerate_vectorized(s));
		else if (name == "scalar") record(run_enumerate(symbols));
		else if (name == "reference_avx2" || name == "reference_scalar") {
			if (name == "reference_avx2") record(canard::run_enumerate(std::string_view(s), canard::execution::avx2{}));
			else record(canard::run_enumerate(std::string_view(s)));
		} else if (name == "hash") {
			record(reference_hash::calculate_runs(s));
		} else {
			std::vector<Run> runs;
			auto span = std::span<const char>(s.data(), s.size());
			unsigned budget = name == "budget8" ? 8 : name == "budget32" ? 32 : 64;
			if (name == "divide" || !run_enumerate_detail::lyndon(span, runs, std::uint64_t(n) * budget)) {
				++fallbacks;
				runs.clear();
				run_enumerate_detail::divide(span, runs);
			}
			if (name == "comparison_sort") {
				std::sort(runs.begin(), runs.end());
				runs.erase(std::unique(runs.begin(), runs.end()), runs.end());
			} else run_enumerate_detail::sort_runs(runs, n);
			record(runs);
		}
	}
	double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count() / repetitions;
	std::cout << "{\"variant\":\"" << name << "\",\"kind\":\"" << kind << "\",\"n\":" << n
		<< ",\"seconds\":" << seconds << ",\"runs\":" << count << ",\"fallbacks\":" << fallbacks
		<< ",\"digest\":\"" << digest << "\"}\n";
}
