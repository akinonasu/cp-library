#ifndef LCS_HPP
#define LCS_HPP

#include "../components/lcs_kernel.hpp"
#include "../components/sequence_compare.hpp"
#include "../components/sequence_bit_masks.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#if defined(__x86_64__)
#include <immintrin.h>
#endif

namespace lcs_detail {
inline bool subsequence(std::string_view s, std::string_view t) {
	if (s.empty()) return true;
	std::size_t i = 0;
	for (char c : t)
		if (s[i] == c && ++i == s.size()) return true;
	return false;
}

[[gnu::always_inline]] inline std::pair<std::string_view, std::string_view> trim(std::string_view &s, std::string_view &t) {
	std::size_t limit = std::min(s.size(), t.size());
	std::size_t prefix = limit && s.front() == t.front() ? sequence_compare_detail::prefix(s.data(), t.data(), int(limit)) : 0;
	std::string_view head = s.substr(0, prefix);
	s.remove_prefix(prefix);
	t.remove_prefix(prefix);
	limit -= prefix;
	std::size_t suffix = limit && s.back() == t.back() ? sequence_compare_detail::suffix(s.data() + s.size(), t.data() + t.size(), int(limit)) : 0;
	std::string_view tail = s.substr(s.size() - suffix);
	s.remove_suffix(suffix);
	t.remove_suffix(suffix);
	return {head, tail};
}

inline void filter(std::string_view &s, std::string_view &t, std::string &a, std::string &b) {
	std::array<unsigned char, 256> used{};
	for (unsigned char c : s) used[c] |= 1;
	for (unsigned char c : t) used[c] |= 2;
	bool change_s = false, change_t = false;
	for (int x : used) {
		change_s |= x == 1;
		change_t |= x == 2;
	}
	if (change_s) {
		a.reserve(s.size());
		for (unsigned char c : s)
			if (used[c] == 3) a.push_back(char(c));
		s = a;
	}
	if (change_t) {
		b.reserve(t.size());
		for (unsigned char c : t)
			if (used[c] == 3) b.push_back(char(c));
		t = b;
	}
}

template <bool reverse = false, int backend = 0>
inline std::vector<Word> profile(std::string_view s, std::string_view t) {
	if (t.empty()) return {};
	const int m = int(t.size()), words = int((t.size() + 63) / 64);
	std::array<int, 256> ids;
	ids.fill(-1);
	int alphabet = 0;
	for (unsigned char c : t)
		if (ids[c] == -1) ids[c] = alphabet++;
#if defined(__x86_64__)
	int kernel = backend;
	if (backend == 0 && words >= 8) {
#if CP_LIBRARY_AVX512
		if (__builtin_cpu_supports("avx512f")) kernel = 3;
		else
#endif
		if (__builtin_cpu_supports("avx2")) kernel = 2;
	}
	if (kernel >= 2) {
		int stride = (words + 7) & ~7;
		std::vector<Word> storage(std::size_t(alphabet + 1) * stride + 7);
		Word *data = storage.data() + ((-reinterpret_cast<std::uintptr_t>(storage.data()) & 63) / sizeof(Word));
		Word *row = data + std::size_t(alphabet) * stride;
		std::fill_n(row, stride, ~Word(0));
		std::array<const Word *, 256> masks{};
		for (int c = 0; c < 256; ++c)
			if (ids[c] != -1) masks[c] = data + std::size_t(ids[c]) * stride;
		for (int j = 0; j < m; ++j) {
			unsigned char c = t[reverse ? m - 1 - j : j];
			data[std::size_t(ids[c]) * stride + j / 64] |= Word(1) << (j % 64);
		}
#if CP_LIBRARY_AVX512
		if (kernel == 3) avx512<reverse>(s, masks, row, stride);
		else
#endif
		avx2<reverse>(s, masks, row, stride);
		return std::vector<Word>(row, row + words);
	}
#endif
	std::vector<Word> masks(std::size_t(alphabet) * words), row(words, ~Word(0));
	for (int j = 0; j < m; ++j) {
		unsigned char c = t[reverse ? m - 1 - j : j];
		masks[std::size_t(ids[c]) * words + j / 64] |= Word(1) << (j % 64);
	}
	if (words == 1) {
		Word v = ~Word(0);
		for (int i = 0, n = int(s.size()); i < n; ++i) {
			int id = ids[static_cast<unsigned char>(s[reverse ? n - 1 - i : i])];
			if (id == -1) continue;
			Word u = v & masks[id];
			v = (v + u) | (v ^ u);
		}
		row[0] = v;
		return row;
	}
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		int id = ids[static_cast<unsigned char>(s[reverse ? n - 1 - i : i])];
		if (id == -1) continue;
		const Word *mask = masks.data() + std::size_t(id) * words;
		Word carry = 0;
		for (int j = 0; j < words; ++j) {
			Word v = row[j], u = v & mask[j];
			__uint128_t sum = __uint128_t(v) + u + carry;
			row[j] = Word(sum) | (v ^ u);
			carry = Word(sum >> 64);
		}
	}
	return row;
}

