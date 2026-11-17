#include "data_structure/weighted_union_find.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <random>
#include <utility>
#include <vector>

using namespace std;

std::uint64_t xor_op(std::uint64_t a, std::uint64_t b) { return a ^ b; }
std::uint64_t xor_e() { return 0; }
std::uint64_t xor_inv(std::uint64_t a) { return a; }

struct Permutation {
	std::array<unsigned char, 3> data;
	Permutation() = delete;
	explicit Permutation(std::array<unsigned char, 3> data) : data(data) {}
	bool operator==(const Permutation &) const = default;
};
Permutation perm_op(const Permutation &a, const Permutation &b) {
	return Permutation({a.data[b.data[0]], a.data[b.data[1]], a.data[b.data[2]]});
}
Permutation perm_e() { return Permutation({0, 1, 2}); }
Permutation perm_inv(const Permutation &a) {
	std::array<unsigned char, 3> result{};
	for (int i = 0; i < 3; ++i) result[a.data[i]] = static_cast<unsigned char>(i);
	return Permutation(result);
}

struct Matrix {
	std::array<int, 4> data;
	Matrix() = delete;
	explicit Matrix(std::array<int, 4> data) : data(data) {}
	bool operator==(const Matrix &) const = default;
};
Matrix matrix_op(const Matrix &a, const Matrix &b) {
	std::array<int, 4> result{};
	for (int i = 0; i < 2; ++i)
		for (int j = 0; j < 2; ++j)
			for (int k = 0; k < 2; ++k) result[i * 2 + j] = (result[i * 2 + j] + a.data[i * 2 + k] * b.data[k * 2 + j]) % 101;
	return Matrix(result);
}
Matrix matrix_e() { return Matrix({1, 0, 0, 1}); }
Matrix matrix_inv(const Matrix &a) {
	int det = (a.data[0] * a.data[3] - a.data[1] * a.data[2] + 10201) % 101;
	assert(det != 0);
	int inverse = 1;
	for (int power = 99; power; power >>= 1, det = det * det % 101)
		if (power & 1) inverse = inverse * det % 101;
	return Matrix({a.data[3] * inverse % 101, (101 - a.data[1]) * inverse % 101,
		(101 - a.data[2]) * inverse % 101, a.data[0] * inverse % 101});
}

template <class T, auto Op, auto E, auto Inv>
void check_group(const std::vector<T> &alphabet, unsigned seed) {
	WeightedUnionFind<T, Op, E, Inv> empty(0);
	assert(empty.size() == 0 && empty.count() == 0);
	std::mt19937 random(seed);
	for (int trial = 0; trial < 40; ++trial) {
		int n = 1 + random() % 18, components = n;
		WeightedUnionFind<T, Op, E, Inv> current(n);
		std::vector<std::vector<std::pair<int, T>>> graph(n);
		for (int step = 0; step < 250; ++step) {
			int x = random() % n, y = random() % n;
			std::vector<std::optional<T>> distance(n);
			std::vector<int> queue{x};
			distance[x] = E();
			for (std::size_t i = 0; i < queue.size(); ++i) {
				int v = queue[i];
				for (const auto &[to, w] : graph[v])
					if (!distance[to]) {
						distance[to] = Op(*distance[v], w);
						queue.push_back(to);
					}
			}
			for (int v = 0; v < n; ++v) {
				assert(current.same(x, v) == bool(distance[v]));
				if (distance[v]) {
					assert(current.diff(x, v) == *distance[v]);
					assert(current.diff(v, x) == Inv(*distance[v]));
					int root = current.leader(v);
					assert(current.weight(v) == Op(Inv(*distance[root]), *distance[v]));
				}
			}
			assert(current.size(x) == int(queue.size()));
			T w = distance[y] && random() % 2 ? *distance[y] : alphabet[random() % alphabet.size()];
			bool valid = !distance[y] || *distance[y] == w;
			assert(current.merge(x, y, w) == valid);
			if (valid && !distance[y]) {
				graph[x].emplace_back(y, w);
				graph[y].emplace_back(x, Inv(w));
				--components;
			}
			assert(current.size() == n && current.vertex_count() == n);
			assert(current.count() == components && current.component_count() == components);
		}
		auto copy = current;
		for (int v = 0; v < n; ++v) {
			assert(copy.same(0, v) == current.same(0, v));
			if (copy.same(0, v)) assert(copy.diff(0, v) == current.diff(0, v));
		}
	}
}

