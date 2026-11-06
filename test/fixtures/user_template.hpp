#ifndef CP_LIBRARY_USER_TEMPLATE_FIXTURE_HPP
#define CP_LIBRARY_USER_TEMPLATE_FIXTURE_HPP

#if defined(CP_LIBRARY_TEST_WITH_ATCODER)
#include <atcoder/all>
#endif
#if __has_include(<bits/stdc++.h>)
#include <bits/stdc++.h>
#endif
#include <algorithm>
#include <array>
#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <type_traits>
#include <utility>
#include <vector>
using namespace std;
using std::cin, std::cout, std::cerr;
#define vec vector
typedef long double ld;
#if LONG_MAX == 2147483647L
#undef long
#define long long long
#endif
#define uint unsigned int
#define ulong unsigned long
#define overload3(a, b, c, name, ...) name
#define rep3(i, a, b) for (int i = (a); i < (b); i++)
#define rep2(i, n) rep3(i, 0, n)
#define rep1(n) rep2(__i__, n)
#define rep(...) overload3(__VA_ARGS__, rep3, rep2, rep1)(__VA_ARGS__)
#define per3(i, a, b) for (int i = (b) - 1; i >= (a); i--)
#define per2(i, n) per3(i, 0, n)
#define per1(n) per2(__i__, n)
#define per(...) overload3(__VA_ARGS__, per3, per2, per1)(__VA_ARGS__)
#define CP_ALL_SELECT(first, ...) first
#define CP_ALL_RANGE(a) std::begin(a), std::end(a)
#define all(...) CP_ALL_SELECT(__VA_OPT__(CP_ALL_RANGE,) all)(__VA_ARGS__)
#define UNIQUE(a) sort(all(a)), a.erase(unique(all(a)), a.end()), a.shrink_to_fit()
#define sz(a) static_cast<int>(a.size())
#ifndef DEBUG
#define cerr if (false) cerr
#undef endl
#define endl '\n'
#endif
inline auto operator^=(vector<bool>::reference a, bool b) noexcept {
	if (b) a.flip();
	return a;
}
inline auto operator&=(vector<bool>::reference a, bool b) noexcept {
	if (!b) a = false;
	return a;
}
inline auto operator|=(vector<bool>::reference a, bool b) noexcept {
	if (b) a = true;
	return a;
}
inline ostream &operator<<(ostream &os, __int128_t value) {
	ostream::sentry s(os);
	if (s) {
		__uint128_t tmp = value;
		if (value < 0) tmp = -tmp;
		char buffer[128];
		char *d = std::end(buffer);
		do { --d; *d = "0123456789"[tmp % 10]; tmp /= 10; } while (tmp != 0);
		if (value < 0) --d, *d = '-';
		const int len = std::end(buffer) - d;
		if (os.rdbuf()->sputn(d, len) != len) os.setstate(std::ios_base::badbit);
	}
	return os;
}
template <typename T, typename S>
ostream &operator<<(ostream &os, pair<T, S> a) { return os << a.first << ' ' << a.second; }
template <typename T>
ostream &operator<<(ostream &os, vector<T> a) {
	const int n = a.size();
	rep(i, n) { os << a[i]; if (i + 1 != n) os << " "; }
	return os;
}
template <typename T, typename S>
istream &operator>>(istream &is, pair<T, S> &a) { return is >> a.first >> a.second; }
template <typename T, size_t n>
ostream &operator<<(ostream &os, array<T, n> a) {
	rep(i, n) { os << a[i]; if (i + 1 != n) os << " "; }
	return os;
}
template <typename T>
istream &operator>>(istream &is, vector<T> &a) { for (T &i : a) is >> i; return is; }
template <typename T, typename S>
bool chmin(T &x, S y) { if ((T)y < x) { x = (T)y; return true; } return false; }
template <typename T, typename S>
bool chmax(T &x, S y) { if (x < (T)y) { x = (T)y; return true; } return false; }
template <typename T>
void operator++(vector<T> &a) { for (T &i : a) ++i; }
template <typename T>
void operator--(vector<T> &a) { for (T &i : a) --i; }
template <typename T>
void operator++(vector<T> &a, int) { for (T &i : a) i++; }
template <typename T>
void operator--(vector<T> &a, int) { for (T &i : a) i--; }
void init(), solve();
int main() {
	cin.tie(nullptr);
	ios::sync_with_stdio(false);
	cout << fixed << setprecision(20);
	cerr << fixed << setprecision(20);
	init();
	int t = 1;
	while (t--) solve();
}

#endif
