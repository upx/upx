/* except.cpp --

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

#include "conf.h"

/*************************************************************************
//
**************************************************************************/

/*static*/ upx_std_atomic(size_t) Throwable::debug_counter;

Throwable::Throwable(const char *m, int e, bool w) noexcept : super(),
                                                              msg(nullptr),
                                                              err(e),
                                                              is_warning(w) {
    if (m)
        msg = strdup(m);
#if 0
    fprintf(stderr, "construct exception: %s %zu\n", msg, debug_counter);
    debug_counter += 1;
#endif
}

Throwable::Throwable(const Throwable &other) noexcept : super(other),
                                                        msg(nullptr),
                                                        err(other.err),
                                                        is_warning(other.is_warning) {
    if (other.msg)
        msg = strdup(other.msg);
#if 0
    fprintf(stderr, "copy exception: %s %zu\n", msg, debug_counter);
    debug_counter += 1;
#endif
}

Throwable::~Throwable() noexcept {
#if 0
    debug_counter -= 1;
    fprintf(stderr, "destruct exception: %s %zu\n", msg, debug_counter);
#endif
    if (msg)
        free(msg);
}

/*************************************************************************
// compression
**************************************************************************/

void throwCantPack(const char *msg) {
    // UGLY, but makes things easier
    if (opt->cmd == CMD_NONE)
        throw CantPackException(msg);
    else if (opt->cmd == CMD_COMPRESS)
        throw CantPackException(msg);
    else if (opt->cmd == CMD_FILEINFO)
        throw CantPackException(msg);
    else
        throw CantUnpackException(msg);
}

void throwCantPackExact() { throwCantPack("option '--exact' does not work with this file"); }

void throwFilterException() { throwCantPack("filter problem"); }

void throwUnknownExecutableFormat(const char *msg, bool warn) {
    throw UnknownExecutableFormatException(msg, warn);
}

void throwNotCompressible(const char *msg) { throw NotCompressibleException(msg); }

void throwAlreadyPacked(const char *msg) { throw AlreadyPackedException(msg); }

void throwAlreadyPackedByUPX(const char *msg) {
    if (msg == nullptr)
        msg = "already packed by UPX";
    throwAlreadyPacked(msg);
}

/*************************************************************************
// decompression
**************************************************************************/

void throwCantUnpack(const char *msg) {
    // UGLY, but makes things easier
    throwCantPack(msg);
}

void throwNotPacked(const char *msg) {
    if (msg == nullptr)
        msg = "not packed by UPX";
    throw NotPackedException(msg);
}

void throwChecksumError() { throw Exception("checksum error"); }

void throwCompressedDataViolation() { throw Exception("compressed data violation"); }

/*************************************************************************
// other
**************************************************************************/

void throwInternalError(const char *msg) { throw InternalError(msg); }

void throwBadLoader() { throwInternalError("bad loader"); }

void throwOutOfMemoryException(const char *msg) {
    if (msg == nullptr)
        msg = "out of memory";
    throw OutOfMemoryException(msg);
}

void throwIOException(const char *msg, int e) { throw IOException(msg, e); }

void throwEOFException(const char *msg, int e) {
    if (msg == nullptr && e == 0)
        msg = "premature end of file";
    throw EOFException(msg, e);
}

/*************************************************************************
// varargs overloads
**************************************************************************/

template <>
void throwCantPack(const char *format, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, format);
    (void) upx_safe_vsnprintf_noexcept(msg, sizeof(msg), format, ap);
    va_end(ap);
    throwCantPack(msg);
}

template <>
void throwCantUnpack(const char *format, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, format);
    (void) upx_safe_vsnprintf_noexcept(msg, sizeof(msg), format, ap);
    va_end(ap);
    throwCantUnpack(msg);
}

template <>
void throwInternalError(const char *format, ...) {
    char msg[1024];
    va_list ap;
    va_start(ap, format);
    (void) upx_safe_vsnprintf_noexcept(msg, sizeof(msg), format, ap);
    va_end(ap);
    throwCantUnpack(msg);
}

/*************************************************************************
// util
**************************************************************************/

void assertFailed(const char *expr, const char *file, int line, const char *func) noexcept {
    fflush(stdout);
    fprintf(stderr, "Assertion failed: %s (%s: %s: %d)\n", expr, file, func, line);
    std::terminate();
}

void throwAssertFailed(const char *expr, const char *file, int line, const char *func) {
    if (opt->debug.debug_level >= 1) {
        throwCantPack("corrupted file; details: %s (%s: %s: %d)", expr, file, func, line);
    } else {
        throwCantPack("corrupted file; try '--debug' for more details");
    }
}

const char *prettyName(const char *n) noexcept {
    if (n == nullptr)
        return "(null)";
    while (*n) {
        if (*n >= '0' && *n <= '9') // Linux ABI
            n++;
        else if (*n == ' ')
            n++;
        else if (strncmp(n, "class ", 6) == 0) // Visual C++ (msvc)
            n += 6;
        else
            break;
    }
    return n;
}

/* vim:set ts=4 sw=4 et: */