inline int count(const std::vector<Word> &row) {
	int result = 0;
	for (Word v : row) result += std::popcount(~v);
	return result;
}

template <int backend = 0>
inline void restore(std::string_view s, std::string_view t, std::string &out) {
	auto [head, tail] = trim(s, t);
	out.append(head);
	if (s.size() < t.size()) std::swap(s, t);
	if (t.empty()) {
		out.append(tail);
		return;
	}
	if (subsequence(t, s)) out.append(t);
	else if (s.size() <= 4096 / t.size()) {
		int n = int(s.size()), m = int(t.size()), stride = m + 1;
		std::vector<std::uint16_t> dp((n + 1) * stride);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j)
				dp[(i + 1) * stride + j + 1] = s[i] == t[j] ? dp[i * stride + j] + 1 : std::max(dp[i * stride + j + 1], dp[(i + 1) * stride + j]);
		std::size_t start = out.size();
		out.resize(start + dp[n * stride + m]);
		std::size_t pos = out.size();
		while (n && m) {
			if (s[n - 1] == t[m - 1]) out[--pos] = s[--n], --m;
			else if (dp[(n - 1) * stride + m] >= dp[n * stride + m - 1]) --n;
			else --m;
		}
	} else if (t.size() > 1) {
		int middle = int(s.size() / 2), split = 0, best;
		{
			auto left = profile<false, backend>(s.substr(0, middle), t);
			auto right = profile<true, backend>(s.substr(middle), t);
			int a = 0, b = count(right);
			best = b;
			for (int j = 0, m = int(t.size()); j < m; ++j) {
				a += int((~left[j / 64] >> (j % 64)) & 1);
				int k = m - 1 - j;
				b -= int((~right[k / 64] >> (k % 64)) & 1);
				if (a + b > best) best = a + b, split = j + 1;
			}
		}
		if (best) {
			restore<backend>(s.substr(0, middle), t.substr(0, split), out);
			restore<backend>(s.substr(middle), t.substr(split), out);
		}
	}
	out.append(tail);
}

template <bool reverse = false>
inline std::vector<Word> profile_scalar(std::span<const sequence_bit_masks_detail::Symbol> s, std::span<const sequence_bit_masks_detail::Symbol> t, sequence_bit_masks_detail::MaskWorkspace &workspace) {
	const int words = int((t.size() + 63) / 64);
	sequence_bit_masks_detail::SparseMasks masks(t, workspace, reverse);
	std::vector<Word> row(words, ~Word(0));
	for (int i = 0, n = int(s.size()); i < n; ++i) {
		int index = masks.first(s[reverse ? n - 1 - i : i]);
		if (index == -1) continue;
		Word carry = 0;
		for (int j = 0; j < words; ++j) {
			Word v = row[j], u = v & masks.next(index, j);
			__uint128_t sum = __uint128_t(v) + u + carry;
			row[j] = Word(sum) | (v ^ u);
			carry = Word(sum >> 64);
		}
	}
	return row;
}


