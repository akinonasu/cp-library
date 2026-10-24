#ifndef ALIGNED_ALLOCATOR_HPP
#define ALIGNED_ALLOCATOR_HPP

#include <cstddef>
#include <limits>
#include <new>

template<class T, std::size_t Alignment = 64> struct AlignedAllocator {
	static_assert(Alignment >= alignof(T) && !(Alignment & (Alignment - 1)));
	using value_type = T;
	template<class U> struct rebind { using other = AlignedAllocator<U, Alignment>; };
	AlignedAllocator() = default;
	template<class U> AlignedAllocator(const AlignedAllocator<U, Alignment> &) {}
	T *allocate(std::size_t n) {
		if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) throw std::bad_array_new_length();
		return static_cast<T *>(::operator new(n * sizeof(T), std::align_val_t(Alignment)));
	}
	void deallocate(T *p, std::size_t) { ::operator delete(p, std::align_val_t(Alignment)); }
	template<class U> bool operator==(const AlignedAllocator<U, Alignment> &) const { return true; }
};

#endif
