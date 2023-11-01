/* cxxlib.h -- C++ support library

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   UPX and the UCL library are free software; you can redistribute them
   and/or modify them under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */

#pragma once

// #include <stddef.h>
// #include <type_traits>

namespace upx {

/*************************************************************************
// type_traits
**************************************************************************/

// is_bounded_array: identical to C++20 std::is_bounded_array
template <class T>
struct is_bounded_array : public std::false_type {};
template <class T, std::size_t N>
struct is_bounded_array<T[N]> : public std::true_type {};
template <class T>
inline constexpr bool is_bounded_array_v = is_bounded_array<T>::value;

// is_same_all and is_same_any: std::is_same for multiple types
template <class T, class... Ts>
struct is_same_all : public std::conjunction<std::is_same<T, Ts>...> {};
template <class T, class... Ts>
inline constexpr bool is_same_all_v = is_same_all<T, Ts...>::value;
template <class T, class... Ts>
struct is_same_any : public std::disjunction<std::is_same<T, Ts>...> {};
template <class T, class... Ts>
inline constexpr bool is_same_any_v = is_same_any<T, Ts...>::value;

/*************************************************************************
// util
**************************************************************************/

template <std::size_t Size>
struct UnsignedSizeOf {
    static_assert(Size >= 1 && Size <= UPX_RSIZE_MAX_MEM);
    static constexpr unsigned value = unsigned(Size);
};

// a reinterpret_cast that does not trigger -Wcast-align warnings
template <class Result, class From>
forceinline Result ptr_reinterpret_cast(From *ptr) noexcept {
    static_assert(std::is_pointer_v<Result>);
    static_assert(!std::is_const_v<std::remove_pointer_t<Result> >); // enforce same constness
    return reinterpret_cast<Result>(reinterpret_cast<void *>(ptr));
}
template <class Result, class From>
forceinline Result ptr_reinterpret_cast(const From *ptr) noexcept {
    static_assert(std::is_pointer_v<Result>);
    static_assert(std::is_const_v<std::remove_pointer_t<Result> >); // required
    return reinterpret_cast<Result>(reinterpret_cast<const void *>(ptr));
}

class noncopyable {
protected:
    forceinline constexpr noncopyable() noexcept {}
#if __cplusplus >= 202002L
    forceinline constexpr ~noncopyable() noexcept = default;
#else
    forceinline ~noncopyable() noexcept = default;
#endif
private:
    noncopyable(const noncopyable &) noexcept DELETED_FUNCTION;            // copy constructor
    noncopyable &operator=(const noncopyable &) noexcept DELETED_FUNCTION; // copy assignment
    noncopyable(noncopyable &&) noexcept DELETED_FUNCTION;                 // move constructor
    noncopyable &operator=(noncopyable &&) noexcept DELETED_FUNCTION;      // move assignment
};

namespace compile_time {
constexpr std::size_t string_len(const char *a) { return *a == '\0' ? 0 : 1 + string_len(a + 1); }
constexpr bool string_eq(const char *a, const char *b) {
    return *a == *b && (*a == '\0' || string_eq(a + 1, b + 1));
}
constexpr bool string_lt(const char *a, const char *b) {
    return (uchar) *a < (uchar) *b || (*a != '\0' && *a == *b && string_lt(a + 1, b + 1));
}
constexpr bool string_ne(const char *a, const char *b) { return !string_eq(a, b); }
constexpr bool string_gt(const char *a, const char *b) { return string_lt(b, a); }
constexpr bool string_le(const char *a, const char *b) { return !string_lt(b, a); }
constexpr bool string_ge(const char *a, const char *b) { return !string_lt(a, b); }
} // namespace compile_time

/*************************************************************************
// TriBool - tri-state bool
// an enum with an underlying type and 3 values
// IsThirdTrue determines if Third is false or true
**************************************************************************/

template <class T = int, bool IsThirdTrue = false> // Third is false by default
struct TriBool final {
    static constexpr bool is_third_true = IsThirdTrue;
    // types
    typedef T underlying_type;
    static_assert(std::is_integral_v<underlying_type>);
    typedef decltype(T(0) + T(0)) promoted_type;
    static_assert(std::is_integral_v<promoted_type>);
    enum value_type : underlying_type { False = 0, True = 1, Third = 2 };
    static_assert(sizeof(value_type) == sizeof(underlying_type));
    static_assert(sizeof(underlying_type) <= sizeof(promoted_type));
    // constructors
    forceinline constexpr TriBool() noexcept {}
    forceinline constexpr TriBool(value_type x) noexcept : value(x) {}
    // permissive, converts all other values to Third!!
    constexpr TriBool(promoted_type x) noexcept : value(x == 0 ? False : (x == 1 ? True : Third)) {}
#if __cplusplus >= 202002L
    forceinline constexpr ~TriBool() noexcept = default;
#else
    forceinline ~TriBool() noexcept = default;
#endif
    // explicit conversion to bool
    explicit constexpr operator bool() const noexcept {
        return IsThirdTrue ? (value != False) : (value == True);
    }
    // query; this is NOT the same as operator bool()
    constexpr bool isStrictFalse() const noexcept { return value == False; }
    constexpr bool isStrictTrue() const noexcept { return value == True; }
    constexpr bool isStrictBool() const noexcept { return value == False || value == True; }
    constexpr bool isThird() const noexcept { return value != False && value != True; }
    // access
    constexpr value_type getValue() const noexcept { return value; }
    // equality
    constexpr bool operator==(TriBool other) const noexcept { return value == other.value; }
    constexpr bool operator==(value_type other) const noexcept { return value == other; }
    constexpr bool operator==(promoted_type other) const noexcept {
        return value == TriBool(other).value;
    }

