#include "data_structure/dynamic_bitset.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Bits = std::vector<unsigned char>;

DynamicBitset make(const Bits &bits) {
	DynamicBitset result(int(bits.size()));
	for (int i = 0; i < int(bits.size()); ++i) if (bits[i]) result.set(i);
	return result;
}

int count(const Bits &bits, int l, int r) {
	return std::accumulate(bits.begin() + l, bits.begin() + r, 0);
}

Bits shifted(const Bits &bits, std::size_t shift, bool left) {
	Bits result(bits.size());
	for (std::size_t i = 0; i < bits.size(); ++i) {
		if (left) { if (shift <= i) result[i] = bits[i - shift]; }
		else if (shift < bits.size() - i) result[i] = bits[i + shift];
	}
	return result;
}

void verify(const DynamicBitset &set, const Bits &bits) {
	int n = int(bits.size()), ones = count(bits, 0, n);
	assert(set.size() == n && set.empty() == bits.empty());
	assert(set.count() == ones && set.any() == (ones != 0));
	assert(set.none() == (ones == 0) && set.all() == (ones == n));
	assert(set.words().size() == (bits.size() + 63) / 64);
	if (n % 64) assert((set.words().back() >> (n % 64)) == 0);
	std::string text;
	for (int i = n; i-- > 0;) text += char('0' + bits[i]);
	assert(set.to_string() == text);
	assert(DynamicBitset(std::string_view(text)) == set);
	assert(DynamicBitset::from_words(n, set.words()) == set);
	for (int i = 0; i < n; ++i) assert(set[i] == bool(bits[i]) && set.test(i) == bool(bits[i]));
	int prefix = 0;
	for (int r = 0; r <= n; ++r) {
		assert(set.rank(r) == prefix);
		assert(set.count(0, r) == prefix && set.count(r, n) == ones - prefix);
		assert(set.count(r, r) == 0);
		if (r < n) prefix += bits[r];
	}
	for (bool value : {false, true}) {
		std::vector<int> positions;
		for (int i = 0; i < n; ++i) if (bool(bits[i]) == value) positions.push_back(i);
		assert(set.find_first(value) == (positions.empty() ? n : positions.front()));
		assert(set.find_last(value) == (positions.empty() ? n : positions.back()));
		for (int pos = -1; pos < n; ++pos) {
			auto it = std::upper_bound(positions.begin(), positions.end(), pos);
			assert(set.find_next(pos, value) == (it == positions.end() ? n : *it));
		}
		for (int pos = 0; pos <= n; ++pos) {
			auto it = std::lower_bound(positions.begin(), positions.end(), pos);
			assert(set.find_prev(pos, value) == (it == positions.begin() ? n : *std::prev(it)));
		}
		for (int k = 0; k <= int(positions.size()); ++k)
			assert(set.select(k, value) == (k == int(positions.size()) ? n : positions[k]));
		assert(set.select(std::numeric_limits<int>::max(), value) == n);
	}
	assert(set.find_first() == set.find_first(true) && set.find_last() == set.find_last(true));
	assert(set.select(0) == set.find_first());
	std::vector<int> listed, expected;
	set.for_each([&](int i) { listed.push_back(i); });
	for (int i = 0; i < n; ++i) if (bits[i]) expected.push_back(i);
	assert(listed == expected);
	int l = n / 3, r = n - n / 4;
	listed.clear(); expected.clear();
	set.for_each(l, r, [&](int i) { listed.push_back(i); });
	for (int i = l; i < r; ++i) if (bits[i]) expected.push_back(i);
	assert(listed == expected && set.count(l, r) == int(expected.size()));
	assert(set.slice(l, r) == make(Bits(bits.begin() + l, bits.begin() + r)));
}

