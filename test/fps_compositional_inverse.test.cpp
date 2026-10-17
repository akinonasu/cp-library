#define PROBLEM "https://judge.yosupo.jp/problem/compositional_inverse_of_formal_power_series"
#include "fps/compositional_inverse.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n; cin >> n;
	fps f(n);
	for (auto &x : f) cin >> x;
	for (auto x : compositional_inverse(f)) cout << x << ' ';
	cout << '\n';
}
