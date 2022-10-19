/* xspan -- a minimally invasive checked memory smart pointer

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
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

/*************************************************************************
//
**************************************************************************/

public:
typedef T element_type;
typedef typename std::add_lvalue_reference<T>::type reference;
typedef typename std::add_pointer<T>::type pointer;
typedef size_t size_type;

// befriend all
template <class>
friend struct PtrOrSpan;
template <class>
friend struct PtrOrSpanOrNull;
template <class>
friend struct Span;

#if SPAN_CONFIG_ENABLE_IMPLICIT_CONVERSION
operator pointer() const { return ptr; }
#endif

private:
pointer ptr; // current view into (base, base+size_in_bytes) iff base != nullptr
pointer base;
size_type size_in_bytes;

// debug - internal sanity check; also serves as pseudo-documentation
#if DEBUG || 1
__acc_noinline void assertInvariants() const {
    if __acc_cte (configRequirePtr)
        assert(ptr != nullptr);
    if __acc_cte (configRequireBase)
        assert(base != nullptr);
    if __acc_cte ((configRequirePtr || ptr != nullptr) && (configRequireBase || base != nullptr))
        span_check_range(ptr, base, size_in_bytes);
}
#else
__acc_forceinline void assertInvariants() const {}
#endif

static __acc_forceinline pointer makeNotNull(pointer p) {
    if __acc_very_unlikely (p == nullptr)
        span_fail_nullptr();
    return p;
}
// enforce config invariants at constructor time - static functions
static __acc_forceinline pointer makePtr(pointer p) {
    if __acc_cte (configRequirePtr && p == nullptr)
        span_fail_nullptr();
    return p;
}
static __acc_forceinline pointer makeBase(pointer b) {
    if __acc_cte (configRequireBase && b == nullptr)
        span_fail_nullptr();
    return b;
}
// inverse logic for ensuring valid pointers from existing objets
__acc_forceinline pointer ensurePtr() const {
    if __acc_cte (!configRequirePtr && ptr == nullptr)
        span_fail_nullptr();
    return ptr;
}
__acc_forceinline pointer ensureBase() const {
    if __acc_cte (!configRequireBase && base == nullptr)
        span_fail_nullptr();
    return ptr;
}

public:
inline ~CSelf() {}
// constructors from pointers
CSelf(pointer first, SpanCount count)
    : ptr(makePtr(first)), base(makeBase(first)), size_in_bytes(span_mem_size<T>(count.count)) {
    assertInvariants();
}
CSelf(pointer first, SpanSizeInBytes bytes)
    : ptr(makePtr(first)), base(makeBase(first)),
      size_in_bytes(span_mem_size<char>(bytes.size_in_bytes)) {
    assertInvariants();
}
// enable this constructor only if the underlying type is char or void
template <class U>
CSelf(U *first, size_type count, SPAN_REQUIRES_SIZE_1_A)
    : ptr(makePtr(first)), base(makeBase(first)), size_in_bytes(span_mem_size<T>(count)) {
    assertInvariants();
}
CSelf(pointer first, SpanCount count, pointer base_)
    : ptr(makePtr(first)), base(makeBase(base_)), size_in_bytes(span_mem_size<T>(count.count)) {
    // check invariants
    if __acc_cte ((configRequirePtr || ptr != nullptr) && (configRequireBase || base != nullptr))
        span_check_range(ptr, base, size_in_bytes);
    // double sanity check
    assertInvariants();
}
CSelf(pointer first, SpanSizeInBytes bytes, pointer base_)
    : ptr(makePtr(first)), base(makeBase(base_)),
      size_in_bytes(span_mem_size<char>(bytes.size_in_bytes)) {
    // check invariants
    if __acc_cte ((configRequirePtr || ptr != nullptr) && (configRequireBase || base != nullptr))
        span_check_range(ptr, base, size_in_bytes);
    // double sanity check
    assertInvariants();
}
// enable this constructor only if the underlying type is char or void
template <class U>
CSelf(pointer first, size_type count, U *base_, SPAN_REQUIRES_SIZE_1_A)
    : ptr(makePtr(first)), base(makeBase(base_)), size_in_bytes(span_mem_size<T>(count)) {
    // check invariants
    if __acc_cte ((configRequirePtr || ptr != nullptr) && (configRequireBase || base != nullptr))
        span_check_range(ptr, base, size_in_bytes);
    // double sanity check
    assertInvariants();
}
#ifdef UPX_VERSION_HEX
// constructors from MemBuffer
CSelf(MemBuffer &mb)
    : CSelf(makeNotNull((pointer) membuffer_get_void_ptr(mb)),
            SpanSizeInBytes(membuffer_get_size(mb))) {}
