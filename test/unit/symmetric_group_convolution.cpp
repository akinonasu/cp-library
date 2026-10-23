#include "convolution/symmetric_group_convolution.hpp"
#include "test/fixtures/recurrence_types.hpp"
#ifdef TEST_SYMMETRIC_GROUP_ACL
#include <atcoder/modint>
#endif
#include <algorithm>
#include <cassert>
#include <map>
#include <numeric>
#include <random>

std::vector<std::vector<int>> permutations(int n) {
	std::vector<int> p(n);
	std::iota(p.begin(), p.end(), 0);
	std::vector<std::vector<int>> result;
	do { result.push_back(p); } while (std::next_permutation(p.begin(), p.end()));
	return result;
}

template <class Mint>
std::vector<Mint> naive(int n, const std::vector<Mint> &a, const std::vector<Mint> &b) {
	auto ps = permutations(n);
	std::map<std::vector<int>, int> index;
	for (int i = 0; i < int(ps.size()); ++i) index[ps[i]] = i;
	std::vector<Mint> result(a.size());
	std::vector<int> p(n);
	for (int i = 0; i < int(a.size()); ++i) for (int j = 0; j < int(b.size()); ++j) {
		for (int k = 0; k < n; ++k) p[k] = ps[i][ps[j][k]];
		result[index.at(p)] += a[i] * b[j];
	}
	return result;
}

template <class Mint>
void check() {
	std::mt19937 random(714963);
	for (int n = 0; n <= 6; ++n) {
		SymmetricGroupFFT<Mint> fft(n);
		assert(fft.degree() == n);
		int size = 1, total = 0;
		for (int i = 1; i <= n; ++i) size *= i;
		assert(fft.size() == size);
		for (const auto &block : fft.blocks()) {
			assert(block.offset == total);
			assert(std::accumulate(block.partition.begin(), block.partition.end(), 0) == n);
			total += block.dimension * block.dimension;
		}
		assert(total == size);
		std::vector<Mint> a(size), b(size);
		for (auto &x : a) x = std::int64_t(random()) - 2147483648LL;
		for (auto &x : b) x = random();
		assert(fft.inverse(fft.forward(a)) == a);
		assert(fft.forward(fft.inverse(a)) == a);
		assert(symmetric_group_convolution(fft, a, b) == naive(n, a, b));
		assert(symmetric_group_convolution(a, b) == naive(n, a, b));
		if (n <= 3) {
			for (int i = 0; i < size; ++i) for (int j = 0; j < size; ++j) {
				std::fill(a.begin(), a.end(), Mint(0));
				std::fill(b.begin(), b.end(), Mint(0));
				a[i] = b[j] = 1;
				assert(symmetric_group_convolution(fft, a, b) == naive(n, a, b));
			}
		}
	}
}

int main() {
	check<mint>();
	check<RecurrenceModint<1000000007>>();
	check<RecurrenceModint<17>>();
#ifdef TEST_SYMMETRIC_GROUP_ACL
	check<atcoder::modint1000000007>();
	atcoder::modint::set_mod(101);
	check<atcoder::modint>();
#endif
	std::mt19937 random(2189);
	for (int n : {7, 8, 9}) {
		SymmetricGroupFFT<> fft(n);
		std::vector<mint> a(fft.size()), unit(fft.size());
		for (auto &x : a) x = random();
		unit[0] = 1;
		assert(fft.inverse(fft.forward(a)) == a);
		assert(fft.forward(fft.inverse(a)) == a);
		assert(symmetric_group_convolution(fft, a, unit) == a);
		assert(symmetric_group_convolution(fft, unit, a) == a);
		std::fill(a.begin(), a.end(), mint(3));
		std::fill(unit.begin(), unit.end(), mint(5));
		assert(symmetric_group_convolution(fft, a, unit) == std::vector<mint>(fft.size(), mint(15) * mint(fft.size())));
	}
	{
		SymmetricGroupFFT<> fft(10);
		std::vector<mint> a(fft.size());
		for (auto &x : a) x = random();
		assert(fft.inverse(fft.forward(a)) == a);
	}
	{
		std::vector<mint> a(6), b(6, mint(123));
		assert(symmetric_group_convolution(a, b) == a);
		bool caught = false;
		try { (void)symmetric_group_convolution(a, std::vector<mint>(2)); }
		catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	for (int degree : {-1, 11}) {
		bool caught = false;
		try { SymmetricGroupFFT<> fft(degree); } catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	for (int length : {0, 3, 7}) {
		bool caught = false;
		try { (void)symmetric_group_convolution(std::vector<mint>(length), std::vector<mint>(length)); }
		catch (const std::invalid_argument &) { caught = true; }
		assert(caught);
	}
	bool caught = false;
	try { (void)SymmetricGroupFFT<>(3).inverse(std::vector<mint>(5)); }
	catch (const std::invalid_argument &) { caught = true; }
	assert(caught);
}
