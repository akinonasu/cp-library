#include "data_structure/sliding_window_aggregation.hpp"
#include <cassert>
#include <deque>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

std::string concatenate(std::string a, std::string b) { return a + b; }
std::string empty_string() { return {}; }
using Queue = SlidingWindowAggregation<std::string, concatenate, empty_string>;

struct Box {
	static inline int remaining = -1, live = 0;
	std::string value;
	Box() = delete;
	explicit Box(std::string value) : value(std::move(value)) { ++live; }
	Box(const Box &other) : value(other.value) {
		if (remaining == 0) throw std::runtime_error("copy");
		if (remaining > 0) --remaining;
		++live;
	}
	Box(Box &&other) noexcept : value(std::move(other.value)) { ++live; }
	~Box() { --live; }
};
int operations = -1;
Box combine(Box a, Box b) {
	if (operations == 0) throw std::runtime_error("operation");
	if (operations > 0) --operations;
	return Box(a.value + b.value);
}
Box identity() { return Box(""); }
using ThrowingQueue = SlidingWindowAggregation<Box, combine, identity>;

int main() {
	Queue queue;
	std::deque<std::string> expected;
	std::mt19937 random(918200);
	queue.reserve(2000);
	for (int step = 0; step < 20000; ++step) {
		if (expected.empty() || random() % 4) {
			std::string value(1, char('a' + random() % 26));
			queue.push_back(value), expected.push_back(value);
		} else queue.pop_front(), expected.pop_front();
		if (step % 211 == 0) {
			Queue copy(queue), moved(std::move(copy));
			assert(copy.empty() && copy.prod().empty());
			copy = moved;
			auto *self = &moved;
			moved = std::move(*self);
			assert(copy.prod() == moved.prod() && moved.prod() == queue.prod());
			copy.clear();
			assert(copy.empty());
			queue.swap(moved);
		}
		std::string product;
		for (const auto &value : expected) product += value;
		assert(queue.prod() == product && queue.size() == expected.size());
		if (!expected.empty()) assert(queue.front() == expected.front() && queue.back() == expected.back());
		if (expected.size() > 2000) {
			queue.clear(), expected.clear();
			assert(queue.empty() && queue.prod().empty());
		}
	}
	Queue built(std::vector<std::string>{"ab", "cd", "ef"});
	assert(built.prod() == "abcdef");
	built.pop_front();
	assert(built.prod() == "cdef" && built.front() == "cd" && built.back() == "ef");
	built.push_back("gh");
	assert(built.prod() == "cdefgh");
	built.pop_front(), built.pop_front(), built.pop_front();
	assert(built.empty() && built.prod().empty());
	for (int failure = 0; failure < 35; ++failure) {
		{
			ThrowingQueue values;
			for (char c : std::string("abcde")) values.push_back(Box(std::string(1, c)));
			Box::remaining = failure;
			try { values.pop_front(); }
			catch (const std::runtime_error &) {
				Box::remaining = -1;
				assert(values.size() == 5 && values.prod().value == "abcde");
				assert(values.front().value == "a" && values.back().value == "e");
				values.pop_front();
			}
			Box::remaining = -1;
			assert(values.size() == 4 && values.prod().value == "bcde");
		}
		assert(Box::live == 0);
	}
	for (int failure = 0; failure < 6; ++failure) {
		ThrowingQueue values;
		for (char c : std::string("abcde")) values.push_back(Box(std::string(1, c)));
		operations = failure;
		try { values.pop_front(); }
		catch (const std::runtime_error &) {
			operations = -1;
			assert(values.size() == 5 && values.prod().value == "abcde");
			values.pop_front();
		}
		operations = -1;
		assert(values.prod().value == "bcde");
	}
	assert(Box::live == 0);
}
