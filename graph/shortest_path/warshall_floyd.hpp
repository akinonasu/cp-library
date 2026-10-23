#ifndef WARSHALL_FLOYD_HPP
#define WARSHALL_FLOYD_HPP

#include "../../components/warshall_floyd_kernel.hpp"
#include <stdexcept>

template<class T = std::int64_t>
class WarshallFloyd {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) == 4 || sizeof(T) == 8));
	static constexpr int block = 32;
	int n_ = 0, blocks_ = 0;
	T infinity_ = std::numeric_limits<T>::max();
	warshall_floyd_detail::Storage<T> data_;
	static int checked_size(std::size_t n) {
		if (n > std::size_t(std::numeric_limits<int>::max())) throw std::length_error("Warshall-Floyd size");
		return int(n);
	}
	std::size_t index(int i, int j) const {
		return (std::size_t(i / block) * blocks_ + j / block) * block * block + i % block * block + j % block;
	}
	template<class U> bool run_as(bool negative) {
		const U inf = std::numeric_limits<U>::max();
		warshall_floyd_detail::Storage<U> work(data_.size());
		for (std::size_t i = 0; i < data_.size(); ++i) work[i] = data_[i] == infinity_ ? inf : U(data_[i]);
		bool success = negative ? warshall_floyd_detail::run<U, block, true>(work.data(), blocks_, inf)
			: warshall_floyd_detail::run<U, block, false>(work.data(), blocks_, inf);
		if (success)
			for (U value : work) if (value != inf && (__int128_t(value) < std::numeric_limits<T>::lowest() || __int128_t(value) >= infinity_))
				throw std::overflow_error("Warshall-Floyd distance");
		for (std::size_t i = 0; i < data_.size(); ++i) {
			if (work[i] == inf) data_[i] = infinity_;
			else data_[i] = T(std::clamp(__int128_t(work[i]), __int128_t(std::numeric_limits<T>::lowest()), __int128_t(infinity_) - 1));
		}
		return success;
	}

  public:
	using value_type = T;
	explicit WarshallFloyd(int n = 0, T inf = std::numeric_limits<T>::max()) : n_(n), infinity_(inf) {
		if (n < 0 || inf <= 0) throw std::invalid_argument("Warshall-Floyd shape or infinity");
		blocks_ = n / block + (n % block != 0);
		std::size_t padded = std::size_t(blocks_) * block;
		data_.assign(padded * padded, infinity_);
		for (std::size_t i = 0; i < padded; ++i) data_[index(int(i), int(i))] = 0;
	}
	explicit WarshallFloyd(const std::vector<std::vector<T>>& distance, T inf = std::numeric_limits<T>::max())
		: WarshallFloyd(checked_size(distance.size()), inf) {
		for (int i = 0; i < n_; ++i) {
			if (distance[i].size() != std::size_t(n_)) throw std::invalid_argument("Warshall-Floyd matrix");
			for (int j = 0; j < n_; ++j) {
				if (distance[i][j] == infinity_) continue;
				add_edge(i, j, distance[i][j]);
			}
		}
	}
	int size() const { return n_; }
	T infinity() const { return infinity_; }
	T operator()(int i, int j) const {
		assert(0 <= i && i < n_ && 0 <= j && j < n_);
		return data_[index(i, j)];
	}
	void add_edge(int from, int to, T weight) {
		assert(0 <= from && from < n_ && 0 <= to && to < n_);
		if (weight >= infinity_) throw std::invalid_argument("Warshall-Floyd edge");
		auto& value = data_[index(from, to)];
		value = std::min(value, weight);
	}
	bool solve() {
		__int128_t maximum = 0;
		std::size_t finite = 0;
		bool negative = false;
		for (T value : data_) if (value != infinity_) {
			++finite;
			__int128_t x = value;
			negative |= x < 0;
			maximum = std::max(maximum, x < 0 ? -x : x);
		}
		__int128_t bound = 2 * (__int128_t(n_) + 1) * maximum;
		bool complete = finite == std::size_t(n_) * n_ + std::size_t(blocks_) * block - n_;
		if constexpr (sizeof(T) == 8) {
#ifndef WARSHALL_FLOYD_DISABLE_NARROWING
			__int128_t limit = std::min(__int128_t(infinity_), __int128_t(std::numeric_limits<std::int32_t>::max()));
			if (negative ? bound < limit : maximum < limit && (complete || (__int128_t(n_) - 1) * maximum < limit))
				return run_as<std::int32_t>(negative);
#endif
		}
		if (negative ? bound >= infinity_ : !complete && (__int128_t(n_) - 1) * maximum >= infinity_) {
			if constexpr (sizeof(T) == 4) {
				if (bound < std::numeric_limits<std::int64_t>::max()) return run_as<std::int64_t>(negative);
			}
			return run_as<__int128_t>(negative);
		}
		return negative ? warshall_floyd_detail::run<T, block, true>(data_.data(), blocks_, infinity_)
			: warshall_floyd_detail::run<T, block, false>(data_.data(), blocks_, infinity_);
	}
	std::vector<std::vector<T>> distances() const {
		std::vector<std::vector<T>> result(n_, std::vector<T>(n_));
		for (int i = 0; i < n_; ++i) for (int j = 0; j < n_; ++j) result[i][j] = data_[index(i, j)];
		return result;
	}
};

template<class T>
bool warshall_floyd(std::vector<std::vector<T>>& distance, T inf = std::numeric_limits<T>::max()) {
	WarshallFloyd<T> work(distance, inf);
	bool success = work.solve();
	distance = work.distances();
	return success;
}

#endif
