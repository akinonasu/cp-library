#ifndef CP_COMPONENTS_FPS_FACTORIALS_HPP
#define CP_COMPONENTS_FPS_FACTORIALS_HPP

#include "mint_factorials.hpp"

namespace fps_detail {
using mint_detail::factorials;
#if defined(__x86_64__) || defined(__i386__)
using mint_detail::factorials8;
using mint_detail::factorial_chain;
using mint_detail::factorial_transpose;
#endif
}

#endif
