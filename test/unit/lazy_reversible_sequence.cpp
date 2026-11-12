#include "data_structure/lazy_reversible_sequence.hpp"
#include "math/mint.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <numeric>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

struct Affine { mint a, b; };
mint add(mint a, mint b) { return a + b; }
mint zero() { return 0; }
mint map_affine(Affine f, mint x, int n) { return f.a * x + f.b * n; }
Affine compose(Affine f, Affine g) { return {f.a * g.a, f.a * g.b + f.b}; }
Affine identity() { return {1, 0}; }
template <bool C, int B = 128>
using Sequence = LazyReversibleSequence<mint, add, zero, Affine, map_affine, compose, identity, C, B>;

std::string concat(const std::string &a, const std::string &b) { return a + b; }
std::string empty_string() { return {}; }
struct Shift { int k; };
std::string shift(Shift f, std::string s, int) {
	for (auto &c : s) c = char('a' + (c - 'a' + f.k) % 26);
	return s;
}
Shift compose_shift(Shift f, Shift g) { return {(f.k + g.k) % 26}; }
Shift no_shift() { return {0}; }
using Text = LazyReversibleSequence<std::string, concat, empty_string, Shift, shift, compose_shift, no_shift, false, 8>;

template <bool C, int B>
void random_test(std::mt19937_64 &rng) {
	for (int initial : {0, 1, 7, 31, 130, 500}) {
		std::vector<mint> a(initial);
		for (auto &value : a) value = rng() % 10000;
		Sequence<C, B> sequence(a);
		sequence.reserve(800);
		for (int step = 0; step < 5000; ++step) {
			int n = int(a.size()), command = int(rng() % 12);
			int l = int(rng() % (n + 1)), r = int(rng() % (n + 1));
			if (l > r) std::swap(l, r);
			if (!n || (command == 0 && n < 700)) {
				mint value = rng() % 100000;
				a.insert(a.begin() + l, value);
				sequence.insert(l, value);
			} else if (command == 1) {
				int p = int(rng() % n);
				assert(sequence.erase(p) == a[p]);
				a.erase(a.begin() + p);
			} else if (command == 2) {
				sequence.reverse(l, r);
				std::reverse(a.begin() + l, a.begin() + r);
			} else if (command == 3) {
				Affine f{rng() % 100, rng() % 10000};
				sequence.apply(l, r, f);
				for (int i = l; i < r; ++i) a[i] = map_affine(f, a[i], 1);
			} else if (command == 4) {
				int p = int(rng() % n);
				mint value = rng() % 100000;
				sequence.set(p, value);
				a[p] = value;
			} else if (command == 5) {
				assert(sequence.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, mint(0)));
			} else if (command == 6) {
				int m = l + int(rng() % (r - l + 1));
				sequence.rotate(l, m, r);
				std::rotate(a.begin() + l, a.begin() + m, a.begin() + r);
			} else if (command == 7) {
				int p = int(rng() % (n - (r - l) + 1));
				sequence.move(l, r, p);
				std::vector<mint> cut(a.begin() + l, a.begin() + r);
				a.erase(a.begin() + l, a.begin() + r);
				a.insert(a.begin() + p, cut.begin(), cut.end());
			} else if (command == 8) {
				int p = int(rng() % n);
				Affine f{rng() % 100, rng() % 100};
				sequence.apply(p, f);
				a[p] = map_affine(f, a[p], 1);
			} else if (command == 9 && step % 10 == 0) {
				auto copied = sequence;
				assert(copied.to_vector() == a);
				auto moved = std::move(copied);
				assert(copied.empty() && moved.to_vector() == a);
				copied.insert(0, 7);
				assert(copied.get(0) == mint(7));
				copied = std::move(moved);
				assert(moved.empty() && copied.to_vector() == a);
				auto *self = &copied;
				copied = *self;
				assert(copied.to_vector() == a);
				swap(copied, moved);
				assert(copied.empty() && moved.to_vector() == a);
			} else if (command == 10) {
				int p = int(rng() % n);
				assert(sequence.get(p) == a[p]);
			} else assert(sequence.to_vector() == a);
			assert(sequence.size() == int(a.size()) && sequence.empty() == a.empty());
			assert(sequence.all_prod() == std::accumulate(a.begin(), a.end(), mint(0)));
		}
		sequence.clear();
		assert(sequence.empty() && sequence.prod(0, 0) == mint(0) && sequence.to_vector().empty());
		sequence.assign(a);
		assert(sequence.to_vector() == a);
	}
}

