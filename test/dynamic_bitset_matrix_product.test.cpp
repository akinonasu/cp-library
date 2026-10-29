#define PROBLEM "https://judge.yosupo.jp/problem/matrix_product_mod_2"
#include "../data_structure/dynamic_bitset.hpp"
#include "../components/fast_io.hpp"

int main() {
	FastInput in;
	FastOutput out;
	int n = 0, m = 0, k = 0;
	in.read(n); in.read(m); in.read(k);
	std::vector<DynamicBitset> rows;
	rows.reserve(n);
	for (int i = 0; i < n; ++i) rows.emplace_back(in.token());
	std::vector<DynamicBitset> columns(k, DynamicBitset(m));
	for (int i = 0; i < m; ++i) {
		auto row = in.token();
		for (int j = 0; j < k; ++j) if (row[j] == '1') columns[j].set(m - 1 - i);
	}
	std::string row(k, '0');
	for (const auto &bits : rows) {
		for (int j = 0; j < k; ++j) row[j] = char('0' + bits.intersection_parity(columns[j]));
		out.println(row);
	}
}
