#define PROBLEM "https://judge.yosupo.jp/problem/exp_of_formal_power_series"
#include "../fps/fps.hpp"
#include <iostream>
#include <tuple>

using namespace std;

int main() {
	cin.tie(nullptr)->sync_with_stdio(false);
	int n;
	cin >> n;
	fps f(n);
	for (auto &v : f) cin >> v;
	for (const auto &value : f.exp(n)) cout << value << ' ';
	cout << '\n';
}
