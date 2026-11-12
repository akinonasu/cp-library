#ifndef CP_COMPONENTS_FPS_FACTORIALS_HPP
#define CP_COMPONENTS_FPS_FACTORIALS_HPP

#include "mint_factorials.hpp"

namespace fps_detail {
using mint_detail::factorials;
#if CP_X86
using mint_detail::factorials8, mint_detail::factorial_chain, mint_detail::factorial_transpose;
#endif
}

#endif
