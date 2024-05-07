/* cxxlib.h -- C++ support library

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2024 Markus Franz Xaver Johannes Oberhumer
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

// #include <atomic>
// #include <cstddef>
// #include <new>
// #include <type_traits>

namespace upx {

/*************************************************************************
// core util
**************************************************************************/

// disable taking the address => force passing by reference (instead of pointer)
#define UPX_CXX_DISABLE_ADDRESS(Klass)                                                             \
private:                                                                                           \
    Klass *operator&() const noexcept DELETED_FUNCTION;

// disable copy and move
#define UPX_CXX_DISABLE_COPY(KlassName)                                                            \
private:                                                                                           \
    KlassName(const KlassName &) noexcept DELETED_FUNCTION;            /* copy constructor */      \
    KlassName &operator=(const KlassName &) noexcept DELETED_FUNCTION; /* copy assignment */
#define UPX_CXX_DISABLE_MOVE(KlassName)                                                            \
private:                                                                                           \
    KlassName(KlassName &&) noexcept DELETED_FUNCTION;            /* move constructor */           \
    KlassName &operator=(KlassName &&) noexcept DELETED_FUNCTION; /* move assignment */
#define UPX_CXX_DISABLE_COPY_MOVE(KlassName)                                                       \
    UPX_CXX_DISABLE_COPY(KlassName)                                                                \
    UPX_CXX_DISABLE_MOVE(KlassName)

// fun with C++: disable common "new" and ALL "delete" operators
// https://en.cppreference.com/w/cpp/memory/new/operator_delete
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL__(Klass)                                                   \
private:                                                                                           \
    /* common allocation functions (4) */                                                          \
    static void *operator new(std::size_t) DELETED_FUNCTION;                                       \
    static void *operator new[](std::size_t) DELETED_FUNCTION;                                     \
    static void *operator new(std::size_t, void *) DELETED_FUNCTION;                               \
    static void *operator new[](std::size_t, void *) DELETED_FUNCTION;                             \
    /* replaceable placement deallocation functions (4) */                                         \
    static void operator delete(void *, const std::nothrow_t &) noexcept DELETED_FUNCTION;         \
    static void operator delete[](void *, const std::nothrow_t &) noexcept DELETED_FUNCTION;       \
    static void operator delete(void *, std::align_val_t, const std::nothrow_t &)                  \
        noexcept DELETED_FUNCTION;                                                                 \
    static void operator delete[](void *, std::align_val_t, const std::nothrow_t &)                \
        noexcept DELETED_FUNCTION;                                                                 \
    /* non-allocating placement deallocation functions (2) */                                      \
    static void operator delete(void *, void *) noexcept DELETED_FUNCTION;                         \
    static void operator delete[](void *, void *) noexcept DELETED_FUNCTION;

/* class-specific usual deallocation functions (8) */
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDF_A__(Klass)                                           \
protected:                                                                                         \
    static void operator delete(void *) noexcept {}                                                \
    static void operator delete(void *, std::align_val_t) noexcept {}                              \
    static void operator delete(void *, std::size_t) noexcept {}                                   \
    static void operator delete(void *, std::size_t, std::align_val_t) noexcept {}                 \
private:                                                                                           \
    static void operator delete[](void *) noexcept DELETED_FUNCTION;                               \
    static void operator delete[](void *, std::align_val_t) noexcept DELETED_FUNCTION;             \
    static void operator delete[](void *, std::size_t) noexcept DELETED_FUNCTION;                  \
    static void operator delete[](void *, std::size_t, std::align_val_t) noexcept DELETED_FUNCTION;
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDF_B__(Klass)                                           \
private:                                                                                           \
    static void operator delete(void *) noexcept DELETED_FUNCTION;                                 \
    static void operator delete[](void *) noexcept DELETED_FUNCTION;                               \
    static void operator delete(void *, std::align_val_t) noexcept DELETED_FUNCTION;               \
    static void operator delete[](void *, std::align_val_t) noexcept DELETED_FUNCTION;             \
    static void operator delete(void *, std::size_t) noexcept DELETED_FUNCTION;                    \
    static void operator delete[](void *, std::size_t) noexcept DELETED_FUNCTION;                  \
    static void operator delete(void *, std::size_t, std::align_val_t) noexcept DELETED_FUNCTION;  \
    static void operator delete[](void *, std::size_t, std::align_val_t) noexcept DELETED_FUNCTION;

/* class-specific usual destroying deallocation functions (4) */
#if __cplusplus >= 202002L
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_A__(Klass)                                          \
protected:                                                                                         \
    static void operator delete(Klass *, std::destroying_delete_t) noexcept {}                     \
    static void operator delete(Klass *, std::destroying_delete_t, std::align_val_t) noexcept {}   \
    static void operator delete(Klass *, std::destroying_delete_t, std::size_t) noexcept {}        \
    static void operator delete(Klass *, std::destroying_delete_t, std::size_t, std::align_val_t)  \
        noexcept {}                                                                                \
private:
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_B__(Klass)                                          \
private:                                                                                           \
    static void operator delete(Klass *, std::destroying_delete_t) noexcept DELETED_FUNCTION;      \
    static void operator delete(Klass *, std::destroying_delete_t, std::align_val_t)               \
        noexcept DELETED_FUNCTION;                                                                 \
    static void operator delete(Klass *, std::destroying_delete_t, std::size_t)                    \
        noexcept DELETED_FUNCTION;                                                                 \
    static void operator delete(Klass *, std::destroying_delete_t, std::size_t, std::align_val_t)  \
        noexcept DELETED_FUNCTION;
#else
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_A__(Klass) private:
#define UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_B__(Klass) private:
#endif

