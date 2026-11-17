#ifndef FLAT_HASH_MAP_HPP
#define FLAT_HASH_MAP_HPP

#include <algorithm>
#include <bit>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
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
	using iterator = typename std::unordered_map<K, V, Hash, Equal>::iterator;
	using const_iterator = typename std::unordered_map<K, V, Hash, Equal>::const_iterator;
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) : Table() { swap(other); }
	Table &operator=(Table other) { swap(other); return *this; }
	void swap(Table &other) noexcept(noexcept(values.swap(other.values))) { values.swap(other.values); }
	std::size_t size() const { return values.size(); }
	bool empty() const { return values.empty(); }
	iterator find(const K &key) { return values.find(key); }
	const_iterator find(const K &key) const { return values.find(key); }
	iterator begin() { return values.begin(); }
	const_iterator begin() const { return values.begin(); }
	iterator end() { return values.end(); }
	const_iterator end() const { return values.end(); }
	V *find_value(const K &key) { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	const V *find_value(const K &key) const { auto it = values.find(key); return it == values.end() ? nullptr : &it->second; }
	V &operator[](const K &key) { return values[key]; }
	void set(const K &key, const V &value) { values.insert_or_assign(key, value); }
	template<class Key, class... Args>
	std::pair<iterator, bool> try_emplace(Key &&key, Args &&...args) {
		return values.try_emplace(std::forward<Key>(key), std::forward<Args>(args)...);
	}
	template<class... Args>
	std::pair<iterator, bool> emplace(Args &&...args) { return values.emplace(std::forward<Args>(args)...); }
	template<class Key, class Value>
	std::pair<iterator, bool> insert_or_assign(Key &&key, Value &&value) {
		return values.insert_or_assign(std::forward<Key>(key), std::forward<Value>(value));
	}
	bool erase(const K &key) { return values.erase(key); }
	iterator erase(const_iterator position) { return values.erase(position); }
	void prefetch(const K &) const noexcept {}
	void reserve(std::size_t n) { values.reserve(n); }
	void clear() { values.clear(); }
};

