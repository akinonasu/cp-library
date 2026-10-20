#include "../fixtures/beats.hpp"
#include "data_structure/advanced/segtree_beats.hpp"
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>

void require(bool ok) { if (!ok) std::abort(); }

std::size_t division_failures = 0;
bool checked_division_mapping(const division_beats::F &f, division_beats::S &s) {
	if (division_beats::mapping(f, s)) return true;
	++division_failures;
	s.sum = -987654321;
	return false;
}

template<class Tree, class T>
void check(Tree &seg, const std::vector<T> &a) {
	require(seg.size() == static_cast<int>(a.size()) && seg.empty() == a.empty());
	const auto total = seg.all_prod();
	require(total.len == seg.size() && total.sum == std::accumulate(a.begin(), a.end(), decltype(total.sum)(0)));
	if (!a.empty()) require(total.lo == *std::min_element(a.begin(), a.end()) && total.hi == *std::max_element(a.begin(), a.end()));
	const auto values = seg.values();
	require(values.size() == a.size());
	for (int i = 0; i < seg.size(); ++i)
		require(values[i].len == 1 && values[i].sum == a[i] && values[i].lo == a[i] && values[i].hi == a[i] && seg.get(i).sum == a[i]);
}

template<class Tree, class T>
void sum_search(Tree &seg, const std::vector<T> &a, int l, int r, unsigned long long budget) {
	int right = l, left = r;
	unsigned long long sum = 0;
	while (right < seg.size() && sum + a[right] <= budget) sum += a[right++];
	sum = 0;
	while (left && sum + a[left - 1] <= budget) sum += a[--left];
	auto predicate = [&](const auto &s) { return static_cast<unsigned long long>(s.sum) <= budget; };
	require(seg.max_right(l, predicate) == right && seg.min_left(r, predicate) == left);
}

void division_test() {
	using namespace division_beats;
	using Tree = GenericSegtreeBeats<S, op, e, F, checked_division_mapping, composition, id>;
	std::mt19937 rng(423951);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 17, 32, 33, 65, 129}) {
		std::vector<long long> a(n);
		std::vector<S> input;
		for (auto &x : a) x = rng() % 100001, input.push_back(leaf(x));
		Tree seg(input);
		for (int q = 0; q < 5000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			if (q % 7 == 0) l = 0, r = n;
			long long x = rng() % 100001;
			switch (rng() % 7) {
			case 0:
			case 1:
				x = 2 + rng() % 31;
				seg.apply(l, r, {-1, static_cast<unsigned long long>(x)});
				for (int i = l; i < r; ++i) a[i] /= x;
				break;
			case 2:
				seg.apply(l, r, {x, 1});
				std::fill(a.begin() + l, a.begin() + r, x);
				break;
			case 3:
				if (n) {
					int p = rng() % n;
					seg.set(p, leaf(x));
					a[p] = x;
					seg.apply(p, {-1, 3});
					a[p] /= 3;
				}
				break;
			case 4:
				require(seg.prod(l, r).sum == std::accumulate(a.begin() + l, a.begin() + r, 0LL));
				break;
			case 5:
				sum_search(seg, a, l, r, rng() % 1000001);
				break;
			default:
				seg.apply(l, r, id());
				seg.apply(l, l, {123, 1});
			}
			if (q % 97 == 0) check(seg, a);
		}
		check(seg, a);
		auto copy = seg;
		seg.build(std::vector<S>(n, leaf(99999)));
		for (int q = 0; q < 200; ++q) seg.apply(0, n, {-1, 2});
		check(seg, std::vector<long long>(n, 0));
		check(copy, a);
		auto moved = std::move(copy);
		copy.build({});
		check(copy, std::vector<long long>{});
		check(moved, a);
	}
	require(division_failures > 0);
	Tree filled(19, leaf(42));
	check(filled, std::vector<long long>(19, 42));
	Tree empty;
	check(empty, std::vector<long long>{});
}

void bitwise_test() {
	using namespace bitwise_beats;
	std::mt19937 rng(192407);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 17, 63, 65}) {
		std::vector<unsigned> a(n);
		std::vector<S> input;
		for (auto &x : a) x = rng(), input.push_back(leaf(x));
		Tree seg(input);
		for (int q = 0; q < 5000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			if (q % 11 == 0) l = 0, r = n;
			if (q % 3 == 0) {
				auto got = seg.prod(l, r);
				unsigned both = ~0U, either = 0;
				for (int i = l; i < r; ++i) both &= a[i], either |= a[i];
				require(got.bit_and == both && got.bit_or == either && got.sum == std::accumulate(a.begin() + l, a.begin() + r, 0ULL));
				sum_search(seg, a, l, r, static_cast<unsigned long long>(rng()) * 8);
			} else {
				F f = q % 3 == 1 ? F{static_cast<unsigned>(rng()), 0} : F{~0U, static_cast<unsigned>(rng())};
				seg.apply(l, r, f);
				for (int i = l; i < r; ++i) a[i] = (a[i] & f.bit_and) | f.bit_or;
			}
			if (q % 97 == 0) check(seg, a);
		}
		check(seg, a);
	}
}

