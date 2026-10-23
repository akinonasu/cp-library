#include "string/z_algorithm.hpp"
#include <cstdint>
#include <cstdlib>
#include <random>
#include <string>
#include <vector>

void require(bool ok) { if (!ok) std::abort(); }

template <class T>
std::vector<int> oracle(const std::vector<T> &a) {
	std::vector<int> z(a.size());
	for (int i = 0; i < int(a.size()); ++i) while (z[i] < int(a.size()) - i && a[z[i]] == a[i + z[i]]) ++z[i];
	return z;
}

struct Value {
	int key;
	char unused;
	friend bool operator==(Value a, Value b) { return a.key == b.key; }
};

int main() {
	for (int n = 0; n <= 10; ++n) for (unsigned mask = 0; mask < (1U << n); ++mask) {
		std::string s(n, 'a');
		for (int i = 0; i < n; ++i) s[i] += (mask >> i) & 1;
		auto expected = oracle(std::vector<char>(s.begin(), s.end()));
		require(z_algorithm(s) == expected);
		require(z_algorithm(std::string_view(s)) == expected);
		require(z_algorithm(std::span<const char>(s.data(), s.size())) == expected);
	}
	std::mt19937_64 random(318597);
	for (int n : {0, 1, 2, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65, 127, 128, 129, 1024}) {
		std::vector<std::uint64_t> a(n);
		std::vector<Value> b(n);
		std::string s(n, '\0');
		for (int repeat = 0; repeat < 20; ++repeat) {
			for (int i = 0; i < n; ++i) {
				a[i] = repeat & 1 ? random() : random() % 3;
				b[i] = {int(random() % 3), char(random())};
				s[i] = char(random());
			}
			require(z_algorithm(a) == oracle(a));
			require(z_algorithm(b) == oracle(b));
			require(z_algorithm(s) == oracle(std::vector<char>(s.begin(), s.end())));
		}
	}
	for (int period : {1, 2, 3, 17, 63, 64, 65}) {
		std::string s(131072, 'a');
		for (int i = 0; i < int(s.size()); ++i) s[i] = char(i % period);
		auto z = z_algorithm(s);
		for (int i = 0; i < int(s.size()); ++i) require(z[i] == (i % period == 0 ? int(s.size()) - i : 0));
	}
}
