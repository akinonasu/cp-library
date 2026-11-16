#include "data_structure/static_threshold_search.hpp"
#include <array>
#include <random>
#include <stdexcept>
#include <string>
#include <type_traits>

template<class T, bool Smaller, bool Reverse> void check(const std::vector<T> &a, std::mt19937_64 &rng) {
	StaticThresholdSearch<T, Smaller, Reverse> index(a);
	int n = int(a.size());
	assert(index.size() == n && index.empty() == a.empty());
	for (int q = 0; q < 220; ++q) {
		int l = int(rng() % (n + 1)), r = int(rng() % (n + 1)); if (l > r) std::swap(l, r);
		T x = q % 4 == 0 ? std::numeric_limits<T>::lowest() : q % 4 == 1 ? std::numeric_limits<T>::max() : q % 4 == 2 && n ? a[rng() % n] : T(rng());
		for (bool strict : {false, true}) {
			int expected = n;
			for (int i = l; i < n; ++i) { bool ok = Smaller ? (strict ? a[i] < x : a[i] <= x) : (strict ? a[i] > x : a[i] >= x); if (ok) { expected = i; break; } }
			if constexpr (!Reverse) {
				assert((strict ? index.next_strict(l, x) : index.next(l, x)) == expected);
				assert((strict ? index.first_strict(l, r, x) : index.first(l, r, x)) == (expected < r ? expected : n));
				if constexpr (Smaller) assert((strict ? index.next_lt(l, x) : index.next_le(l, x)) == expected);
				else assert((strict ? index.next_gt(l, x) : index.next_ge(l, x)) == expected);
			} else {
				expected = n;
				for (int i = 0; i < r; ++i) { bool ok = Smaller ? (strict ? a[i] < x : a[i] <= x) : (strict ? a[i] > x : a[i] >= x); if (ok) expected = i; }
				assert((strict ? index.prev_strict(r, x) : index.prev(r, x)) == expected);
				assert((strict ? index.last_strict(l, r, x) : index.last(l, r, x)) == (expected != n && expected >= l ? expected : n));
				if constexpr (Smaller) assert((strict ? index.prev_lt(r, x) : index.prev_le(r, x)) == expected);
				else assert((strict ? index.prev_gt(r, x) : index.prev_ge(r, x)) == expected);
			}

		}
	}
	auto record = [&](const auto &source, int at) {
		int expected = n;
		if constexpr (Reverse) { for (int i = at; i-- > 0;) if (Smaller ? a[i] < a[at] : a[i] > a[at]) { expected = i; break; } }
		else { for (int i = at + 1; i < n; ++i) if (Smaller ? a[i] < a[at] : a[i] > a[at]) { expected = i; break; } }
		assert(source.next_record(at) == expected);
	};
	if (n <= 129) for (int at = 0; at < n; ++at) record(index, at);
	else for (int at : {0, n / 2, n - 1}) record(index, at);
	auto copy = index; copy.build({T(7)}); assert(index.size() == n);
	copy = index; StaticThresholdSearch<T, Smaller, Reverse> moved(std::move(copy)); assert(copy.empty() && moved.size() == n);
	if (n) for (int at : {0, n / 2, n - 1}) record(moved, at);
	copy = std::move(moved); assert(moved.empty());
	if (n) for (int at : {0, n / 2, n - 1}) record(copy, at);
	copy.clear(); assert(copy.size() == 0);
}

template<class T> void suite() {
	std::mt19937_64 rng(392914);
	for (int n : {0, 1, 2, 31, 32, 63, 64, 65, 127, 128, 129, 255, 4095, 4096, 4097, 10000}) {
		for (int shape = 0; shape < 7; ++shape) {
			std::vector<T> a(n);
			for (int i = 0; i < n; ++i) a[i] = shape == 0 ? T(rng()) : shape == 1 ? T(std::int64_t(i) * i) : shape == 2 ? T(std::int64_t(n - i) * (n - i)) : shape == 3 ? std::numeric_limits<T>::lowest() : shape == 4 ? std::numeric_limits<T>::max() : T(rng() % 10);
			check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng);
		}
	}
	for (int n : {65, 129, 4097}) for (int at : {0, 31, 32, 63, 64, n / 2, n - 1}) {
		for (bool invert : {false, true}) {
			std::vector<T> a(n, T(invert ? 7 : 0)); a[at] = T(invert ? 0 : 7);
			check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng);
		}
	}
	for (int trial = 0; trial < 120; ++trial) { std::vector<T> a(rng() % 90); for (auto &x : a) x = T(rng() % 50); check<T, false, false>(a, rng); check<T, true, false>(a, rng); check<T, false, true>(a, rng); check<T, true, true>(a, rng); }
}