void custom_operations() {
	std::vector<Permutation> permutations;
	std::array<unsigned char, 3> p{0, 1, 2};
	do { permutations.emplace_back(p); } while (std::next_permutation(p.begin(), p.end()));
	assert(perm_op(permutations[1], permutations[2]) != perm_op(permutations[2], permutations[1]));
	for (const auto &a : permutations)
		for (const auto &b : permutations)
			for (const auto &c : permutations) {
				WeightedUnionFind<Permutation, perm_op, perm_e, perm_inv> uf(5);
				assert(uf.merge(0, 1, a));
				assert(uf.merge(2, 3, b));
				assert(uf.merge(0, 2, c));
				auto expected = perm_op(perm_op(perm_inv(a), c), b);
				assert(uf.diff(1, 3) == expected);
				assert(uf.diff(3, 1) == perm_inv(expected));
				assert(uf.merge(4, 3, permutations[1]));
				assert(uf.diff(4, 3) == permutations[1]);
				assert(uf.merge(1, 3, expected));
				assert(!uf.merge(1, 3, perm_op(expected, permutations[1])));
			}
	check_group<Permutation, perm_op, perm_e, perm_inv>(permutations, 9821);
	check_group<std::uint64_t, xor_op, xor_e, xor_inv>({0, 1, 42, UINT64_MAX, UINT64_C(1) << 63}, 8712);
	std::mt19937 random(14321);
	std::vector<Matrix> matrices;
	while (matrices.size() < 50) {
		std::array<int, 4> values{};
		for (int &value : values) value = random() % 101;
		if ((values[0] * values[3] - values[1] * values[2]) % 101 != 0) matrices.emplace_back(values);
	}
	check_group<Matrix, matrix_op, matrix_e, matrix_inv>(matrices, 12812);
}

int main() {
	custom_operations();
	WeightedUnionFind<> empty(0);
	assert(empty.size() == 0 && empty.vertex_count() == 0);
	assert(empty.count() == 0 && empty.component_count() == 0);
	WeightedUnionFind<> uf(6);
	assert(uf.size() == 6 && uf.vertex_count() == 6 && uf.count() == 6);
	assert(uf.merge(0, 1, 3));
	assert(uf.merge(1, 2, -5));
	assert(uf.diff(0, 2) == -2 && uf.diff(2, 0) == 2);
	assert(uf.merge(0, 2, -2));
	assert(!uf.merge(0, 2, 0));
	assert(uf.component_count() == 4);
	assert(uf.diff(0, 2) == -2 && uf.size(1) == 3);
	assert(uf.merge(3, 4, 7));
	assert(uf.merge(3, 0, -9));
	assert(uf.diff(3, 2) == -11 && uf.diff(4, 0) == -16);
	assert(uf.size(4) == 5 && !uf.same(0, 5));
	assert(uf.merge(5, 5, 0) && !uf.merge(5, 5, 1));
	assert(uf.count() == 2);
	auto copy = uf;
	assert(copy.merge(0, 5, 1) && copy.component_count() == 1);
	assert(uf.component_count() == 2);
	empty = copy;
	assert(empty.size() == 6 && empty.count() == 1);
	WeightedUnionFind<mint> modular(3);
	assert(modular.merge(0, 1, mint(-1)));
	assert(modular.merge(1, 2, mint(2)));
	assert(modular.diff(0, 2) == mint(1));
	assert(modular.merge(2, 0, mint(-1)));
	assert(!modular.merge(2, 0, mint(1)));
	WeightedUnionFind<__int128_t> wide(3);
	__int128_t large = __int128_t(1) << 100;
	assert(wide.merge(0, 1, large) && wide.merge(1, 2, large));
	assert(wide.diff(0, 2) == large * 2);
	mt19937 random(123);
	for (int repeat_index = 0; repeat_index < 200; ++repeat_index) {
		int n = 1 + random() % 25;
		WeightedUnionFind<> current(n);
		int components = n;
		vector<vector<pair<int, int64_t>>> g(n);
		for (int repeat_index = 0; repeat_index < 300; ++repeat_index) {
			int x = random() % n, y = random() % n;
			int64_t w = int(random() % 2001) - 1000;
			vector<int> seen(n), todo{x};
			vector<int64_t> distance(n);
			seen[x] = 1;
			for (int i = 0; i < (static_cast<int>(todo.size())); ++i) {
				int v = todo[i];
				for (auto [to, cost] : g[v])
					if (!seen[to]) {
						seen[to] = 1;
						distance[to] = distance[v] + cost;
						todo.push_back(to);
					}
			}
			for (int v = 0; v < n; ++v) {
				assert(current.same(x, v) == bool(seen[v]));
				if (seen[v]) assert(current.diff(x, v) == distance[v]);
			}
			assert(current.size(x) == static_cast<int>(todo.size()));
			bool valid = !seen[y] || distance[y] == w;
			assert(current.merge(x, y, w) == valid);
			if (valid && !seen[y]) {
				--components;
				g[x].emplace_back(y, w);
				g[y].emplace_back(x, -w);
			}
			assert(current.size() == n && current.vertex_count() == n);
			assert(current.count() == components && current.component_count() == components);
		}
	}
	WeightedUnionFind<> chain(100000);
	for (int i = 0; i < 99999; ++i) assert(chain.merge(i, i + 1, 1));
	assert(chain.diff(0, 99999) == 99999 && chain.size(99999) == 100000);
}
