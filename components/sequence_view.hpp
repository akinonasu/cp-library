#ifndef SEQUENCE_VIEW_HPP
#define SEQUENCE_VIEW_HPP

#include <span>
#include <string_view>
#include <type_traits>

namespace sequence_view_detail {
template<class R>
concept CharSequence = requires(const R &s) { std::span<const char>(s); } && !std::is_convertible_v<const R &, std::string_view>;

inline std::string_view char_view(std::string_view s) { return s; }

template<CharSequence R>
std::string_view char_view(const R &s) {
	std::span<const char> view(s);
	return {view.data(), view.size()};
}
}

#endif