void text_test() {
	for (int n = 0; n <= 10; ++n) {
		std::string letters;
		for (int i = 0; i < n; ++i) letters += char('a' + i);
		std::vector<std::string> symbols;
		for (char c : letters) symbols.emplace_back(1, c);
		for (int l = 0; l <= n; ++l) for (int r = l; r <= n; ++r) {
			Text sequence(symbols);
			sequence.apply(0, n, {7});
			sequence.reverse(l, r);
			auto expected = shift({7}, letters, n);
			std::reverse(expected.begin() + l, expected.begin() + r);
			assert(sequence.all_prod() == expected);
			for (int a = 0; a <= n; ++a) for (int b = a; b <= n; ++b)
				assert(sequence.prod(a, b) == expected.substr(a, b - a));
		}
	}
	std::mt19937 rng(78521);
	std::vector<std::string> a(100, "a");
	Text sequence(a);
	for (int step = 0; step < 12000; ++step) {
		int n = int(a.size()), command = int(rng() % 6);
		int l = int(rng() % (n + 1)), r = int(rng() % (n + 1));
		if (l > r) std::swap(l, r);
		if (!n || command == 0) {
			std::string value(1, char('a' + rng() % 26));
			sequence.insert(l, value);
			a.insert(a.begin() + l, value);
		} else if (command == 1) {
			int p = int(rng() % n);
			assert(sequence.erase(p) == a[p]);
			a.erase(a.begin() + p);
		} else if (command == 2) {
			sequence.reverse(l, r);
			std::reverse(a.begin() + l, a.begin() + r);
		} else if (command == 3) {
			Shift f{int(rng() % 26)};
			sequence.apply(l, r, f);
			for (int i = l; i < r; ++i) a[i] = shift(f, a[i], 1);
		} else if (command == 4) {
			assert(sequence.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, std::string{}));
		} else assert(sequence.to_vector() == a);
	}
}

using Matrix = std::array<mint, 4>;
Matrix multiply(const Matrix &a, const Matrix &b) {
	return {a[0] * b[0] + a[1] * b[2], a[0] * b[1] + a[1] * b[3],
		a[2] * b[0] + a[3] * b[2], a[2] * b[1] + a[3] * b[3]};
}
Matrix matrix_identity() { return {1, 0, 0, 1}; }
struct MatrixAction { Matrix forward, inverse; };
Matrix conjugate(const MatrixAction &f, const Matrix &a, int) { return multiply(multiply(f.forward, a), f.inverse); }
MatrixAction compose_matrix(const MatrixAction &f, const MatrixAction &g) {
	return {multiply(f.forward, g.forward), multiply(g.inverse, f.inverse)};
}
MatrixAction matrix_action_identity() { return {matrix_identity(), matrix_identity()}; }

void matrix_test() {
	using Matrices = LazyReversibleSequence<Matrix, multiply, matrix_identity, MatrixAction, conjugate, compose_matrix, matrix_action_identity, false, 8>;
	std::mt19937 rng(71983);
	auto random_matrix = [&] { return Matrix{rng() % 100, rng() % 100, rng() % 100, rng() % 100}; };
	std::vector<Matrix> a(129);
	for (auto &value : a) value = random_matrix();
	Matrices sequence(a);
	for (int step = 0; step < 4000; ++step) {
		int n = int(a.size()), command = int(rng() % 7);
		int l = int(rng() % (n + 1)), r = int(rng() % (n + 1));
		if (l > r) std::swap(l, r);
		if (!n || command == 0) {
			auto value = random_matrix();
			sequence.insert(l, value);
			a.insert(a.begin() + l, value);
		} else if (command == 1) {
			int p = int(rng() % n);
			assert(sequence.erase(p) == a[p]);
			a.erase(a.begin() + p);
		} else if (command == 2) {
			sequence.reverse(l, r);
			std::reverse(a.begin() + l, a.begin() + r);
		} else if (command == 3) {
			mint k = rng() % 100;
			MatrixAction f{{1, k, 0, 1}, {1, -k, 0, 1}};
			if (step & 1) f = {{1, 0, k, 1}, {1, 0, -k, 1}};
			sequence.apply(l, r, f);
			for (int i = l; i < r; ++i) a[i] = conjugate(f, a[i], 1);
		} else if (command == 4) {
			int p = int(rng() % n);
			a[p] = random_matrix();
			sequence.set(p, a[p]);
		} else {
			assert(sequence.prod(l, r) == std::accumulate(a.begin() + l, a.begin() + r, matrix_identity(), multiply));
			if (n) { int p = int(rng() % n); assert(sequence.get(p) == a[p]); }
		}
		assert(sequence.all_prod() == std::accumulate(a.begin(), a.end(), matrix_identity(), multiply));
		if (step % 97 == 0) assert(sequence.to_vector() == a);
	}
}

