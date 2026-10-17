#ifndef CP_FPS_INTERPOLATE_HPP
#define CP_FPS_INTERPOLATE_HPP

#include "fps.hpp"
#include "../components/fps_product_tree.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

template<class T>
FormalPowerSeries<T> interpolate(const std::vector<T> &xs, const std::vector<T> &ys) {
	using F = FormalPowerSeries<T>;
	using base = std::vector<T>;
	using O = fps_detail::Operations<T>;
	if constexpr (!std::is_same_v<T, fps_detail::Mint>) return O::export_work(interpolate<fps_detail::Mint>(O::import_work(xs), O::import_work(ys)));
	assert(xs.size() == ys.size());
	const int n = static_cast<int>(xs.size());
	if (n == 0) return {};
	constexpr int block = 8;
	int size = 1;
	std::vector<base> frequency;
	auto tree = fps_detail::ProductTree<T>::build(xs, size, block, true, &frequency);
	const int length = size * block;
	F root(length);
	for (int i = 0; i < n; ++i) root[length - n + i] = T(n - i) * tree[1][i];
	auto flip = [&] {
		for (int i = 1; i < 2 * size; ++i) {
			std::reverse(tree[i].begin(), tree[i].end());
			base &a = frequency[i];
			if (a.empty()) continue;
			const int length = static_cast<int>(a.size()), d = static_cast<int>(tree[i].size()) - 1;
			for (int j = 2; j < length; j <<= 1) std::reverse(a.begin() + j, a.begin() + 2 * j);
			if (d == length / 2) {
				for (int j = length / 2; j < length; ++j) a[j] = -a[j];
			} else {
				const auto &roots = O::ntt_roots(length);
				base phase(length, T(1));
				for (int half = 1; half < length; half <<= 1) {
					const T step = roots[half].pow(d);
					for (int j = 0; j < half; ++j) phase[half + j] = phase[j] * step;
				}
				for (int j = 0; j < length; ++j) a[j] *= phase[j];
			}
		}
	};
	root = root.div_series(tree[1], length);
	base den = fps_detail::ProductTree<T>::evaluate(xs, tree, frequency, size, block, std::move(root));
	flip();
	std::vector<F> value(2 * size);
	base weights(n);
	O::batch_inverse(weights, den);
	O::pointwise(weights, weights, ys, n);
	for (int i = 0; i * block < n; ++i) {
		const int d = std::min(block, n - i * block);
		F &f = value[size + i];
		f.resize(d);
		const F &q = tree[size + i];
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (d == 8 && fps_detail::NTT::vectorized()) {
				fps_detail::NTT::interpolate_leaf8(O::words(f), O::words(q), O::words(xs) + i * block, O::words(weights) + i * block);
				continue;
			}
		}
#endif
		for (int j = i * block; j < i * block + d; ++j) {
			T coefficient = 1;
			f[d - 1] += weights[j];
			for (int k = d - 2; k >= 0; --k) {
				coefficient = q[k + 1] + xs[j] * coefficient;
				f[k] += weights[j] * coefficient;
			}
		}
	}
	for (int i = size - 1; i >= 1; --i) {
		if (value[2 * i + 1].empty()) {
			value[i] = std::move(value[2 * i]);
			continue;
		}
		const int length = static_cast<int>(frequency[2 * i].size());
		if (length && static_cast<int>(frequency[2 * i + 1].size()) == length) {
			base a = std::move(value[2 * i]), b = std::move(value[2 * i + 1]);
			a.resize(length);
			b.resize(length);
			O::ntt(a);
			O::ntt(b);
			if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::combine(O::words(a), O::words(b), O::words(frequency[2 * i]), O::words(frequency[2 * i + 1]), length);
			else
				for (int j = 0; j < length; ++j) a[j] = a[j] * frequency[2 * i + 1][j] + b[j] * frequency[2 * i][j];
			O::intt(a);
			a.resize(tree[i].size() - 1);
			value[i] = std::move(a);
		} else {
			value[i] = value[2 * i] * tree[2 * i + 1] + value[2 * i + 1] * tree[2 * i];
		}
	}
	return std::move(value[1]);
}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
