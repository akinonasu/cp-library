#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "fps/linear_recurrence.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int d; long long k; cin >> d >> k;
	vector<mint> a(d), c(d);
	for (auto &x : a) cin >> x;
	for (auto &x : c) cin >> x;
	cout << linear_recurrence(a, c, k) << '\n';
}
