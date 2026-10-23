#include "fps/multipoint_eval.hpp"
#include <cassert>
#include <random>
#include <vector>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <type_traits>

using namespace std;

namespace fps_scalar_return_test {
constexpr std::uint64_t mod = 998244353;
std::uint64_t checks = 0;
void require(bool condition) {
	++checks;
	if (!condition) { std::cerr << "failed check " << checks << '\n'; std::abort(); }
}
std::uint64_t canonical(long long value) {
	value %= static_cast<long long>(mod);
	return value < 0 ? value + mod : value;
}
std::uint64_t inverse(std::uint64_t value) {
	require(value != 0);
	std::uint64_t result = 1;
	for (std::uint64_t power = mod - 2; power; power >>= 1, value = value * value % mod)
		if (power & 1) result = result * value % mod;
	return result;
}

struct NontrivialMint {
	std::uint64_t encoded = 3 * ::fps_scalar_return_test::mod;
	std::string label = "coefficient-with-nontrivial-storage";
	static inline std::uint64_t inversions = 0;
	static inline std::uint64_t copies = 0;
	static constexpr int mod() { return 998244353; }
	NontrivialMint() = default;
	NontrivialMint(long long value) : encoded(canonical(value) + 3 * ::fps_scalar_return_test::mod) {}
	NontrivialMint(const NontrivialMint &other) : encoded(other.encoded), label(other.label) { ++copies; }
	NontrivialMint(NontrivialMint &&) noexcept = default;
	NontrivialMint &operator=(const NontrivialMint &) = default;
	NontrivialMint &operator=(NontrivialMint &&) noexcept = default;
	static NontrivialMint raw(unsigned value) { return NontrivialMint(value); }
	unsigned val() const { return encoded % ::fps_scalar_return_test::mod; }
	NontrivialMint &operator+=(const NontrivialMint &other) {
		encoded = (std::uint64_t(val()) + other.val()) % ::fps_scalar_return_test::mod + 3 * ::fps_scalar_return_test::mod;
		return *this;
	}
	NontrivialMint &operator-=(const NontrivialMint &other) {
		encoded = (std::uint64_t(val()) + ::fps_scalar_return_test::mod - other.val()) % ::fps_scalar_return_test::mod + 3 * ::fps_scalar_return_test::mod;
		return *this;
	}
	NontrivialMint &operator*=(const NontrivialMint &other) {
		encoded = std::uint64_t(val()) * other.val() % ::fps_scalar_return_test::mod + 3 * ::fps_scalar_return_test::mod;
		return *this;
	}
	NontrivialMint operator-() const { return NontrivialMint(-static_cast<long long>(val())); }
	NontrivialMint inv() const { ++inversions; return NontrivialMint(inverse(val())); }
	friend bool operator==(const NontrivialMint &a, const NontrivialMint &b) { return a.val() == b.val(); }
};
static_assert(!std::is_trivially_copyable_v<NontrivialMint>);
static_assert(sizeof(NontrivialMint) > sizeof(unsigned));

template<class T> using F = FormalPowerSeries<T>;
template<class T> std::vector<std::uint64_t> values(const F<T> &f) {
	std::vector<std::uint64_t> result;
	for (const auto &coefficient : f) result.push_back(coefficient.val());
	return result;
}
template<class T> bool identical(const T &a, const T &b) {
	if constexpr (std::is_same_v<T, NontrivialMint>) return a.encoded == b.encoded && a.label == b.label;
	else return a.v == b.v;
}
template<class T> void untouched(const F<T> &f, const F<T> &saved) {
	require(f.size() == saved.size());
	for (std::size_t i = 0; i < f.size(); ++i) require(identical(f[i], saved[i]));
}
template<class T> F<T> make_series(std::size_t n, int pattern, std::mt19937_64 &random) {
	F<T> f(n);
	const std::array<long long, 8> boundaries{0, 1, -1, 2, static_cast<long long>(mod)-2,
		static_cast<long long>(mod), static_cast<long long>(mod)+1, -static_cast<long long>(mod)-1};
	for (std::size_t i = 0; i < n; ++i) {
		f[i] = pattern == 0 ? 0 : pattern == 1 ? boundaries[i % boundaries.size()] : pattern >= 3 ? boundaries[pattern - 2] : static_cast<long long>(random() % mod);
		if constexpr (std::is_same_v<T, NontrivialMint>) {
			f[i].encoded += (i % 7) * mod;
			f[i].label += ":" + std::to_string(i);
		}
	}
	return f;
}
std::vector<std::uint64_t> expected_scalar(std::vector<std::uint64_t> a, std::uint64_t x, char op) {
	if (op == '+' || op == '-') {
		if (a.empty()) a.push_back(0);
		a[0] = (a[0] + (op == '+' ? x : mod - x)) % mod;
	} else {
		if (op == '/') x = inverse(x);
		for (auto &coefficient : a) coefficient = coefficient * x % mod;
	}
	return a;
}
template<class T> void run_type(const char *name) {
	std::mt19937_64 random(731);
	std::uint64_t scalar_cases = 0, division_cases = 0, adjacent_cases = 0;
	for (std::size_t n : {0, 1, 2, 3, 7, 8, 15, 16, 31, 32, 33, 63, 64, 65, 127, 128, 129, 257, 1024, 4097}) {
		for (int pattern : {0, 1, 2, 3, 4, 5, 6}) {
			if (n != 1 && pattern >= 3) continue;
			const F<T> source = make_series<T>(n, pattern, random);
			const F<T> saved = source;
			const auto *source_data = source.data();
			const auto source_capacity = source.capacity();
			const auto raw_values = values(source);
			for (long long scalar : {0, 1, -1, 17}) {
				const T x(scalar), saved_x(x);
				for (char op : {'+', '-', '*', '/'}) {
					if (op == '/' && scalar == 0) continue;
					const auto expected = expected_scalar(raw_values, canonical(scalar), op);
					NontrivialMint::inversions = 0;
					F<T> result;
					if (op == '+') result = source + x;
					if (op == '-') result = source - x;
					if (op == '*') result = source * x;
					if (op == '/') result = source / x;
					require(values(result) == expected);
					untouched(source, saved);
					require(source.data() == source_data && source.capacity() == source_capacity);
					require(identical(x, saved_x));
					if constexpr (std::is_same_v<T, NontrivialMint>)
						require(NontrivialMint::inversions == (op == '/' ? 1 : 0));
					if (op == '+' || op == '-')
						for (std::size_t i = 1; i < n; ++i) require(identical(result[i], source[i]));
					if (op == '+' || op == '/') {
						F<T> alias = source;
						NontrivialMint::inversions = 0;
						if (op == '+') alias = alias + x;
						else alias = alias / x;
						require(values(alias) == expected);
						if constexpr (std::is_same_v<T, NontrivialMint>)
							require(NontrivialMint::inversions == (op == '/' ? 1 : 0));
					}
					++scalar_cases;
					division_cases += op == '/';
				}
			}
			auto doubled = raw_values, zero = raw_values;
			for (auto &coefficient : doubled) coefficient = 2 * coefficient % mod;
			for (auto &coefficient : zero) coefficient = 0;
			NontrivialMint::copies = 0;
			require(values(source + source) == doubled);
			if constexpr (std::is_same_v<T, NontrivialMint>) require(NontrivialMint::copies == n);
			NontrivialMint::copies = 0;
			require(values(source - source) == zero);
			if constexpr (std::is_same_v<T, NontrivialMint>) require(NontrivialMint::copies == n);
			untouched(source, saved);
			adjacent_cases += 2;
			std::vector<std::size_t> shifts{0, n, n + 1};
			if (n) shifts.push_back(n - 1);
			for (std::size_t shift : shifts) {
				std::vector<std::uint64_t> left(n), right(n);
				for (std::size_t i = 0; i < n; ++i) {
					if (i >= shift) left[i] = raw_values[i - shift];
					if (i + shift < n) right[i] = raw_values[i + shift];
				}
				NontrivialMint::copies = 0;
				require(values(source << static_cast<int>(shift)) == left);
				if constexpr (std::is_same_v<T, NontrivialMint>) if (shift >= n) require(NontrivialMint::copies == n);
				NontrivialMint::copies = 0;
				require(values(source >> static_cast<int>(shift)) == right);
				if constexpr (std::is_same_v<T, NontrivialMint>) if (shift >= n) require(NontrivialMint::copies == n);
				untouched(source, saved);
				adjacent_cases += 2;
			}
		}
	}
	for (std::size_t n : {0, 1, 3, 17}) for (std::size_t m : {0, 1, 3, 17}) {
		const F<T> a = make_series<T>(n, 2, random), b = make_series<T>(m, 1, random);
		const F<T> saved_a = a, saved_b = b;
		auto av = values(a), bv = values(b);
		std::vector<std::uint64_t> sum(std::max(n, m)), difference(sum.size());
		for (std::size_t i = 0; i < sum.size(); ++i) {
			const auto x = i < n ? av[i] : 0, y = i < m ? bv[i] : 0;
			sum[i] = (x + y) % mod;
			difference[i] = (x + mod - y) % mod;
		}
		require(values(a + b) == sum);
		require(values(a - b) == difference);
		untouched(a, saved_a);
		untouched(b, saved_b);
		adjacent_cases += 2;
	}
	std::cout << name << ": scalar_cases=" << scalar_cases << " division_cases=" << division_cases
		<< " adjacent_cases=" << adjacent_cases << '\n';
}
static_assert(std::is_same_v<fps_mint, mint>);
void run() {
	run_type<fps_mint>("fps_mint");
	run_type<NontrivialMint>("NontrivialMint");
	std::cout << "PASS scalar checks=" << checks << '\n';
}
}

