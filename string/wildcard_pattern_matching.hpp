#ifndef STRING_WILDCARD_PATTERN_MATCHING_HPP
#define STRING_WILDCARD_PATTERN_MATCHING_HPP

#include "../components/wildcard_pattern_matching.hpp"
#include <algorithm>
#include <array>
#include <climits>
#include <concepts>
#include <cstdint>
#include <map>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <vector>

struct WildcardOptions {
	std::optional<unsigned char> text_wildcard = '*';
	std::optional<unsigned char> pattern_wildcard = '*';
};

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, WildcardOptions options) {
	if (text.size() > std::size_t(INT_MAX) || pattern.size() > (std::size_t(1) << 22))
		throw std::length_error("wildcard_pattern_matching: string too long");
	const std::size_t n = text.size(), m = pattern.size();
	if (m > n) return {};
	std::vector<bool> matches(n - m + 1, true);
	if (!m) return matches;
	auto wildcard = [](unsigned char c, std::optional<unsigned char> w) { return w && c == *w; };
	if (n == m) {
		for (std::size_t i = 0; i < m; ++i)
			if (text[i] != pattern[i] && !wildcard(text[i], options.text_wildcard) && !wildcard(pattern[i], options.pattern_wildcard)) {
				matches[0] = false;
				break;
			}
		return matches;
	}
	if (m <= 64) {
		std::array<std::uint64_t, 256> masks{};
		std::uint64_t any = 0, state = 0, last = std::uint64_t(1) << (m - 1);
		for (std::size_t i = 0; i < m; ++i) {
			unsigned char c = pattern[i];
			if (wildcard(c, options.pattern_wildcard)) any |= std::uint64_t(1) << i;
			else masks[c] |= std::uint64_t(1) << i;
		}
		for (auto &mask : masks) mask |= any;
		for (std::size_t i = 0; i < n; ++i) {
			unsigned char c = text[i];
			state = ((state << 1) | 1) & (wildcard(c, options.text_wildcard) ? ~std::uint64_t(0) : masks[c]);
			if (i + 1 >= m) matches[i + 1 - m] = (state & last) != 0;
		}
		return matches;
	}
	std::array<bool, 256> used{};
	std::array<std::size_t, 256> frequency{};
	bool text_wild = false, pattern_wild = false;
	std::size_t literals = 0, text_literals = 0;
	for (unsigned char c : text) {
		if (wildcard(c, options.text_wildcard)) text_wild = true;
		else { used[c] = true; ++frequency[c]; ++text_literals; }
	}
	for (unsigned char c : pattern) {
		if (wildcard(c, options.pattern_wildcard)) pattern_wild = true;
		else { used[c] = true; ++literals; }
	}
	if (!text_wild && !pattern_wild) {
		std::vector<int> prefix(m);
		for (std::size_t i = 1, j = 0; i < m; ++i) {
			while (j && pattern[i] != pattern[j]) j = prefix[j - 1];
			if (pattern[i] == pattern[j]) ++j;
			prefix[i] = static_cast<int>(j);
		}
		std::fill(matches.begin(), matches.end(), false);
		for (std::size_t i = 0, j = 0; i < n; ++i) {
			while (j && text[i] != pattern[j]) j = prefix[j - 1];
			if (text[i] == pattern[j]) ++j;
			if (j == m) { matches[i + 1 - m] = true; j = prefix[j - 1]; }
		}
		return matches;
	}
	std::array<std::uint16_t, 256> codes{};
	std::uint16_t alphabet = 0;
	for (std::size_t c = 0; c < used.size(); ++c)
		if (used[c]) codes[c] = ++alphabet;
	if (alphabet <= 1 || !literals || !text_literals) return matches;
	std::vector<std::uint16_t> a(n), b(m);
	for (std::size_t i = 0; i < n; ++i) {
		unsigned char c = text[i];
		a[i] = wildcard(c, options.text_wildcard) ? 0 : codes[c];
	}
	std::size_t first = m;
	for (std::size_t i = 0; i < m; ++i) {
		unsigned char c = pattern[i];
		if (!wildcard(c, options.pattern_wildcard)) {
			b[i] = codes[c];
			if (first == m || frequency[c] < frequency[static_cast<unsigned char>(pattern[first])]) first = i;
		}
	}
	std::vector<int> candidates;
	candidates.reserve(matches.size());
	for (std::size_t i = 0; i < matches.size(); ++i) candidates.push_back(static_cast<int>(i));
	std::uint64_t budget = 8 * (std::uint64_t(n) + m);
	auto filter = [&](std::size_t position) {
		if (candidates.size() > budget) return false;
		budget -= candidates.size();
		std::size_t kept = 0;
		for (int start : candidates) {
			auto value = a[std::size_t(start) + position];
			if (!value || value == b[position]) candidates[kept++] = start;
			else matches[start] = false;
		}
		candidates.resize(kept);
		return true;
	};
	filter(first);
	for (std::size_t i = 0; i < m && !candidates.empty(); ++i)
		if (b[i] && i != first && !filter(i)) {
			std::uint64_t bound = std::uint64_t(literals) * (alphabet - 1) * (alphabet - 1);
			wildcard_matching_detail::filter_ntt(a, b, matches, bound, text_wild, pattern_wild);
			break;
		}
	return matches;
}

