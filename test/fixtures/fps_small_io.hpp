#ifndef TEST_FPS_SMALL_IO_HPP
#define TEST_FPS_SMALL_IO_HPP

#include <cstdio>

class FpsInput {
	char buffer[1 << 16];
	std::size_t at = 0, size = 0;
	int get() {
		if (at == size) {
			size = std::fread(buffer, 1, sizeof(buffer), stdin);
			at = 0;
			if (!size) return -1;
		}
		return static_cast<unsigned char>(buffer[at++]);
	}
public:
	template <class T> void read(T &value) {
		int c = get();
		while (0 <= c && c <= ' ') c = get();
		T x = 0;
		while ('0' <= c && c <= '9') x = x * 10 + T(c - '0'), c = get();
		value = x;
	}
};

class FpsOutput {
	char buffer[1 << 16];
	std::size_t size = 0;
	void put(char c) {
		if (size == sizeof(buffer)) flush();
		buffer[size++] = c;
	}
	void flush() {
		std::fwrite(buffer, 1, size, stdout);
		size = 0;
	}
public:
	~FpsOutput() { flush(); }
	void write(unsigned value) {
		char digits[10];
		int count = 0;
		do digits[count++] = char('0' + value % 10), value /= 10; while (value);
		while (count) put(digits[--count]);
	}
	void write(char c) { put(c); }
};

template <class F> F read_series(FpsInput &input, int n) {
	F result(n);
	for (auto &x : result) { unsigned value = 0; input.read(value); x = F::value_type::raw(value); }
	return result;
}

template <class F> void write_series(FpsOutput &output, const F &f) {
	for (auto x : f) output.write(x.val()), output.write(' ');
	output.write('\n');
}

#endif