CSelf(pointer first, MemBuffer &mb)
    : CSelf(first, SpanSizeInBytes(membuffer_get_size(mb)),
            makeNotNull((pointer) membuffer_get_void_ptr(mb))) {}
CSelf(std::nullptr_t, MemBuffer &) SPAN_DELETED_FUNCTION;
#endif
// disable constructors from nullptr to catch compile-time misuse
private:
CSelf(std::nullptr_t, SpanCount) SPAN_DELETED_FUNCTION;
CSelf(std::nullptr_t, SpanCount, std::nullptr_t) SPAN_DELETED_FUNCTION;
CSelf(const void *, SpanCount, std::nullptr_t) SPAN_DELETED_FUNCTION;
CSelf(std::nullptr_t, SpanSizeInBytes) SPAN_DELETED_FUNCTION;
CSelf(std::nullptr_t, SpanSizeInBytes, std::nullptr_t) SPAN_DELETED_FUNCTION;
CSelf(const void *, SpanSizeInBytes, std::nullptr_t) SPAN_DELETED_FUNCTION;
CSelf(std::nullptr_t, size_type) SPAN_DELETED_FUNCTION;
CSelf(std::nullptr_t, size_type, std::nullptr_t) SPAN_DELETED_FUNCTION;
CSelf(const void *, size_type, std::nullptr_t) SPAN_DELETED_FUNCTION;

// unchecked constructor
private:
enum ModeUnchecked { Unchecked };
CSelf(ModeUnchecked, pointer p, size_type bytes, pointer b)
    : ptr(p), base(b), size_in_bytes(bytes) {
    assertInvariants();
}
#if 0
// unchecked assignment
Self &assign(ModeUnchecked, pointer p, size_type bytes, pointer b) {
    ptr = p;
    base = b;
    size_in_bytes = bytes;
    assertInvariants();
    return *this;
}
Self &assign(ModeUnchecked, const Self &other) {
    ptr = other.ptr;
    base = other.base;
    size_in_bytes = other.size_in_bytes;
    assertInvariants();
    return *this;
}
#endif

public:
// assignment - here we can rely on invariants enforced at construction time by makePtr/makeBase
// NOTE: *this remains unmodified in case of failure
Self &assign(pointer other) {
    assertInvariants();
    other = makePtr(other);
    if __acc_cte ((configRequirePtr || other != nullptr) && (configRequireBase || base != nullptr))
        span_check_range(other, base, size_in_bytes);
    // ok
    ptr = other;
    assertInvariants();
    return *this;
}
Self &assign(const Self &other) {
    assertInvariants();
    other.assertInvariants();
    if __acc_cte (!configRequireBase && base == nullptr) {
        // magic 1: if base is unset, automatically set base/size_in_bytes from other
        if __acc_cte ((configRequirePtr || other.ptr != nullptr) &&
                      (configRequireBase || other.base != nullptr))
            span_check_range(other.ptr, other.base, other.size_in_bytes);
        // ok
        ptr = other.ptr;
        base = other.base;
        size_in_bytes = other.size_in_bytes;
    } else {
        // magic 2: assert same base (but ignore size_in_bytes !)
        if __acc_cte (configRequireBase || other.base != nullptr)
            if __acc_very_unlikely (base != other.base)
                span_fail_not_same_base();
        if __acc_cte ((configRequirePtr || other.ptr != nullptr) &&
                      (configRequireBase || base != nullptr))
            span_check_range(other.ptr, base, size_in_bytes);
        // ok
        ptr = other.ptr;
    }
    assertInvariants();
    return *this;
}

Self &operator=(pointer other) { return assign(other); }

Self &operator=(const Self &other) { return assign(other); }

// FIXME: this is not called??
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(Self &)
operator=(const CSelf<U> &other) {
    // assert(0);
    return assign(Self(other));
}

#ifdef UPX_VERSION_HEX
Self &operator=(MemBuffer &mb) { return assign(Self(mb)); }
#endif

Self subspan(ptrdiff_t offset, ptrdiff_t count) {
    pointer begin = check_add(ptr, offset);
    pointer end = check_add(begin, count);
    if (begin <= end)
        return Self(Unchecked, begin, (end - begin) * sizeof(T), begin);
    else
        return Self(Unchecked, end, (begin - end) * sizeof(T), end);
}

