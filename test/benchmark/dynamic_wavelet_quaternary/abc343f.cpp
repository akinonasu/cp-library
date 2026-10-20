#ifndef DWM_HEADER
#define DWM_HEADER "data_structure/wavelet_matrix/dynamic_wavelet_matrix.hpp"
#endif
#include DWM_HEADER
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <vector>
#ifndef DWM_FAST_IO
#define DWM_FAST_IO 0
#endif
#ifndef DWM_COUNT_DIRECT
#define DWM_COUNT_DIRECT 0
#endif
#ifndef DWM_INSTRUMENT
#define DWM_INSTRUMENT 0
#endif
#ifndef DWM_VALUE_TYPE
#define DWM_VALUE_TYPE int
#endif
using Value = DWM_VALUE_TYPE;
#ifndef DWM_BITS
#define DWM_BITS (sizeof(Value) * 8)
#endif
using Matrix = DynamicWaveletMatrix<Value, DWM_BITS>;
#if DWM_FAST_IO
#include "components/fast_io.hpp"
#else
#include <iostream>
#endif

struct InputOutput {
#if DWM_FAST_IO
	FastInput input;
	FastOutput output;
	template<class T> void read(T& x) { input.read(x); }
	void answer(int x) { output.println(x); }
#else
	InputOutput() { std::cin.tie(nullptr)->sync_with_stdio(false); }
	template<class T> void read(T& x) { std::cin >> x; }
	void answer(int x) { std::cout << x << '\n'; }
#endif
};

struct Operation { int kind, a, b; };

int consume(Matrix& wm, Operation op) {
	if (op.kind == 1) { wm.set(op.a - 1, Value(op.b)); return -1; }
	int l = op.a - 1, r = op.b;
	Value maximum = wm.kth_largest(l, r, 0);
	auto previous = wm.prev_value(l, r, maximum);
	if (!previous) return 0;
#if DWM_COUNT_DIRECT
	return wm.count(l, r, *previous);
#else
	return wm.range_freq(l, r, *previous, Value(*previous + 1));
#endif
}

int main() {
	InputOutput io;
	int n = 0, q = 0;
	io.read(n); io.read(q);
	std::vector<Value> a(n);
	for (auto& x : a) io.read(x);
#if DWM_INSTRUMENT
	std::vector<Operation> operations(q);
	std::vector<int> answers(q);
	for (auto& op : operations) { io.read(op.kind); io.read(op.a); io.read(op.b); }
	using Clock = std::chrono::steady_clock;
	auto start = Clock::now();
	std::clock_t start_cpu = std::clock();
	Matrix wm(a);
	std::clock_t built_cpu = std::clock();
	auto built = Clock::now();
	int count = 0;
	for (auto op : operations) {
		int result = consume(wm, op);
		if (op.kind == 2) answers[count++] = result;
	}
	std::clock_t done_cpu = std::clock();
	auto done = Clock::now();
	std::fprintf(stderr, "{\"build_cpu\":%.12f,\"operation_cpu\":%.12f,\"build_wall\":%.12f,\"operation_wall\":%.12f,\"answers\":%d}\n",
		double(built_cpu - start_cpu) / CLOCKS_PER_SEC, double(done_cpu - built_cpu) / CLOCKS_PER_SEC,
		std::chrono::duration<double>(built - start).count(), std::chrono::duration<double>(done - built).count(), count);
	for (int i = 0; i < count; ++i) io.answer(answers[i]);
#else
	Matrix wm(a);
	for (int i = 0; i < q; ++i) {
		Operation op{};
		io.read(op.kind); io.read(op.a); io.read(op.b);
		int result = consume(wm, op);
		if (op.kind == 2) io.answer(result);
	}
#endif
}