template<class K, class V, class Hash, class Equal>
class Table<K, V, Hash, Equal, true> {
	enum : unsigned char { vacant = 0, occupied = 1, tombstone = 2 };
	struct Entry { K key{}; V value{}; };
	struct ValueSlot { V value{}; };
	static constexpr bool combined = sizeof(Entry) == sizeof(K) + sizeof(V) && sizeof(V) <= 16;
	using Storage = std::conditional_t<combined, std::vector<Entry>, std::pair<std::vector<K>, std::vector<ValueSlot>>>;
	Storage storage;
	using Slot = std::conditional_t<combined, Entry, ValueSlot>;
	Slot *stored_slot(std::size_t p) {
		if constexpr (combined) return std::addressof(storage[p]);
		else return std::addressof(storage.second[p]);
	}
	const Slot *stored_slot(std::size_t p) const {
		if constexpr (combined) return std::addressof(storage[p]);
		else return std::addressof(storage.second[p]);
	}
	K &stored_key(std::size_t p) {
		if constexpr (combined) return storage[p].key;
		else return storage.first[p];
	}
	const K &stored_key(std::size_t p) const {
		if constexpr (combined) return storage[p].key;
		else return storage.first[p];
	}
	V &stored_value(std::size_t p) {
		if constexpr (combined) return storage[p].value;
		else return storage.second[p].value;
	}
	const V &stored_value(std::size_t p) const {
		if constexpr (combined) return storage[p].value;
		else return storage.second[p].value;
	}
	void resize(std::size_t capacity) {
		if constexpr (combined) storage.resize(capacity);
		else storage.first.resize(capacity), storage.second.resize(capacity);
	}
	std::size_t storage_limit() const {
		if constexpr (combined) return storage.max_size();
		else return std::min(storage.first.max_size(), storage.second.max_size());
	}
	std::vector<unsigned char> states;
	std::size_t count = 0, deleted = 0;
	unsigned shift = 64;
	std::uint64_t salt = std::chrono::steady_clock::now().time_since_epoch().count();
	std::size_t index(K key) const {
		return ((static_cast<std::uint64_t>(key) ^ salt) * UINT64_C(11400714819323198485)) >> shift;
	}
	std::size_t locate(K key) const {
		std::size_t p = index(key), mask = states.size() - 1;
		while (states[p] != vacant && (states[p] != occupied || stored_key(p) != key)) p = (p + 1) & mask;
		return p;
	}
	void rehash(std::size_t capacity) {
		Table next;
		next.salt = salt;
		next.resize(capacity), next.states.resize(capacity);
		next.shift = 64 - std::countr_zero(capacity);
		for (std::size_t i = 0; i < states.size(); ++i) if (states[i] == occupied) {
			std::size_t p = next.locate(stored_key(i));
			next.stored_key(p) = stored_key(i), next.stored_value(p) = stored_value(i), next.states[p] = occupied;
		}
		next.count = count;
		swap(next);
	}
	void prepare() {
		if (states.empty()) rehash(16);
		else if (count + deleted == states.size() / 2) {
			if (deleted > count / 2) rehash(states.size());
			else {
				if (states.size() > states.max_size() / 2 || states.size() > storage_limit() / 2)
					throw std::length_error("FlatHashMap capacity");
				rehash(states.size() * 2);
			}
		}
	}
	std::size_t insert(K key) {
		prepare();
		std::size_t p = index(key), first = states.size(), mask = states.size() - 1;
		while (states[p] != vacant) {
			if (states[p] == occupied && stored_key(p) == key) return p;
			if (states[p] == tombstone && first == states.size()) first = p;
			p = (p + 1) & mask;
		}
		if (first != states.size()) p = first, --deleted;
		stored_key(p) = key, stored_value(p) = V{}, states[p] = occupied;
		++count;
		return p;
	}
public:
	template<bool Const> class Iterator {
		using Owner = std::conditional_t<Const, const Table, Table>;
		using Item = std::conditional_t<Const, const Slot, Slot>;
		Item *item = nullptr;
		Owner *owner = nullptr;
		friend class Table;
		template<bool> friend class Iterator;
		Iterator(Item *item, Owner *owner) : item(item), owner(owner) {}
	public:
		using value_type = std::pair<const K, V>;
		using difference_type = std::ptrdiff_t;
		using reference = std::pair<const K &, std::conditional_t<Const, const V &, V &>>;
		struct pointer {
			reference value;
			const reference *operator->() const { return std::addressof(value); }
		};
		using iterator_category = std::input_iterator_tag;
		using iterator_concept = std::input_iterator_tag;
		Iterator() = default;
		template<bool Other> requires (Const && !Other)
		Iterator(const Iterator<Other> &other) : item(other.item), owner(other.owner) {}
		reference operator*() const {
			if constexpr (combined) return {item->key, item->value};
			else return {owner->storage.first[item - owner->storage.second.data()], item->value};
		}
		pointer operator->() const { return {operator*()}; }
		Iterator &operator++() {
			std::size_t position;
			if constexpr (combined) position = item - owner->storage.data();
			else position = item - owner->storage.second.data();
			do { ++position; } while (position < owner->states.size() && owner->states[position] != occupied);
			item = position < owner->states.size() ? owner->stored_slot(position) : nullptr;
			return *this;
		}
		Iterator operator++(int) { Iterator result(*this); ++*this; return result; }
		template<bool Other> bool operator==(const Iterator<Other> &other) const { return item == other.item; }
	};
	using iterator = Iterator<false>;
	using const_iterator = Iterator<true>;
	Table() = default;
	Table(const Table &) = default;
	Table(Table &&other) noexcept { swap(other); }
	Table &operator=(Table other) noexcept { swap(other); return *this; }
	void swap(Table &other) noexcept {
		storage.swap(other.storage), states.swap(other.states);
		std::swap(count, other.count), std::swap(deleted, other.deleted), std::swap(shift, other.shift), std::swap(salt, other.salt);
	}
	std::size_t size() const { return count; }
	bool empty() const { return !count; }
	iterator find(K key) {
		if (states.empty()) return end();
		std::size_t p = locate(key);
		return states[p] != vacant ? iterator(stored_slot(p), this) : end();
	}
	const_iterator find(K key) const {
		if (states.empty()) return end();
		std::size_t p = locate(key);
		return states[p] != vacant ? const_iterator(stored_slot(p), this) : end();
	}
	iterator begin() {
		if (!count) return end();
		std::size_t p = 0;
		while (states[p] != occupied) ++p;
		return iterator(stored_slot(p), this);
	}
	const_iterator begin() const {
		if (!count) return end();
		std::size_t p = 0;
		while (states[p] != occupied) ++p;
		return const_iterator(stored_slot(p), this);
	}
	iterator end() { return iterator(); }
	const_iterator end() const { return const_iterator(); }
	V *find_value(K key) { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] != vacant ? &stored_value(p) : nullptr; }
	const V *find_value(K key) const { if (states.empty()) return nullptr; std::size_t p = locate(key); return states[p] != vacant ? &stored_value(p) : nullptr; }
	V &operator[](K key) { return stored_value(insert(key)); }
	void set(K key, const V &value) { V copy = value; stored_value(insert(key)) = copy; }
	template<class... Args>
	std::pair<iterator, bool> try_emplace(K key, Args &&...args) {
		if (auto it = find(key); it != end()) return {it, false};
		V value(std::forward<Args>(args)...);
		std::size_t p = insert(key);
		stored_value(p) = value;
		return {iterator(stored_slot(p), this), true};
	}
	template<class... Args>
	std::pair<iterator, bool> emplace(Args &&...args) {
		std::pair<K, V> value(std::forward<Args>(args)...);
		return try_emplace(value.first, std::move(value.second));
	}
	template<class Value>
	std::pair<iterator, bool> insert_or_assign(K key, Value &&value) {
		if (auto it = find(key); it != end()) {
			it->second = std::forward<Value>(value);
			return {it, false};
		}
		V copy(std::forward<Value>(value));
		std::size_t p = insert(key);
		stored_value(p) = copy;
		return {iterator(stored_slot(p), this), true};
	}
	bool erase(K key) {
		if (states.empty()) return false;
		std::size_t p = locate(key);
		if (states[p] == vacant) return false;
		states[p] = tombstone, --count, ++deleted;
		return true;
	}
	iterator erase(const_iterator position) {
		assert(position.owner == this && position.item);
		std::size_t p;
		if constexpr (combined) p = position.item - storage.data();
		else p = position.item - storage.second.data();
		assert(states[p] == occupied);
		iterator next(stored_slot(p), this);
		++next;
		states[p] = tombstone, --count, ++deleted;
		return next;
	}
	void prefetch(K key) const noexcept {
#if defined(__GNUC__) || defined(__clang__)
		if (!states.empty()) {
			std::size_t p = index(key);
			__builtin_prefetch(states.data() + p, 0, 3);
			__builtin_prefetch(std::addressof(stored_key(p)), 0, 3);
			if constexpr (!combined) __builtin_prefetch(std::addressof(stored_value(p)), 0, 3);
		}
#else
		(void)key;
#endif
	}
	void reserve(std::size_t n) {
		if (n <= states.size() / 2) return;
		std::size_t limit = std::min(states.max_size(), storage_limit());
		if (n > limit / 2) throw std::length_error("FlatHashMap capacity");
		std::size_t capacity = std::bit_ceil(std::max(std::size_t(16), 2 * n));
		if (capacity > limit) throw std::length_error("FlatHashMap capacity");
		rehash(capacity);
	}
	void clear() { std::fill(states.begin(), states.end(), vacant); count = deleted = 0; }
};
}