int main() {
	fps_scalar_return_test::run();
	mt19937 random(123);
	for (int n : {1, 2, 3, 8, 31, 32, 63, 64, 127, 128, 257}) {
		fps a(n), b(n), expected(2 * n - 1);
		for (int i = 0; i < n; ++i) a[i] = random(), b[i] = random();
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < n; ++j) expected[i + j] += a[i] * b[j];
		assert(a * b == expected);
		a[0] = 1;
		fps inverse = a.inv(n), product = a * inverse;
		assert(product[0] == mint(1));
		for (int i = 1; i < n; ++i) assert(product[i] == mint(0));
		assert(a.log(n).exp(n) == a);
		auto square = (a * a).pre(n);
		auto root = square.sqrt(n);
		assert(root.size() == static_cast<std::size_t>(n) && (root * root).pre(n) == square);
		fps power(n); power[0] = 1;
		for (int k = 0; k <= 5; ++k) {
			assert(a.pow(k) == power);
			power = (power * a).pre(n);
		}
		auto [quotient, remainder] = (a * b).divmod(a);
		assert(quotient == b && remainder.empty());
		auto recovered = a.diff().integral();
		recovered[0] = a[0];
		assert(recovered == a);
		vector<mint> xs(n);
		for (int i = 0; i < n; ++i) xs[i] = random();
		auto values = multipoint_eval(a, xs);
		for (int i = 0; i < n; ++i) {
			mint value = 0;
			for (int j = (n)-1; j >= 0; --j) value = value * xs[i] + a[j];
			assert(values[i] == value);
		}
	}
}