template <class T, bool Smaller, bool Reverse, class Compare>
void inspect_ordered(const StaticThresholdSearch<T, Smaller, Reverse, Compare> &index,
		const std::vector<T> &values, const std::vector<T> &thresholds, Compare compare) {
	int n = int(values.size());
	assert(index.size() == n && index.empty() == values.empty());
	assert(index.values().size() == values.size());
	for (int i = 0; i < n; ++i) assert(index[i] == values[i]);
	auto accepts = [&](const T &value, const T &threshold, bool strict) {
		if constexpr (Smaller) return strict ? compare(value, threshold) : !compare(threshold, value);
		else return strict ? compare(threshold, value) : !compare(value, threshold);
	};
	std::mt19937 random(12371 + n);
	for (int q = 0; q < 80; ++q) {
		int l = random() % (n + 1), r = random() % (n + 1);
		if (l > r) std::swap(l, r);
		const T &threshold = thresholds[random() % thresholds.size()];
		for (bool strict : {false, true}) {
			int expected = n, interval = n;
			if constexpr (Reverse) {
				for (int i = 0; i < r; ++i)
					if (accepts(values[i], threshold, strict)) {
						expected = i;
						if (i >= l) interval = i;
					}
				assert((strict ? index.prev_strict(r, threshold) : index.prev(r, threshold)) == expected);
				assert((strict ? index.last_strict(l, r, threshold) : index.last(l, r, threshold)) == interval);
				if constexpr (Smaller) assert((strict ? index.prev_lt(r, threshold) : index.prev_le(r, threshold)) == expected);
				else assert((strict ? index.prev_gt(r, threshold) : index.prev_ge(r, threshold)) == expected);
			} else {
				for (int i = l; i < n; ++i)
					if (accepts(values[i], threshold, strict)) { expected = i; break; }
				if (expected < r) interval = expected;
				assert((strict ? index.next_strict(l, threshold) : index.next(l, threshold)) == expected);
				assert((strict ? index.first_strict(l, r, threshold) : index.first(l, r, threshold)) == interval);
				if constexpr (Smaller) assert((strict ? index.next_lt(l, threshold) : index.next_le(l, threshold)) == expected);
				else assert((strict ? index.next_gt(l, threshold) : index.next_ge(l, threshold)) == expected);
			}
		}
	}
	std::vector<int> positions;
	if (n <= 129) { for (int i = 0; i < n; ++i) positions.push_back(i); }
	else positions = {0, 1, 63, 64, 65, n / 2, n - 1};
	for (int at : positions) {
		int expected = n;
		if constexpr (Reverse) {
			for (int i = at; i-- > 0;)
				if (accepts(values[i], values[at], true)) { expected = i; break; }
		} else {
			for (int i = at + 1; i < n; ++i)
				if (accepts(values[i], values[at], true)) { expected = i; break; }
		}
		assert(index.next_record(at) == expected);
	}
}

template <class T, class Compare>
void ordered_shapes(const std::vector<T> &alphabet, Compare compare) {
	std::mt19937 random(42931);
	for (int n : {0, 1, 2, 63, 64, 65, 127, 128, 129, 191, 192, 193, 255, 256, 257, 511, 512, 513, 4097}) {
		for (int shape = 0; shape < 5; ++shape) {
			std::vector<T> values;
			for (int i = 0; i < n; ++i) {
				std::size_t key = random() % alphabet.size();
				if (shape == 1) key = 0;
				if (shape == 2) key = alphabet.size() - 1;
				if (shape == 3) key = i ? 0 : alphabet.size() - 1;
				if (shape == 4) key = i ? alphabet.size() - 1 : 0;
				values.push_back(alphabet[key]);
			}
			inspect_ordered(StaticThresholdSearch<T, false, false, Compare>(values, compare), values, alphabet, compare);
			inspect_ordered(StaticThresholdSearch<T, true, false, Compare>(values, compare), values, alphabet, compare);
			inspect_ordered(StaticThresholdSearch<T, false, true, Compare>(values, compare), values, alphabet, compare);
			inspect_ordered(StaticThresholdSearch<T, true, true, Compare>(values, compare), values, alphabet, compare);
		}
	}
}