template <bool reverse = false, int backend = 0>
inline std::vector<Word> profile(std::span<const Symbol> s, std::span<const Symbol> t, MaskWorkspace &workspace) {
	if (t.empty()) return {};
	int words = int((t.size() + 63) / 64);
	if constexpr (backend == 0) if (words < 8) return profile_scalar<reverse>(s, t, workspace);
	MaskPositions positions(t, workspace, reverse);
	if (positions.used.size() <= 256) {
		for (std::size_t i = 0; i < positions.used.size(); ++i) workspace.head[positions.used[i]] = int(i);
		std::string a, b;
		a.reserve(s.size()); b.reserve(t.size());
		for (Symbol c : s)
			if (workspace.head[c] != -1) a.push_back(char(workspace.head[c]));
		if (a.empty()) return std::vector<Word>(words, ~Word(0));
		for (Symbol c : t) b.push_back(char(workspace.head[c]));
		return profile<reverse, backend>(std::string_view(a), std::string_view(b));
	}
	std::uint64_t cost = 0, rows = s.size();
	std::size_t i = 0;
	for (; i < s.size(); ++i) {
		Symbol c = s[i];
		if (workspace.head[c] == -1) break;
		cost += std::min(workspace.head[c] - workspace.tail[c], words);
	}
	std::vector<Symbol> common;
	if (i < s.size()) {
		common.reserve(s.size());
		common.insert(common.end(), s.begin(), s.begin() + i);
		for (++i; i < s.size(); ++i) {
			Symbol c = s[i];
			if (workspace.head[c] == -1) continue;
			common.push_back(c);
			cost += std::min(workspace.head[c] - workspace.tail[c], words);
		}
		s = common;
	}
	if (!cost) return std::vector<Word>(words, ~Word(0));
	if constexpr (backend == 0)
		if (cost <= rows * words / 48) return profile_sparse<reverse>(s, positions, words);
	return profile_dense<reverse, backend>(s, positions, words);
}

inline void restore(std::span<const sequence_bit_masks_detail::Symbol> s, std::span<const sequence_bit_masks_detail::Symbol> t, std::vector<sequence_bit_masks_detail::Symbol> &out, sequence_bit_masks_detail::MaskWorkspace &workspace) {
	auto [head, tail] = sequence_bit_masks_detail::trim(s, t);
	out.insert(out.end(), head.begin(), head.end());
	if (s.size() < t.size()) std::swap(s, t);
	if (t.empty()) {
		out.insert(out.end(), tail.begin(), tail.end());
		return;
	}
	if (sequence_bit_masks_detail::subsequence(t, s)) out.insert(out.end(), t.begin(), t.end());
	else if (s.size() <= 4096 / t.size()) {
		int n = int(s.size()), m = int(t.size()), stride = m + 1;
		std::vector<std::uint16_t> dp((n + 1) * stride);
		for (int i = 0; i < n; ++i)
			for (int j = 0; j < m; ++j)
				dp[(i + 1) * stride + j + 1] = s[i] == t[j] ? dp[i * stride + j] + 1 : std::max(dp[i * stride + j + 1], dp[(i + 1) * stride + j]);
		std::size_t start = out.size();
		out.resize(start + dp[n * stride + m]);
		std::size_t pos = out.size();
		while (n && m) {
			if (s[n - 1] == t[m - 1]) out[--pos] = s[--n], --m;
			else if (dp[(n - 1) * stride + m] >= dp[n * stride + m - 1]) --n;
			else --m;
		}
	} else if (t.size() > 1) {
		int middle = int(s.size() / 2), split = 0, best;
		{
			auto left = profile(s.first(middle), t, workspace);
			auto right = profile<true>(s.subspan(middle), t, workspace);
			int a = 0, b = count(right);
			best = b;
			for (int j = 0, m = int(t.size()); j < m; ++j) {
				a += int((~left[j / 64] >> (j % 64)) & 1);
				int k = m - 1 - j;
				b -= int((~right[k / 64] >> (k % 64)) & 1);
				if (a + b > best) best = a + b, split = j + 1;
			}
		}
		if (best) {
			restore(s.first(middle), t.first(split), out, workspace);
			restore(s.subspan(middle), t.subspan(split), out, workspace);
		}
	}
	out.insert(out.end(), tail.begin(), tail.end());
}
}

inline int lcs_length(std::string_view s, std::string_view t) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()) && t.size() <= std::size_t(std::numeric_limits<int>::max()));
	auto [head, tail] = lcs_detail::trim(s, t);
	int result = int(head.size() + tail.size());
	if (s.size() < t.size()) std::swap(s, t);
	if (t.empty() || lcs_detail::subsequence(t, s)) return result + int(t.size());
	if (lcs_detail::near_candidate(s.size(), t.size())) {
		int near = lcs_detail::near(s, t);
		if (near >= 0) return result + near;
	}
	std::string a, b;
	lcs_detail::filter(s, t, a, b);
	if (s.size() < t.size()) std::swap(s, t);
	return result + lcs_detail::count(lcs_detail::profile(s, t));
}

