#include "api.hpp"
#include <bit>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <random>
#include <string>
#include <vector>
constexpr std::uint64_t mod = 998244353;
std::uint64_t power(std::uint64_t a, std::uint64_t n) {
	std::uint64_t r = 1;
	for (; n; n >>= 1, a = a * a % mod) if (n & 1) r = r * a % mod;
	return r;
}
std::uint64_t ref_checksum(const std::vector<std::uint64_t> &v) {
	std::uint64_t a = 0x243f6a8885a308d3ULL, b = v.size();
	for (std::size_t i = 0; i < v.size(); ++i) { a += v[i] * (i + 1); b += v[i] * v[i]; }
	return a ^ std::rotl(b, 17);
}
std::uint64_t mix(std::uint64_t a, std::uint64_t b) { return (a ^ b) * 0x100000001b3ULL; }
template<class T, class F>
int run(const std::string &type, const std::string &operation, std::size_t n, int repeats,
	F (*api)(const F &, T), std::uint64_t (*checksum)(const F &)) {
	std::mt19937 random(731);
	F source(n);
	std::vector<std::uint64_t> reference(n);
	for (std::size_t i = 0; i < n; ++i) {
		const std::uint64_t v = random() % mod;
		source[i] = T(v);
		reference[i] = v;
	}
	const auto source_hash = checksum(source);
	if (operation == "add") reference[0] = (reference[0] + 17) % mod;
	if (operation == "sub") reference[0] = (reference[0] + mod - 17) % mod;
	if (operation == "mul") for (auto &v : reference) v = v * 17 % mod;
	if (operation == "div") { const auto inverse = power(17, mod - 2); for (auto &v : reference) v = v * inverse % mod; }
	const auto expected_output = ref_checksum(reference);
	std::uint64_t expected_aggregate = 0xcbf29ce484222325ULL;
	for (int i = 0; i < repeats; ++i) expected_aggregate = mix(expected_aggregate, expected_output);
	std::uint64_t aggregate = 0xcbf29ce484222325ULL;
	const T scalar(17);
	const auto start = std::chrono::steady_clock::now();
	for (int i = 0; i < repeats; ++i) {
		F result;
		result = api(source, scalar);
		aggregate = mix(aggregate, checksum(result));
	}
	const auto stop = std::chrono::steady_clock::now();
	const auto unchanged_source_hash = checksum(source);
	if (aggregate != expected_aggregate || unchanged_source_hash != source_hash) {
		std::cerr << "checksum mismatch\n";
		return 1;
	}
	const double elapsed = std::chrono::duration<double, std::nano>(stop - start).count();
	std::cout << "{\"type\":\"" << type << "\",\"operation\":\"" << operation
		<< "\",\"n\":" << n << ",\"repeats\":" << repeats << ",\"total_ns\":" << elapsed
		<< ",\"ns_per_call\":" << elapsed / repeats << ",\"source_checksum\":\"" << source_hash
		<< "\",\"result_checksum\":\"" << expected_output << "\",\"aggregate_checksum\":\"" << aggregate << "\"}\n";
	return 0;
}
int main(int argc, char **argv) {
	if (argc != 5) return 2;
	const std::string type = argv[1], operation = argv[2];
	const std::size_t n = std::stoull(argv[3]);
	const int repeats = std::stoi(argv[4]);
	if (!n || repeats < 1) return 2;
#define SELECT(T, F, prefix) \
	F (*api)(const F &, T) = nullptr; \
	if (operation == "add") api = prefix##_add; \
	else if (operation == "sub") api = prefix##_sub; \
	else if (operation == "mul") api = prefix##_mul; \
	else if (operation == "div") api = prefix##_div; \
	if (!api) return 2; \
	return run<T, F>(type, operation, n, repeats, api, prefix##_checksum);
	if (type == "public") { SELECT(fps_mint, PublicF, public_fps) }
	if (type == "internal") { SELECT(fps_detail::Mint, InternalF, internal_fps) }
	return 2;
}