template<class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class FlatHashMap {
	flat_hash_map_detail::Table<Key, T, Hash, KeyEqual> table;
public:
	using key_type = Key;
	using mapped_type = T;
	using size_type = std::size_t;
	using value_type = std::pair<const Key, T>;
	using iterator = typename decltype(table)::iterator;
	using const_iterator = typename decltype(table)::const_iterator;
	FlatHashMap() = default;
	explicit FlatHashMap(size_type n) { reserve(n); }
	bool empty() const { return table.empty(); }
	size_type size() const { return table.size(); }
	iterator find(const Key &key) { return table.find(key); }
	const_iterator find(const Key &key) const { return table.find(key); }
	iterator begin() { return table.begin(); }
	const_iterator begin() const { return table.begin(); }
	const_iterator cbegin() const { return table.begin(); }
	iterator end() { return table.end(); }
	const_iterator end() const { return table.end(); }
	const_iterator cend() const { return table.end(); }
	bool contains(const Key &key) const { return table.find_value(key) != nullptr; }
	size_type count(const Key &key) const { return contains(key); }
	std::optional<T> get(const Key &key) const { const T *p = table.find_value(key); return p ? std::optional<T>(*p) : std::nullopt; }
	T &at(const Key &key) { T *p = table.find_value(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	const T &at(const Key &key) const { const T *p = table.find_value(key); if (!p) throw std::out_of_range("FlatHashMap key"); return *p; }
	T &operator[](const Key &key) { return table[key]; }
	void set(const Key &key, const T &value) { table.set(key, value); }
	std::pair<iterator, bool> insert(const value_type &value) { return try_emplace(value.first, value.second); }
	std::pair<iterator, bool> insert(value_type &&value) { return try_emplace(value.first, std::move(value.second)); }
	template<class... Args>
	std::pair<iterator, bool> emplace(Args &&...args) { return table.emplace(std::forward<Args>(args)...); }
	template<class... Args>
	std::pair<iterator, bool> try_emplace(const Key &key, Args &&...args) {
		return table.try_emplace(key, std::forward<Args>(args)...);
	}
	template<class... Args>
	std::pair<iterator, bool> try_emplace(Key &&key, Args &&...args) {
		return table.try_emplace(std::move(key), std::forward<Args>(args)...);
	}
	template<class Value>
	std::pair<iterator, bool> insert_or_assign(const Key &key, Value &&value) {
		return table.insert_or_assign(key, std::forward<Value>(value));
	}
	template<class Value>
	std::pair<iterator, bool> insert_or_assign(Key &&key, Value &&value) {
		return table.insert_or_assign(std::move(key), std::forward<Value>(value));
	}
	bool erase(const Key &key) { return table.erase(key); }
	iterator erase(const_iterator position) { return table.erase(position); }
	void prefetch(const Key &key) const noexcept { table.prefetch(key); }
	void reserve(size_type n) { table.reserve(n); }
	void clear() { table.clear(); }
	void swap(FlatHashMap &other) noexcept(noexcept(table.swap(other.table))) { table.swap(other.table); }
	friend void swap(FlatHashMap &a, FlatHashMap &b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
};

#endif
