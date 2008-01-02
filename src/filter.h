/* filter.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2008 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2008 Laszlo Molnar
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
   <markus@oberhumer.com>               <ml1050@users.sourceforge.net>
 */


#ifndef __UPX_FILTER_H
#define __UPX_FILTER_H

class Filter;
class FilterImp;


/*************************************************************************
// A filter is a reversible operation that modifies a given
// block of memory.
//
// A filter can fail and return false. In this case the buffer
// must be unmodified (or otherwise restored).
//
// If a filter fails and somehow cannot restore the block it must
// call throwFilterException() - this will cause the compression
// to fail.
//
// Unfilters throw exceptions in case of errors.
//
// The main idea behind filters is to convert relative jumps and calls
// to absolute addresses so that the buffer compresses better.
**************************************************************************/

class Filter
{
public:
    Filter(int level) { clevel = level; init(); }
    void init(int id=0, unsigned addvalue=0);

    bool filter(upx_byte *buf, unsigned buf_len);
    void unfilter(upx_byte *buf, unsigned buf_len, bool verify_checksum=false);
    void verifyUnfilter();
    bool scan(const upx_byte *buf, unsigned buf_len);

    static bool isValidFilter(int filter_id);
    static bool isValidFilter(int filter_id, const int *allowed_filters);

public:
    // Will be set by each call to filter()/unfilter().
    // Read-only afterwards.
    upx_byte *buf;
    unsigned buf_len;

    // Checksum of the buffer before applying the filter
    // or after un-applying the filter.
    unsigned adler;

    // Input parameters used by various filters.
    unsigned addvalue;
    const int *preferred_ctos;

    // Input/output parameters used by various filters
    unsigned char cto;              // call trick offset

    // Output used by various filters. Read only.
    unsigned calls;
    unsigned noncalls;
    unsigned wrongcalls;
    unsigned firstcall;
    unsigned lastcall;
    unsigned n_mru;  // ctojr only

    // Read only.
    int id;

private:
    int clevel;         // compression level
};


/*************************************************************************
// We don't want a full OO interface here because of
// certain implementation speed reasons.
//
// This class is private to Filter - don't look.
**************************************************************************/

class FilterImp
{
    friend class Filter;

private:
    struct FilterEntry
    {
        int id;                             // 0 .. 255
        unsigned min_buf_len;
        unsigned max_buf_len;
        int (*do_filter)(Filter *);         // filter a buffer
        int (*do_unfilter)(Filter *);       // unfilter a buffer
        int (*do_scan)(Filter *);           // scan a buffer
    };

    // get a specific filter entry
    static const FilterEntry *getFilter(int id);

private:
    // strictly private filter database
    static const FilterEntry filters[];
    static const int n_filters;             // number of filters[]
};


#endif /* already included */


/*
vi:ts=4:et
*/