void pairs(const Bits &a, const Bits &b) {
	assert(a.size() == b.size());
	DynamicBitset x = make(a), y = make(b);
	Bits intersection(a.size()), either(a.size()), difference(a.size()), exclusive(a.size()), complement(a.size());
	for (int i = 0; i < int(a.size()); ++i) {
		intersection[i] = a[i] && b[i]; either[i] = a[i] || b[i];
		difference[i] = a[i] && !b[i]; exclusive[i] = a[i] != b[i]; complement[i] = !a[i];
	}
	assert((x & y) == make(intersection) && (x | y) == make(either));
	assert((x - y) == make(difference) && (x ^ y) == make(exclusive) && ~x == make(complement));
	auto z = x; z &= y; assert(z == make(intersection));
	z = x; z |= y; assert(z == make(either));
	z = x; z ^= y; assert(z == make(exclusive));
	z = x; z -= y; assert(z == make(difference));
	z = x; z &= z; assert(z == x);
	z = x; z |= z; assert(z == x);
	z = x; z ^= z; assert(z.none() && z.size() == x.size());
	z = x; z -= z; assert(z.none() && z.size() == x.size());
	int common = count(intersection, 0, int(a.size())), different = count(difference, 0, int(a.size()));
	assert(x.count_and(y) == common && x.count_or(y) == count(either, 0, int(a.size())));
	assert(x.count_xor(y) == count(exclusive, 0, int(a.size())) && x.count_and_not(y) == different);
	assert(x.intersects(y) == (common > 0));
	assert(x.intersection_parity(y) == bool(common & 1));
	assert(x.is_subset_of(y) == (different == 0));
	assert(x.is_proper_subset_of(y) == (different == 0 && a != b));
	assert((x == y) == (a == b));
	int comparison = 0;
	for (int i = int(a.size()); i-- > 0;) if (a[i] != b[i]) { comparison = a[i] ? 1 : -1; break; }
	assert(x.compare(y) == comparison && y.compare(x) == -comparison);
}

void shifts(const Bits &bits, std::size_t shift) {
	DynamicBitset x = make(bits);
	for (bool left : {false, true}) {
		Bits expected = shifted(bits, shift, left);
		auto actual = left ? x << shift : x >> shift;
		assert(actual == make(expected));
		actual = x;
		if (left) actual <<= shift; else actual >>= shift;
		assert(actual == make(expected));
		for (int i = 0; i < int(bits.size()); ++i) expected[i] |= bits[i];
		actual = x;
		if (left) actual.or_shift_left(shift); else actual.or_shift_right(shift);
		assert(actual == make(expected));
	}
}

void lifetime() {
	DynamicBitset empty;
	verify(empty, {});
	empty.reverse().rotate_left(std::numeric_limits<std::size_t>::max()).rotate_right(0);
	verify(empty, {});
	assert(empty != DynamicBitset(1));
	std::array<std::uint64_t, 3> words{~std::uint64_t(0), ~std::uint64_t(0), ~std::uint64_t(0)};
	for (int n : {1, 63, 64, 65, 127, 128, 129}) {
		auto x = DynamicBitset::from_words(n, std::span(words).first((n + 63) / 64));
		verify(x, Bits(n, 1));
		auto copy = x;
		copy.flip_all();
		assert(x.all() && copy.none());
		DynamicBitset moved(std::move(copy));
		assert(copy.empty() && copy.count() == 0 && copy.words().empty());
		copy = std::move(moved);
		assert(moved.empty() && copy.size() == n && copy.none());
		copy.swap(x);
		assert(copy.all() && x.none());
		copy.set_all(false); assert(copy.none());
		copy.flip_all(); assert(copy.all());
		copy.reset(n - 1); assert(!copy.test(n - 1));
		copy.set(n - 1); assert(copy.all());
		copy.clear(); verify(copy, {});
		copy.resize(n, true); verify(copy, Bits(n, 1));
	}
	for (std::string text : std::vector<std::string>{"x", "10x01", " 01", "01\n", "012", std::string("1\0", 2)}) {
		bool failed = false;
		try { DynamicBitset invalid{std::string_view(text)}; } catch (const std::invalid_argument &) { failed = true; }
		assert(failed);
	}
	DynamicBitset x("101010");
	x[0] = x[1]; assert(x.test(0));
	x[1] = false; assert(!x.test(1));
	x[2].flip(); assert(x.test(2));
	x.reserve(1000);
	auto reference = x[1];
	reference = true;
	assert(x.test(1));
	reference.flip();
	assert(!x.test(1));
}