    // "Third" can mean many things - depending on usage context, so provide some alternate names:
#if 0
    // constexpr bool isDefault() const noexcept { return isThird(); } // might be misleading
    constexpr bool isIndeterminate() const noexcept { return isThird(); }
    constexpr bool isNone() const noexcept { return isThird(); }
    constexpr bool isOther() const noexcept { return isThird(); }
    constexpr bool isUndecided() const noexcept { return isThird(); }
    // constexpr bool isUnset() const noexcept { return isThird(); } // might be misleading
#endif

private:
    value_type value = False; // the actual value of this type
};

typedef TriBool<> tribool;

/*************************************************************************
// OptVar and oassign
**************************************************************************/

template <class T, T default_value_, T min_value_, T max_value_>
struct OptVar final {
    static_assert(std::is_integral_v<T>);
    typedef T value_type;
    static constexpr T default_value = default_value_;
    static constexpr T min_value = min_value_;
    static constexpr T max_value = max_value_;
    static_assert(min_value <= default_value && default_value <= max_value);

    // automatic conversion
    constexpr operator T() const noexcept { return value; }

    static void assertValue(const T &value) noexcept {
        // info: this generates annoying warnings "unsigned >= 0 is always true"
        // assert_noexcept(value >= min_value);
        assert_noexcept(value == min_value || value >= min_value + 1);
        assert_noexcept(value <= max_value);
    }
    void assertValue() const noexcept { assertValue(value); }

    constexpr OptVar() noexcept {}
    OptVar &operator=(const T &other) noexcept {
        assertValue(other);
        value = other;
        is_set = true;
        return *this;
    }

    void reset() noexcept {
        value = default_value;
        is_set = false;
    }

