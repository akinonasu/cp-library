#include "../../data_structure/persistent_deque.hpp"
#include <algorithm>
#include <cassert>
#include <climits>
#include <random>
#include <string>
#include <vector>

std::uint64_t weight(const std::string &s) { return s.size(); }

void weighted_rope() {
	PersistentRope<std::string, weight> rope;
	auto splitter = [](const std::string &s, std::uint64_t p) { return std::pair{s.substr(0, p), s.substr(p)}; };
	std::vector<int> roots{rope.build({"abc", "defgh", "ijk"})};
	std::vector<std::string> values{"abcdefghijk"};
	std::mt19937 random(876);
	for (int t = 0; t < 5000; ++t) {
		unsigned k = random() % roots.size();
		auto r = roots[k]; auto text = values[k];
		unsigned p = random() % (text.size() + 1);
		if (t % 3 == 0) {
			std::string value(1 + random() % 8, 'a' + random() % 26);
			r = rope.insert(r, p, value, splitter); text.insert(p, value);
		} else if (t % 3 == 1) {
			unsigned q = random() % (text.size() + 1); if (p > q) std::swap(p, q);
			r = rope.erase(r, p, q, splitter); text.erase(p, q - p);
		} else if (p < text.size()) {
			auto [chunk, offset] = rope.at(r, p);
			std::string value(chunk.size(), 'A' + random() % 26);
			r = rope.replace(r, p, value); text.replace(p - offset, chunk.size(), value);
		}
		roots.push_back(r); values.push_back(text);
		k = random() % roots.size();
		std::string actual;
		rope.for_each(roots[k], [&](const std::string &s) { actual += s; });
		assert(actual == values[k] && rope.size(roots[k]) == actual.size());
		for (unsigned i = 0; i < actual.size(); ++i) {
			auto [chunk, offset] = rope.at(roots[k], i);
			assert(chunk[offset] == actual[i]);
		}
	}
}

int main() {
	weighted_rope();
	PersistentDeque<std::string> deque;
	std::vector<int> roots{0};
	std::vector<std::vector<std::string>> values(1);
	std::mt19937 random(456);
	for (int t = 0; t < 12000; ++t) {
		unsigned k = random() % roots.size();
		auto v = values[k]; auto r = roots[k];
		int p = random() % (v.size() + 1), type = random() % 9;
		std::string x = std::to_string(t);
		if (type == 0) { v.insert(v.begin(), x); r = deque.push_front(r, x); }
		else if (type == 1) { v.push_back(x); r = deque.push_back(r, x); }
		else if (type == 2) { v.insert(v.begin() + p, x); r = deque.insert(r, p, x); }
		else if (type == 3 && !v.empty()) { v.erase(v.begin()); r = deque.pop_front(r); }
		else if (type == 4 && !v.empty()) { v.pop_back(); r = deque.pop_back(r); }
		else if (type == 5 && p < static_cast<int>(v.size())) { v[p] = x; r = deque.set(r, p, x); }
		else if (type == 6) {
			int q = random() % (v.size() + 1); if (p > q) std::swap(p, q);
			v.erase(v.begin() + p, v.begin() + q); r = deque.erase(r, p, q);
		} else if (type == 7) {
			auto [a, b] = deque.split(r, p);
			assert(deque.size(a) == p && deque.size(b) == static_cast<int>(v.size()) - p);
			r = deque.concat(b, a); std::rotate(v.begin(), v.begin() + p, v.end());
		} else if (type == 8 && v.size() < 100) { r = deque.concat(r, r); auto copy = v; v.insert(v.end(), copy.begin(), copy.end()); }
		roots.push_back(r); values.push_back(v);
		k = random() % roots.size(); auto before = deque.node_count();
		assert(deque.to_vector(roots[k]) == values[k]);
		if (!values[k].empty()) {
			assert(deque.front(roots[k]) == values[k].front() && deque.back(roots[k]) == values[k].back());
			for (int i = 0; i < deque.size(roots[k]); ++i) assert(deque.get(roots[k], i) == values[k][i]);
		}
		assert(deque.node_count() == before);
	}
	PersistentDeque<int> big;
	auto r = big.build({7});
	for (int i = 0; i < 30; ++i) r = big.concat(r, r);
	assert(big.size(r) == (1 << 30) && big.get(r, (1 << 30) - 1) == 7);
	r = big.set(r, 100, 9);
	assert(big.get(r, 100) == 9 && big.get(r, 101) == 7);
	auto [a, b] = big.split(r, 100);
	assert(big.size(a) == 100 && big.front(b) == 9);
	assert(big.size(big.erase(r, 0, (1 << 30) - 1)) == 1);
	assert(big.node_count() < 1000);
}
