#pragma once
#include "fps/fps.hpp"
#include <cstdint>
using PublicF = FormalPowerSeries<fps_mint>;
using InternalF = FormalPowerSeries<fps_detail::Mint>;
#define DECLARE(T, F, prefix) \
F prefix##_add(const F &, T); \
F prefix##_sub(const F &, T); \
F prefix##_mul(const F &, T); \
F prefix##_div(const F &, T); \
std::uint64_t prefix##_checksum(const F &);
DECLARE(fps_mint, PublicF, public_fps)
DECLARE(fps_detail::Mint, InternalF, internal_fps)
#undef DECLARE
