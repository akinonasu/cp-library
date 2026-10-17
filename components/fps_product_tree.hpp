#ifndef CP_COMPONENTS_FPS_PRODUCT_TREE_HPP
#define CP_COMPONENTS_FPS_PRODUCT_TREE_HPP

#include "../fps/fps.hpp"

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif

namespace fps_detail {

template<class T>
struct ProductTree {
	using F = FormalPowerSeries<T>;
	using base = std::vector<T>;
	using O = Operations<T>;
	using Word = typename O::Word;

	static base evaluate(const base &points, const std::vector<F> &tree,
							  std::vector<base> &frequency, int size, int block, F root, bool consume = false) {
		base result(points.size());
#if defined(__x86_64__) || defined(__i386__)
		if constexpr (std::is_same_v<T, fps_detail::Mint>) {
			if (block == 8 && fps_detail::NTT::vectorized()) {
				const int total = size * block;
				auto &plan = O::transform(std::max(1, total / 2));
				fps_detail::NTT::CompositionWords scratch;
				if (!consume) scratch.reset(new (std::align_val_t(64)) unsigned int[2 * total]);
				for (int level = 1; level <= size; level <<= 1) {
					const int length = total / level, half = length / 2;
					for (int node = level; node < 2 * level; ++node) {
						const int offset = (node - level) * length;
						if (offset >= static_cast<int>(points.size())) break;
						Word *cur = O::words(root) + offset;
						if (length == 8) {
							fps_detail::NTT::evaluate_leaf8(O::words(result) + offset, cur, O::words(tree[node]), tree[node].size(), O::words(points) + offset, std::min<int>(8, points.size() - offset));
						} else if (offset + half >= static_cast<int>(points.size())) {
							std::copy_n(cur + half, half, cur);
						} else if (!frequency[2 * node].empty()) {
							Word *a = consume ? O::words(frequency[2 * node]) : scratch.get();
							Word *b = consume ? O::words(frequency[2 * node + 1]) : scratch.get() + total;
							if (!consume) {
								std::copy_n(O::words(frequency[2 * node]), length, a);
								std::copy_n(O::words(frequency[2 * node + 1]), length, b);
							}
							plan.template fft<false>(cur, length);
							fps_detail::NTT::product(a, a, cur, length);
							fps_detail::NTT::product(b, b, cur, length);
							plan.template fft<true>(a, length);
							plan.template fft<true>(b, length);
							const Word norm = T::mod() - (T::mod() - 1) / length;
							fps_detail::NTT::scale_copy(cur, b + half, half, norm);
							fps_detail::NTT::scale_copy(cur + half, a + half, half, norm);
						} else {
							alignas(32) Word left[8], right[8];
							assert(length == 16);
							fps_detail::NTT::evaluate_middle8(left, right, cur, O::words(tree[2 * node + 1]), tree[2 * node + 1].size(), O::words(tree[2 * node]), tree[2 * node].size());
							std::copy_n(left, 8, cur);
							std::copy_n(right, 8, cur + 8);
						}
					}
				}
				return result;
			}
		}
#endif
		auto dfs = [&](auto &&self, int node, int l, int r, F cur) -> void {
			if (l * block >= static_cast<int>(points.size())) return;
			const int length = (r - l) * block, half = length / 2;
			if (r - l == 1 || length <= 8) {
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (std::is_same_v<T, fps_detail::Mint>) {
					if (length == 8 && fps_detail::NTT::vectorized()) {
						const int count = std::min<int>(8, points.size() - l * block);
						fps_detail::NTT::evaluate_leaf8(O::words(result) + l * block, O::words(cur), O::words(tree[node]), tree[node].size(), O::words(points) + l * block, count);
						return;
					}
				}
#endif
				base polynomial(length);
				for (int i = 0; i < length; ++i)
					for (int j = 0; j + i < length && j < static_cast<int>(tree[node].size()); ++j)
						polynomial[length - 1 - i - j] += cur[i] * tree[node][j];
				for (int i = l * block; i < std::min<int>(r * block, points.size()); ++i) {
					T value = 0;
					for (int j = length - 1; j >= 0; --j) value = value * points[i] + polynomial[j];
					result[i] = value;
				}
				return;
			}
			const int mid = (l + r) / 2;
			if (mid * block >= static_cast<int>(points.size())) {
				self(self, 2 * node, l, mid, F(cur.begin() + half, cur.end()));
				return;
			}
			F left, right;
			if (!frequency[2 * node].empty()) {
				O::ntt(static_cast<base &>(cur));
				F a = consume ? std::move(frequency[2 * node]) : frequency[2 * node];
				F b = consume ? std::move(frequency[2 * node + 1]) : frequency[2 * node + 1];
				O::pointwise(a, a, cur, length);
				O::pointwise(b, b, cur, length);
				O::ntt_inverse(a);
				O::ntt_inverse(b);
				const Word norm = T::mod() - (T::mod() - 1) / length;
				fps_detail::NTT::scale_copy(O::words(b), O::words(b) + half, half, norm);
				fps_detail::NTT::scale_copy(O::words(a), O::words(a) + half, half, norm);
				b.resize(half);
				a.resize(half);
				left = std::move(b);
				right = std::move(a);
			} else {
				left.resize(half);
				right.resize(half);
				bool done = false;
#if defined(__x86_64__) || defined(__i386__)
				if constexpr (std::is_same_v<T, fps_detail::Mint>) {
					if (length == 16 && fps_detail::NTT::vectorized()) {
						fps_detail::NTT::evaluate_middle8(O::words(left), O::words(right), O::words(cur), O::words(tree[2 * node + 1]), tree[2 * node + 1].size(), O::words(tree[2 * node]), tree[2 * node].size());
						done = true;
					}
				}
#endif
				if (!done) {
					F a = F::multiply(cur, tree[2 * node + 1]), b = F::multiply(cur, tree[2 * node]);
					std::copy_n(a.begin() + half, half, left.begin());
					std::copy_n(b.begin() + half, half, right.begin());
				}
			}
			self(self, 2 * node, l, mid, std::move(left));
			self(self, 2 * node + 1, mid, r, std::move(right));
		};
		dfs(dfs, 1, 0, size, std::move(root));
		return result;
	}
	static std::vector<F> build(const std::vector<T> &xs, int &size, int block,
									   bool reciprocal,
									   std::vector<std::vector<T>> *frequency = nullptr) {
		const int groups = (static_cast<int>(xs.size()) + block - 1) / block;
		while (size < groups) size <<= 1;
		std::vector<F> tree(2 * size);
		if (frequency) frequency->resize(2 * size);
		for (int i = 0; i < size; ++i) {
			if (i >= groups) {
				tree[size + i] = {1};
				continue;
			}
			F p;
			p.reserve(block + 1);
			p.push_back(T(1));
			for (int j = i * block; j < std::min<int>((i + 1) * block, xs.size()); ++j) {
				const int d = static_cast<int>(p.size()) - 1;
				if (reciprocal) {
					p.push_back(0);
					for (int k = d + 1; k >= 1; --k) p[k] -= xs[j] * p[k - 1];
				} else {
					p.push_back(p.back());
					for (int k = d; k >= 1; --k) p[k] = p[k - 1] - xs[j] * p[k];
					p[0] *= -xs[j];
				}
			}
			tree[size + i] = std::move(p);
		}
		for (int i = size - 1; i >= 1; --i) {
			if (tree[2 * i + 1].size() == 1 && tree[2 * i + 1][0] == T(1)) {
				tree[i] = tree[2 * i];
				continue;
			}
			int depth = 31 - __builtin_clz((unsigned int)i);
			int length = (size >> depth) * block;
			if (frequency && length >= 32) {
				auto &a = (*frequency)[2 * i];
				auto &b = (*frequency)[2 * i + 1];
				auto extend_transform = [&](base &a, const F &p) {
					if (a.empty()) {
						a.resize(length);
						std::copy(p.begin(), p.end(), a.begin());
						O::ntt(a);
					} else {
						const int half = length / 2;
						base odd(half);
						std::copy_n(p.begin(), std::min<int>(half, p.size()), odd.begin());
						if (static_cast<int>(p.size()) > half) odd[0] -= p[half];
						O::geometric_scale(odd, T(1), O::ntt_roots(length)[length / 2]);
						O::ntt(odd);
						a.insert(a.end(), odd.begin(), odd.end());
					}
				};
				extend_transform(a, tree[2 * i]);
				extend_transform(b, tree[2 * i + 1]);
				std::vector<T> c(length);
				O::pointwise(c, a, b, length);
				if (i > 1) (*frequency)[i] = c;
				O::ntt_inverse(c);
				const int n = (int)(tree[2 * i].size() + tree[2 * i + 1].size() - 1);
				tree[i].resize(n);
				const T inv = T::raw(T::mod() - (T::mod() - 1) / length);
				if constexpr (std::is_same_v<T, fps_detail::Mint>) fps_detail::NTT::scale_copy(O::words(tree[i]), O::words(c), std::min(n, length), inv.val());
				else
					for (int j = 0; j < std::min(n, length); ++j) tree[i][j] = c[j] * inv;
				if (n == length + 1) {
					const T lead = tree[2 * i].back() * tree[2 * i + 1].back();
					tree[i][0] -= lead;
					tree[i][length] = lead;
				}
			} else {
				tree[i] = F::multiply(tree[2 * i], tree[2 * i + 1]);
			}
		}
		return tree;
	}
};

}

#if defined(__GNUC__) && !defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
#pragma GCC pop_options
#endif

#endif