    value_type value = default_value;
    bool is_set = false;
};

// optional assignments
template <class T, T a, T b, T c>
inline void oassign(OptVar<T, a, b, c> &self, const OptVar<T, a, b, c> &other) noexcept {
    if (other.is_set) {
        self.value = other.value;
        self.is_set = true;
    }
}
template <class T, T a, T b, T c>
inline void oassign(T &v, const OptVar<T, a, b, c> &other) noexcept {
    if (other.is_set)
        v = other.value;
}

/*************************************************************************
// OwningPointer(T)
// simple pointer type alias to explicitly mark ownership of objects; purely
// cosmetic to improve source code readability, no real functionality
**************************************************************************/

#if 0

// this works
#define OwningPointer(T) T *

#elif !(DEBUG)

// this also works
template <class T>
using OwningPointer = T *;
#define OwningPointer(T) upx::OwningPointer<T>

#else

// also works: a trivial class with just a number of no-ops
template <class T>
struct OwningPointer final {
    static_assert(std::is_class_v<T>); // UPX convention
    typedef typename std::add_lvalue_reference<T>::type reference;
    typedef typename std::add_lvalue_reference<const T>::type const_reference;
    typedef typename std::add_pointer<T>::type pointer;
    typedef typename std::add_pointer<const T>::type const_pointer;
    constexpr OwningPointer(pointer p) noexcept : ptr(p) {}
    constexpr operator pointer() noexcept { return ptr; }
    constexpr operator const_pointer() const noexcept { return ptr; }
    constexpr reference operator*() noexcept { return *ptr; }
    constexpr const_reference operator*() const noexcept { return *ptr; }
    constexpr pointer operator->() noexcept { return ptr; }
    constexpr const_pointer operator->() const noexcept { return ptr; }
private:
    pointer ptr;
    reference operator[](std::ptrdiff_t) noexcept = delete;
    const_reference operator[](std::ptrdiff_t) const noexcept = delete;
#if 0 // fun with C++
    // disable common "new" and ALL "delete" operators
    static void *operator new(std::size_t) = delete;
    static void *operator new[](std::size_t) = delete;
    static void *operator new(std::size_t, void *) = delete;
    static void *operator new[](std::size_t, void *) = delete;
    // replaceable usual deallocation functions (8)
    static void operator delete(void *) noexcept = delete;
    static void operator delete[](void *) noexcept = delete;
    static void operator delete(void *, std::align_val_t) noexcept = delete;
    static void operator delete[](void *, std::align_val_t) noexcept = delete;
    static void operator delete(void *, std::size_t) noexcept = delete;
    static void operator delete[](void *, std::size_t) noexcept = delete;
    static void operator delete(void *, std::size_t, std::align_val_t) noexcept = delete;
    static void operator delete[](void *, std::size_t, std::align_val_t) noexcept = delete;
    // replaceable placement deallocation functions (4)
    static void operator delete(void *, const std::nothrow_t &) noexcept = delete;
    static void operator delete[](void *, const std::nothrow_t &) noexcept = delete;
    static void operator delete(void *, std::align_val_t, const std::nothrow_t &) noexcept = delete;
    static void operator delete[](void *, std::align_val_t,
                                  const std::nothrow_t &) noexcept = delete;
    // non-allocating placement deallocation functions (2)
    static void operator delete(void *, void *) noexcept = delete;
    static void operator delete[](void *, void *) noexcept = delete;
#endif
};
// must overload mem_clear()
template <class T>
inline void mem_clear(OwningPointer<T> object) noexcept {
    mem_clear((T *) object);
}
#define OwningPointer(T) upx::OwningPointer<T>

#endif

template <class T>
inline void owner_delete(OwningPointer(T)(&object)) noexcept {
    static_assert(std::is_class_v<T>); // UPX convention
    static_assert(std::is_nothrow_destructible_v<T>);
    if (object != nullptr) {
        delete (T *) object;
        object = nullptr;
    }
    assert_noexcept((T *) object == nullptr);
    assert_noexcept(object == nullptr);
}

// disable some overloads
#if defined(__clang__) || __GNUC__ != 7
template <class T>
inline void owner_delete(T (&array)[]) noexcept DELETED_FUNCTION;
#endif
template <class T, std::size_t N>
inline void owner_delete(T (&array)[N]) noexcept DELETED_FUNCTION;

} // namespace upx
