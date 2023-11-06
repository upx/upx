/* except.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2023 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2023 Laszlo Molnar
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

   Markus F.X.J. Oberhumer              Laszlo Molnar
   <markus@oberhumer.com>               <ezerotven+github@gmail.com>
 */

#pragma once

const char *prettyName(const char *n) noexcept;

noinline void assertFailed(const char *expr, const char *file, int line, const char *func) noexcept;
noinline void throwAssertFailed(const char *expr, const char *file, int line, const char *func);

/*************************************************************************
// exceptions
**************************************************************************/

class Throwable : public std::exception {
    typedef std::exception super;

protected:
    Throwable(const char *m = nullptr, int e = 0, bool w = false) noexcept;

public:
    Throwable(const Throwable &) noexcept;
    virtual ~Throwable() noexcept;
    const char *getMsg() const noexcept { return msg; }
    int getErrno() const noexcept { return err; }
    bool isWarning() const noexcept { return is_warning; }

private:
    char *msg = nullptr;
    int err = 0;
protected:
    bool is_warning = false; // can be set by subclasses

private:
    // disable copy assignment and move
    Throwable &operator=(const Throwable &) DELETED_FUNCTION;
    Throwable(Throwable &&) noexcept DELETED_FUNCTION;
    Throwable &operator=(Throwable &&) noexcept DELETED_FUNCTION;
    // disable dynamic allocation => force throwing by value
    ACC_CXX_DISABLE_NEW_DELETE
    // disable taking the address => force passing by reference
    // [I'm not too sure about this design decision, but we can always allow it if needed]
    Throwable *operator&() const noexcept DELETED_FUNCTION;

private:
    static upx_std_atomic(size_t) debug_counter; // for debugging
};

// Exceptions can/should be caught
class Exception : public Throwable {
    typedef Throwable super;
public:
    Exception(const char *m = nullptr, int e = 0, bool w = false) noexcept : super(m, e, w) {}
};

// Errors should not be caught (or re-thrown)
class Error : public Throwable {
    typedef Throwable super;
public:
    Error(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

/*************************************************************************
// system exception
**************************************************************************/

class OutOfMemoryException final : public Exception {
    typedef Exception super;
public:
    OutOfMemoryException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class IOException /*not_final*/ : public Exception {
    typedef Exception super;
public:
    IOException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class EOFException final : public IOException {
    typedef IOException super;
public:
    EOFException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class FileNotFoundException final : public IOException {
    typedef IOException super;
public:
    FileNotFoundException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class FileAlreadyExistsException final : public IOException {
    typedef IOException super;
public:
    FileAlreadyExistsException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

/*************************************************************************
// application exceptions
**************************************************************************/

class OverlayException final : public Exception {
    typedef Exception super;
public:
    OverlayException(const char *m = nullptr, bool w = false) noexcept : super(m, 0, w) {}
};

class CantPackException /*not_final*/ : public Exception {
    typedef Exception super;
public:
    CantPackException(const char *m = nullptr, bool w = false) noexcept : super(m, 0, w) {}
};

class UnknownExecutableFormatException : public CantPackException {
    typedef CantPackException super;
public:
    UnknownExecutableFormatException(const char *m = nullptr, bool w = false) noexcept
        : super(m, w) {}
};

class AlreadyPackedException final : public CantPackException {
    typedef CantPackException super;
public:
    AlreadyPackedException(const char *m = nullptr) noexcept : super(m) { is_warning = true; }
};

class NotCompressibleException final : public CantPackException {
    typedef CantPackException super;
public:
    NotCompressibleException(const char *m = nullptr) noexcept : super(m) {}
};

class CantUnpackException /*not_final*/ : public Exception {
    typedef Exception super;
public:
    CantUnpackException(const char *m = nullptr, bool w = false) noexcept : super(m, 0, w) {}
};

class NotPackedException final : public CantUnpackException {
    typedef CantUnpackException super;
public:
    NotPackedException(const char *m = nullptr) noexcept : super(m, true) {}
};

/*************************************************************************
// errors
**************************************************************************/

class InternalError final : public Error {
    typedef Error super;
public:
    InternalError(const char *m = nullptr) noexcept : super(m, 0) {}
};

/*************************************************************************
// util
**************************************************************************/

#undef NORET
#if 1 && defined(__GNUC__)
#define NORET noinline __attribute__((__noreturn__))
#else
#define NORET noinline
#endif

NORET void throwCantPack(const char *msg) may_throw;
NORET void throwCantPackExact() may_throw;
NORET void throwUnknownExecutableFormat(const char *msg = nullptr, bool warn = false) may_throw;
NORET void throwNotCompressible(const char *msg = nullptr) may_throw;
NORET void throwAlreadyPacked(const char *msg = nullptr) may_throw;
NORET void throwAlreadyPackedByUPX(const char *msg = nullptr) may_throw;
NORET void throwCantUnpack(const char *msg) may_throw;
NORET void throwNotPacked(const char *msg = nullptr) may_throw;
NORET void throwFilterException() may_throw;
NORET void throwBadLoader() may_throw;
NORET void throwChecksumError() may_throw;
NORET void throwCompressedDataViolation() may_throw;
NORET void throwInternalError(const char *msg) may_throw;
NORET void throwOutOfMemoryException(const char *msg = nullptr) may_throw;
NORET void throwIOException(const char *msg = nullptr, int e = 0) may_throw;
NORET void throwEOFException(const char *msg = nullptr, int e = 0) may_throw;

// some C++ template wizardry is needed to overload throwCantPack() for varargs
template <class T>
void throwCantPack(const T *, ...) DELETED_FUNCTION;
template <>
NORET void throwCantPack(const char *format, ...) may_throw attribute_format(1, 2);
template <class T>
void throwCantUnpack(const T *, ...) DELETED_FUNCTION;
template <>
NORET void throwCantUnpack(const char *format, ...) may_throw attribute_format(1, 2);
template <class T>
void throwInternalError(const T *, ...) DELETED_FUNCTION;
template <>
NORET void throwInternalError(const char *format, ...) may_throw attribute_format(1, 2);

#undef NORET

/* vim:set ts=4 sw=4 et: */
