#ifndef FLAT_HASH_MAP_HPP
#define FLAT_HASH_MAP_HPP

#include <algorithm>
#include <bit>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace flat_hash_map_detail {
template<class K, class V, class Hash, class Equal>
inline constexpr bool packed = std::is_integral_v<K> && !std::is_same_v<K, bool> && sizeof(K) <= 8 &&
	std::is_same_v<Hash, std::hash<K>> && std::is_same_v<Equal, std::equal_to<K>> &&
	std::is_trivially_copyable_v<V> && std::is_nothrow_default_constructible_v<V> &&
	std::is_nothrow_copy_constructible_v<V> && std::is_nothrow_copy_assignable_v<V>;

template<class K, class V, class Hash, class Equal, bool = packed<K, V, Hash, Equal>>
class Table {
	std::unordered_map<K, V, Hash, Equal> values;
public:
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) : Table() { swap(other); }
	Table &operator=(Table other) { swap(other); return *this; }
	void swap(Table &other) noexcept(noexcept(values.swap(other.values))) { values.swap(other.values); }
	std::size_t size() const { return values.size(); }
	bool empty() const { return values.empty(); }
	V *find(const K &key) { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	const V *find(const K &key) const { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	V &operator[](const K &key) { return values[key]; }
	void set(const K &key, const V &value) { values.insert_or_assign(key, value); }
	bool erase(const K &key) { return values.erase(key); }
	void reserve(std::size_t n) { values.reserve(n); }
	void clear() { values.clear(); }
};

template<class K, class V, class Hash, class Equal>
class Table<K, V, Hash, Equal, true> {
	std::vector<K> keys;
	std::vector<V> values;
	std::vector<unsigned char> states;
	std::size_t count = 0, deleted = 0;
	unsigned shift = 64;
	std::uint64_t salt = std::chrono::steady_clock::now().time_since_epoch().count();
	std::size_t index(K key) const {
		return ((static_cast<std::uint64_t>(key) ^ salt) * UINT64_C(11400714819323198485)) >> shift;
	}
	std::size_t locate(K key) const {
		std::size_t p = index(key), mask = states.size() - 1;
		while (states[p] && (states[p] != 1 || keys[p] != key)) p = (p + 1) & mask;
		return p;
	}
	void rehash(std::size_t capacity) {
		Table next;
		next.salt = salt;
		next.keys.resize(capacity), next.values.resize(capacity), next.states.resize(capacity);
		next.shift = 64 - std::countr_zero(capacity);
		for (std::size_t i = 0; i < states.size(); ++i) if (states[i] == 1) {
			std::size_t p = next.locate(keys[i]);
			next.keys[p] = keys[i], next.values[p] = values[i], next.states[p] = 1;
		}
		next.count = count;
		swap(next);
	}
	void prepare() {
		if (states.empty()) rehash(16);
		else if (count + deleted == states.size() / 2) {
			if (deleted > count / 2) rehash(states.size());
			else {
				if (states.size() > states.max_size() / 2 || states.size() > keys.max_size() / 2 || states.size() > values.max_size() / 2)
					throw std::length_error("FlatHashMap capacity");
				rehash(states.size() * 2);
			}
		}
	}
	std::size_t insert(K key) {
		prepare();
		std::size_t p = index(key), first = states.size(), mask = states.size() - 1;
		while (states[p]) {
			if (states[p] == 1 && keys[p] == key) return p;
			if (states[p] == 2 && first == states.size()) first = p;
			p = (p + 1) & mask;
		}
		if (first != states.size()) p = first, --deleted;
		keys[p] = key, values[p] = V{}, states[p] = 1;
		++count;
		return p;
	}
public:
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) noexcept { swap(other); }
	Table &operator=(Table other) noexcept { swap(other); return *this; }
	void swap(Table &other) noexcept {
		keys.swap(other.keys), values.swap(other.values), states.swap(other.states);
		std::swap(count, other.count), std::swap(deleted, other.deleted), std::swap(shift, other.shift), std::swap(salt, other.salt);
	}
	std::size_t size() const { return count; }
	bool empty() const { return !count; }
	V *find(K key) { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] ? &values[p] : nullptr; }
	const V *find(K key) const { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] ? &values[p] : nullptr; }
	V &operator[](K key) { return values[insert(key)]; }
	void set(K key, const V &value) { V copy = value; values[insert(key)] = copy; }
	bool erase(K key) {
		if (states.empty()) return false;
		std::size_t p = locate(key);
		if (!states[p]) return false;
		states[p] = 2, --count, ++deleted;
		return true;
	}
	void reserve(std::size_t n) {
		if (n <= states.size() / 2) return;
		std::size_t limit = std::min({states.max_size(), keys.max_size(), values.max_size()});
		if (n > limit / 2) throw std::length_error("FlatHashMap capacity");
		std::size_t capacity = std::bit_ceil(std::max(std::size_t(16), 2 * n));
		if (capacity > limit) throw std::length_error("FlatHashMap capacity");
		rehash(capacity);
	}
	void clear() { std::fill(states.begin(), states.end(), 0); count = deleted = 0; }
};
}

template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class FlatHashMap {
	flat_hash_map_detail::Table<Key, T, Hash, KeyEqual> table;
public:
	using key_type = Key;
	using mapped_type = T;
	using size_type = std::size_t;
	FlatHashMap() = default;
	explicit FlatHashMap(size_type n) { reserve(n); }
	bool empty() const { return table.empty(); }
	size_type size() const { return table.size(); }
	T *find(const Key &key) { return table.find(key); }
	const T *find(const Key &key) const { return table.find(key); }
	bool contains(const Key &key) const { return find(key) != nullptr; }
	size_type count(const Key &key) const { return contains(key); }
	std::optional<T> get(const Key &key) const { const T *p = find(key); return p ? std::optional<T>(*p) : std::nullopt; }
	T &at(const Key &key) { T *p = find(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	const T &at(const Key &key) const { const T *p = find(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	T &operator[](const Key &key) { return table[key]; }
	void set(const Key &key, const T &value) { table.set(key, value); }
	bool erase(const Key &key) { return table.erase(key); }
	void reserve(size_type n) { table.reserve(n); }
	void clear() { table.clear(); }
	void swap(FlatHashMap &other) noexcept(noexcept(table.swap(other.table))) { table.swap(other.table); }
	friend void swap(FlatHashMap &a, FlatHashMap &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

#endif
