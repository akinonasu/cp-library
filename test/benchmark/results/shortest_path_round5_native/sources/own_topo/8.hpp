#ifndef OVERWRITE_ARRAY_HPP
#define OVERWRITE_ARRAY_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

template<class T>
class OverwriteArray {
	static_assert(std::is_trivially_destructible_v<T> && std::is_nothrow_copy_constructible_v<T>);
	T *data_ = nullptr;
	std::size_t size_ = 0, capacity_ = 0;
	[[no_unique_address]] std::allocator<T> allocator_;
	void release() {
		if (data_) allocator_.deallocate(data_, capacity_);
	}
  public:
	OverwriteArray() = default;
	OverwriteArray(const OverwriteArray &other) {
		resize(other.size_);
		if (size_) std::uninitialized_copy_n(other.data_, size_, data_);
	}
	OverwriteArray(OverwriteArray &&other) noexcept : data_(std::exchange(other.data_, nullptr)), size_(std::exchange(other.size_, 0)), capacity_(std::exchange(other.capacity_, 0)) {}
	OverwriteArray &operator=(const OverwriteArray &other) {
		if (this != &other) {
			resize(other.size_);
			if (size_) std::uninitialized_copy_n(other.data_, size_, data_);
		}
		return *this;
	}
	OverwriteArray &operator=(OverwriteArray &&other) noexcept {
		if (this != &other) {
			release();
			data_ = std::exchange(other.data_, nullptr);
			size_ = std::exchange(other.size_, 0);
			capacity_ = std::exchange(other.capacity_, 0);
		}
		return *this;
	}
	~OverwriteArray() { release(); }
	void resize(std::size_t n) {
		if (n > capacity_) {
			const auto max = std::allocator_traits<std::allocator<T>>::max_size(allocator_);
			const auto capacity = std::max(n, size_ <= max / 2 ? size_ * 2 : max);
			auto next = allocator_.allocate(capacity);
			release();
			data_ = next;
			capacity_ = capacity;
		}
		size_ = n;
	}
	void clear() { size_ = 0; }
	T *data() { return data_; }
	const T *data() const { return data_; }
	T *begin() { return data_; }
	const T *begin() const { return data_; }
	T *end() { return size_ ? data_ + size_ : data_; }
	const T *end() const { return size_ ? data_ + size_ : data_; }
	std::size_t size() const { return size_; }
	bool empty() const { return size_ == 0; }
	T &operator[](std::size_t i) { return data_[i]; }
	const T &operator[](std::size_t i) const { return data_[i]; }
};

#endif