void clamp_test() {
	using namespace clamp_beats;
	std::mt19937 rng(682493);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 31, 65, 129}) {
		std::vector<long long> a(n);
		std::vector<S> input;
		for (auto &x : a) x = static_cast<int>(rng() % 201) - 100, input.push_back(leaf(x));
		Tree seg(input);
		SegtreeBeats dedicated(a);
		for (int q = 0; q < 5000; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			long long x = static_cast<int>(rng() % 201) - 100;
			F f = id();
			switch (rng() % 6) {
			case 0: f.hi = x; dedicated.chmin(l, r, x); break;
			case 1: f.lo = x; dedicated.chmax(l, r, x); break;
			case 2: f.add = x; dedicated.add(l, r, x); break;
			case 3: f.lo = f.hi = x; dedicated.assign(l, r, x); break;
			default:
				require(seg.prod(l, r).sum == std::accumulate(a.begin() + l, a.begin() + r, 0LL));
				int right = l, left = r;
				while (right < n && a[right] <= x) ++right;
				while (left && a[left - 1] <= x) --left;
				auto predicate = [&](const S &s) { return s.hi <= x; };
				require(seg.max_right(l, predicate) == right && seg.min_left(r, predicate) == left);
			}
			seg.apply(l, r, f);
			for (int i = l; i < r; ++i) a[i] = std::clamp(a[i] + f.add, f.lo, f.hi);
			if (q % 97 == 0) {
				check(seg, a);
				require(dedicated.values() == a);
			}
		}
		check(seg, a);
	}
}

namespace sequence_beats {
struct S {
	std::string text;
	S() = delete;
	explicit S(std::string text) : text(std::move(text)) {}
};
struct F {
	int letter, shift;
	F() = delete;
	F(int letter, int shift) : letter(letter), shift(shift) {}
};
std::size_t failures = 0;
S e() { return S(""); }
S op(const S &a, const S &b) { return S(a.text + b.text); }
F id() { return {-1, 0}; }
F composition(const F &f, const F &g) {
	return f.letter >= 0 ? f : F(g.letter, (g.shift + f.shift) % 26);
}
bool mapping(const F &f, S &a) {
	if (a.text.size() == 2 && (f.letter >= 0 || f.shift)) {
		++failures;
		a.text.clear();
		return false;
	}
	for (char &c : a.text) c = char('a' + ((f.letter >= 0 ? f.letter : c - 'a') + f.shift) % 26);
	return true;
}
using Tree = GenericSegtreeBeats<S, op, e, F, mapping, composition, id>;
}

void sequence_test() {
	using namespace sequence_beats;
	Tree delayed(8, S("a"));
	delayed.apply(F(1, 0));
	require(failures == 0 && delayed.all_prod().text == std::string(8, 'b'));
	require(delayed.get(3).text == "b" && failures > 0);
	std::mt19937 rng(179841);
	for (int n : {0, 1, 2, 3, 7, 8, 9, 17}) {
		std::string a(n, 'a');
		Tree seg(n, S("a"));
		for (int q = 0; q < 1500; ++q) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			F f = rng() % 2 ? F(rng() % 3, 0) : F(-1, rng() % 3);
			if (q % 3 == 0) { l = 0; r = n; seg.apply(f); }
			else seg.apply(l, r, f);
			for (int i = l; i < r; ++i) a[i] = char('a' + ((f.letter >= 0 ? f.letter : a[i] - 'a') + f.shift) % 26);
			if (n && q % 7 == 0) {
				int p = rng() % n;
				seg.set(p, S("a"));
				a[p] = 'a';
				seg.apply(p, F(-1, 1));
				a[p] = 'b';
			}
			require(seg.all_prod().text == a && seg.prod(l, r).text == a.substr(l, r - l));
			auto predicate = [](const S &s) { return s.text.find("ab") == std::string::npos; };
			int right = l, left = r;
			while (right < n && a.substr(l, right - l + 1).find("ab") == std::string::npos) ++right;
			while (left && a.substr(left - 1, r - left + 1).find("ab") == std::string::npos) --left;
			require(seg.max_right(l, predicate) == right && seg.min_left(r, predicate) == left);
			if (q % 97 == 0) {
				auto values = seg.values();
				for (int i = 0; i < n; ++i) require(values[i].text == a.substr(i, 1));
			}
		}
		seg.build({});
		require(seg.empty() && seg.all_prod().text.empty());
	}
}

int main() {
	division_test();
	bitwise_test();
	clamp_test();
	sequence_test();
	const int n = 500000;
	std::vector<division_beats::S> a(n);
	for (int i = 0; i < n; ++i) a[i] = division_beats::leaf(i);
	division_beats::Tree seg(a);
	for (int q = 0; q < 24; ++q) seg.apply(division_beats::F{-1, 2});
	require(seg.all_prod().sum == 0);
	seg.apply(0, n, {100000, 1});
	seg.apply(0, n, {-1, ~0ULL});
	require(seg.get(n - 1).sum == 0 && seg.prod(1, n - 1).sum == 0);
}
