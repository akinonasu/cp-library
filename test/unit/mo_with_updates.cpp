#include "data_structure/range_query/mo_with_updates.hpp"
#include <map>
#include <random>
#include <set>

int main() {
	std::mt19937 rng(625398);
	for (int n : {0, 1, 2, 7, 31, 64}) for (int trial = 0; trial < 20; ++trial) {
		MoWithUpdates mo(n);
		std::vector<int> initial(n), current;
		for (int &v : initial) v = rng() % 9;
		current = initial;
		struct Update { int p, before, after; };
		std::vector<Update> updates;
		std::vector<std::vector<int>> expected;
		for (int i = 0; i < 200; ++i) {
			if (n && rng() % 3 == 0) {
				int p = rng() % n, v = rng() % 9;
				assert(mo.add_update(p) == int(updates.size()));
				updates.push_back({p, current[p], v});
				current[p] = v;
			} else {
				int l = rng() % (n + 1), r = rng() % (n + 1);
				if (l > r) std::swap(l, r);
				assert(mo.add_query(l, r) == int(expected.size()));
				expected.emplace_back(current.begin() + l, current.begin() + r);
				assert(mo.query(mo.query_count() - 1).time == int(updates.size()));
			}
		}
		for (int block : {0, 1, 3, 1000}) {
			current = initial;
			std::map<int, int> active;
			std::vector<int> seen(expected.size());
			int time = 0;
			auto add_left = [&](int p) { assert(active.empty() || p < active.begin()->first); assert(active.emplace(p, current[p]).second); };
			auto add_right = [&](int p) { assert(active.empty() || p > active.rbegin()->first); assert(active.emplace(p, current[p]).second); };
			auto erase_left = [&](int p) { assert(!active.empty() && active.begin()->first == p); assert(active.erase(p) == 1); };
			auto erase_right = [&](int p) { assert(!active.empty() && active.rbegin()->first == p); assert(active.erase(p) == 1); };
			auto change = [&](int id, bool inside, bool forward) {
				auto [p, before, after] = updates[id];
				assert(mo.update_position(id) == p && inside == active.contains(p));
				assert(current[p] == (forward ? before : after));
				current[p] = forward ? after : before;
				if (inside) active[p] = current[p];
			};
			mo.run(add_left, add_right, erase_left, erase_right,
				[&](int id, bool inside) { assert(id == time++); change(id, inside, true); },
				[&](int id, bool inside) { assert(id == --time); change(id, inside, false); },
				[&](int id) {
					assert(!seen[id]++ && mo.query(id).time == time);
					std::vector<int> values;
					for (auto [p, v] : active) { assert(current[p] == v); values.push_back(v); }
					assert(values == expected[id]);
				}, block);
			for (int x : seen) assert(x == 1);
		}
	}
	MoWithUpdates empty;
	empty.run([](int) { assert(false); }, [](int) { assert(false); }, [](int, bool) { assert(false); }, [](int, bool) { assert(false); }, [](int) { assert(false); });
	MoWithUpdates no_updates(5);
	no_updates.add_query(0, 5);
	no_updates.add_query(1, 3);
	assert(no_updates.order() == mo_order({ {0, 5}, {1, 3} }));
	MoWithUpdates wide(INT32_MAX);
	wide.add_update(INT32_MAX - 1);
	wide.add_query(INT32_MAX, INT32_MAX);
	assert(wide.order() == std::vector<int>{0});
}
