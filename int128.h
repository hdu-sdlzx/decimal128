//
// Copyright 2017 The Abseil Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#pragma once

// -----------------------------------------------------------------------------
// File: int128.h
// -----------------------------------------------------------------------------
//
// This header file defines 128-bit integer types, `uint128` and `int128`.
//

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <limits>
#include <string>
#include <utility>

// -----------------------------------------------------------------------------
// C++ Version Check
// -----------------------------------------------------------------------------

#if defined(__GNUC__) && (__GNUC__ < 8)
#error "Gcc 8.0 or later is required."
#endif  // __GNUC__ && (__GNUC__ < 8)

#if defined(__clang__) && (__clang_major__ < 9)
#error "Clang 9.0 or later is required."
#endif  // __clang__ && (__clang_major__ < 9)

#if defined(_MSC_VER ) && (_MSC_VER  < 1930)
#error "Visual Studio 2022 or later is required."
#endif  // _MSC_VER && (_MSC_VER < 1930)

// Enforce C++20 as the minimum.
#if defined(_MSVC_LANG)
#if _MSVC_LANG < 202002L
#error "C++ versions less than C++20 are not supported."
#endif  // _MSVC_LANG < 202002L
#elif defined(__cplusplus)
#if __cplusplus < 202002L
#error "C++ versions less than C++20 are not supported."
#endif  // __cplusplus < 202002L
#endif

// -----------------------------------------------------------------------------
// File: config.h
// -----------------------------------------------------------------------------
//
// This header file defines a set of macros for checking the presence of
// important compiler and platform features. Such macros can be used to
// produce portable code by parameterizing compilation based on the presence or
// lack of a given feature.
//
// We define a "feature" as some interface we wish to program to: for example,
// a library function or system call. A value of `1` indicates support for
// that feature; any other value indicates the feature support is undefined.
//
// Example:
//
// Suppose a programmer wants to write a program that uses the 'mmap()' system
// call. The Abseil macro for that feature (`ABSL_HAVE_MMAP`) allows you to
// selectively include the `mmap.h` header and bracket code using that feature
// in the macro:
//
//   #include "absl/base/config.h"
//
//   #ifdef ABSL_HAVE_MMAP
//   #include "sys/mman.h"
//   #endif  //ABSL_HAVE_MMAP
//
//   ...
//   #ifdef ABSL_HAVE_MMAP
//   void *ptr = mmap(...);
//   ...
//   #endif  // ABSL_HAVE_MMAP

// -----------------------------------------------------------------------------
// Compiler Feature Checks
// -----------------------------------------------------------------------------

// ABSL_HAVE_BUILTIN()
//
// Checks whether the compiler supports a Clang Feature Checking Macro, and if
// so, checks whether it supports the provided builtin function "x" where x
// is one of the functions noted in
// https://clang.llvm.org/docs/LanguageExtensions.html
//
// Note: Use this macro to avoid an extra level of #ifdef __has_builtin check.
// http://releases.llvm.org/3.3/tools/clang/docs/LanguageExtensions.html
#ifdef __has_builtin
#define ABSL_HAVE_BUILTIN(x) __has_builtin(x)
#else
#define ABSL_HAVE_BUILTIN(x) 0
#endif

#ifdef __has_feature
#define ABSL_HAVE_FEATURE(f) __has_feature(f)
#else
#define ABSL_HAVE_FEATURE(f) 0
#endif

// ABSL_HAVE_INTRINSIC_INT128
//
// Checks whether the __int128 compiler extension for a 128-bit integral type is
// supported.
//
// Note: __SIZEOF_INT128__ is defined by Clang and GCC when __int128 is
// supported, but we avoid using it in certain cases:
// * On Clang:
//   * Building using Clang for Windows, where the Clang runtime library has
//     128-bit support only on LP64 architectures, but Windows is LLP64.
// * On Nvidia's nvcc:
//   * nvcc also defines __GNUC__ and __SIZEOF_INT128__, but not all versions
//     actually support __int128.
#ifdef ABSL_HAVE_INTRINSIC_INT128
#error ABSL_HAVE_INTRINSIC_INT128 cannot be directly set
#elif defined(__SIZEOF_INT128__)
#if (defined(__clang__) && !defined(_WIN32)) ||           \
    (defined(__CUDACC__) && __CUDACC_VER_MAJOR__ >= 9) || \
    (defined(__GNUC__) && !defined(__clang__) && !defined(__CUDACC__))
#define ABSL_HAVE_INTRINSIC_INT128 1
#elif defined(__CUDACC__)
// __CUDACC_VER__ is a full version number before CUDA 9, and is defined to a
// string explaining that it has been removed starting with CUDA 9. We use
// nested #ifs because there is no short-circuiting in the preprocessor.
// NOTE: `__CUDACC__` could be undefined while `__CUDACC_VER__` is defined.
#if __CUDACC_VER__ >= 70000
#define ABSL_HAVE_INTRINSIC_INT128 1
#endif  // __CUDACC_VER__ >= 70000
#endif  // defined(__CUDACC__)
#endif  // ABSL_HAVE_INTRINSIC_INT128

// ABSL_IS_LITTLE_ENDIAN
// ABSL_IS_BIG_ENDIAN
//
// Checks the endianness of the platform.
//
// Prefer using `std::endian` in C++20, or `absl::endian` from
// absl/numeric/bits.h prior to C++20.
//
// Notes: uses the built in endian macros provided by GCC (since 4.6) and
// Clang (since 3.2); see
// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html.
// Otherwise, if _WIN32, assume little endian. Otherwise, bail with an error.
#if defined(ABSL_IS_BIG_ENDIAN)
#error "ABSL_IS_BIG_ENDIAN cannot be directly set."
#endif
#if defined(ABSL_IS_LITTLE_ENDIAN)
#error "ABSL_IS_LITTLE_ENDIAN cannot be directly set."
#endif

#if (defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
     __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define ABSL_IS_LITTLE_ENDIAN 1
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
    __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define ABSL_IS_BIG_ENDIAN 1
#elif defined(_WIN32)
#define ABSL_IS_LITTLE_ENDIAN 1
#else
#error "absl endian detection needs to be set up for your compiler"
#endif

// -----------------------------------------------------------------------------
// File: optimization.h
// -----------------------------------------------------------------------------
//
// This header file defines portable macros for performance optimization.
//
// This header is included in both C++ code and legacy C code and thus must
// remain compatible with both C and C++. C compatibility will be removed if
// the legacy code is removed or converted to C++. Do not include this header in
// new code that requires C compatibility or assume C compatibility will remain
// indefinitely.

// ABSL_INTERNAL_CPLUSPLUS_LANG
//
// MSVC does not set the value of __cplusplus correctly, but instead uses
// _MSVC_LANG as a stand-in.
// https://docs.microsoft.com/en-us/cpp/preprocessor/predefined-macros
//
// However, there are reports that MSVC even sets _MSVC_LANG incorrectly at
// times, for example:
// https://github.com/microsoft/vscode-cpptools/issues/1770
// https://reviews.llvm.org/D70996
//
// For this reason, this symbol is considered INTERNAL and code outside of
// Abseil must not use it.
#if defined(_MSVC_LANG)
#define ABSL_INTERNAL_CPLUSPLUS_LANG _MSVC_LANG
#elif defined(__cplusplus)
#define ABSL_INTERNAL_CPLUSPLUS_LANG __cplusplus
#endif

// ABSL_ASSUME(cond)
//
// Informs the compiler that a condition is always true and that it can assume
// it to be true for optimization purposes.
//
// WARNING: If the condition is false, the program can produce undefined and
// potentially dangerous behavior.
//
// In !NDEBUG mode, the condition is checked with an assert().
//
// NOTE: The expression must not have side effects, as it may only be evaluated
// in some compilation modes and not others. Some compilers may issue a warning
// if the compiler cannot prove the expression has no side effects. For example,
// the expression should not use a function call since the compiler cannot prove
// that a function call does not have side effects.
//
// Example:
//
//   int x = ...;
//   ABSL_ASSUME(x >= 0);
//   // The compiler can optimize the division to a simple right shift using the
//   // assumption specified above.
//   int y = x / 16;
//
#if !defined(NDEBUG)
#define ABSL_ASSUME(cond) assert(cond)
#elif ABSL_HAVE_BUILTIN(__builtin_assume)
#define ABSL_ASSUME(cond) __builtin_assume(cond)
#elif defined(_MSC_VER)
#define ABSL_ASSUME(cond) __assume(cond)
#elif defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define ABSL_ASSUME(cond) ((cond) ? void() : std::unreachable())
#elif defined(__GNUC__) || ABSL_HAVE_BUILTIN(__builtin_unreachable)
#define ABSL_ASSUME(cond) ((cond) ? void() : __builtin_unreachable())
#elif ABSL_INTERNAL_CPLUSPLUS_LANG >= 202002L
// Unimplemented. Uses the same definition as ABSL_ASSERT in the NDEBUG case.
#define ABSL_ASSUME(expr) (decltype((expr) ? void() : void())())
#else
#define ABSL_ASSUME(expr) (false ? ((expr) ? void() : void()) : void())
#endif

