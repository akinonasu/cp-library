#include "data_structure/sliding_window_aggregation.hpp"
#include <cassert>
#include <deque>
#include <memory>
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

struct Affine {
	int a, b;
	int operator()(int x) const { return (a * x + b) % 1009; }
};
Affine compose_affine(Affine f, Affine g) { return {g.a * f.a % 1009, (g.a * f.b + g.b) % 1009}; }
Affine identity_affine() { return {1, 0}; }
using AffineQueue = SlidingWindowAggregation<Affine, compose_affine, identity_affine>;
struct CopyBox {
	static inline int remaining = -1;
	int value;
	explicit CopyBox(int x) : value(x) {}
	CopyBox(const CopyBox &other) : value(other.value) {
		if (remaining == 0) throw std::runtime_error("copy");
		if (remaining > 0) --remaining;
	}
	CopyBox(CopyBox &&) noexcept = default;
	CopyBox &operator=(const CopyBox &) noexcept = default;
	CopyBox &operator=(CopyBox &&) noexcept = default;
};
CopyBox add_boxes(CopyBox a, CopyBox b) { return CopyBox(a.value + b.value); }
CopyBox zero_box() { return CopyBox(0); }
std::shared_ptr<int> last_pointer(std::shared_ptr<int>, std::shared_ptr<int> b) { return b; }
std::shared_ptr<int> no_pointer() { return {}; }

int checked_sum(int a, int b) {
	if (operations == 0) throw std::runtime_error("operation");
	if (operations > 0) --operations;
	return a + b;
}
int zero() { return 0; }

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
	AffineQueue functions;
	std::deque<Affine> reference;
	functions.reserve(8);
	for (int step = 0; step < 30000; ++step) {
		if (reference.empty() || random() % 3) {
			Affine f{int(random() % 1009), int(random() % 1009)};
			functions.push_back(f);
			reference.push_back(f);
		} else functions.pop_front(), reference.pop_front();
		int x = int(random() % 1009), answer = x;
		for (Affine f : reference) answer = f(answer);
		assert(functions.apply(x) == answer && functions.prod()(x) == answer);
		assert(functions.apply(x, [](Affine f, int value) { return f(value); }) == answer);
		const auto &constant = functions;
		assert(constant.apply(x) == answer && constant.size() == reference.size());
		if (!reference.empty()) {
			assert(functions.front().a == reference.front().a && functions.front().b == reference.front().b);
			assert(functions.back().a == reference.back().a && functions.back().b == reference.back().b);
		}
		if (step % 79 == 0) {
			AffineQueue copied(functions), moved(std::move(copied));
			assert(copied.empty() && copied.apply(x) == x);
			copied = moved;
			auto *self = &moved;
			moved = std::move(*self);
			assert(copied.apply(x) == answer && moved.apply(x) == answer);
			functions.swap(moved);
		}
		if (reference.size() > 500) functions.clear(), reference.clear();
	}
	functions.clear();
	assert(functions.apply(27) == 27 && functions.prod()(27) == 27);
	for (int failure = 0; failure < 10; ++failure) {
		SlidingWindowAggregation<int, checked_sum, zero> values;
		for (int i = 1; i <= 8; ++i) values.push_back(i);
		operations = failure;
		try { values.pop_front(); }
		catch (const std::runtime_error &) {
			operations = -1;
			assert(values.size() == 8 && values.prod() == 36);
			assert(values.front() == 1 && values.back() == 8);
			values.push_back(9);
			assert(values.prod() == 45);
			values.pop_front();
			assert(values.size() == 8 && values.prod() == 44);
			values.pop_front();
			assert(values.prod() == 42);
			continue;
		}
		operations = -1;
		assert(values.size() == 7 && values.prod() == 35);
	}

	for (int failure = 0; failure < 20; ++failure) {
		SlidingWindowAggregation<CopyBox, add_boxes, zero_box> values;
		for (int i = 1; i <= 8; ++i) values.push_back(CopyBox(i));
		CopyBox::remaining = failure;
		try { values.pop_front(); }
		catch (const std::runtime_error &) {
			CopyBox::remaining = -1;
			assert(values.size() == 8 && values.prod().value == 36);
			assert(values.front().value == 1 && values.back().value == 8);
			values.push_back(CopyBox(9));
			assert(values.prod().value == 45);
			values.pop_front();
			assert(values.prod().value == 44);
			continue;
		}
		CopyBox::remaining = -1;
		assert(values.size() == 7 && values.prod().value == 35);
	}
	SlidingWindowAggregation<std::shared_ptr<int>, last_pointer, no_pointer> pointers;
	auto pointer = std::make_shared<int>(1);
	std::weak_ptr<int> removed = pointer;
	pointers.push_back(pointer);
	pointers.push_back(std::make_shared<int>(2));
	pointer.reset();
	pointers.pop_front();
	assert(removed.expired() && *pointers.front() == 2 && *pointers.prod() == 2);

}
