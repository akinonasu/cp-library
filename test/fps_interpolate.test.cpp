#define PROBLEM "https://judge.yosupo.jp/problem/polynomial_interpolation"
#include "fps/interpolate.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n; cin >> n;
	vector<mint> xs(n), ys(n);
	for (auto &x : xs) cin >> x;
	for (auto &y : ys) cin >> y;
	for (auto x : interpolate(xs, ys)) cout << x << ' ';
	cout << '\n';
}
