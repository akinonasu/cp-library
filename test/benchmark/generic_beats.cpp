#include "../fixtures/beats.hpp"
#include "../../data_structure/segtree_beats.hpp"
#include "reference.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

struct Query { int type, l, r; long long x; };

struct GenericClamp {
	clamp_beats::Tree tree;
	static std::vector<clamp_beats::S> convert(const std::vector<long long> &a) {
		std::vector<clamp_beats::S> result;
		result.reserve(a.size());
		for (auto x : a) result.push_back(clamp_beats::leaf(x));
		return result;
	}
	explicit GenericClamp(const std::vector<long long> &a) : tree(convert(a)) {}
	void update(const Query &q) {
		auto f = clamp_beats::id();
		if (q.type == 0) f.hi = q.x;
		if (q.type == 1) f.lo = q.x;
		if (q.type == 2) f.add = q.x;
		if (q.type == 4) f.lo = f.hi = q.x;
		tree.apply(q.l, q.r, f);
	}
	long long query(const Query &q) { return tree.prod(q.l, q.r).sum; }
};

template<class Tree> struct Clamp {
	Tree tree;
	explicit Clamp(const std::vector<long long> &a) : tree(a) {}
	void update(const Query &q) {
		if (q.type == 0) tree.chmin(q.l, q.r, q.x);
		if (q.type == 1) tree.chmax(q.l, q.r, q.x);
		if (q.type == 2) tree.add(q.l, q.r, q.x);
		if (q.type == 4) tree.assign(q.l, q.r, q.x);
	}
	long long query(const Query &q) { return tree.sum(q.l, q.r); }
};

struct GenericDivision {
	division_beats::Tree tree;
	static std::vector<division_beats::S> convert(const std::vector<long long> &a) {
		std::vector<division_beats::S> result;
		result.reserve(a.size());
		for (auto x : a) result.push_back(division_beats::leaf(x));
		return result;
	}
	explicit GenericDivision(const std::vector<long long> &a) : tree(convert(a)) {}
	void update(const Query &q) {
		if (q.type == 0) tree.apply(q.l, q.r, {-1, static_cast<unsigned long long>(q.x)});
		else tree.apply(q.l, q.r, {q.x, 1});
	}
	long long query(const Query &q) { return tree.prod(q.l, q.r).sum; }
};

class UniformDivision {
	struct Node { long long sum, value; bool same; };
	int n;
	std::vector<Node> data;
	void fill(int k, int l, int r, long long value) { data[k] = {value * (r - l), value, true}; }
	void pull(int k) {
		const auto &a = data[2 * k], &b = data[2 * k + 1];
		data[k] = {a.sum + b.sum, a.value, a.same && b.same && a.value == b.value};
	}
	void push(int k, int l, int m, int r) {
		if (data[k].same) fill(2 * k, l, m, data[k].value), fill(2 * k + 1, m, r, data[k].value);
	}
	void build(int k, int l, int r, const std::vector<long long> &a) {
		if (r - l == 1) return fill(k, l, r, a[l]);
		int m = (l + r) / 2;
		build(2 * k, l, m, a), build(2 * k + 1, m, r, a);
		pull(k);
	}
	void update(int k, int l, int r, const Query &q) {
		if (q.l <= l && r <= q.r && (q.type == 4 || data[k].same))
			return fill(k, l, r, q.type == 4 ? q.x : data[k].value / q.x);
		int m = (l + r) / 2;
		push(k, l, m, r);
		if (q.l < m) update(2 * k, l, m, q);
		if (m < q.r) update(2 * k + 1, m, r, q);
		pull(k);
	}
	long long query(int k, int l, int r, const Query &q) {
		if (q.l <= l && r <= q.r) return data[k].sum;
		if (data[k].same) return data[k].value * (std::min(r, q.r) - std::max(l, q.l));
		int m = (l + r) / 2;
		push(k, l, m, r);
		long long result = 0;
		if (q.l < m) result += query(2 * k, l, m, q);
		if (m < q.r) result += query(2 * k + 1, m, r, q);
		return result;
	}
  public:
	explicit UniformDivision(const std::vector<long long> &a) : n(a.size()), data(4 * n) { build(1, 0, n, a); }
	void update(const Query &q) { update(1, 0, n, q); }
	long long query(const Query &q) { return query(1, 0, n, q); }
};

template<class Tree>
void run(const std::vector<long long> &a, const std::vector<Query> &queries) {
	auto start = std::chrono::steady_clock::now();
	Tree tree(a);
	auto built = std::chrono::steady_clock::now();
	std::uint64_t hash = 0;
	for (const auto &q : queries) {
		if (q.type == 3) hash = hash * 1000000007 + tree.query(q);
		else tree.update(q);
	}
	auto end = std::chrono::steady_clock::now();
	std::cout << std::fixed << std::setprecision(6) << std::chrono::duration<double, std::milli>(built - start).count()
		<< ' ' << std::chrono::duration<double, std::milli>(end - built).count() << ' ' << hash << '\n';
}

int main(int argc, char **argv) {
	if (argc != 5) return 1;
	std::string workload = argv[1], algorithm = argv[2];
	int n = std::stoi(argv[3]), count = std::stoi(argv[4]);
	bool division = workload.starts_with("division");
	std::mt19937_64 rng(814239);
	std::vector<long long> a(n);
	for (auto &x : a) x = workload == "uniform" ? 0 : rng() % (division ? 100001 : 1000000000);
	std::vector<Query> queries(count);
	for (auto &q : queries) {
		q.l = rng() % n, q.r = rng() % n;
		if (q.l > q.r) std::swap(q.l, q.r);
		++q.r;
		if (workload.ends_with("short")) q.r = std::min(n, q.l + static_cast<int>(rng() % 64) + 1);
		q.type = rng() % 4;
		q.x = q.type == 2 ? static_cast<long long>(rng() % 2000001) - 1000000 : rng() % 1000000000;
		if (workload == "assign") q.type = rng() % 5;
		if (workload == "uniform" && q.type < 3) q.l = 0, q.r = n;
		if (division) {
			q.type = q.type == 0 ? 4 : q.type == 3 ? 3 : 0;
			if (workload == "division_decay" && q.type == 4) q.type = 0;
			q.x = q.type == 0 ? 2 + rng() % 31 : rng() % 100001;
		}
	}
	if (division) {
		if (algorithm == "generic") run<GenericDivision>(a, queries);
		else run<UniformDivision>(a, queries);
	} else {
		if (algorithm == "generic") run<GenericClamp>(a, queries);
		else if (algorithm == "dedicated") run<Clamp<SegtreeBeats<>>>(a, queries);
		else run<Clamp<Beats_reference>>(a, queries);
	}
}
