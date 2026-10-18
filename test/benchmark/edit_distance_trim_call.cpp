#define edit_distance ED_API
#include "string/edit_distance.hpp"
extern "C" __attribute__((noinline, noclone)) int ED_WRAPPER(std::string_view s, std::string_view t) {
	return ED_API(s, t);
}