#if defined(_MSC_VER)
// In very old versions of MSVC and when the /Zc:wchar_t flag is off, wchar_t is
// a typedef for unsigned short.  Otherwise wchar_t is mapped to the __wchar_t
// builtin type.  We need to make sure not to define operator wchar_t()
// alongside operator unsigned short() in these instances.
#define ABSL_INTERNAL_WCHAR_T __wchar_t
#if defined(_M_X64) && !defined(_M_ARM64EC)
#include <intrin.h>
#pragma intrinsic(_umul128)
#endif  // defined(_M_X64)
#else   // defined(_MSC_VER)
#define ABSL_INTERNAL_WCHAR_T wchar_t
#endif  // defined(_MSC_VER)

namespace absl {

class int128;

// uint128
//
// An unsigned 128-bit integer type. The API is meant to mimic an intrinsic type
// as closely as is practical, including exhibiting undefined behavior in
// analogous cases (e.g. division by zero). This type is intended to be a
// drop-in replacement once C++ supports an intrinsic `uint128_t` type; when
// that occurs, existing well-behaved uses of `uint128` will continue to work
// using that new type.
//
// Note: code written with this type will continue to compile once `uint128_t`
// is introduced, provided the replacement helper functions
// `Uint128(Low|High)64()` and `MakeUint128()` are made.
//
// A `uint128` supports the following:
//
//   * Implicit construction from integral types
//   * Explicit conversion to integral types
//
// Additionally, if your compiler supports `__int128`, `uint128` is
// interoperable with that type. (Abseil checks for this compatibility through
// the `ABSL_HAVE_INTRINSIC_INT128` macro.)
//
// However, a `uint128` differs from intrinsic integral types in the following
// ways:
//
//   * Errors on implicit conversions that do not preserve value (such as
//     loss of precision when converting to float values).
//   * Requires explicit construction from and conversion to floating point
//     types.
//   * Conversion to integral types requires an explicit static_cast() to
//     mimic use of the `-Wnarrowing` compiler flag.
//   * The alignment requirement of `uint128` may differ from that of an
//     intrinsic 128-bit integer type depending on platform and build
//     configuration.
//
// Example:
//
//     float y = absl::Uint128Max();  // Error. uint128 cannot be implicitly
//                                    // converted to float.
//
//     absl::uint128 v;
//     uint64_t i = v;                         // Error
//     uint64_t i = static_cast<uint64_t>(v);  // OK
//
class
#if defined(ABSL_HAVE_INTRINSIC_INT128)
    alignas(unsigned __int128)
#endif  // ABSL_HAVE_INTRINSIC_INT128
        uint128 {
 public:
  uint128() = default;

  // Constructors from arithmetic types
  constexpr uint128(int v);                 // NOLINT(runtime/explicit)
  constexpr uint128(unsigned int v);        // NOLINT(runtime/explicit)
  constexpr uint128(long v);                // NOLINT(runtime/int)
  constexpr uint128(unsigned long v);       // NOLINT(runtime/int)
  constexpr uint128(long long v);           // NOLINT(runtime/int)
  constexpr uint128(unsigned long long v);  // NOLINT(runtime/int)
#ifdef ABSL_HAVE_INTRINSIC_INT128
  constexpr uint128(__int128 v);           // NOLINT(runtime/explicit)
  constexpr uint128(unsigned __int128 v);  // NOLINT(runtime/explicit)
#endif                                     // ABSL_HAVE_INTRINSIC_INT128
  constexpr uint128(int128 v);             // NOLINT(runtime/explicit)
  explicit uint128(float v);
  explicit uint128(double v);
  explicit uint128(long double v);

  // Assignment operators from arithmetic types
  uint128& operator=(int v);
  uint128& operator=(unsigned int v);
  uint128& operator=(long v);                // NOLINT(runtime/int)
  uint128& operator=(unsigned long v);       // NOLINT(runtime/int)
  uint128& operator=(long long v);           // NOLINT(runtime/int)
  uint128& operator=(unsigned long long v);  // NOLINT(runtime/int)
#ifdef ABSL_HAVE_INTRINSIC_INT128
  uint128& operator=(__int128 v);
  uint128& operator=(unsigned __int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128
  uint128& operator=(int128 v);

  // Conversion operators to other arithmetic types
  constexpr explicit operator bool() const;
  constexpr explicit operator char() const;
  constexpr explicit operator signed char() const;
  constexpr explicit operator unsigned char() const;
  constexpr explicit operator char16_t() const;
  constexpr explicit operator char32_t() const;
  constexpr explicit operator ABSL_INTERNAL_WCHAR_T() const;
  constexpr explicit operator short() const;  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned short() const;
  constexpr explicit operator int() const;
  constexpr explicit operator unsigned int() const;
  constexpr explicit operator long() const;  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned long() const;
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator long long() const;
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned long long() const;
#ifdef ABSL_HAVE_INTRINSIC_INT128
  constexpr explicit operator __int128() const;
  constexpr explicit operator unsigned __int128() const;
#endif  // ABSL_HAVE_INTRINSIC_INT128
  explicit operator float() const;
  explicit operator double() const;
  explicit operator long double() const;

  // Trivial copy constructor, assignment operator and destructor.

  // Arithmetic operators.
  uint128& operator+=(uint128 other);
  uint128& operator-=(uint128 other);
  uint128& operator*=(uint128 other);
  // Long division/modulo for uint128.
  uint128& operator/=(uint128 other);
  uint128& operator%=(uint128 other);
  uint128 operator++(int);
  uint128 operator--(int);
  uint128& operator<<=(int);
  uint128& operator>>=(int);
  uint128& operator&=(uint128 other);
  uint128& operator|=(uint128 other);
  uint128& operator^=(uint128 other);
  uint128& operator++();
  uint128& operator--();

  // Uint128Low64()
  //
  // Returns the lower 64-bit value of a `uint128` value.
  friend constexpr uint64_t Uint128Low64(uint128 v);

  // Uint128High64()
  //
  // Returns the higher 64-bit value of a `uint128` value.
  friend constexpr uint64_t Uint128High64(uint128 v);

  // MakeUInt128()
  //
  // Constructs a `uint128` numeric value from two 64-bit unsigned integers.
  // Note that this factory function is the only way to construct a `uint128`
  // from integer values greater than 2^64.
  //
  // Example:
  //
  //   absl::uint128 big = absl::MakeUint128(1, 0);
  friend constexpr uint128 MakeUint128(uint64_t high, uint64_t low);

  // Uint128Max()
  //
  // Returns the highest value for a 128-bit unsigned integer.
  friend constexpr uint128 Uint128Max();

  // Support for absl::Hash.
  template <typename H>
  friend H AbslHashValue(H h, uint128 v) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
    return H::combine(std::move(h), static_cast<unsigned __int128>(v));
#else
    return H::combine(std::move(h), Uint128High64(v), Uint128Low64(v));
#endif
  }

  // Support for absl::StrCat() etc.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, uint128 v) {
    sink.Append(v.ToString());
  }

 private:
  constexpr uint128(uint64_t high, uint64_t low);

  std::string ToString() const;

  // TODO(strel) Update implementation to use __int128 once all users of
  // uint128 are fixed to not depend on alignof(uint128) == 8. Also add
  // alignas(16) to class definition to keep alignment consistent across
  // platforms.
#if defined(ABSL_IS_LITTLE_ENDIAN)
  uint64_t lo_;
  uint64_t hi_;
#elif defined(ABSL_IS_BIG_ENDIAN)
  uint64_t hi_;
  uint64_t lo_;
#else  // byte order
#error "Unsupported byte order: must be little-endian or big-endian."
#endif  // byte order
};

// allow uint128 to be logged
std::ostream& operator<<(std::ostream& os, uint128 v);

// TODO(strel) add operator>>(std::istream&, uint128)

constexpr uint128 Uint128Max() {
  return uint128((std::numeric_limits<uint64_t>::max)(),
                 (std::numeric_limits<uint64_t>::max)());
}

}  // namespace absl

