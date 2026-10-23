#include "../../data_structure/persistent/persistent_stack.hpp"
#include "../../data_structure/persistent/persistent_queue.hpp"
#include <cassert>
#include <deque>
#include <random>
#include <string>
#include <vector>

struct Value { int x; explicit Value(int v) : x(v) {} };

int main() {
	PersistentStack<Value> stack;
	auto z = stack.initial_root(), a = stack.push(z, Value(1)), b = stack.push(a, Value(2));
	assert(stack.top(a).x == 1 && stack.get(b, 1).x == 1 && stack.pop(b) == a);
	assert(stack.ancestor(b, 2) == z);
	PersistentQueue<std::string> queue;
	std::vector<decltype(queue)::root_type> roots{queue.initial_root()};
	std::vector<std::deque<std::string>> values(1);
	std::mt19937 random(123);
	for (int t = 0; t < 20000; ++t) {
		unsigned k = random() % roots.size();
		auto next = values[k]; auto root = roots[k];
		if (next.empty() || random() % 3) {
			auto x = std::to_string(t); next.push_back(x); root = queue.push(root, x);
		} else { next.pop_front(); root = queue.pop(root); }
		roots.push_back(root); values.push_back(next);
		k = random() % roots.size(); auto before = queue.node_count();
		assert(queue.size(roots[k]) == static_cast<int>(values[k].size()));
		if (!values[k].empty()) {
			assert(queue.front(roots[k]) == values[k].front() && queue.back(roots[k]) == values[k].back());
			for (int i = 0; i < queue.size(roots[k]); ++i) assert(queue.get(roots[k], i) == values[k][i]);
		}
		assert(queue.node_count() == before);
	}
	PersistentStack<int> deep;
	deep.reserve(500000);
	auto tail = deep.initial_root();
	for (int i = 0; i < 500000; ++i) tail = deep.push(tail, i);
	for (int i = 0; i < 500000; ++i) assert(deep.get(tail, i) == 499999 - i);
	assert(deep.ancestor(tail, 500000) == 0);
	PersistentQueue<int> branch;
	auto q = branch.initial_root();
	for (int i = 0; i < 100000; ++i) q = branch.push(q, i);
	for (int i = 0; i < 100000; ++i) {
		auto r = branch.pop(q);
		assert(branch.front(r) == 1 && branch.front(q) == 0);
	}
	assert(branch.node_count() == 100000);
}