inline std::string lcs(std::string_view s, std::string_view t) {
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()) && t.size() <= std::size_t(std::numeric_limits<int>::max()));
	std::string a, b, result;
	result.reserve(std::min(s.size(), t.size()));
	auto [head, tail] = lcs_detail::trim(s, t);
	result.append(head);
	if (!s.empty() && !t.empty() &&
		(!lcs_detail::near_candidate(s.size(), t.size()) || lcs_detail::near(s, t, &result) < 0)) {
		lcs_detail::filter(s, t, a, b);
		lcs_detail::restore(s, t, result);
	}
	result.append(tail);
	return result;
}

template <class S, std::size_t SizeS, class T, std::size_t SizeT>
requires sequence_bit_masks_detail::CompatibleSymbols<S, T>
int lcs_length(std::span<S, SizeS> first, std::span<T, SizeT> second) {
	using Value = std::remove_const_t<S>;
	std::span<const Value> s = first, t = second;
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()) && t.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (s.empty() || t.empty()) return 0;
	auto [head, tail] = sequence_bit_masks_detail::trim(s, t);
	int result = int(head.size() + tail.size());
	if (s.size() < t.size()) std::swap(s, t);
	if (t.empty() || sequence_bit_masks_detail::subsequence(t, s)) return result + int(t.size());
	if (lcs_detail::near_candidate(s.size(), t.size())) {
		int near = lcs_detail::near(s, t);
		if (near >= 0) return result + near;
	}
	sequence_bit_masks_detail::CompressedPair<Value> input(s, t);
	sequence_bit_masks_detail::MaskWorkspace workspace(input.symbols.size());
	return result + lcs_detail::count(lcs_detail::profile(input.first, input.second, workspace));
}

template <class S, std::size_t SizeS, class T, std::size_t SizeT>
requires sequence_bit_masks_detail::CompatibleSymbols<S, T>
std::vector<std::remove_const_t<S>> lcs(std::span<S, SizeS> first, std::span<T, SizeT> second) {
	using Value = std::remove_const_t<S>;
	std::span<const Value> s = first, t = second;
	assert(s.size() <= std::size_t(std::numeric_limits<int>::max()) && t.size() <= std::size_t(std::numeric_limits<int>::max()));
	if (s.empty() || t.empty()) return {};
	std::vector<Value> result;
	result.reserve(std::min(s.size(), t.size()));
	auto [head, tail] = sequence_bit_masks_detail::trim(s, t);
	result.insert(result.end(), head.begin(), head.end());
	if (!s.empty() && !t.empty() &&
		(!lcs_detail::near_candidate(s.size(), t.size()) || lcs_detail::near(s, t, &result) < 0)) {
		sequence_bit_masks_detail::CompressedPair<Value> input(s, t);
		sequence_bit_masks_detail::MaskWorkspace workspace(input.symbols.size());
		std::vector<sequence_bit_masks_detail::Symbol> restored;
		restored.reserve(std::min(s.size(), t.size()));
		lcs_detail::restore(input.first, input.second, restored, workspace);
		for (auto c : restored) result.push_back(input.symbols[c]);
	}
	result.insert(result.end(), tail.begin(), tail.end());
	return result;
}

template <sequence_bit_masks_detail::IntegralSymbol T, class AllocatorS, class AllocatorT>
int lcs_length(const std::vector<T, AllocatorS> &s, const std::vector<T, AllocatorT> &t) {
	return lcs_length(std::span<const T>(s.data(), s.size()), std::span<const T>(t.data(), t.size()));
}

template <sequence_bit_masks_detail::IntegralSymbol T, class AllocatorS, class AllocatorT>
std::vector<T> lcs(const std::vector<T, AllocatorS> &s, const std::vector<T, AllocatorT> &t) {
	return lcs(std::span<const T>(s.data(), s.size()), std::span<const T>(t.data(), t.size()));
}

#endif