// Specialized numeric_limits for uint128.
namespace std {
template <>
class numeric_limits<absl::uint128> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = false;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
  static constexpr float_denorm_style has_denorm = denorm_absent;
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = true;
  static constexpr int digits = 128;
  static constexpr int digits10 = 38;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
#ifdef ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool traps = numeric_limits<unsigned __int128>::traps;
#else   // ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool traps = numeric_limits<uint64_t>::traps;
#endif  // ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool tinyness_before = false;

  static constexpr absl::uint128(min)() { return 0; }
  static constexpr absl::uint128 lowest() { return 0; }
  static constexpr absl::uint128(max)() { return absl::Uint128Max(); }
  static constexpr absl::uint128 epsilon() { return 0; }
  static constexpr absl::uint128 round_error() { return 0; }
  static constexpr absl::uint128 infinity() { return 0; }
  static constexpr absl::uint128 quiet_NaN() { return 0; }
  static constexpr absl::uint128 signaling_NaN() { return 0; }
  static constexpr absl::uint128 denorm_min() { return 0; }
};
}  // namespace std

namespace absl {

// int128
//
// A signed 128-bit integer type. The API is meant to mimic an intrinsic
// integral type as closely as is practical, including exhibiting undefined
// behavior in analogous cases (e.g. division by zero).
//
// An `int128` supports the following:
//
//   * Implicit construction from integral types
//   * Explicit conversion to integral types
//
// However, an `int128` differs from intrinsic integral types in the following
// ways:
//
//   * It is not implicitly convertible to other integral types.
//   * Requires explicit construction from and conversion to floating point
//     types.

// Additionally, if your compiler supports `__int128`, `int128` is
// interoperable with that type. (Abseil checks for this compatibility through
// the `ABSL_HAVE_INTRINSIC_INT128` macro.)
//
// The design goal for `int128` is that it will be compatible with a future
// `int128_t`, if that type becomes a part of the standard.
//
// Example:
//
//     float y = absl::int128(17);  // Error. int128 cannot be implicitly
//                                  // converted to float.
//
//     absl::int128 v;
//     int64_t i = v;                        // Error
//     int64_t i = static_cast<int64_t>(v);  // OK
//
class int128 {
 public:
  int128() = default;

  // Constructors from arithmetic types
  constexpr int128(int v);                 // NOLINT(runtime/explicit)
  constexpr int128(unsigned int v);        // NOLINT(runtime/explicit)
  constexpr int128(long v);                // NOLINT(runtime/int)
  constexpr int128(unsigned long v);       // NOLINT(runtime/int)
  constexpr int128(long long v);           // NOLINT(runtime/int)
  constexpr int128(unsigned long long v);  // NOLINT(runtime/int)
#ifdef ABSL_HAVE_INTRINSIC_INT128
  constexpr int128(__int128 v);  // NOLINT(runtime/explicit)
  constexpr explicit int128(unsigned __int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128
  constexpr explicit int128(uint128 v);
  explicit int128(float v);
  explicit int128(double v);
  explicit int128(long double v);

  // Assignment operators from arithmetic types
  int128& operator=(int v);
  int128& operator=(unsigned int v);
  int128& operator=(long v);                // NOLINT(runtime/int)
  int128& operator=(unsigned long v);       // NOLINT(runtime/int)
  int128& operator=(long long v);           // NOLINT(runtime/int)
  int128& operator=(unsigned long long v);  // NOLINT(runtime/int)
#ifdef ABSL_HAVE_INTRINSIC_INT128
  int128& operator=(__int128 v);
#endif  // ABSL_HAVE_INTRINSIC_INT128

  // Conversion operators to other arithmetic types
  constexpr explicit operator bool() const;
  constexpr explicit operator char() const;
  constexpr explicit operator signed char() const;
  constexpr explicit operator unsigned char() const;
  constexpr explicit operator char16_t() const;
  constexpr explicit operator char32_t() const;
  constexpr explicit operator ABSL_INTERNAL_WCHAR_T() const;
  constexpr explicit operator short() const;  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned short() const;
  constexpr explicit operator int() const;
  constexpr explicit operator unsigned int() const;
  constexpr explicit operator long() const;  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned long() const;
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator long long() const;
  // NOLINTNEXTLINE(runtime/int)
  constexpr explicit operator unsigned long long() const;
#ifdef ABSL_HAVE_INTRINSIC_INT128
  constexpr explicit operator __int128() const;
  constexpr explicit operator unsigned __int128() const;
#endif  // ABSL_HAVE_INTRINSIC_INT128
  explicit operator float() const;
  explicit operator double() const;
  explicit operator long double() const;

  // Trivial copy constructor, assignment operator and destructor.

  // Arithmetic operators
  int128& operator+=(int128 other);
  int128& operator-=(int128 other);
  int128& operator*=(int128 other);
  int128& operator/=(int128 other);
  int128& operator%=(int128 other);
  int128 operator++(int);  // postfix increment: i++
  int128 operator--(int);  // postfix decrement: i--
  int128& operator++();    // prefix increment:  ++i
  int128& operator--();    // prefix decrement:  --i
  int128& operator&=(int128 other);
  int128& operator|=(int128 other);
  int128& operator^=(int128 other);
  int128& operator<<=(int amount);
  int128& operator>>=(int amount);

  // Int128Low64()
  //
  // Returns the lower 64-bit value of a `int128` value.
  friend constexpr uint64_t Int128Low64(int128 v);

  // Int128High64()
  //
  // Returns the higher 64-bit value of a `int128` value.
  friend constexpr int64_t Int128High64(int128 v);

  // MakeInt128()
  //
  // Constructs a `int128` numeric value from two 64-bit integers. Note that
  // signedness is conveyed in the upper `high` value.
  //
  //   (absl::int128(1) << 64) * high + low
  //
  // Note that this factory function is the only way to construct a `int128`
  // from integer values greater than 2^64 or less than -2^64.
  //
  // Example:
  //
  //   absl::int128 big = absl::MakeInt128(1, 0);
  //   absl::int128 big_n = absl::MakeInt128(-1, 0);
  friend constexpr int128 MakeInt128(int64_t high, uint64_t low);

  // Int128Max()
  //
  // Returns the maximum value for a 128-bit signed integer.
  friend constexpr int128 Int128Max();

  // Int128Min()
  //
  // Returns the minimum value for a 128-bit signed integer.
  friend constexpr int128 Int128Min();

  // Support for absl::Hash.
  template <typename H>
  friend H AbslHashValue(H h, int128 v) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
    return H::combine(std::move(h), v.v_);
#else
    return H::combine(std::move(h), Int128High64(v), Int128Low64(v));
#endif
  }

  // Support for absl::StrCat() etc.
  template <typename Sink>
  friend void AbslStringify(Sink& sink, int128 v) {
    sink.Append(v.ToString());
  }

 private:
  constexpr int128(int64_t high, uint64_t low);

  std::string ToString() const;

#if defined(ABSL_HAVE_INTRINSIC_INT128)
  __int128 v_;
#else  // ABSL_HAVE_INTRINSIC_INT128
#if defined(ABSL_IS_LITTLE_ENDIAN)
  uint64_t lo_;
  int64_t hi_;
#elif defined(ABSL_IS_BIG_ENDIAN)
  int64_t hi_;
  uint64_t lo_;
#else  // byte order
#error "Unsupported byte order: must be little-endian or big-endian."
#endif  // byte order
#endif  // ABSL_HAVE_INTRINSIC_INT128
};

std::ostream& operator<<(std::ostream& os, int128 v);

// TODO(absl-team) add operator>>(std::istream&, int128)

constexpr int128 Int128Max() {
  return int128((std::numeric_limits<int64_t>::max)(),
                (std::numeric_limits<uint64_t>::max)());
}

constexpr int128 Int128Min() {
  return int128((std::numeric_limits<int64_t>::min)(), 0);
}

}  // namespace absl

// Specialized numeric_limits for int128.
namespace std {
template <>
class numeric_limits<absl::int128> {
 public:
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = true;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = false;
  static constexpr bool has_quiet_NaN = false;
  static constexpr bool has_signaling_NaN = false;
  static constexpr float_denorm_style has_denorm = denorm_absent;
  static constexpr bool has_denorm_loss = false;
  static constexpr float_round_style round_style = round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = false;
  static constexpr int digits = 127;
  static constexpr int digits10 = 38;
  static constexpr int max_digits10 = 0;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
#ifdef ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool traps = numeric_limits<__int128>::traps;
#else   // ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool traps = numeric_limits<uint64_t>::traps;
#endif  // ABSL_HAVE_INTRINSIC_INT128
  static constexpr bool tinyness_before = false;

