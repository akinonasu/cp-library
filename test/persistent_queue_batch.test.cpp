#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include "data_structure/persistent/persistent_queue.hpp"
#include "components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	PersistentQueue<int> queue;
	queue.reserve(q);
	using Root = decltype(queue)::root_type;
	std::vector<Root> roots(q + 1), queries;
	queries.reserve(q);
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		if (type == 0) { int x = 0; input.read(x); roots[i] = queue.push(roots[k + 1], x); }
		else { queries.push_back(roots[k + 1]); roots[i] = queue.pop(roots[k + 1]); }
	}
	for (int value : queue.front_many(queries)) output.println(value);
}
