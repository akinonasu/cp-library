#include "components/dijkstra_queue.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <random>
#include <vector>

using namespace std;

template<class T>
struct QueueCheck {
	shortest_path_detail::RadixQueue<T> queue;
	priority_queue<T, vector<T>, greater<T>> expected;
	vector<T> keys;
	vector<bool> alive;
	T last = 0;
	void push(T key) {
		assert(key >= last);
		queue.push(key, int(keys.size()));
		expected.push(key);
		keys.push_back(key);
		alive.push_back(true);
		assert(!queue.empty());
	}
	void pop() {
		assert(!expected.empty() && !queue.empty());
		auto [key, vertex] = queue.pop();
		assert(key == expected.top() && key >= last);
		assert(vertex >= 0 && size_t(vertex) < keys.size() && alive[vertex]);
		assert(keys[vertex] == key);
		alive[vertex] = false;
		last = key;
		expected.pop();
		assert(queue.empty() == expected.empty());
	}
	void drain() {
		while (!expected.empty()) pop();
		assert(queue.empty());
		for (bool value : alive) assert(!value);
	}
};

int main() {
	using U = uint64_t;
	const U cycle = U(1) << 32, half = U(1) << 31;
	{
		QueueCheck<U> q;
		assert(q.queue.empty());
		for (int i = 31; i >= 0; --i) q.push(U(i % 7));
		q.drain();
		q.push(numeric_limits<U>::max());
		q.drain();
	}
	{
		QueueCheck<U> q;
		for (int i = 127; i >= 0; --i) q.push(U(i % 37));
		for (int i = 0; i < 20; ++i) q.pop();
		q.push(q.last + half - 1);
		q.push(q.last + half);
		q.drain();
	}
	{
		QueueCheck<U> q;
		for (int i = 0; i < 32; ++i) q.push(U(i));
		q.push(half);
		q.push(numeric_limits<U>::max());
		q.drain();
	}
	for (U base : {cycle - 64, cycle * 6 - 256, (U(1) << 63) + cycle - 64, numeric_limits<U>::max() - 1024}) {
		QueueCheck<U> q;
		q.push(base);
		q.pop();
		for (int i = 127; i >= 0; --i) q.push(base + U(i * 7 % 512));
		for (int i = 0; i < 40; ++i) q.pop();
		for (int i = 0; i < 96; ++i) q.push(q.last + U(i % 31));
		q.drain();
	}
	{
		QueueCheck<U> q;
		q.push((U(1) << 63) + 17);
		q.pop();
		for (int i = 0; i < 96; ++i) q.push(q.last + U(i % 13));
		q.push(q.last + half);
		q.push(numeric_limits<U>::max());
		q.drain();
	}
	{
		QueueCheck<uint32_t> q;
		for (int i = 0; i < 64; ++i) q.push(uint32_t(i));
		q.push(numeric_limits<uint32_t>::max());
		q.drain();
	}
	{
		QueueCheck<int64_t> q;
		for (int i = 0; i < 64; ++i) q.push(int64_t(i));
		q.push(numeric_limits<int64_t>::max());
		q.drain();
	}
	mt19937_64 random(50231);
	for (int trial = 0; trial < 96; ++trial) {
		QueueCheck<U> q;
		U base = trial % 4 == 0 ? 0 : trial % 4 == 1 ? cycle - 512
			: trial % 4 == 2 ? (U(1) << 63) : numeric_limits<U>::max() - (U(1) << 60);
		q.push(base);
		q.pop();
		for (int step = 0; step < 4000; ++step) {
			if (q.expected.empty() || (q.expected.size() < 128 && random() % 3 != 0)) {
				U delta = random() % 1024;
				if (trial % 3 == 0 && step > 200 && random() % 97 == 0) delta += half;
				if (trial % 3 == 1 && step > 200 && random() % 89 == 0) delta += U(1) << 40;
				q.push(q.last + delta);
			} else q.pop();
		}
		q.drain();
	}
}