struct ThresholdRecord {
	int key, payload;
	ThresholdRecord() = delete;
	explicit ThresholdRecord(int key, int payload = 0) : key(key), payload(payload) {}
	bool operator==(const ThresholdRecord &) const = default;
};
struct ThresholdCompare {
	bool reverse;
	ThresholdCompare() = delete;
	explicit ThresholdCompare(bool reverse) : reverse(reverse) {}
	bool operator()(const ThresholdRecord &a, const ThresholdRecord &b) const { return reverse ? a.key > b.key : a.key < b.key; }
};
struct ThrowingThresholdCompare {
	bool reverse;
	static inline bool fail_copy = false, fail_swap = false;
	ThrowingThresholdCompare() = delete;
	explicit ThrowingThresholdCompare(bool reverse) : reverse(reverse) {}
	ThrowingThresholdCompare(const ThrowingThresholdCompare &other) : reverse(other.reverse) { if (fail_copy) throw std::runtime_error("copy"); }
	ThrowingThresholdCompare &operator=(const ThrowingThresholdCompare &) = default;
	bool operator()(const ThresholdRecord &a, const ThresholdRecord &b) const { return reverse ? a.key > b.key : a.key < b.key; }
	friend void swap(ThrowingThresholdCompare &a, ThrowingThresholdCompare &b) noexcept(false) {
		if (fail_swap) throw std::runtime_error("swap");
		std::swap(a.reverse, b.reverse);
	}
};


struct SplitCopyThresholdCompare {
	bool reverse;
	static inline bool reject_mutable = false;
	static inline int mutable_copies = 0, const_copies = 0;
	SplitCopyThresholdCompare() = delete;
	explicit SplitCopyThresholdCompare(bool reverse) : reverse(reverse) {}
	SplitCopyThresholdCompare(SplitCopyThresholdCompare &other) : reverse(other.reverse) {
		++mutable_copies;
		if (reject_mutable) throw std::runtime_error("mutable copy");
	}
	SplitCopyThresholdCompare(const SplitCopyThresholdCompare &other) noexcept : reverse(other.reverse) { ++const_copies; }
	SplitCopyThresholdCompare &operator=(const SplitCopyThresholdCompare &) = default;
	bool operator()(const ThresholdRecord &a, const ThresholdRecord &b) const { return reverse ? a.key > b.key : a.key < b.key; }
};

void split_copy_comparator() {
	using Index = StaticThresholdSearch<ThresholdRecord, false, false, SplitCopyThresholdCompare>;
	static_assert(std::is_nothrow_move_constructible_v<Index> && std::is_nothrow_move_assignable_v<Index>);
	std::vector<ThresholdRecord> values{ThresholdRecord(2), ThresholdRecord(8), ThresholdRecord(4)};
	std::vector<ThresholdRecord> thresholds{ThresholdRecord(0), ThresholdRecord(3), ThresholdRecord(7), ThresholdRecord(9)};
	Index source(values, SplitCopyThresholdCompare(true));
	Index destination(values, SplitCopyThresholdCompare(false));
	int mutable_copies = SplitCopyThresholdCompare::mutable_copies;
	int const_copies = SplitCopyThresholdCompare::const_copies;
	SplitCopyThresholdCompare::reject_mutable = true;
	Index moved(std::move(source));
	assert(source.empty() && SplitCopyThresholdCompare::mutable_copies == mutable_copies);
	assert(SplitCopyThresholdCompare::const_copies == const_copies + 1);
	inspect_ordered(moved, values, thresholds, SplitCopyThresholdCompare(true));
	destination = std::move(moved);
	assert(moved.empty() && SplitCopyThresholdCompare::mutable_copies == mutable_copies);
	inspect_ordered(destination, values, thresholds, SplitCopyThresholdCompare(true));
	SplitCopyThresholdCompare::reject_mutable = false;
	source.build(values);
	moved.build(values);
	inspect_ordered(source, values, thresholds, SplitCopyThresholdCompare(true));
	inspect_ordered(moved, values, thresholds, SplitCopyThresholdCompare(true));
}