  static constexpr absl::int128(min)() { return absl::Int128Min(); }
  static constexpr absl::int128 lowest() { return absl::Int128Min(); }
  static constexpr absl::int128(max)() { return absl::Int128Max(); }
  static constexpr absl::int128 epsilon() { return 0; }
  static constexpr absl::int128 round_error() { return 0; }
  static constexpr absl::int128 infinity() { return 0; }
  static constexpr absl::int128 quiet_NaN() { return 0; }
  static constexpr absl::int128 signaling_NaN() { return 0; }
  static constexpr absl::int128 denorm_min() { return 0; }
};
}  // namespace std

// --------------------------------------------------------------------------
//                      Implementation details follow
// --------------------------------------------------------------------------
namespace absl {

constexpr uint128 MakeUint128(uint64_t high, uint64_t low) {
  return uint128(high, low);
}

// Assignment from integer types.

inline uint128& uint128::operator=(int v) { return *this = uint128(v); }

inline uint128& uint128::operator=(unsigned int v) {
  return *this = uint128(v);
}

inline uint128& uint128::operator=(long v) {  // NOLINT(runtime/int)
  return *this = uint128(v);
}

// NOLINTNEXTLINE(runtime/int)
inline uint128& uint128::operator=(unsigned long v) {
  return *this = uint128(v);
}

// NOLINTNEXTLINE(runtime/int)
inline uint128& uint128::operator=(long long v) { return *this = uint128(v); }

// NOLINTNEXTLINE(runtime/int)
inline uint128& uint128::operator=(unsigned long long v) {
  return *this = uint128(v);
}

#ifdef ABSL_HAVE_INTRINSIC_INT128
inline uint128& uint128::operator=(__int128 v) { return *this = uint128(v); }

inline uint128& uint128::operator=(unsigned __int128 v) {
  return *this = uint128(v);
}
#endif  // ABSL_HAVE_INTRINSIC_INT128

inline uint128& uint128::operator=(int128 v) { return *this = uint128(v); }

// Arithmetic operators.

constexpr uint128 operator<<(uint128 lhs, int amount);
constexpr uint128 operator>>(uint128 lhs, int amount);
constexpr uint128 operator+(uint128 lhs, uint128 rhs);
constexpr uint128 operator-(uint128 lhs, uint128 rhs);
uint128 operator*(uint128 lhs, uint128 rhs);
uint128 operator/(uint128 lhs, uint128 rhs);
uint128 operator%(uint128 lhs, uint128 rhs);

inline uint128& uint128::operator<<=(int amount) {
  *this = *this << amount;
  return *this;
}

inline uint128& uint128::operator>>=(int amount) {
  *this = *this >> amount;
  return *this;
}

inline uint128& uint128::operator+=(uint128 other) {
  *this = *this + other;
  return *this;
}

inline uint128& uint128::operator-=(uint128 other) {
  *this = *this - other;
  return *this;
}

inline uint128& uint128::operator*=(uint128 other) {
  *this = *this * other;
  return *this;
}

inline uint128& uint128::operator/=(uint128 other) {
  *this = *this / other;
  return *this;
}

inline uint128& uint128::operator%=(uint128 other) {
  *this = *this % other;
  return *this;
}

constexpr uint64_t Uint128Low64(uint128 v) { return v.lo_; }

constexpr uint64_t Uint128High64(uint128 v) { return v.hi_; }

// Constructors from integer types.

#if defined(ABSL_IS_LITTLE_ENDIAN)

constexpr uint128::uint128(uint64_t high, uint64_t low) : lo_{low}, hi_{high} {}

constexpr uint128::uint128(int v)
    : lo_{static_cast<uint64_t>(v)},
      hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0} {}
constexpr uint128::uint128(long v)  // NOLINT(runtime/int)
    : lo_{static_cast<uint64_t>(v)},
      hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0} {}
constexpr uint128::uint128(long long v)  // NOLINT(runtime/int)
    : lo_{static_cast<uint64_t>(v)},
      hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0} {}

constexpr uint128::uint128(unsigned int v) : lo_{v}, hi_{0} {}
// NOLINTNEXTLINE(runtime/int)
constexpr uint128::uint128(unsigned long v) : lo_{v}, hi_{0} {}
// NOLINTNEXTLINE(runtime/int)
constexpr uint128::uint128(unsigned long long v) : lo_{v}, hi_{0} {}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr uint128::uint128(__int128 v)
    : lo_{static_cast<uint64_t>(v & ~uint64_t{0})},
      hi_{static_cast<uint64_t>(static_cast<unsigned __int128>(v) >> 64)} {}
constexpr uint128::uint128(unsigned __int128 v)
    : lo_{static_cast<uint64_t>(v & ~uint64_t{0})},
      hi_{static_cast<uint64_t>(v >> 64)} {}
#endif  // ABSL_HAVE_INTRINSIC_INT128

constexpr uint128::uint128(int128 v)
    : lo_{Int128Low64(v)}, hi_{static_cast<uint64_t>(Int128High64(v))} {}

#elif defined(ABSL_IS_BIG_ENDIAN)

constexpr uint128::uint128(uint64_t high, uint64_t low) : hi_{high}, lo_{low} {}

constexpr uint128::uint128(int v)
    : hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0},
      lo_{static_cast<uint64_t>(v)} {}
constexpr uint128::uint128(long v)  // NOLINT(runtime/int)
    : hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0},
      lo_{static_cast<uint64_t>(v)} {}
constexpr uint128::uint128(long long v)  // NOLINT(runtime/int)
    : hi_{v < 0 ? (std::numeric_limits<uint64_t>::max)() : 0},
      lo_{static_cast<uint64_t>(v)} {}

constexpr uint128::uint128(unsigned int v) : hi_{0}, lo_{v} {}
// NOLINTNEXTLINE(runtime/int)
constexpr uint128::uint128(unsigned long v) : hi_{0}, lo_{v} {}
// NOLINTNEXTLINE(runtime/int)
constexpr uint128::uint128(unsigned long long v) : hi_{0}, lo_{v} {}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr uint128::uint128(__int128 v)
    : hi_{static_cast<uint64_t>(static_cast<unsigned __int128>(v) >> 64)},
      lo_{static_cast<uint64_t>(v & ~uint64_t{0})} {}
constexpr uint128::uint128(unsigned __int128 v)
    : hi_{static_cast<uint64_t>(v >> 64)},
      lo_{static_cast<uint64_t>(v & ~uint64_t{0})} {}
#endif  // ABSL_HAVE_INTRINSIC_INT128

constexpr uint128::uint128(int128 v)
    : hi_{static_cast<uint64_t>(Int128High64(v))}, lo_{Int128Low64(v)} {}

#else  // byte order
#error "Unsupported byte order: must be little-endian or big-endian."
#endif  // byte order

// Conversion operators to integer types.

constexpr uint128::operator bool() const { return lo_ || hi_; }

constexpr uint128::operator char() const { return static_cast<char>(lo_); }

constexpr uint128::operator signed char() const {
  return static_cast<signed char>(lo_);
}

constexpr uint128::operator unsigned char() const {
  return static_cast<unsigned char>(lo_);
}

constexpr uint128::operator char16_t() const {
  return static_cast<char16_t>(lo_);
}

constexpr uint128::operator char32_t() const {
  return static_cast<char32_t>(lo_);
}

constexpr uint128::operator ABSL_INTERNAL_WCHAR_T() const {
  return static_cast<ABSL_INTERNAL_WCHAR_T>(lo_);
}

// NOLINTNEXTLINE(runtime/int)
constexpr uint128::operator short() const { return static_cast<short>(lo_); }

constexpr uint128::operator unsigned short() const {  // NOLINT(runtime/int)
  return static_cast<unsigned short>(lo_);            // NOLINT(runtime/int)
}

constexpr uint128::operator int() const { return static_cast<int>(lo_); }

constexpr uint128::operator unsigned int() const {
  return static_cast<unsigned int>(lo_);
}

// NOLINTNEXTLINE(runtime/int)
constexpr uint128::operator long() const { return static_cast<long>(lo_); }

constexpr uint128::operator unsigned long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long>(lo_);            // NOLINT(runtime/int)
}

constexpr uint128::operator long long() const {  // NOLINT(runtime/int)
  return static_cast<long long>(lo_);            // NOLINT(runtime/int)
}

constexpr uint128::operator unsigned long long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long long>(lo_);            // NOLINT(runtime/int)
}

#ifdef ABSL_HAVE_INTRINSIC_INT128
constexpr uint128::operator __int128() const {
  return (static_cast<__int128>(hi_) << 64) + lo_;
}

constexpr uint128::operator unsigned __int128() const {
  return (static_cast<unsigned __int128>(hi_) << 64) + lo_;
}
#endif  // ABSL_HAVE_INTRINSIC_INT128

// Conversion operators to floating point types.

