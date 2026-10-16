#include "math/fraction.hpp"
#include "fraction_baseline.hpp"
#include "variants.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using I = std::int64_t;
using U = std::uint64_t;
using Input = std::array<I, 4>;

bool continued_less(auto a, auto b) {
	I an = a.numerator(), bn = b.numerator();
	if ((an < 0) != (bn < 0)) return an < 0;
	U p = an < 0 ? U(0) - U(an) : U(an), q = a.denominator();
	U r = bn < 0 ? U(0) - U(bn) : U(bn), s = b.denominator();
	bool reverse = an < 0;
	for (;;) {
		U x = p / q, y = r / s;
		if (x != y) return reverse ? x > y : x < y;
		U u = p % q, v = r % s;
		if (!u || !v) return reverse ? u > v : u < v;
		p = q, q = u, r = s, s = v;
		reverse = !reverse;
	}
}

U fingerprint(auto f) { return U(f.numerator()) * 1000000007 + U(f.denominator()); }

template <class F>
int run(const std::vector<Input> &inputs, const std::string &name, const std::string &operation, const std::string &kind) {
	std::vector<std::array<F, 2>> pairs;
	pairs.reserve(inputs.size());
	for (auto [an, ad, bn, bd] : inputs) pairs.push_back({F(an, ad), F(bn, bd)});
	U digest = 0;
	auto begin = std::chrono::steady_clock::now();
	if (operation == "construct") for (auto [an, ad, bn, bd] : inputs) digest += fingerprint(F(an, ad));
	else if (operation == "add") for (auto [a, b] : pairs) digest += fingerprint(a + b);
	else if (operation == "sub") for (auto [a, b] : pairs) digest += fingerprint(a - b);
	else if (operation == "mul") for (auto [a, b] : pairs) digest += fingerprint(a * b);
	else if (operation == "div") for (auto [a, b] : pairs) digest += fingerprint(a / b);
	else if (operation == "compare") for (auto [a, b] : pairs) digest += a < b;
	else if (operation == "continued") for (auto [a, b] : pairs) digest += continued_less(a, b);
	else return 1;
	double elapsed = std::chrono::duration<double>(std::chrono::steady_clock::now() - begin).count();
	std::cout << "{\"variant\":\"" << name << "\",\"operation\":\"" << operation
		<< "\",\"kind\":\"" << kind << "\",\"count\":" << inputs.size()
		<< ",\"seconds\":" << elapsed << ",\"digest\":\"" << digest << "\"}\n";
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 6) return 1;
	std::string name = argv[1], operation = argv[2], kind = argv[3];
	int count = std::stoi(argv[4]);
	std::mt19937_64 random(std::stoull(argv[5]));
	std::vector<Input> inputs;
	for (int i = 0; i < count; ++i) {
		I an = I(random() % 2000000001) - 1000000000, ad = random() % 1000000000 + 1;
		I bn = I(random() % 2000000000) - 1000000000, bd = random() % 1000000000 + 1;
		if (!bn) bn = 1;
		if (kind == "same_den") bd = ad;
		if (kind == "integer") ad = bd = 1;
		if (kind == "cancel") {
			an = random() % 1000000000 + 1;
			ad = random() % 1000000000 + 1;
			bn = ad, bd = an;
		}
		if (kind == "wide") {
			an = std::bit_cast<I>(random()); bn = std::bit_cast<I>(random());
			ad = I(random() % 9223372036854775807ULL) + 1;
			bd = I(random() % 9223372036854775807ULL) + 1;
		}
		if (kind == "fibonacci") {
			an = 2880067194370816120LL, ad = 4660046610375530309LL;
			bn = 1779979416004714189LL, bd = 2880067194370816120LL;
			if (i & 1) std::swap(an, bn), std::swap(ad, bd);
		}
		inputs.push_back({an, ad, bn, bd});
	}
	if (name == "check") {
		for (auto [an, ad, bn, bd] : inputs) {
			Fraction<> a(an, ad), b(bn, bd);
			FractionBaseline x(an, ad), y(bn, bd);
			FractionEuclid<> e(an, ad), f(bn, bd);
			FractionHybrid<> g(an, ad), h(bn, bd);
			auto same = [](auto a, auto b) { return a.numerator() == b.numerator() && a.denominator() == b.denominator(); };
			if (!same(a, x) || !same(a, e) || !same(a, g)) return 2;
			if ((a < b) != (x < y) || (a < b) != continued_less(a, b)) return 3;
			if (kind != "wide" && kind != "fibonacci") {
				if (!same(a + b, x + y) || !same(a + b, e + f) || !same(a + b, g + h)) return 4;
				if (!same(a - b, x - y) || !same(a - b, e - f) || !same(a - b, g - h)) return 5;
				if (!same(a * b, x * y) || !same(a * b, e * f) || !same(a * b, g * h)) return 6;
				if (!same(a / b, x / y) || !same(a / b, e / f) || !same(a / b, g / h)) return 7;
			}
		}
		std::cout << "verified " << kind << ' ' << count << '\n';
		return 0;
	}
	if (name == "library") return run<Fraction<>>(inputs, name, operation, kind);
	if (name == "baseline") return run<FractionBaseline>(inputs, name, operation, kind);
	if (name == "euclid") return run<FractionEuclid<>>(inputs, name, operation, kind);
	if (name == "hybrid") return run<FractionHybrid<>>(inputs, name, operation, kind);
	return 1;
}
