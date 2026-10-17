#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_taylor_shift"
#include "fps/taylor_shift.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n; mint c; cin >> n >> c;
	fps f(n);
	for (auto &x : f) cin >> x;
	for (auto x : taylor_shift(std::move(f), c)) cout << x << ' ';
	cout << '\n';
}