inline uint128::operator float() const {
  // Note: This method might return Inf.
  constexpr float pow_2_64 = 18446744073709551616.0f;
  return static_cast<float>(lo_) + static_cast<float>(hi_) * pow_2_64;
}

inline uint128::operator double() const {
  constexpr double pow_2_64 = 18446744073709551616.0;
  return static_cast<double>(lo_) + static_cast<double>(hi_) * pow_2_64;
}

inline uint128::operator long double() const {
  constexpr long double pow_2_64 = 18446744073709551616.0L;
  return static_cast<long double>(lo_) +
         static_cast<long double>(hi_) * pow_2_64;
}

// Comparison operators.

constexpr bool operator==(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) ==
         static_cast<unsigned __int128>(rhs);
#else
  return (Uint128Low64(lhs) == Uint128Low64(rhs) &&
          Uint128High64(lhs) == Uint128High64(rhs));
#endif
}

constexpr bool operator!=(uint128 lhs, uint128 rhs) { return !(lhs == rhs); }

constexpr bool operator<(uint128 lhs, uint128 rhs) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
  return static_cast<unsigned __int128>(lhs) <
         static_cast<unsigned __int128>(rhs);
#else
  return (Uint128High64(lhs) == Uint128High64(rhs))
             ? (Uint128Low64(lhs) < Uint128Low64(rhs))
             : (Uint128High64(lhs) < Uint128High64(rhs));
#endif
}

constexpr bool operator>(uint128 lhs, uint128 rhs) { return rhs < lhs; }

constexpr bool operator<=(uint128 lhs, uint128 rhs) { return !(rhs < lhs); }

constexpr bool operator>=(uint128 lhs, uint128 rhs) { return !(lhs < rhs); }

#ifdef __cpp_impl_three_way_comparison
constexpr std::strong_ordering operator<=>(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  if (auto lhs_128 = static_cast<unsigned __int128>(lhs),
      rhs_128 = static_cast<unsigned __int128>(rhs);
      lhs_128 < rhs_128) {
    return std::strong_ordering::less;
  } else if (lhs_128 > rhs_128) {
    return std::strong_ordering::greater;
  } else {
    return std::strong_ordering::equal;
  }
#else
  if (uint64_t lhs_high = Uint128High64(lhs), rhs_high = Uint128High64(rhs);
      lhs_high < rhs_high) {
    return std::strong_ordering::less;
  } else if (lhs_high > rhs_high) {
    return std::strong_ordering::greater;
  } else if (uint64_t lhs_low = Uint128Low64(lhs), rhs_low = Uint128Low64(rhs);
             lhs_low < rhs_low) {
    return std::strong_ordering::less;
  } else if (lhs_low > rhs_low) {
    return std::strong_ordering::greater;
  } else {
    return std::strong_ordering::equal;
  }
#endif
}
#endif

// Unary operators.

constexpr inline uint128 operator+(uint128 val) { return val; }

constexpr inline int128 operator+(int128 val) { return val; }

constexpr uint128 operator-(uint128 val) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return -static_cast<unsigned __int128>(val);
#else
  return MakeUint128(
      ~Uint128High64(val) + static_cast<unsigned long>(Uint128Low64(val) == 0),
      ~Uint128Low64(val) + 1);
#endif
}

constexpr inline bool operator!(uint128 val) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return !static_cast<unsigned __int128>(val);
#else
  return !Uint128High64(val) && !Uint128Low64(val);
#endif
}

// Logical operators.

constexpr inline uint128 operator~(uint128 val) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return ~static_cast<unsigned __int128>(val);
#else
  return MakeUint128(~Uint128High64(val), ~Uint128Low64(val));
#endif
}

constexpr inline uint128 operator|(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) |
         static_cast<unsigned __int128>(rhs);
#else
  return MakeUint128(Uint128High64(lhs) | Uint128High64(rhs),
                     Uint128Low64(lhs) | Uint128Low64(rhs));
#endif
}

constexpr inline uint128 operator&(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) &
         static_cast<unsigned __int128>(rhs);
#else
  return MakeUint128(Uint128High64(lhs) & Uint128High64(rhs),
                     Uint128Low64(lhs) & Uint128Low64(rhs));
#endif
}

constexpr inline uint128 operator^(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) ^
         static_cast<unsigned __int128>(rhs);
#else
  return MakeUint128(Uint128High64(lhs) ^ Uint128High64(rhs),
                     Uint128Low64(lhs) ^ Uint128Low64(rhs));
#endif
}

inline uint128& uint128::operator|=(uint128 other) {
  *this = *this | other;
  return *this;
}

inline uint128& uint128::operator&=(uint128 other) {
  *this = *this & other;
  return *this;
}

inline uint128& uint128::operator^=(uint128 other) {
  *this = *this ^ other;
  return *this;
}

// Arithmetic operators.

constexpr uint128 operator<<(uint128 lhs, int amount) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
  return static_cast<unsigned __int128>(lhs) << amount;
#else
  // uint64_t shifts of >= 64 are undefined, so we will need some
  // special-casing.
  return amount >= 64  ? MakeUint128(Uint128Low64(lhs) << (amount - 64), 0)
         : amount == 0 ? lhs
                       : MakeUint128((Uint128High64(lhs) << amount) |
                                         (Uint128Low64(lhs) >> (64 - amount)),
                                     Uint128Low64(lhs) << amount);
#endif
}

constexpr uint128 operator>>(uint128 lhs, int amount) {
#ifdef ABSL_HAVE_INTRINSIC_INT128
  return static_cast<unsigned __int128>(lhs) >> amount;
#else
  // uint64_t shifts of >= 64 are undefined, so we will need some
  // special-casing.
  return amount >= 64  ? MakeUint128(0, Uint128High64(lhs) >> (amount - 64))
         : amount == 0 ? lhs
                       : MakeUint128(Uint128High64(lhs) >> amount,
                                     (Uint128Low64(lhs) >> amount) |
                                         (Uint128High64(lhs) << (64 - amount)));
#endif
}

#if !defined(ABSL_HAVE_INTRINSIC_INT128)
namespace int128_internal {
constexpr uint128 AddResult(uint128 result, uint128 lhs) {
  // check for carry
  return (Uint128Low64(result) < Uint128Low64(lhs))
             ? MakeUint128(Uint128High64(result) + 1, Uint128Low64(result))
             : result;
}
}  // namespace int128_internal
#endif

constexpr uint128 operator+(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) +
         static_cast<unsigned __int128>(rhs);
#else
  return int128_internal::AddResult(
      MakeUint128(Uint128High64(lhs) + Uint128High64(rhs),
                  Uint128Low64(lhs) + Uint128Low64(rhs)),
      lhs);
#endif
}

#if !defined(ABSL_HAVE_INTRINSIC_INT128)
namespace int128_internal {
constexpr uint128 SubstructResult(uint128 result, uint128 lhs, uint128 rhs) {
  // check for carry
  return (Uint128Low64(lhs) < Uint128Low64(rhs))
             ? MakeUint128(Uint128High64(result) - 1, Uint128Low64(result))
             : result;
}
}  // namespace int128_internal
#endif

constexpr uint128 operator-(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  return static_cast<unsigned __int128>(lhs) -
         static_cast<unsigned __int128>(rhs);
#else
  return int128_internal::SubstructResult(
      MakeUint128(Uint128High64(lhs) - Uint128High64(rhs),
                  Uint128Low64(lhs) - Uint128Low64(rhs)),
      lhs, rhs);
#endif
}

inline uint128 operator*(uint128 lhs, uint128 rhs) {
#if defined(ABSL_HAVE_INTRINSIC_INT128)
  // TODO(strel) Remove once alignment issues are resolved and unsigned __int128
  // can be used for uint128 storage.
  return static_cast<unsigned __int128>(lhs) *
         static_cast<unsigned __int128>(rhs);
#elif defined(_MSC_VER) && defined(_M_X64) && !defined(_M_ARM64EC)
  uint64_t carry;
  uint64_t low = _umul128(Uint128Low64(lhs), Uint128Low64(rhs), &carry);
  return MakeUint128(Uint128Low64(lhs) * Uint128High64(rhs) +
                         Uint128High64(lhs) * Uint128Low64(rhs) + carry,
                     low);
#else   // ABSL_HAVE_INTRINSIC128
  uint64_t a32 = Uint128Low64(lhs) >> 32;
  uint64_t a00 = Uint128Low64(lhs) & 0xffffffff;
  uint64_t b32 = Uint128Low64(rhs) >> 32;
  uint64_t b00 = Uint128Low64(rhs) & 0xffffffff;
  uint128 result =
      MakeUint128(Uint128High64(lhs) * Uint128Low64(rhs) +
                      Uint128Low64(lhs) * Uint128High64(rhs) + a32 * b32,
                  a00 * b00);
  result += uint128(a32 * b00) << 32;
  result += uint128(a00 * b32) << 32;
  return result;
#endif  // ABSL_HAVE_INTRINSIC128
}

