#define PROBLEM "https://judge.yosupo.jp/problem/persistent_queue"
#include "../data_structure/persistent_queue.hpp"
#include "../components/fast_io.hpp"
#include <vector>

int main() {
	FastInput input;
	FastOutput output;
	int q = 0;
	input.read(q);
	PersistentQueue<int> queue;
	queue.reserve(q);
	std::vector<decltype(queue)::root_type> roots(q + 1);
	for (int i = 1; i <= q; ++i) {
		int type = 0, k = 0;
		input.read(type), input.read(k);
		if (type == 0) { int x = 0; input.read(x); roots[i] = queue.push(roots[k + 1], x); }
		else { output.println(queue.front(roots[k + 1])); roots[i] = queue.pop(roots[k + 1]); }
	}
}
