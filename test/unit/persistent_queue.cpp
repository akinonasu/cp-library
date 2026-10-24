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
	auto c = stack.push(a, Value(3));
	const auto &view = stack;
	auto batch = view.get_many({{b, 1}, {c, 0}, {a, 0}, {b, 0}, {c, 1}});
	assert(batch.size() == 5);
	const int expected[] = {1, 3, 1, 2, 1};
	for (int i = 0; i < 5; ++i) assert(batch[i].x == expected[i]);
	assert(stack.get_many({}).empty());
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
	std::vector<decltype(queue)::root_type> batch_roots;
	std::vector<std::string> batch_values;
	for (std::size_t i = roots.size(); i--;) if (!values[i].empty()) {
		batch_roots.push_back(roots[i]); batch_values.push_back(values[i].front());
	}
	auto before = queue.node_count();
	const auto &constant = queue;
	assert(constant.front_many(batch_roots) == batch_values && queue.node_count() == before);
	assert(queue.front_many({}).empty());
	PersistentQueue<Value> boxes;
	auto first_box = boxes.push(boxes.initial_root(), Value(7));
	auto second_box = boxes.push(first_box, Value(9));
	auto box_values = boxes.front_many({boxes.pop(second_box), second_box, first_box});
	assert(box_values[0].x == 9 && box_values[1].x == 7 && box_values[2].x == 7);
	PersistentStack<int> deep;
	deep.reserve(500000);
	auto tail = deep.initial_root();
	for (int i = 0; i < 500000; ++i) tail = deep.push(tail, i);
	for (int i = 0; i < 500000; ++i) assert(deep.get(tail, i) == 499999 - i);
	assert(deep.ancestor(tail, 500000) == 0);
	assert(deep.get_many({{tail, 499999}, {tail, 0}, {tail, 123456}}) == std::vector<int>({0, 499999, 376543}));
	PersistentQueue<int> branch;
	auto q = branch.initial_root();
	for (int i = 0; i < 100000; ++i) q = branch.push(q, i);
	for (int i = 0; i < 100000; ++i) {
		auto r = branch.pop(q);
		assert(branch.front(r) == 1 && branch.front(q) == 0);
	}
	assert(branch.node_count() == 100000);
	assert(branch.front_many(std::vector<decltype(branch)::root_type>(100000, q)) == std::vector<int>(100000, 0));
	PersistentStack<Value> numbered;
	auto last = numbered.initial_root();
	for (int i = 0; i < 100; ++i) last = numbered.push(last, Value(i));
	auto numbered_values = numbered.get_many(std::vector<std::pair<decltype(numbered)::root_type, int>>(100, {last, 99}));
	for (const auto &value : numbered_values) assert(value.x == 0);
}