inline std::vector<bool> wildcard_pattern_matching(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_pattern_matching(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, WildcardOptions options) {
	auto matches = wildcard_pattern_matching(text, pattern, options);
	std::vector<int> positions;
	for (std::size_t i = 0; i < matches.size(); ++i)
		if (matches[i]) positions.push_back(static_cast<int>(i));
	return positions;
}

inline std::vector<int> wildcard_find_all(std::string_view text, std::string_view pattern, char wildcard = '*') {
	return wildcard_find_all(text, pattern, WildcardOptions{static_cast<unsigned char>(wildcard), static_cast<unsigned char>(wildcard)});
}

namespace wildcard_matching_detail {

template <class T>
concept IntegerSymbol = std::is_integral_v<T> && sizeof(T) <= 8 && !std::same_as<T, bool> && !std::is_volatile_v<T>;

}

template <wildcard_matching_detail::IntegerSymbol T>
struct BasicWildcardOptions {
	std::optional<T> text_wildcard = std::same_as<T, char> ? std::optional<T>('*') : std::nullopt;
	std::optional<T> pattern_wildcard = std::same_as<T, char> ? std::optional<T>('*') : std::nullopt;
};

template <class T, std::size_t TextExtent, class U, std::size_t PatternExtent>
	requires wildcard_matching_detail::IntegerSymbol<std::remove_const_t<T>> &&
		std::same_as<std::remove_const_t<T>, std::remove_const_t<U>> && (!std::is_volatile_v<T> && !std::is_volatile_v<U>)
std::vector<bool> wildcard_pattern_matching(std::span<T, TextExtent> text, std::span<U, PatternExtent> pattern,
		BasicWildcardOptions<std::remove_const_t<T>> options = {}) {
	using Symbol = std::remove_const_t<T>;
	if constexpr (std::same_as<Symbol, char>) {
		WildcardOptions bytes{std::nullopt, std::nullopt};
		if (options.text_wildcard) bytes.text_wildcard = static_cast<unsigned char>(*options.text_wildcard);
		if (options.pattern_wildcard) bytes.pattern_wildcard = static_cast<unsigned char>(*options.pattern_wildcard);
		return wildcard_pattern_matching(std::string_view(text.data(), text.size()), std::string_view(pattern.data(), pattern.size()), bytes);
	}
	if (text.size() > std::size_t(INT_MAX) || pattern.size() > (std::size_t(1) << 22))
		throw std::length_error("wildcard_pattern_matching: sequence too long");
	const std::size_t n = text.size(), m = pattern.size();
	if (m > n) return {};
	std::vector<bool> matches(n - m + 1, true);
	if (!m) return matches;
	auto wildcard = [](Symbol value, std::optional<Symbol> marker) { return marker && value == *marker; };
	if (n == m) {
		for (std::size_t i = 0; i < m; ++i)
			if (text[i] != pattern[i] && !wildcard(text[i], options.text_wildcard) && !wildcard(pattern[i], options.pattern_wildcard)) {
				matches[0] = false;
				break;
			}
		return matches;
	}
	if (m <= 64) {
		std::map<Symbol, std::uint64_t> masks;
		std::uint64_t any = 0, state = 0, last = std::uint64_t(1) << (m - 1);
		for (std::size_t i = 0; i < m; ++i)
			if (wildcard(pattern[i], options.pattern_wildcard)) any |= std::uint64_t(1) << i;
			else masks[pattern[i]] |= std::uint64_t(1) << i;
		for (std::size_t i = 0; i < n; ++i) {
			std::uint64_t mask = ~std::uint64_t(0);
			if (!wildcard(text[i], options.text_wildcard)) {
				auto it = masks.find(text[i]);
				mask = any | (it == masks.end() ? 0 : it->second);
			}
			state = ((state << 1) | 1) & mask;
			if (i + 1 >= m) matches[i + 1 - m] = (state & last) != 0;
		}
		return matches;
	}
	bool text_wild = false, pattern_wild = false;
	std::size_t literals = 0, text_literals = 0;
	for (auto value : text)
		if (wildcard(value, options.text_wildcard)) text_wild = true;
		else ++text_literals;
	for (auto value : pattern)
		if (wildcard(value, options.pattern_wildcard)) pattern_wild = true;
		else ++literals;
	if (!text_wild && !pattern_wild) {
		std::vector<int> prefix(m);
		for (std::size_t i = 1, j = 0; i < m; ++i) {
			while (j && pattern[i] != pattern[j]) j = prefix[j - 1];
			if (pattern[i] == pattern[j]) ++j;
			prefix[i] = static_cast<int>(j);
		}
		std::fill(matches.begin(), matches.end(), false);
		for (std::size_t i = 0, j = 0; i < n; ++i) {
			while (j && text[i] != pattern[j]) j = prefix[j - 1];
			if (text[i] == pattern[j]) ++j;
			if (j == m) { matches[i + 1 - m] = true; j = prefix[j - 1]; }
		}
		return matches;
	}
	if (!literals || !text_literals) return matches;
	std::vector<Symbol> alphabet;
	alphabet.reserve(literals);
	for (auto value : pattern)
		if (!wildcard(value, options.pattern_wildcard)) alphabet.push_back(value);
	std::sort(alphabet.begin(), alphabet.end());
	alphabet.erase(std::unique(alphabet.begin(), alphabet.end()), alphabet.end());
	auto code = [&](Symbol value) -> std::uint32_t {
		auto it = std::lower_bound(alphabet.begin(), alphabet.end(), value);
		return it != alphabet.end() && *it == value ? std::uint32_t(it - alphabet.begin()) + 1 : std::uint32_t(alphabet.size()) + 1;
	};
	std::vector<std::size_t> frequency(alphabet.size() + 2);
	std::vector<std::uint32_t> a(n), b(m);
	for (std::size_t i = 0; i < n; ++i)
		if (!wildcard(text[i], options.text_wildcard)) ++frequency[a[i] = code(text[i])];
	std::size_t first = m;
	for (std::size_t i = 0; i < m; ++i)
		if (!wildcard(pattern[i], options.pattern_wildcard)) {
			b[i] = code(pattern[i]);
			if (first == m || frequency[b[i]] < frequency[b[first]]) first = i;
		}
	std::vector<int> candidates;
	candidates.reserve(matches.size());
	for (std::size_t i = 0; i < matches.size(); ++i) candidates.push_back(static_cast<int>(i));
	std::uint64_t budget = 8 * (std::uint64_t(n) + m);
	auto filter = [&](std::size_t position) {
		if (candidates.size() > budget) return false;
		budget -= candidates.size();
		std::size_t kept = 0;
		for (int start : candidates) {
			auto value = a[std::size_t(start) + position];
			if (!value || value == b[position]) candidates[kept++] = start;
			else matches[start] = false;
		}
		candidates.resize(kept);
		return true;
	};
	filter(first);
	for (std::size_t i = 0; i < m && !candidates.empty(); ++i)
		if (b[i] && i != first && !filter(i)) {
			__uint128_t bound = __uint128_t(literals) * alphabet.size() * alphabet.size();
			wildcard_matching_detail::filter_ntt(a, b, matches, bound, text_wild, pattern_wild);
			break;
		}
	return matches;
}

template <class T, std::size_t TextExtent, class U, std::size_t PatternExtent>
	requires wildcard_matching_detail::IntegerSymbol<std::remove_const_t<T>> &&
		std::same_as<std::remove_const_t<T>, std::remove_const_t<U>>
std::vector<bool> wildcard_pattern_matching(std::span<T, TextExtent> text, std::span<U, PatternExtent> pattern, std::remove_const_t<T> wildcard) {
	return wildcard_pattern_matching(text, pattern, BasicWildcardOptions<std::remove_const_t<T>>{wildcard, wildcard});
}

template <wildcard_matching_detail::IntegerSymbol T, class TextAllocator, class PatternAllocator>
std::vector<bool> wildcard_pattern_matching(const std::vector<T, TextAllocator> &text, const std::vector<T, PatternAllocator> &pattern,
		BasicWildcardOptions<T> options = {}) {
	return wildcard_pattern_matching(std::span<const T>(text.data(), text.size()), std::span<const T>(pattern.data(), pattern.size()), options);
}

template <wildcard_matching_detail::IntegerSymbol T, class TextAllocator, class PatternAllocator>
std::vector<bool> wildcard_pattern_matching(const std::vector<T, TextAllocator> &text, const std::vector<T, PatternAllocator> &pattern, std::type_identity_t<T> wildcard) {
	return wildcard_pattern_matching(text, pattern, BasicWildcardOptions<T>{wildcard, wildcard});
}

template <class Text, class Pattern, class... Options>
	requires requires(const Text &text, const Pattern &pattern, Options... options) { wildcard_pattern_matching(text, pattern, options...); } &&
		(!std::is_convertible_v<Text, std::string_view> || !std::is_convertible_v<Pattern, std::string_view>)
std::vector<int> wildcard_find_all(const Text &text, const Pattern &pattern, Options... options) {
	auto matches = wildcard_pattern_matching(text, pattern, options...);
	std::vector<int> positions;
	for (std::size_t i = 0; i < matches.size(); ++i)
		if (matches[i]) positions.push_back(static_cast<int>(i));
	return positions;
}

#endif