void all_positions_test() {
	for (int n = 0; n <= 35; ++n) for (int p = 0; p <= n; ++p) {
		std::vector<mint> expected(n);
		std::iota(expected.begin(), expected.end(), mint(0));
		Sequence<false, 8> sequence(expected);
		sequence.apply(0, n, {2, 3});
		sequence.reverse(0, n);
		for (auto &value : expected) value = value * 2 + 3;
		std::reverse(expected.begin(), expected.end());
		sequence.insert(p, 917);
		expected.insert(expected.begin() + p, mint(917));
		for (int i = 0; i <= n; ++i) assert(sequence.get(i) == expected[i]);
		assert(sequence.erase(p) == mint(917));
		expected.erase(expected.begin() + p);
		assert(sequence.to_vector() == expected);
	}
	Sequence<true, 65535> wide(std::vector<mint>(65535, 1));
	wide.insert(32768, 7);
	wide.reverse(1, 65535);
	assert(wide.erase(32767) == mint(7));
	wide.apply(0, wide.size(), {2, 3});
	assert(wide.size() == 65535 && wide.all_prod() == mint(65535 * 5));
	assert(wide.get(0) == mint(5) && wide.get(65534) == mint(5));
}

struct ThrowingMoveValue {
	int value;
	explicit ThrowingMoveValue(int x) : value(x) {}
	ThrowingMoveValue(const ThrowingMoveValue &) = default;
	ThrowingMoveValue &operator=(const ThrowingMoveValue &) = default;
	ThrowingMoveValue(ThrowingMoveValue &&other) noexcept(false) : value(other.value) {}
	ThrowingMoveValue &operator=(ThrowingMoveValue &&other) noexcept(false) { value = other.value; return *this; }
};
struct CopyOnlyValue {
	int value;
	explicit CopyOnlyValue(int x) : value(x) {}
	CopyOnlyValue(const CopyOnlyValue &other) noexcept(false) : value(other.value) {}
	CopyOnlyValue &operator=(const CopyOnlyValue &other) noexcept(false) { value = other.value; return *this; }
};
template<class V> V memory_add(V a, V b) { return V(a.value + b.value); }
template<class V> V memory_zero() { return V(0); }
template<class V> V memory_map(V f, V x, int n) { return V(x.value + f.value * n); }

template<class V>
void memory_accounting_test() {
	static_assert(!std::is_nothrow_move_constructible_v<V>);
	using Values = LazyReversibleSequence<V, memory_add<V>, memory_zero<V>, V, memory_map<V>, memory_add<V>, memory_zero<V>, true, 8>;
	std::vector<V> a(128, V(1));
	Values sequence(a);
	for (int i = 0; i < 96; ++i) {
		int p = (7 * i) % (sequence.size() + 1);
		sequence.insert(p, V(i));
		assert(sequence.erase(p).value == i);
	}
	sequence.apply(0, sequence.size(), V(2));
	sequence.reverse(0, sequence.size());
	for (auto &value : a) value = V(3);
	sequence.to_vector();
	Values fresh(a);
	sequence.reserve(4096), fresh.reserve(4096);
	assert(sequence.memory_usage() == fresh.memory_usage());
	for (int i = 0; i < sequence.size(); ++i) assert(sequence.get(i).value == a[i].value);
	for (int i = 0; i < 4096; ++i) sequence.insert((7 * i) % (sequence.size() + 1), V(i));
	auto copied = sequence;
	auto moved = std::move(copied);
	assert(copied.empty() && moved.all_prod().value == sequence.all_prod().value);
	sequence.clear();
	sequence.insert(0, V(9));
	assert(sequence.get(0).value == 9);
}