#if defined(ABSL_HAVE_INTRINSIC_INT128)
inline uint128 operator/(uint128 lhs, uint128 rhs) {
  return static_cast<unsigned __int128>(lhs) /
         static_cast<unsigned __int128>(rhs);
}

inline uint128 operator%(uint128 lhs, uint128 rhs) {
  return static_cast<unsigned __int128>(lhs) %
         static_cast<unsigned __int128>(rhs);
}
#endif

// Increment/decrement operators.

inline uint128 uint128::operator++(int) {
  uint128 tmp(*this);
  *this += 1;
  return tmp;
}

inline uint128 uint128::operator--(int) {
  uint128 tmp(*this);
  *this -= 1;
  return tmp;
}

inline uint128& uint128::operator++() {
  *this += 1;
  return *this;
}

inline uint128& uint128::operator--() {
  *this -= 1;
  return *this;
}

constexpr int128 MakeInt128(int64_t high, uint64_t low) {
  return int128(high, low);
}

// Assignment from integer types.
inline int128& int128::operator=(int v) { return *this = int128(v); }

inline int128& int128::operator=(unsigned int v) { return *this = int128(v); }

inline int128& int128::operator=(long v) {  // NOLINT(runtime/int)
  return *this = int128(v);
}

// NOLINTNEXTLINE(runtime/int)
inline int128& int128::operator=(unsigned long v) { return *this = int128(v); }

// NOLINTNEXTLINE(runtime/int)
inline int128& int128::operator=(long long v) { return *this = int128(v); }

// NOLINTNEXTLINE(runtime/int)
inline int128& int128::operator=(unsigned long long v) {
  return *this = int128(v);
}

// Arithmetic operators.
constexpr int128 operator-(int128 v);
constexpr int128 operator+(int128 lhs, int128 rhs);
constexpr int128 operator-(int128 lhs, int128 rhs);
int128 operator*(int128 lhs, int128 rhs);
int128 operator/(int128 lhs, int128 rhs);
int128 operator%(int128 lhs, int128 rhs);
constexpr int128 operator|(int128 lhs, int128 rhs);
constexpr int128 operator&(int128 lhs, int128 rhs);
constexpr int128 operator^(int128 lhs, int128 rhs);
constexpr int128 operator<<(int128 lhs, int amount);
constexpr int128 operator>>(int128 lhs, int amount);

inline int128& int128::operator+=(int128 other) {
  *this = *this + other;
  return *this;
}

inline int128& int128::operator-=(int128 other) {
  *this = *this - other;
  return *this;
}

inline int128& int128::operator*=(int128 other) {
  *this = *this * other;
  return *this;
}

inline int128& int128::operator/=(int128 other) {
  *this = *this / other;
  return *this;
}

inline int128& int128::operator%=(int128 other) {
  *this = *this % other;
  return *this;
}

inline int128& int128::operator|=(int128 other) {
  *this = *this | other;
  return *this;
}

inline int128& int128::operator&=(int128 other) {
  *this = *this & other;
  return *this;
}

inline int128& int128::operator^=(int128 other) {
  *this = *this ^ other;
  return *this;
}

inline int128& int128::operator<<=(int amount) {
  *this = *this << amount;
  return *this;
}

inline int128& int128::operator>>=(int amount) {
  *this = *this >> amount;
  return *this;
}

// Forward declaration for comparison operators.
constexpr bool operator!=(int128 lhs, int128 rhs);

namespace int128_internal {

// Casts from unsigned to signed while preserving the underlying binary
// representation.
constexpr int64_t BitCastToSigned(uint64_t v) {
  // Casting an unsigned integer to a signed integer of the same
  // width is implementation defined behavior if the source value would not fit
  // in the destination type. We step around it with a roundtrip bitwise not
  // operation to make sure this function remains constexpr. Clang, GCC, and
  // MSVC optimize this to a no-op on x86-64.
  return v & (uint64_t{1} << 63) ? ~static_cast<int64_t>(~v)
                                 : static_cast<int64_t>(v);
}

}  // namespace int128_internal

#if defined(ABSL_HAVE_INTRINSIC_INT128)

// This file contains :int128 implementation details that depend on internal
// representation when ABSL_HAVE_INTRINSIC_INT128 is defined. This file relies
// on ABSL_INTERNAL_WCHAR_T being defined.

namespace int128_internal {

// Casts from unsigned to signed while preserving the underlying binary
// representation.
constexpr __int128 BitCastToSigned(unsigned __int128 v) {
  // Casting an unsigned integer to a signed integer of the same
  // width is implementation defined behavior if the source value would not fit
  // in the destination type. We step around it with a roundtrip bitwise not
  // operation to make sure this function remains constexpr. Clang and GCC
  // optimize this to a no-op on x86-64.
  return v & (static_cast<unsigned __int128>(1) << 127)
             ? ~static_cast<__int128>(~v)
             : static_cast<__int128>(v);
}

}  // namespace int128_internal

inline int128& int128::operator=(__int128 v) {
  v_ = v;
  return *this;
}

constexpr uint64_t Int128Low64(int128 v) {
  return static_cast<uint64_t>(v.v_ & ~uint64_t{0});
}

constexpr int64_t Int128High64(int128 v) {
  // Initially cast to unsigned to prevent a right shift on a negative value.
  return int128_internal::BitCastToSigned(
      static_cast<uint64_t>(static_cast<unsigned __int128>(v.v_) >> 64));
}

constexpr int128::int128(int64_t high, uint64_t low)
    // Initially cast to unsigned to prevent a left shift that overflows.
    : v_(int128_internal::BitCastToSigned(static_cast<unsigned __int128>(high)
                                           << 64) |
         low) {}


constexpr int128::int128(int v) : v_{v} {}

constexpr int128::int128(long v) : v_{v} {}       // NOLINT(runtime/int)

constexpr int128::int128(long long v) : v_{v} {}  // NOLINT(runtime/int)

constexpr int128::int128(__int128 v) : v_{v} {}

constexpr int128::int128(unsigned int v) : v_{v} {}

constexpr int128::int128(unsigned long v) : v_{v} {}  // NOLINT(runtime/int)

// NOLINTNEXTLINE(runtime/int)
constexpr int128::int128(unsigned long long v) : v_{v} {}

constexpr int128::int128(unsigned __int128 v) : v_{static_cast<__int128>(v)} {}

inline int128::int128(float v) {
  v_ = static_cast<__int128>(v);
}

inline int128::int128(double v) {
  v_ = static_cast<__int128>(v);
}

inline int128::int128(long double v) {
  v_ = static_cast<__int128>(v);
}

constexpr int128::int128(uint128 v) : v_{static_cast<__int128>(v)} {}

constexpr int128::operator bool() const { return static_cast<bool>(v_); }

constexpr int128::operator char() const { return static_cast<char>(v_); }

constexpr int128::operator signed char() const {
  return static_cast<signed char>(v_);
}

constexpr int128::operator unsigned char() const {
  return static_cast<unsigned char>(v_);
}

constexpr int128::operator char16_t() const {
  return static_cast<char16_t>(v_);
}

constexpr int128::operator char32_t() const {
  return static_cast<char32_t>(v_);
}

constexpr int128::operator ABSL_INTERNAL_WCHAR_T() const {
  return static_cast<ABSL_INTERNAL_WCHAR_T>(v_);
}

