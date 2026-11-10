#ifndef SIMD_CAPABILITIES_HPP
#define SIMD_CAPABILITIES_HPP

#if (defined(__x86_64__) || defined(__i386__)) && defined(__GNUC__) && !defined(__clang__)
_Pragma("GCC push_options")
_Pragma("GCC target(\"no-avx512f,avx512f\")")
#ifdef __AVX512F__
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif
_Pragma("GCC pop_options")
#elif (defined(__x86_64__) || defined(__i386__)) && defined(__clang__)
#define CP_LIBRARY_AVX512 1
#else
#define CP_LIBRARY_AVX512 0
#endif

#endif
