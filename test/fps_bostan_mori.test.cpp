#define PROBLEM "https://judge.yosupo.jp/problem/kth_term_of_linearly_recurrent_sequence"
#include "fps/bostan_mori.hpp"
#include <iostream>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int d; long long k; cin >> d >> k;
	fps a(d), q(d + 1); q[0] = 1;
	for (auto &x : a) cin >> x;
	for (int i = 1; i <= d; ++i) { mint c; cin >> c; q[i] = -c; }
	fps p = (a * q).pre(d);
	cout << bostan_mori(std::move(p), std::move(q), k) << '\n';
}
