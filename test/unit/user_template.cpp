#include "../fixtures/user_template.hpp"
#define COMB_MAX 20
#include "math/mint.hpp"
#include "math/modint.hpp"
#include "math/fraction.hpp"
#include "math/bigint.hpp"
#include "math/comb.hpp"
#include "math/comb_light.hpp"
#include "math/matrix.hpp"
#include "math/matrix_light.hpp"
#include "math/stern_brocot_tree.hpp"
#include "fps/fps.hpp"
#include "fps/fps_light.hpp"
#include "components/fps_io.hpp"
#include "components/fps_write.hpp"
#include "convolution/multivariate_convolution.hpp"
#include "data_structure/dynamic_bitset.hpp"
#include "data_structure/segtree.hpp"
#include "data_structure/fenwick_tree.hpp"
#include "data_structure/union_find.hpp"
#include "data_structure/top_k_sum.hpp"
#include "data_structure/persistent_heap.hpp"
#include "graph/k_shortest_walk.hpp"
#include "geometry/point.hpp"
#include "tree/tree_center.hpp"
#include "tree/tree_diameter.hpp"

#if !defined(all) || !defined(rep) || !defined(per) || !defined(vec) || !defined(sz) || !defined(uint) || !defined(ulong)
#error "Library headers must preserve user macros"
#endif

void init() {}

void solve() {
	vec<int> values{3, 1, 1, 2};
	UNIQUE(values);
	assert(sz(values) == 3);
	++values; --values; values++; values--;
	assert((values == vec<int>{1, 2, 3}));
	uint visits = 0;
	rep(3) ++visits;
	rep(i, 3) visits += i;
	rep(i, 1, 3) visits += i;
	per(3) ++visits;
	per(i, 3) visits += i;
	per(i, 1, 3) visits += i;
	assert(visits == 18);
	ulong positive = 1;
	ld real = 2;
	assert(positive == 1 && real == 2);
	int best = 3;
	assert(chmin(best, 2LL) && chmax(best, 4LL) && best == 4);
	vector<bool> flags(2);
	flags[0] |= true; flags[0] &= true; flags[0] ^= true;
	assert(!flags[0]);
	vec<mint> modular{1, 2};
	++modular; modular--;
	assert((modular == vec<mint>{1, 2}));
	ostringstream text;
	text << __int128_t(-1234567890123456789LL) * 10 << ' ' << pair<int, int>{1, 2} << ' ' << values << ' ' << modular;
	assert(text.str() == "-12345678901234567890 1 2 1 2 3 1 2");
	istringstream input("7 8 9 10");
	pair<int, int> pair_value;
	vec<int> vector_value(2);
	input >> pair_value >> vector_value;
	assert((pair_value == pair<int, int>{7, 8}) && (vector_value == vec<int>{9, 10}));
	text.str(""); text.clear();
	text << fraction(2, 3) << ' ' << BigInt("12345678901234567890");
	assert(text.str() == "2/3 12345678901234567890");
	assert(pow(fraction(2, 3), 2) == fraction(4, 9));
	assert(StaticModint<17>(3).pow(2).val() == 9);
	assert(Comb<>::comb(5, 2) == mint(10));
	assert(light_combinatorics::Comb<>::comb(5, 2) == mint(10));
	fps f{1, 1};
	light_fps g{1, 1};
	assert((f * f == fps{1, 2, 1}) && (g * g == light_fps{1, 2, 1}));
	Matrix<mint> matrix{ {1, 2}, {3, 4} };
	LightMatrix<mint> light{ {1, 2}, {3, 4} };
	assert(matrix.det() == mint(-2) && light.det() == mint(-2));
	SternBrocotTree<> rational;
	assert((rational.value() == pair<std::int64_t, std::int64_t>{1, 1}));
	geometry::Point<long long> point{1, 2};
	assert((point + point == geometry::Point<long long>{2, 4}));
	PersistentHeap<int> heap;
	const auto first = heap.push(heap.initial_root(), 3), second = heap.push(first, 1);
	assert(heap.top(first) == 3 && heap.top(second) == 1 && heap.top(heap.pop(second)) == 3);
	vector<vector<pair<int, int>>> graph(3);
	graph[0] = { {1, 1}, {2, 5} }; graph[1] = { {2, 1} };
	assert((k_shortest_walk(graph, 0, 2, 2) == vector<int64_t>{2, 5}));
	SmallestKSum<int> top{1, 2, 3};
	assert(top.sum(2) == 3);
	const int n = 4;
	vec<DynamicBitset> d(n, DynamicBitset(n));
	rep(i, n) d[i][i] = true;
	rep(i, n - 1) d[i][i + 1] = true;
	rep(k, n) rep(i, n) if (d[i][k]) d[i] |= d[k];
	assert((d[0].all)() && d[3].count() == 1);
	assert((tree_center(vector<vector<int>>{ {1}, {0, 2}, {1} }) == vector<int>{1}));
	TreeDiameter<> diameter(2);
	diameter.add_edge(0, 1);
	assert(diameter.solve().distance == 1);
	vector<mint> a{1, 2, 3, 4, 5, 6}, b{7, 8, 9, 10, 11, 12};
	assert((multivariate_convolution(a, b, {2, 3}) == vector<mint>{7, 22, 30, 80, 73, 182}));
	vector<mint> binary(8, 1);
	const auto truncated = multivariate_convolution(binary, binary, {2, 2, 2});
	rep(i, 8) assert(truncated[i] == mint(1 << std::popcount(unsigned(i))));
	FILE *file = std::tmpfile();
	assert(file);
	{
		FastOutput output(file);
		fps_detail::write_coefficients(output, f.data(), f.size());
	}
	std::rewind(file);
	int coefficient = 0;
	assert(std::fscanf(file, "%d", &coefficient) == 1 && coefficient == 1);
	assert(std::fscanf(file, "%d", &coefficient) == 1 && coefficient == 1);
	assert(std::fscanf(file, "%d", &coefficient) == EOF);
	std::fclose(file);
#if defined(CP_LIBRARY_TEST_WITH_ATCODER)
	static_assert(!is_same_v<mint, atcoder::modint998244353>);
	FormalPowerSeries<atcoder::modint998244353> acl_fps{1, 1};
	assert((acl_fps * acl_fps == decltype(acl_fps){1, 2, 1}));
	atcoder::dsu acl(3);
	UnionFind own(3);
	acl.merge(0, 1); own.merge(0, 1);
	assert(acl.same(0, 1) && own.same(0, 1));
#endif
}
