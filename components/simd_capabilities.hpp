#ifndef SIMD_CAPABILITIES_HPP
#define SIMD_CAPABILITIES_HPP
#include "compiler_features.hpp"

#if CP_GCC_X86
_Pragma("GCC push_options")
_Pragma("GCC target(\"no-avx512f,avx512f\")")
#ifdef __AVX512F__
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif
_Pragma("GCC pop_options")
#elif CP_X86 && defined(__clang__)
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif

#endif