void overlaps() {
	std::mt19937_64 rng(2953187);
	for (int n : {0, 1, 2, 7, 13, 63, 64, 65, 127, 128, 129}) {
		Bits original(n);
		for (auto &bit : original) bit = rng() % 2;
		auto source = make(original);
		std::vector<int> lengths{0, 1, 2, 3, 31, 32, 63, 64, 65, 66, 127, n};
		if (n < 20) for (int length = 0; length <= n; ++length) lengths.push_back(length);
		std::sort(lengths.begin(), lengths.end());
		lengths.erase(std::unique(lengths.begin(), lengths.end()), lengths.end());
		for (int length : lengths) {
			if (length > n) continue;
			for (int l = 0; l + length <= n; ++l) for (int dst = 0; dst + length <= n; ++dst) {
				auto actual = source;
				actual.copy_range(dst, actual, l, l + length);
				auto expected = original;
				for (int i = 0; i < length; ++i) expected[dst + i] = original[l + i];
				assert(actual == make(expected));
			}
		}
	}
}

void large_words() {
	std::mt19937_64 rng(629721);
	for (int n : {511, 512, 513, 2047, 2048, 2049, 4095, 4096, 4097, 65535}) {
		Bits a(n), b(n);
		for (int i = 0; i < n; ++i) { a[i] = rng() & 1; b[i] = rng() % 7 == 0; }
		pairs(a, b);
		auto x = make(a);
		assert(x.count() == count(a, 0, n));
		for (int trial = 0; trial < 80; ++trial) {
			int l = rng() % (n + 1), r = rng() % (n + 1);
			if (l > r) std::swap(l, r);
			assert(x.count(l, r) == count(a, l, r));
			assert(x.rank(r) == count(a, 0, r));
		}
		for (std::size_t shift : {std::size_t(1), std::size_t(63), std::size_t(64), std::size_t(65), std::size_t(511), std::size_t(n - 1)}) shifts(a, shift);
		x.flip_all();
		assert(x.count() == n - count(a, 0, n));
		x.set_all(); assert(x.count() == n && x.all());
		x.reset_all(); assert(x.count() == 0 && x.none());
		std::vector<std::uint64_t> words((n + 63) / 64, ~std::uint64_t(0));
		auto full = DynamicBitset::from_words(n, words);
		assert(full.count() == n && full.count_and(full) == n);
		assert(full.count_xor(full) == 0 && full.count_and_not(full) == 0);
	}
}