void comparator_lifecycle() {
	using Index = StaticThresholdSearch<ThresholdRecord, false, false, ThresholdCompare>;
	std::vector<ThresholdRecord> values;
	for (int i = 0; i < 65; ++i) values.emplace_back(i % 9, i);
	std::vector<ThresholdRecord> thresholds{ThresholdRecord(-1), ThresholdRecord(0), ThresholdRecord(4), ThresholdRecord(8), ThresholdRecord(9)};
	Index index(values, ThresholdCompare(true));
	inspect_ordered(index, values, thresholds, ThresholdCompare(true));
	auto copied = index;
	copied.build(values, ThresholdCompare(false));
	inspect_ordered(copied, values, thresholds, ThresholdCompare(false));
	inspect_ordered(index, values, thresholds, ThresholdCompare(true));
	swap(index, copied);
	inspect_ordered(index, values, thresholds, ThresholdCompare(false));
	copied.clear();
	assert(copied.empty());
	copied.build(values);
	inspect_ordered(copied, values, thresholds, ThresholdCompare(true));
	Index moved(std::move(copied));
	assert(copied.empty());
	copied.build(values);
	inspect_ordered(copied, values, thresholds, ThresholdCompare(true));
	inspect_ordered(moved, values, thresholds, ThresholdCompare(true));
	index = std::move(moved);
	assert(moved.empty());
	inspect_ordered(index, values, thresholds, ThresholdCompare(true));
	moved.build(values);
	inspect_ordered(moved, values, thresholds, ThresholdCompare(true));
	index = copied;
	inspect_ordered(index, values, thresholds, ThresholdCompare(true));
	using Ordinary = StaticThresholdSearch<int>;
	static_assert(std::is_nothrow_move_constructible_v<Ordinary> && std::is_nothrow_move_assignable_v<Ordinary> && std::is_nothrow_swappable_v<Ordinary>);
	using Throwing = StaticThresholdSearch<ThresholdRecord, false, false, ThrowingThresholdCompare>;
	static_assert(!std::is_nothrow_move_constructible_v<Throwing> && !std::is_nothrow_move_assignable_v<Throwing> && !std::is_nothrow_swappable_v<Throwing>);
	Throwing original(values, ThrowingThresholdCompare(true));
	ThrowingThresholdCompare::fail_copy = true;
	bool threw = false;
	try { Throwing failed(std::move(original)); } catch (const std::runtime_error &) { threw = true; }
	assert(threw && original.size() == int(values.size()));
	ThrowingThresholdCompare::fail_copy = false;
	inspect_ordered(original, values, thresholds, ThrowingThresholdCompare(true));
	Throwing other(values, ThrowingThresholdCompare(false));
	ThrowingThresholdCompare::fail_swap = true;
	threw = false;
	try { original.swap(other); } catch (const std::runtime_error &) { threw = true; }
	assert(threw);
	ThrowingThresholdCompare::fail_swap = false;
	inspect_ordered(original, values, thresholds, ThrowingThresholdCompare(true));
	inspect_ordered(other, values, thresholds, ThrowingThresholdCompare(false));
}

void generic_keys() {
	ordered_shapes<std::string>({"", "a", "ab", "b", "long", "z"}, std::less<std::string>{});
	ordered_shapes<std::string>({"", "a", "ab", "b", "long", "z"}, std::greater<std::string>{});
	ordered_shapes<std::pair<int, int>>({{-100, 0}, {-1, 2}, {0, 0}, {0, 1}, {100, -1}}, std::less<std::pair<int, int>>{});
	ordered_shapes<ThresholdRecord>({ThresholdRecord(-10, 3), ThresholdRecord(0, 1), ThresholdRecord(0, 9), ThresholdRecord(10, -4)}, ThresholdCompare(false));
	ordered_shapes<ThresholdRecord>({ThresholdRecord(-10, 3), ThresholdRecord(0, 1), ThresholdRecord(0, 9), ThresholdRecord(10, -4)}, ThresholdCompare(true));
	ordered_shapes<double>({-std::numeric_limits<double>::infinity(), -100.0, -0.0, 0.5, 100.0, std::numeric_limits<double>::infinity()}, std::less<double>{});
	comparator_lifecycle();
	split_copy_comparator();
}

int main() { generic_keys();  suite<std::int64_t>(); suite<std::uint64_t>(); suite<int>(); suite<unsigned>(); suite<signed char>(); suite<unsigned short>(); }
