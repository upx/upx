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
#ifndef UPX_EXCEPT_H__
#define UPX_EXCEPT_H__ 1

#ifdef __cplusplus

const char *prettyName(const char *n) noexcept;

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
    // disable assignment
    Throwable &operator=(const Throwable &) = delete;
    // disable dynamic allocation => force throwing by value
    ACC_CXX_DISABLE_NEW_DELETE

private:
    static unsigned long counter; // for debugging
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

class OutOfMemoryException : public Exception {
    typedef Exception super;

public:
    OutOfMemoryException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class IOException : public Exception {
    typedef Exception super;

public:
    IOException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class EOFException : public IOException {
    typedef IOException super;

public:
    EOFException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class FileNotFoundException : public IOException {
    typedef IOException super;

public:
    FileNotFoundException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

class FileAlreadyExistsException : public IOException {
    typedef IOException super;

public:
    FileAlreadyExistsException(const char *m = nullptr, int e = 0) noexcept : super(m, e) {}
};

/*************************************************************************
// application exceptions
**************************************************************************/

class OverlayException : public Exception {
    typedef Exception super;

public:
    OverlayException(const char *m = nullptr, bool w = false) noexcept : super(m, 0, w) {}
};

class CantPackException : public Exception {
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

class AlreadyPackedException : public CantPackException {
    typedef CantPackException super;

public:
    AlreadyPackedException(const char *m = nullptr) noexcept : super(m) { is_warning = true; }
};

class NotCompressibleException : public CantPackException {
    typedef CantPackException super;

public:
    NotCompressibleException(const char *m = nullptr) noexcept : super(m) {}
};

class CantUnpackException : public Exception {
    typedef Exception super;

public:
    CantUnpackException(const char *m = nullptr, bool w = false) noexcept : super(m, 0, w) {}
};

class NotPackedException : public CantUnpackException {
    typedef CantUnpackException super;

public:
    NotPackedException(const char *m = nullptr) noexcept : super(m, true) {}
};

/*************************************************************************
// errors
**************************************************************************/

class InternalError : public Error {
    typedef Error super;

public:
    InternalError(const char *m = nullptr) noexcept : super(m, 0) {}
};

/*************************************************************************
// util
**************************************************************************/

#undef NORET
#if 1 && defined(__GNUC__)
#define NORET __acc_noinline __attribute__((__noreturn__))
#else
#define NORET __acc_noinline
#endif

NORET void throwCantPack(const char *msg);
NORET void throwCantPackExact();
NORET void throwUnknownExecutableFormat(const char *msg = nullptr, bool warn = false);
NORET void throwNotCompressible(const char *msg = nullptr);
NORET void throwAlreadyPacked(const char *msg = nullptr);
NORET void throwAlreadyPackedByUPX(const char *msg = nullptr);
NORET void throwCantUnpack(const char *msg);
NORET void throwNotPacked(const char *msg = nullptr);
NORET void throwFilterException();
NORET void throwBadLoader();
NORET void throwChecksumError();
NORET void throwCompressedDataViolation();
NORET void throwInternalError(const char *msg);
NORET void throwOutOfMemoryException(const char *msg = nullptr);
NORET void throwIOException(const char *msg = nullptr, int e = 0);
NORET void throwEOFException(const char *msg = nullptr, int e = 0);

#undef NORET

#endif /* __cplusplus */

#endif /* already included */

/* vim:set ts=4 sw=4 et: */