int main() {
	lifetime();
	overlaps();
	large_words();
	std::mt19937_64 random(392107);
	std::vector<int> sizes{0, 1, 2, 7, 31, 32, 33, 63, 64, 65, 66, 127, 128, 129, 191, 192, 193, 255, 256, 257};
	for (int n : sizes) {
		for (int pattern = 0; pattern < 5; ++pattern) {
			Bits bits(n);
			for (int i = 0; i < n; ++i) bits[i] = pattern == 0 ? 0 : pattern == 1 ? 1 : pattern == 2 ? i % 2 : pattern == 3 ? i % 63 == 0 : random() % 2;
			verify(make(bits), bits);
			Bits other(n);
			for (auto &bit : other) bit = random() % 2;
			pairs(bits, other);
			for (std::size_t shift : {std::size_t(0), std::size_t(1), std::size_t(63), std::size_t(64), std::size_t(65), std::size_t(n), std::size_t(n + 1), std::numeric_limits<std::size_t>::max()}) shifts(bits, shift);
			for (int l = 0; l <= n; ++l) {
				int r = l + int(random() % (n - l + 1));
				auto x = make(bits); x.set_range(l, r); auto expected = bits;
				std::fill(expected.begin() + l, expected.begin() + r, 1); assert(x == make(expected));
				x = make(bits); x.reset_range(l, r); expected = bits;
				std::fill(expected.begin() + l, expected.begin() + r, 0); assert(x == make(expected));
				x = make(bits); x.flip_range(l, r); expected = bits;
				for (int i = l; i < r; ++i) expected[i] ^= 1;
				assert(x == make(expected));
			}
		}
	}
	for (int trial = 0; trial < 80; ++trial) {
		int n = sizes[random() % sizes.size()];
		Bits bits(n);
		for (auto &bit : bits) bit = random() % 2;
		auto x = make(bits);
		for (int step = 0; step < 250; ++step) {
			n = int(bits.size());
			int op = random() % 19, l = int(random() % (n + 1)), r = int(random() % (n + 1));
			if (l > r) std::swap(l, r);
			if (op == 0 && n) { int i = random() % n; bool value = random() % 2; x.set(i, value); bits[i] = value; }
			if (op == 1 && n) { int i = random() % n; x.flip(i); bits[i] ^= 1; }
			if (op == 2) { bool value = random() % 2; x.set_range(l, r, value); std::fill(bits.begin() + l, bits.begin() + r, value); }
			if (op == 3) { x.flip_range(l, r); for (int i = l; i < r; ++i) bits[i] ^= 1; }
			if (op == 4) { int target = random() % 350; bool value = random() % 2; x.resize(target, value); bits.resize(target, value); }
			if (op == 5) { bool value = random() % 2; x.push_back(value); bits.push_back(value); }
			if (op == 6 && n) { x.pop_back(); bits.pop_back(); }
			if (op == 7) { x.reverse(); std::reverse(bits.begin(), bits.end()); }
			if (op == 8 || op == 9) {
				std::size_t shift = random();
				if (op == 8) x.rotate_left(shift); else x.rotate_right(shift);
				Bits expected(n);
				if (n) for (int i = 0; i < n; ++i) expected[(i + (op == 8 ? shift % n : n - shift % n)) % n] = bits[i];
				bits = expected;
			}
			if (op == 10 || op == 11) {
				std::size_t shift = random() % (n + 100);
				if (op == 10) x <<= shift; else x >>= shift;
				bits = shifted(bits, shift, op == 10);
			}
			if (op == 12 || op == 13) {
				std::size_t shift = random() % (n + 100);
				if (op == 12) x.or_shift_left(shift); else x.or_shift_right(shift);
				auto added = shifted(bits, shift, op == 12);
				for (int i = 0; i < n; ++i) bits[i] |= added[i];
			}
			if (op == 14) {
				int dst = random() % (n - (r - l) + 1);
				x.copy_range(dst, x, l, r);
				auto old = bits;
				for (int i = l; i < r; ++i) bits[dst + i - l] = old[i];
			}
			if (op == 15) {
				Bits source(n); for (auto &bit : source) bit = random() % 2;
				int dst = random() % (n - (r - l) + 1);
				x.copy_range(dst, make(source), l, r);
				std::copy(source.begin() + l, source.begin() + r, bits.begin() + dst);
			}
			if (op == 16 && n < 150) {
				x.append(x); auto old = bits; bits.insert(bits.end(), old.begin(), old.end());
			}
			if (op == 17) {
				Bits source(random() % 20); for (auto &bit : source) bit = random() % 2;
				x.append(make(source)); bits.insert(bits.end(), source.begin(), source.end());
			}
			if (op == 18) {
				bool value = random() % 2;
				if (value) x.set_all(); else x.reset_all();
				std::fill(bits.begin(), bits.end(), value);
			}
			if (step % 20 == 0) verify(x, bits);
			else assert(x == make(bits));
		}
		verify(x, bits);
	}
}
