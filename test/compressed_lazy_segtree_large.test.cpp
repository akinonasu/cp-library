#define PROBLEM "https://judge.yosupo.jp/problem/range_affine_range_sum_large_array"
#include "data_structure/segment_tree/lazy_segtree.hpp"
#include "components/fast_io.hpp"
#include "components/radix_sort.hpp"
#include "math/mint.hpp"
#include <cstdint>
#include <vector>

struct Sum { mint sum, length; };
struct Affine { mint a, b; };
Sum op(Sum a, Sum b) { return {a.sum + b.sum, a.length + b.length}; }
Sum e() { return {0, 0}; }
Sum mapping(Affine f, Sum x) { return {f.a * x.sum + f.b * x.length, x.length}; }
Affine composition(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine id() { return {1, 0}; }

int main() {
	FastInput input;
	FastOutput output;
	std::uint32_t n = 0;
	int q = 0;
	input.read(n), input.read(q);
	struct Query { int type = 0, l = 0, r = 0; Affine f; };
	struct Endpoint { std::uint32_t value; int index; };
	std::vector<Query> queries(q);
	std::vector<Endpoint> endpoints;
	endpoints.reserve(2 * std::size_t(q) + 2);
	endpoints.push_back({0, -1}), endpoints.push_back({n, -1});
	for (int i = 0; i < q; ++i) {
		auto &query = queries[i];
		std::uint32_t l = 0, r = 0;
		input.read(query.type), input.read(l), input.read(r);
		endpoints.push_back({l, 2 * i}), endpoints.push_back({r, 2 * i + 1});
		if (!query.type) {
			unsigned a = 0, b = 0;
			input.read(a), input.read(b);
			query.f = {mint::raw(a), mint::raw(b)};
		}
	}
	stable_radix_sort<11>(endpoints, [](const Endpoint &point) { return point.value; });
	std::vector<std::uint32_t> coordinates;
	coordinates.reserve(endpoints.size());
	for (auto point : endpoints) {
		if (coordinates.empty() || coordinates.back() != point.value) coordinates.push_back(point.value);
		if (point.index < 0) continue;
		int index = int(coordinates.size()) - 1;
		if (point.index & 1) queries[point.index / 2].r = index;
		else queries[point.index / 2].l = index;
	}
	std::vector<Sum> values(coordinates.size() - 1);
	for (std::size_t i = 0; i < values.size(); ++i) values[i] = {0, coordinates[i + 1] - coordinates[i]};
	LazySegtree<Sum, op, e, Affine, mapping, composition, id> tree(values);
	for (const auto &query : queries) {
		if (!query.type) tree.apply(query.l, query.r, query.f);
		else output.println(tree.prod(query.l, query.r).sum.val());
	}
}