bool operator==(pointer other) const { return ptr == other; }
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator==(U *other) const {
    return ptr == other;
}
bool operator!=(pointer other) const { return ptr != other; }
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator!=(U *other) const {
    return ptr != other;
}

template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator==(const PtrOrSpan<U> &other) const {
    return ptr == other.ptr;
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator==(const PtrOrSpanOrNull<U> &other) const {
    return ptr == other.ptr;
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator==(const Span<U> &other) const {
    return ptr == other.ptr;
}

template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator!=(const PtrOrSpan<U> &other) const {
    return !(*this == other);
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator!=(const PtrOrSpanOrNull<U> &other) const {
    return !(*this == other);
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator!=(const Span<U> &other) const {
    return !(*this == other);
}

// check for notNull here
bool operator<(std::nullptr_t) const SPAN_DELETED_FUNCTION;
bool operator<(pointer other) const { return ensurePtr() < makeNotNull(other); }

template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator<(const PtrOrSpan<U> &other) const {
    return ensurePtr() < other.ensurePtr();
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator<(const PtrOrSpanOrNull<U> &other) const {
    return ensurePtr() < other.ensurePtr();
}
template <class U>
SPAN_REQUIRES_CONVERTIBLE_R(bool)
operator<(const Span<U> &other) const {
    return ensurePtr() < other.ensurePtr();
}

// dereference
reference operator*() const { return *check_deref(ptr); }

// array access
reference operator[](ptrdiff_t i) const { return *check_deref(ptr, i); }

// arrow operator
pointer operator->() const { return check_deref(ptr); }

Self &operator++() {
    ptr = check_add(ptr, 1);
    return *this;
}
Self operator++(int) {
    Self tmp = *this;
    ++*this;
    return tmp;
}
Self &operator--() {
    ptr = check_add(ptr, -1);
    return *this;
}
Self operator--(int) {
    Self tmp = *this;
    --*this;
    return tmp;
}

Self &operator+=(ptrdiff_t n) {
    ptr = check_add(ptr, n);
    return *this;
}
Self &operator-=(ptrdiff_t n) {
    ptr = check_add(ptr, -n);
    return *this;
}

Self operator+(ptrdiff_t n) const {
    pointer first = check_add(ptr, n);
    return Self(Unchecked, first, size_in_bytes, base);
}
Self operator-(ptrdiff_t n) const {
    pointer first = check_add(ptr, -n);
    return Self(Unchecked, first, size_in_bytes, base);
}

private:
pointer check_deref(pointer p) const {
    if __acc_cte (!configRequirePtr && p == nullptr)
        span_fail_nullptr();
    if __acc_cte (configRequireBase || base != nullptr)
        span_check_range(p, base, size_in_bytes - sizeof(T));
    assertInvariants();
    return p;
}
pointer check_deref(pointer p, ptrdiff_t n) const {
    if __acc_cte (!configRequirePtr && p == nullptr)
        span_fail_nullptr();
    span_mem_size_assert_ptrdiff<T>(n);
    p += n;
    if __acc_cte (configRequireBase || base != nullptr)
        span_check_range(p, base, size_in_bytes - sizeof(T));
    assertInvariants();
    return p;
}
pointer check_add(pointer p, ptrdiff_t n) const {
    if __acc_cte (!configRequirePtr && p == nullptr)
        span_fail_nullptr();
    span_mem_size_assert_ptrdiff<T>(n);
    p += n;
    if __acc_cte (configRequireBase || base != nullptr)
        span_check_range(p, base, size_in_bytes);
    assertInvariants();
    return p;
}

// disable taking the address => force passing by reference
// [I'm not too sure about this design decision, but we can always allow it if needed]
Self *operator&() const SPAN_DELETED_FUNCTION;

public: // raw access
pointer raw_ptr() const { return ptr; }
pointer raw_base() const { return base; }
size_type raw_size_in_bytes() const { return size_in_bytes; }

pointer raw_bytes(size_t bytes) const {
    assertInvariants();
    if (bytes > 0) {
        if __acc_cte (!configRequirePtr && ptr == nullptr)
            span_fail_nullptr();
        if __acc_cte (configRequireBase || base != nullptr) {
            span_check_range(ptr, base, size_in_bytes - bytes);
        }
    }
    return ptr;
}

/* vim:set ts=4 sw=4 et: */
