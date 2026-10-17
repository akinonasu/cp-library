#define PROBLEM "https://judge.yosupo.jp/problem/composition_of_formal_power_series"
#include "fps/compose.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n; cin >> n;
	fps f(n), g(n);
	for (auto &x : f) cin >> x;
	for (auto &x : g) cin >> x;
	for (auto x : compose(f, g)) cout << x << ' ';
	cout << '\n';
}