struct Box {
	int value;
	Box() = delete;
	explicit Box(int value) : value(value) {}
};
Box box_add(Box a, Box b) { return Box(a.value + b.value); }
Box box_zero() { return Box(0); }
Box box_map(Box f, Box a, int n) { return Box(a.value + f.value * n); }

void types_and_reuse() {
	LazyReversibleSequence<Box, box_add, box_zero, Box, box_map, box_add, box_zero, true> boxes;
	boxes.insert(0, Box(2)); boxes.insert(1, Box(3));
	boxes.apply(0, 2, Box(7)); boxes.reverse(0, 2);
	assert(boxes.get(0).value == 10 && boxes.all_prod().value == 19 && boxes.erase(1).value == 9);
	constexpr auto bit_op = [](bool a, bool b) { return a != b; };
	constexpr auto bit_id = [] { return false; };
	constexpr auto bit_map = [](bool f, bool a, int n) { return a != (f && (n & 1)); };
	LazyReversibleSequence<bool, bit_op, bit_id, bool, bit_map, bit_op, bit_id, true> bits(std::vector<bool>{true, false, true});
	bits.apply(0, 2, true);
	bits.reverse(0, 3);
	assert(bits.to_vector() == std::vector<bool>({true, true, false}));
	assert(!bits.all_prod() && bits.get(0) && bits.prod(1, 3));
	Sequence<true> recycled;
	recycled.insert(0, 0);
	assert(recycled.erase(0) == mint(0));
	std::size_t initial_bytes = 0;
	for (int i = 0; i < 10000; ++i) {
		recycled.insert(0, i);
		auto bytes = recycled.memory_usage();
		assert(recycled.erase(0) == mint(i));
		assert(recycled.memory_usage() == bytes);
		if (!i) initial_bytes = bytes;
		else assert(bytes == initial_bytes);
	}
	Sequence<true> ends;
	std::vector<mint> expected;
	for (int i = 0; i < 2048; ++i) {
		int p = i & 1 ? ends.size() : 0;
		ends.insert(p, i);
		expected.insert(expected.begin() + p, mint(i));
		assert(ends.all_prod() == std::accumulate(expected.begin(), expected.end(), mint(0)));
		assert(ends.get(0) == expected.front() && ends.get(ends.size() - 1) == expected.back());
		if (i % 13 == 0) {
			const auto &view = ends;
			assert(view.get(i / 3) == expected[i / 3]);
			assert(view.prod(i / 3, i) == std::accumulate(expected.begin() + i / 3, expected.begin() + i, mint(0)));
		}
		if (i % 79 == 0) {
			auto copied = ends;
			assert(copied.to_vector() == expected);
		}
	}
	assert(ends.to_vector() == expected);
	constexpr int n = 8192;
	std::vector<mint> a(n);
	std::iota(a.begin(), a.end(), mint(0));
	Sequence<true> sequence(a);
	std::mt19937 rng(778124);
	for (int step = 0; step < 20000; ++step) {
		int l = int(rng() % n), r = int(rng() % n);
		if (l > r) std::swap(l, r);
		sequence.reverse(l, r);
		std::reverse(a.begin() + l, a.begin() + r);
		int p = int(rng() % n);
		sequence.insert(p, 3);
		assert(sequence.erase(p) == mint(3));
		if (step % 97 == 0) assert(sequence.to_vector() == a);
	}
	assert(sequence.to_vector() == a);
	assert(sequence.memory_usage() < std::size_t(n) * sizeof(mint) * 64);
	Sequence<true> large(std::vector<mint>(200000, 1));
	large.apply(0, large.size(), {3, 7});
	large.reverse(0, large.size());
	large.reverse(1, large.size() - 1);
	assert(large.all_prod() == mint(2000000));
	assert(large.get(0) == mint(10) && large.get(large.size() - 1) == mint(10));
	assert(large.prod(12345, 13456) == mint(11110));
}

int main() {
	std::mt19937_64 rng(3495682);
	random_test<true, 8>(rng);
	random_test<false, 8>(rng);
	random_test<true, 128>(rng);
	random_test<false, 128>(rng);
	text_test();
	matrix_test();
	all_positions_test();
	memory_accounting_test<ThrowingMoveValue>();
	memory_accounting_test<CopyOnlyValue>();
	types_and_reuse();
}