constexpr int128::operator short() const {  // NOLINT(runtime/int)
  return static_cast<short>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator unsigned short() const {  // NOLINT(runtime/int)
  return static_cast<unsigned short>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator int() const {
  return static_cast<int>(v_);
}

constexpr int128::operator unsigned int() const {
  return static_cast<unsigned int>(v_);
}

constexpr int128::operator long() const {  // NOLINT(runtime/int)
  return static_cast<long>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator unsigned long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator long long() const {  // NOLINT(runtime/int)
  return static_cast<long long>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator unsigned long long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long long>(v_);            // NOLINT(runtime/int)
}

constexpr int128::operator __int128() const { return v_; }

constexpr int128::operator unsigned __int128() const {
  return static_cast<unsigned __int128>(v_);
}

// Clang on PowerPC sometimes produces incorrect __int128 to floating point
// conversions. In that case, we do the conversion with a similar implementation
// to the conversion operators in int128_no_intrinsic.inc.
#if defined(__clang__) && !defined(__ppc64__)
inline int128::operator float() const { return static_cast<float>(v_); }

inline int128::operator double() const { return static_cast<double>(v_); }

inline int128::operator long double() const {
  return static_cast<long double>(v_);
}

#else  // Clang on PowerPC

inline int128::operator float() const {
  // We must convert the absolute value and then negate as needed, because
  // floating point types are typically sign-magnitude. Otherwise, the
  // difference between the high and low 64 bits when interpreted as two's
  // complement overwhelms the precision of the mantissa.
  //
  // Also check to make sure we don't negate Int128Min()
  constexpr float pow_2_64 = 18446744073709551616.0f;
  return v_ < 0 && *this != Int128Min()
             ? -static_cast<float>(-*this)
             : static_cast<float>(Int128Low64(*this)) +
                   static_cast<float>(Int128High64(*this)) * pow_2_64;
}

inline int128::operator double() const {
  // See comment in int128::operator float() above.
  constexpr double pow_2_64 = 18446744073709551616.0;
  return v_ < 0 && *this != Int128Min()
             ? -static_cast<double>(-*this)
             : static_cast<double>(Int128Low64(*this)) +
                   static_cast<double>(Int128High64(*this)) * pow_2_64;
}

inline int128::operator long double() const {
  // See comment in int128::operator float() above.
  constexpr long double pow_2_64 = 18446744073709551616.0L;
  return v_ < 0 && *this != Int128Min()
             ? -static_cast<long double>(-*this)
             : static_cast<long double>(Int128Low64(*this)) +
                   static_cast<long double>(Int128High64(*this)) * pow_2_64;
}
#endif  // Clang on PowerPC

// Comparison operators.

constexpr bool operator==(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) == static_cast<__int128>(rhs);
}

constexpr bool operator!=(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) != static_cast<__int128>(rhs);
}

constexpr bool operator<(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) < static_cast<__int128>(rhs);
}

constexpr bool operator>(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) > static_cast<__int128>(rhs);
}

constexpr bool operator<=(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) <= static_cast<__int128>(rhs);
}

constexpr bool operator>=(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) >= static_cast<__int128>(rhs);
}

#ifdef __cpp_impl_three_way_comparison
constexpr std::strong_ordering operator<=>(int128 lhs, int128 rhs) {
  if (auto lhs_128 = static_cast<__int128>(lhs),
      rhs_128 = static_cast<__int128>(rhs);
      lhs_128 < rhs_128) {
    return std::strong_ordering::less;
  } else if (lhs_128 > rhs_128) {
    return std::strong_ordering::greater;
  } else {
    return std::strong_ordering::equal;
  }
}
#endif

// Unary operators.

constexpr int128 operator-(int128 v) { return -static_cast<__int128>(v); }

constexpr bool operator!(int128 v) { return !static_cast<__int128>(v); }

constexpr int128 operator~(int128 val) { return ~static_cast<__int128>(val); }

// Arithmetic operators.

constexpr int128 operator+(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) + static_cast<__int128>(rhs);
}

constexpr int128 operator-(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) - static_cast<__int128>(rhs);
}

inline int128 operator*(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) * static_cast<__int128>(rhs);
}

inline int128 operator/(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) / static_cast<__int128>(rhs);
}

inline int128 operator%(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) % static_cast<__int128>(rhs);
}

inline int128 int128::operator++(int) {
  int128 tmp(*this);
  ++v_;
  return tmp;
}

inline int128 int128::operator--(int) {
  int128 tmp(*this);
  --v_;
  return tmp;
}

inline int128& int128::operator++() {
  ++v_;
  return *this;
}

inline int128& int128::operator--() {
  --v_;
  return *this;
}

constexpr int128 operator|(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) | static_cast<__int128>(rhs);
}

constexpr int128 operator&(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) & static_cast<__int128>(rhs);
}

constexpr int128 operator^(int128 lhs, int128 rhs) {
  return static_cast<__int128>(lhs) ^ static_cast<__int128>(rhs);
}

constexpr int128 operator<<(int128 lhs, int amount) {
  return static_cast<__int128>(lhs) << amount;
}

constexpr int128 operator>>(int128 lhs, int amount) {
  return static_cast<__int128>(lhs) >> amount;
}

#else  // ABSL_HAVE_INTRINSIC_INT128
// This file contains :int128 implementation details that depend on internal
// representation when ABSL_HAVE_INTRINSIC_INT128 is *not* defined. This file
// is included by int128.h and relies on ABSL_INTERNAL_WCHAR_T being defined.

constexpr uint64_t Int128Low64(int128 v) { return v.lo_; }

constexpr int64_t Int128High64(int128 v) { return v.hi_; }

#if defined(ABSL_IS_LITTLE_ENDIAN)

constexpr int128::int128(int64_t high, uint64_t low) : lo_(low), hi_(high) {}

constexpr int128::int128(int v)
    : lo_{static_cast<uint64_t>(v)}, hi_{v < 0 ? ~int64_t{0} : 0} {}
constexpr int128::int128(long v)  // NOLINT(runtime/int)
    : lo_{static_cast<uint64_t>(v)}, hi_{v < 0 ? ~int64_t{0} : 0} {}
constexpr int128::int128(long long v)  // NOLINT(runtime/int)
    : lo_{static_cast<uint64_t>(v)}, hi_{v < 0 ? ~int64_t{0} : 0} {}

constexpr int128::int128(unsigned int v) : lo_{v}, hi_{0} {}
// NOLINTNEXTLINE(runtime/int)
constexpr int128::int128(unsigned long v) : lo_{v}, hi_{0} {}
// NOLINTNEXTLINE(runtime/int)
constexpr int128::int128(unsigned long long v) : lo_{v}, hi_{0} {}

constexpr int128::int128(uint128 v)
    : lo_{Uint128Low64(v)}, hi_{static_cast<int64_t>(Uint128High64(v))} {}

#elif defined(ABSL_IS_BIG_ENDIAN)

constexpr int128::int128(int64_t high, uint64_t low) : hi_{high}, lo_{low} {}

constexpr int128::int128(int v)
    : hi_{v < 0 ? ~int64_t{0} : 0}, lo_{static_cast<uint64_t>(v)} {}
constexpr int128::int128(long v)  // NOLINT(runtime/int)
    : hi_{v < 0 ? ~int64_t{0} : 0}, lo_{static_cast<uint64_t>(v)} {}
constexpr int128::int128(long long v)  // NOLINT(runtime/int)
    : hi_{v < 0 ? ~int64_t{0} : 0}, lo_{static_cast<uint64_t>(v)} {}

constexpr int128::int128(unsigned int v) : hi_{0}, lo_{v} {}
// NOLINTNEXTLINE(runtime/int)
constexpr int128::int128(unsigned long v) : hi_{0}, lo_{v} {}
// NOLINTNEXTLINE(runtime/int)
constexpr int128::int128(unsigned long long v) : hi_{0}, lo_{v} {}

constexpr int128::int128(uint128 v)
    : hi_{static_cast<int64_t>(Uint128High64(v))}, lo_{Uint128Low64(v)} {}

#else  // byte order
#error "Unsupported byte order: must be little-endian or big-endian."
#endif  // byte order

constexpr int128::operator bool() const { return lo_ || hi_; }

constexpr int128::operator char() const {
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<char>(static_cast<long long>(*this));
}

constexpr int128::operator signed char() const {
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<signed char>(static_cast<long long>(*this));
}

constexpr int128::operator unsigned char() const {
  return static_cast<unsigned char>(lo_);
}

constexpr int128::operator char16_t() const {
  return static_cast<char16_t>(lo_);
}

constexpr int128::operator char32_t() const {
  return static_cast<char32_t>(lo_);
}

constexpr int128::operator ABSL_INTERNAL_WCHAR_T() const {
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<ABSL_INTERNAL_WCHAR_T>(static_cast<long long>(*this));
}

constexpr int128::operator short() const {  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<short>(static_cast<long long>(*this));
}

constexpr int128::operator unsigned short() const {  // NOLINT(runtime/int)
  return static_cast<unsigned short>(lo_);           // NOLINT(runtime/int)
}

constexpr int128::operator int() const {
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<int>(static_cast<long long>(*this));
}

constexpr int128::operator unsigned int() const {
  return static_cast<unsigned int>(lo_);
}

constexpr int128::operator long() const {  // NOLINT(runtime/int)
  // NOLINTNEXTLINE(runtime/int)
  return static_cast<long>(static_cast<long long>(*this));
}

constexpr int128::operator unsigned long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long>(lo_);           // NOLINT(runtime/int)
}

