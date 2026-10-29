#ifndef DYNAMIC_BITSET_HPP
#define DYNAMIC_BITSET_HPP

#include "../components/bitset_ops.hpp"
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class DynamicBitset {
	using Word = std::uint64_t;
	using Operation = bitset_ops_detail::Operation;
	std::vector<Word> data_;
	int n_ = 0;
	static std::size_t word_count(int n) { return (std::size_t(n) + 63) / 64; }
	static Word mask(int n) { return n == 64 ? ~Word(0) : (Word(1) << n) - 1; }
	void trim() { if (n_ & 63) data_.back() &= mask(n_ & 63); }
	void check([[maybe_unused]] int p) const { assert(0 <= p && p < n_); }
	void check_range([[maybe_unused]] int l, [[maybe_unused]] int r) const { assert(0 <= l && l <= r && r <= n_); }
	void check_size([[maybe_unused]] const DynamicBitset &other) const { assert(n_ == other.n_); }
	Word chosen(std::size_t i, bool value) const {
		Word x = value ? data_[i] : ~data_[i];
		if (!value && i + 1 == data_.size() && (n_ & 63)) x &= mask(n_ & 63);
		return x;
	}
	Word extract(int p, int length) const {
		std::size_t w = unsigned(p) / 64;
		int offset = p & 63;
		Word x = data_[w] >> offset;
		if (offset && length > 64 - offset) x |= data_[w + 1] << (64 - offset);
		return x & mask(length);
	}
	template <class F> void update_range(int l, int r, F apply) {
		check_range(l, r);
		if (l == r) return;
		std::size_t first = unsigned(l) / 64, last = unsigned(r - 1) / 64;
		Word left = ~Word(0) << (l & 63), right = mask(((r - 1) & 63) + 1);
		if (first == last) { apply(data_[first], left & right); return; }
		apply(data_[first], left);
		for (auto i = first + 1; i < last; ++i) apply(data_[i], ~Word(0));
		apply(data_[last], right);
	}
	template <Operation Op> int count_with(const DynamicBitset &other) const {
		check_size(other);
		if (data_.size() >= 8) return int(bitset_ops_detail::count<Op>(data_.data(), other.data_.data(), data_.size()));
		int result = 0;
		for (std::size_t i = 0; i < data_.size(); ++i) result += std::popcount(bitset_ops_detail::combine<Op>(data_[i], other.data_[i]));
		return result;
	}
	template <bool Left, bool Or> DynamicBitset &shift(std::size_t amount) {
		if (amount >= std::size_t(n_)) { if constexpr (!Or) reset_all(); return *this; }
		if (!amount) return *this;
		std::size_t skip = amount / 64, length = data_.size();
		unsigned bits = amount & 63;
		if constexpr (Left) {
			for (std::size_t i = length; i-- > skip;) {
				Word x = data_[i - skip] << bits;
				if (bits && i > skip) x |= data_[i - skip - 1] >> (64 - bits);
				if constexpr (Or) data_[i] |= x;
				else data_[i] = x;
			}
			if constexpr (!Or) std::fill(data_.begin(), data_.begin() + skip, 0);
		} else {
			for (std::size_t i = 0; i < length - skip; ++i) {
				Word x = data_[i + skip] >> bits;
				if (bits && i + skip + 1 < length) x |= data_[i + skip + 1] << (64 - bits);
				if constexpr (Or) data_[i] |= x;
				else data_[i] = x;
			}
			if constexpr (!Or) std::fill(data_.end() - skip, data_.end(), 0);
		}
		trim();
		return *this;
	}
	static Word reverse_word(Word x) {
		x = ((x >> 1) & 0x5555555555555555ULL) | ((x & 0x5555555555555555ULL) << 1);
		x = ((x >> 2) & 0x3333333333333333ULL) | ((x & 0x3333333333333333ULL) << 2);
		x = ((x >> 4) & 0x0f0f0f0f0f0f0f0fULL) | ((x & 0x0f0f0f0f0f0f0f0fULL) << 4);
		return std::byteswap(x);
	}

  public:
	class reference {
		Word *word_;
		Word mask_;
		friend class DynamicBitset;
		reference(Word &word, int bit) : word_(&word), mask_(Word(1) << bit) {}
	  public:
		reference(const reference &) = default;
		operator bool() const { return (*word_ & mask_) != 0; }
		reference &operator=(bool value) { if (value) *word_ |= mask_; else *word_ &= ~mask_; return *this; }
		reference &operator=(const reference &other) { return *this = bool(other); }
		reference &flip() { *word_ ^= mask_; return *this; }
	};
	DynamicBitset() = default;
	explicit DynamicBitset(int n, bool value = false) : n_(n) {
		assert(n >= 0);
		data_.assign(word_count(n), value ? ~Word(0) : 0); trim();
	}
	explicit DynamicBitset(std::string_view text) {
		assert(text.size() <= std::size_t(std::numeric_limits<int>::max()));
		n_ = int(text.size()); data_.resize(word_count(n_));
		for (int i = 0; i < n_; ++i) {
			char c = text[std::size_t(n_ - 1 - i)];
			if (c != '0' && c != '1') throw std::invalid_argument("DynamicBitset");
			if (c == '1') data_[unsigned(i) / 64] |= Word(1) << (i & 63);
		}
	}
	DynamicBitset(const DynamicBitset &) = default;
	DynamicBitset &operator=(const DynamicBitset &) = default;
	DynamicBitset(DynamicBitset &&other) noexcept { swap(other); }
	DynamicBitset &operator=(DynamicBitset &&other) noexcept {
		if (this != &other) { DynamicBitset moved(std::move(other)); swap(moved); } return *this;
	}
	static DynamicBitset from_words(int n, std::span<const Word> words) {
		assert(n >= 0 && words.size() == word_count(n));
		DynamicBitset result; result.data_.assign(words.begin(), words.end()); result.n_ = n; result.trim(); return result;
	}
	void swap(DynamicBitset &other) noexcept { data_.swap(other.data_); std::swap(n_, other.n_); }
	int size() const { return n_; }
	bool empty() const { return !n_; }
	std::span<const Word> words() const { return data_; }
	bool test(int p) const { check(p); return (data_[unsigned(p) / 64] >> (p & 63)) & 1; }
	bool operator[](int p) const { return test(p); }
	reference operator[](int p) { check(p); return reference(data_[unsigned(p) / 64], p & 63); }
	DynamicBitset &set(int p, bool value = true) { (*this)[p] = value; return *this; }
	DynamicBitset &reset(int p) { return set(p, false); }
	DynamicBitset &flip(int p) { (*this)[p].flip(); return *this; }
	DynamicBitset &set_all(bool value = true) { std::fill(data_.begin(), data_.end(), value ? ~Word(0) : 0); trim(); return *this; }
	DynamicBitset &reset_all() { return set_all(false); }
	DynamicBitset &flip_all() { for (auto &x : data_) x = ~x; trim(); return *this; }
	DynamicBitset &set_range(int l, int r, bool value = true) {
		update_range(l, r, [value](Word &x, Word m) { if (value) x |= m; else x &= ~m; }); return *this;
	}
	DynamicBitset &reset_range(int l, int r) { return set_range(l, r, false); }
	DynamicBitset &flip_range(int l, int r) { update_range(l, r, [](Word &x, Word m) { x ^= m; }); return *this; }
	void reserve(int n) { assert(n >= 0); data_.reserve(word_count(n)); }
	void resize(int n, bool value = false) {
		assert(n >= 0);
		int old = n_;
		data_.resize(word_count(n), 0); n_ = n;
		if (n > old && value) set_range(old, n); else trim();
	}
	void push_back(bool value) { assert(n_ < std::numeric_limits<int>::max()); int p = n_; resize(n_ + 1); set(p, value); }
	void pop_back() { assert(n_); resize(n_ - 1); }
	void clear() { data_.clear(); n_ = 0; }
	int count() const {
		if (data_.size() >= 8) return int(bitset_ops_detail::count<Operation::identity>(data_.data(), nullptr, data_.size()));
		int result = 0; for (auto x : data_) result += std::popcount(x); return result;
	}
	int count(int l, int r) const {
		check_range(l, r); if (l == r) return 0;
		std::size_t first = unsigned(l) / 64, last = unsigned(r - 1) / 64;
		Word left = ~Word(0) << (l & 63), right = mask(((r - 1) & 63) + 1);
		if (first == last) return std::popcount(data_[first] & left & right);
		int result = std::popcount(data_[first] & left) + std::popcount(data_[last] & right);
		result += int(bitset_ops_detail::count<Operation::identity>(data_.data() + first + 1, nullptr, last - first - 1));
		return result;
	}
	int rank(int r) const { return count(0, r); }
	bool any() const { for (auto x : data_) if (x) return true; return false; }
	bool none() const { return !any(); }
	bool all() const {
		std::size_t full = unsigned(n_) / 64;
		for (std::size_t i = 0; i < full; ++i) if (data_[i] != ~Word(0)) return false;
		return !(n_ & 63) || data_[full] == mask(n_ & 63);
	}
	int count_and(const DynamicBitset &other) const { return count_with<Operation::bit_and>(other); }
	int count_or(const DynamicBitset &other) const { return count_with<Operation::bit_or>(other); }
	int count_xor(const DynamicBitset &other) const { return count_with<Operation::bit_xor>(other); }
	int count_and_not(const DynamicBitset &other) const { return count_with<Operation::and_not>(other); }
	bool intersection_parity(const DynamicBitset &other) const {
		check_size(other); Word x = 0;
		for (std::size_t i = 0; i < data_.size(); ++i) x ^= data_[i] & other.data_[i];
		return std::popcount(x) & 1;
	}
	bool intersects(const DynamicBitset &other) const { check_size(other); for (std::size_t i = 0; i < data_.size(); ++i) if (data_[i] & other.data_[i]) return true; return false; }
	bool is_subset_of(const DynamicBitset &other) const { check_size(other); for (std::size_t i = 0; i < data_.size(); ++i) if (data_[i] & ~other.data_[i]) return false; return true; }
	bool is_proper_subset_of(const DynamicBitset &other) const { return is_subset_of(other) && *this != other; }
	int compare(const DynamicBitset &other) const {
		check_size(other); for (auto i = data_.size(); i-- > 0;) if (data_[i] != other.data_[i]) return data_[i] < other.data_[i] ? -1 : 1; return 0;
	}
	int find_next(int p, bool value = true) const {
		assert(-1 <= p && p < n_); ++p;
		if (p == n_) return n_;
		std::size_t i = unsigned(p) / 64;
		Word x = chosen(i, value) & (~Word(0) << (p & 63));
		for (;;) {
			if (x) return int(i * 64 + std::countr_zero(x));
			if (++i == data_.size()) return n_;
			x = chosen(i, value);
		}
	}
	int find_prev(int p, bool value = true) const {
		assert(0 <= p && p <= n_); if (!p) return n_; --p;
		std::size_t i = unsigned(p) / 64;
		Word x = chosen(i, value) & mask((p & 63) + 1);
		for (;;) {
			if (x) return int(i * 64 + 63 - std::countl_zero(x));
			if (!i) return n_;
			x = chosen(--i, value);
		}
	}
	int find_first(bool value = true) const { return n_ ? find_next(-1, value) : 0; }
	int find_last(bool value = true) const { return find_prev(n_, value); }
	int select(int k, bool value = true) const {
		assert(k >= 0);
		for (std::size_t i = 0; i < data_.size(); ++i) {
			Word x = chosen(i, value); int c = std::popcount(x);
			if (k < c) { while (k--) x &= x - 1; return int(i * 64 + std::countr_zero(x)); }
			k -= c;
		}
		return n_;
	}
	template <class F> void for_each(int l, int r, F &&emit) const {
		check_range(l, r); if (l == r) return;
		for (std::size_t i = unsigned(l) / 64, last = unsigned(r - 1) / 64; i <= last; ++i) {
			Word x = data_[i];
			if (i == unsigned(l) / 64) x &= ~Word(0) << (l & 63);
			if (i == last) x &= mask(((r - 1) & 63) + 1);
			while (x) { emit(int(i * 64 + std::countr_zero(x))); x &= x - 1; }
		}
	}
	template <class F> void for_each(F &&emit) const { for_each(0, n_, std::forward<F>(emit)); }
	DynamicBitset &operator&=(const DynamicBitset &other) { check_size(other); bitset_ops_detail::apply<Operation::bit_and>(data_.data(), other.data_.data(), data_.size()); return *this; }
	DynamicBitset &operator|=(const DynamicBitset &other) { check_size(other); bitset_ops_detail::apply<Operation::bit_or>(data_.data(), other.data_.data(), data_.size()); return *this; }
	DynamicBitset &operator^=(const DynamicBitset &other) { check_size(other); bitset_ops_detail::apply<Operation::bit_xor>(data_.data(), other.data_.data(), data_.size()); return *this; }
	DynamicBitset &operator-=(const DynamicBitset &other) { check_size(other); bitset_ops_detail::apply<Operation::and_not>(data_.data(), other.data_.data(), data_.size()); return *this; }
	friend DynamicBitset operator&(DynamicBitset a, const DynamicBitset &b) { a &= b; return a; }
	friend DynamicBitset operator|(DynamicBitset a, const DynamicBitset &b) { a |= b; return a; }
	friend DynamicBitset operator^(DynamicBitset a, const DynamicBitset &b) { a ^= b; return a; }
	friend DynamicBitset operator-(DynamicBitset a, const DynamicBitset &b) { a -= b; return a; }
	DynamicBitset operator~() const { auto result = *this; result.flip_all(); return result; }
	bool operator==(const DynamicBitset &) const = default;
	DynamicBitset &operator<<=(std::size_t k) { return shift<true, false>(k); }
	DynamicBitset &operator>>=(std::size_t k) { return shift<false, false>(k); }
	friend DynamicBitset operator<<(DynamicBitset a, std::size_t k) { a <<= k; return a; }
	friend DynamicBitset operator>>(DynamicBitset a, std::size_t k) { a >>= k; return a; }
	DynamicBitset &or_shift_left(std::size_t k) { return shift<true, true>(k); }
	DynamicBitset &or_shift_right(std::size_t k) { return shift<false, true>(k); }
	void copy_range(int dst, const DynamicBitset &source, int l, int r) {
		source.check_range(l, r); assert(0 <= dst && r - l <= n_ - dst);
		int length = r - l;
		auto copy = [&](int offset, int count) {
			Word x = source.extract(l + offset, count);
			int p = dst + offset; Word m = mask(count) << (p & 63);
			auto &target = data_[unsigned(p) / 64]; target = (target & ~m) | (x << (p & 63));
		};
		if (this == &source && dst > l) {
			while (length) { int count = std::min(length, ((dst + length - 1) & 63) + 1); length -= count; copy(length, count); }
		} else {
			for (int offset = 0; offset < length;) { int count = std::min(length - offset, 64 - ((dst + offset) & 63)); copy(offset, count); offset += count; }
		}
	}
	DynamicBitset slice(int l, int r) const { check_range(l, r); DynamicBitset result(r - l); result.copy_range(0, *this, l, r); return result; }
	void append(const DynamicBitset &other) {
		int length = other.n_, old = n_; assert(length <= std::numeric_limits<int>::max() - old);
		resize(old + length); copy_range(old, other, 0, length);
	}
	DynamicBitset &reverse() {
		std::reverse(data_.begin(), data_.end()); for (auto &x : data_) x = reverse_word(x);
		if (n_ & 63) {
			unsigned bits = 64 - (n_ & 63);
			for (std::size_t i = 0; i < data_.size(); ++i)
				data_[i] = (data_[i] >> bits) | (i + 1 < data_.size() ? data_[i + 1] << (64 - bits) : 0);
		}
		return *this;
	}
	DynamicBitset &rotate_left(std::size_t amount) {
		if (!n_ || !(amount %= std::size_t(n_))) return *this;
		auto wrap = *this; wrap >>= std::size_t(n_) - amount; *this <<= amount; return *this |= wrap;
	}
	DynamicBitset &rotate_right(std::size_t amount) { if (n_ && (amount %= std::size_t(n_))) rotate_left(std::size_t(n_) - amount); return *this; }
	std::string to_string() const {
		std::string result(n_, '0'); for_each([&](int p) { result[std::size_t(n_ - 1 - p)] = '1'; }); return result;
	}
};

#endif