// for classes which may have virtual methods
#define UPX_CXX_DISABLE_NEW_DELETE(Klass)                                                          \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL__(Klass)                                                       \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDF_A__(Klass)                                               \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_A__(Klass)

// this only works for classes WITHOUT any virtual methods
#define UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(Klass)                                               \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL__(Klass)                                                       \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDF_B__(Klass)                                               \
    UPX_CXX_DISABLE_NEW_DELETE_IMPL_CSUDDF_B__(Klass)

#if defined(_LIBCPP_HAS_NO_LIBRARY_ALIGNED_ALLOCATION) // do not use std::align_val_t
#undef UPX_CXX_DISABLE_NEW_DELETE
#undef UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL
#define UPX_CXX_DISABLE_NEW_DELETE(Klass)            private:
#define UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(Klass) private:
#endif

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

// a static_cast that does not trigger -Wcast-align warnings
template <class Result, class From>
forceinline Result ptr_static_cast(From *ptr) noexcept {
    static_assert(std::is_pointer_v<Result>);
    static_assert(!std::is_const_v<std::remove_pointer_t<Result> >); // enforce same constness
    return static_cast<Result>(static_cast<void *>(ptr));
}
template <class Result, class From>
forceinline Result ptr_static_cast(const From *ptr) noexcept {
    static_assert(std::is_pointer_v<Result>);
    static_assert(std::is_const_v<std::remove_pointer_t<Result> >); // required
    return static_cast<Result>(static_cast<const void *>(ptr));
}

#if WITH_THREADS
// cast "T *" to "std::atomic<T> *"
template <class T>
forceinline std::atomic<T> *ptr_std_atomic_cast(T *ptr) noexcept {
    // TODO later: make sure that this cast is indeed legal
    std::atomic<T> *result = ptr_static_cast<std::atomic<T> *>(ptr);
    static_assert(sizeof(*result) == sizeof(*ptr));
    static_assert(alignof(decltype(*result)) == alignof(decltype(*ptr)));
    return result;
}
#endif // WITH_THREADS

// atomic_exchange
template <class T>
forceinline T atomic_exchange(T *ptr, T new_value) noexcept {
    static_assert(std::is_standard_layout_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);
#if __has_builtin(__atomic_exchange_n) && defined(__ATOMIC_SEQ_CST)
    return __atomic_exchange_n(ptr, new_value, __ATOMIC_SEQ_CST);
#elif __has_builtin(__sync_swap)
    return __sync_swap(ptr, new_value);
#elif WITH_THREADS
    return std::atomic_exchange(ptr_std_atomic_cast(ptr), new_value);
#else
    T old_value = *ptr;
    *ptr = new_value;
    return old_value;
#endif
}

// helper classes so we don't leak memory on exceptions
template <class T>
struct ObjectDeleter final {
    T **items;         // public
    std::size_t count; // public
    explicit ObjectDeleter(T **p, std::size_t n) noexcept : items(p), count(n) {}
    ~ObjectDeleter() noexcept { delete_items(); }
    void delete_items() noexcept {
        for (std::size_t i = 0; i < count; i++) {
            T *item = atomic_exchange(&items[i], (T *) nullptr);
            delete item; // single object delete
        }
    }
};
template <class T>
struct ArrayDeleter final {
    T **items;         // public
    std::size_t count; // public
    explicit ArrayDeleter(T **p, std::size_t n) noexcept : items(p), count(n) {}
    ~ArrayDeleter() noexcept { delete_items(); }
    void delete_items() noexcept {
        for (std::size_t i = 0; i < count; i++) {
            T *item = atomic_exchange(&items[i], (T *) nullptr);
            delete[] item; // array delete
        }
    }
};
template <class T>
struct MallocDeleter final {
    T **items;         // public
    std::size_t count; // public
    explicit MallocDeleter(T **p, std::size_t n) noexcept : items(p), count(n) {}
    ~MallocDeleter() noexcept { delete_items(); }
    void delete_items() noexcept {
        for (std::size_t i = 0; i < count; i++) {
            T *item = atomic_exchange(&items[i], (T *) nullptr);
            ::free(item); // free memory from malloc()
        }
    }
};

class noncopyable {
protected:
    forceinline constexpr noncopyable() noexcept {}
#if __cplusplus >= 202002L
    forceinline constexpr ~noncopyable() noexcept = default;
#else
    forceinline ~noncopyable() noexcept = default;
#endif
    UPX_CXX_DISABLE_COPY_MOVE(noncopyable)
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

constexpr bool mem_eq(const char *a, const char *b, std::size_t n) {
    return n == 0 || (*a == *b && mem_eq(a + 1, b + 1, n - 1));
}
constexpr bool mem_eq(const unsigned char *a, const unsigned char *b, std::size_t n) {
    return n == 0 || (*a == *b && mem_eq(a + 1, b + 1, n - 1));
}
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
    // IMPORTANT NOTE: permissive, converts all other values to Third!
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
    value_type value = False;                      // the actual value of this type
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(TriBool) // UPX convention
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
    OptVar &operator=(const T &other) noexcept { // copy constructor
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
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(OptVar) // UPX convention
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
    reference operator[](std::ptrdiff_t) noexcept DELETED_FUNCTION;
    const_reference operator[](std::ptrdiff_t) const noexcept DELETED_FUNCTION;
    UPX_CXX_DISABLE_ADDRESS(OwningPointer)               // UPX convention
    UPX_CXX_DISABLE_NEW_DELETE_NO_VIRTUAL(OwningPointer) // UPX convention
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

template <class T>
inline void owner_free(OwningPointer(T)(&object)) noexcept {
    static_assert(!std::is_class_v<T>); // UPX convention
    if (object != nullptr) {
        ::free((T *) object);
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