constexpr int128::operator long long() const {  // NOLINT(runtime/int)
  // We don't bother checking the value of hi_. If *this < 0, lo_'s high bit
  // must be set in order for the value to fit into a long long. Conversely, if
  // lo_'s high bit is set, *this must be < 0 for the value to fit.
  return int128_internal::BitCastToSigned(lo_);
}

constexpr int128::operator unsigned long long() const {  // NOLINT(runtime/int)
  return static_cast<unsigned long long>(lo_);           // NOLINT(runtime/int)
}

inline int128::operator float() const {
  // We must convert the absolute value and then negate as needed, because
  // floating point types are typically sign-magnitude. Otherwise, the
  // difference between the high and low 64 bits when interpreted as two's
  // complement overwhelms the precision of the mantissa.
  //
  // Also check to make sure we don't negate Int128Min()
  constexpr float pow_2_64 = 18446744073709551616.0f;
  return hi_ < 0 && *this != Int128Min()
             ? -static_cast<float>(-*this)
             : static_cast<float>(lo_) +
                   static_cast<float>(hi_) * pow_2_64;
}

inline int128::operator double() const {
  // See comment in int128::operator float() above.
  constexpr double pow_2_64 = 18446744073709551616.0;
  return hi_ < 0 && *this != Int128Min()
             ? -static_cast<double>(-*this)
             : static_cast<double>(lo_) +
                   static_cast<double>(hi_) * pow_2_64;
}

inline int128::operator long double() const {
  // See comment in int128::operator float() above.
  constexpr long double pow_2_64 = 18446744073709551616.0L;
  return hi_ < 0 && *this != Int128Min()
             ? -static_cast<long double>(-*this)
             : static_cast<long double>(lo_) +
                   static_cast<long double>(hi_) * pow_2_64;
}

// Comparison operators.

constexpr bool operator==(int128 lhs, int128 rhs) {
  return (Int128Low64(lhs) == Int128Low64(rhs) &&
          Int128High64(lhs) == Int128High64(rhs));
}

constexpr bool operator!=(int128 lhs, int128 rhs) { return !(lhs == rhs); }

constexpr bool operator<(int128 lhs, int128 rhs) {
  return (Int128High64(lhs) == Int128High64(rhs))
             ? (Int128Low64(lhs) < Int128Low64(rhs))
             : (Int128High64(lhs) < Int128High64(rhs));
}

constexpr bool operator>(int128 lhs, int128 rhs) {
  return (Int128High64(lhs) == Int128High64(rhs))
             ? (Int128Low64(lhs) > Int128Low64(rhs))
             : (Int128High64(lhs) > Int128High64(rhs));
}

constexpr bool operator<=(int128 lhs, int128 rhs) { return !(lhs > rhs); }

constexpr bool operator>=(int128 lhs, int128 rhs) { return !(lhs < rhs); }

#ifdef __cpp_impl_three_way_comparison
constexpr std::strong_ordering operator<=>(int128 lhs, int128 rhs) {
  if (int64_t lhs_high = Int128High64(lhs), rhs_high = Int128High64(rhs);
      lhs_high < rhs_high) {
    return std::strong_ordering::less;
  } else if (lhs_high > rhs_high) {
    return std::strong_ordering::greater;
  } else if (uint64_t lhs_low = Uint128Low64(lhs), rhs_low = Uint128Low64(rhs);
             lhs_low < rhs_low) {
    return std::strong_ordering::less;
  } else if (lhs_low > rhs_low) {
    return std::strong_ordering::greater;
  } else {
    return std::strong_ordering::equal;
  }
}
#endif

// Unary operators.

constexpr int128 operator-(int128 v) {
  return MakeInt128(~Int128High64(v) + (Int128Low64(v) == 0),
                    ~Int128Low64(v) + 1);
}

constexpr bool operator!(int128 v) {
  return !Int128Low64(v) && !Int128High64(v);
}

constexpr int128 operator~(int128 val) {
  return MakeInt128(~Int128High64(val), ~Int128Low64(val));
}

// Arithmetic operators.

namespace int128_internal {
constexpr int128 SignedAddResult(int128 result, int128 lhs) {
  // check for carry
  return (Int128Low64(result) < Int128Low64(lhs))
             ? MakeInt128(Int128High64(result) + 1, Int128Low64(result))
             : result;
}
}  // namespace int128_internal
constexpr int128 operator+(int128 lhs, int128 rhs) {
  return int128_internal::SignedAddResult(
      MakeInt128(Int128High64(lhs) + Int128High64(rhs),
                 Int128Low64(lhs) + Int128Low64(rhs)),
      lhs);
}

namespace int128_internal {
constexpr int128 SignedSubstructResult(int128 result, int128 lhs, int128 rhs) {
  // check for carry
  return (Int128Low64(lhs) < Int128Low64(rhs))
             ? MakeInt128(Int128High64(result) - 1, Int128Low64(result))
             : result;
}
}  // namespace int128_internal
constexpr int128 operator-(int128 lhs, int128 rhs) {
  return int128_internal::SignedSubstructResult(
      MakeInt128(Int128High64(lhs) - Int128High64(rhs),
                 Int128Low64(lhs) - Int128Low64(rhs)),
      lhs, rhs);
}

inline int128 operator*(int128 lhs, int128 rhs) {
  return MakeInt128(
      int128_internal::BitCastToSigned(Uint128High64(uint128(lhs) * rhs)),
      Uint128Low64(uint128(lhs) * rhs));
}

inline int128 int128::operator++(int) {
  int128 tmp(*this);
  *this += 1;
  return tmp;
}

inline int128 int128::operator--(int) {
  int128 tmp(*this);
  *this -= 1;
  return tmp;
}

inline int128& int128::operator++() {
  *this += 1;
  return *this;
}

inline int128& int128::operator--() {
  *this -= 1;
  return *this;
}

constexpr int128 operator|(int128 lhs, int128 rhs) {
  return MakeInt128(Int128High64(lhs) | Int128High64(rhs),
                    Int128Low64(lhs) | Int128Low64(rhs));
}

constexpr int128 operator&(int128 lhs, int128 rhs) {
  return MakeInt128(Int128High64(lhs) & Int128High64(rhs),
                    Int128Low64(lhs) & Int128Low64(rhs));
}

constexpr int128 operator^(int128 lhs, int128 rhs) {
  return MakeInt128(Int128High64(lhs) ^ Int128High64(rhs),
                    Int128Low64(lhs) ^ Int128Low64(rhs));
}

constexpr int128 operator<<(int128 lhs, int amount) {
  // int64_t shifts of >= 63 are undefined, so we need some special-casing.
  assert(amount >= 0 && amount < 127);
  if (amount <= 0) {
    return lhs;
  } else if (amount < 63) {
    return MakeInt128(
        (Int128High64(lhs) << amount) |
            static_cast<int64_t>(Int128Low64(lhs) >> (64 - amount)),
        Int128Low64(lhs) << amount);
  } else if (amount == 63) {
    return MakeInt128(((Int128High64(lhs) << 32) << 31) |
                          static_cast<int64_t>(Int128Low64(lhs) >> 1),
                      (Int128Low64(lhs) << 32) << 31);
  } else if (amount == 127) {
    return MakeInt128(static_cast<int64_t>(Int128Low64(lhs) << 63), 0);
  } else if (amount > 127) {
    return MakeInt128(0, 0);
  } else {
    // amount >= 64 && amount < 127
    return MakeInt128(static_cast<int64_t>(Int128Low64(lhs) << (amount - 64)),
                      0);
  }
}

constexpr int128 operator>>(int128 lhs, int amount) {
  // int64_t shifts of >= 63 are undefined, so we need some special-casing.
  assert(amount >= 0 && amount < 127);
  if (amount <= 0) {
    return lhs;
  } else if (amount < 63) {
    return MakeInt128(
        Int128High64(lhs) >> amount,
        Int128Low64(lhs) >> amount | static_cast<uint64_t>(Int128High64(lhs))
                                         << (64 - amount));
  } else if (amount == 63) {
    return MakeInt128((Int128High64(lhs) >> 32) >> 31,
                      static_cast<uint64_t>(Int128High64(lhs) << 1) |
                          (Int128Low64(lhs) >> 32) >> 31);

  } else if (amount >= 127) {
    return MakeInt128((Int128High64(lhs) >> 32) >> 31,
                      static_cast<uint64_t>((Int128High64(lhs) >> 32) >> 31));
  } else {
    // amount >= 64 && amount < 127
    return MakeInt128(
        (Int128High64(lhs) >> 32) >> 31,
        static_cast<uint64_t>(Int128High64(lhs) >> (amount - 64)));
  }
}

#endif  // ABSL_HAVE_INTRINSIC_INT128

}  // namespace absl

#undef ABSL_INTERNAL_WCHAR_T
