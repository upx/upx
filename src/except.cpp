/* except.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2000 Laszlo Molnar

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

   Markus F.X.J. Oberhumer                   Laszlo Molnar
   markus.oberhumer@jk.uni-linz.ac.at        ml1050@cdata.tvnet.hu
 */


#include "conf.h"


/*************************************************************************
// compression
**************************************************************************/

void throwCantPack(const char *msg)
{
    // UGLY, but makes things easier
    if (opt->cmd == CMD_COMPRESS)
        throw CantPackException(msg);
    else if (opt->cmd == CMD_FILEINFO)
        throw CantPackException(msg);
    else
        throw CantUnpackException(msg);
}

void throwFilterException()
{
    throwCantPack("filter problem");
}

void throwUnknownExecutableFormat(const char *msg, bool warn)
{
    throw UnknownExecutableFormatException(msg, warn);
}

void throwNotCompressible(const char *msg)
{
    throw NotCompressibleException(msg);
}

void throwAlreadyPacked(const char *msg)
{
    throw AlreadyPackedException(msg);
}

void throwAlreadyPackedByUPX(const char *msg)
{
    if (msg == NULL)
        msg = "already packed by UPX";
    throwAlreadyPacked(msg);
}


/*************************************************************************
// decompression
**************************************************************************/

void throwCantUnpack(const char *msg)
{
    // UGLY, but makes things easier
    throwCantPack(msg);
}

void throwNotPacked(const char *msg)
{
    if (msg == NULL)
        msg = "not packed by UPX";
    throw NotPackedException(msg);
}

void throwChecksumError()
{
    throw Exception("checksum error");
}

void throwCompressedDataViolation()
{
    throw Exception("compressed data violation");
}


/*************************************************************************
// other
**************************************************************************/

void throwInternalError(const char *msg)
{
    throw InternalError(msg);
}

void throwBadLoader()
{
    throwInternalError("bad loader");
}


void throwIOException(const char *msg, int e)
{
    throw IOException(msg, e);
}


void throwEOFException(const char *msg, int e)
{
    if (msg == NULL && e == 0)
        msg = "premature end of file";
    throw EOFException(msg, e);
}


/*************************************************************************
//
**************************************************************************/

const char *prettyName(const char *n)
{
    while (*n >= '0' && *n <= '9')              // gcc / egcs
        n++;
    if (strncmp(n, "class ", 6) == 0)           // Visual C++
        n += 6;
    return n;
}

const char *prettyName(const type_info &ti)
{
    return prettyName(ti.name());
}


/*
vi:ts=4:et
*/

