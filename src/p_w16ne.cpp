/* p_w16ne.cpp --

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


#include "conf.h"
#include "file.h"
#include "filter.h"
#include "packer.h"
#include "p_w16ne.h"
#include "linker.h"


/*************************************************************************
//
**************************************************************************/

PackW16Ne::PackW16Ne(InputFile *f) :
    super(f)
{
    bele = &N_BELE_RTP::le_policy;
}


const int *PackW16Ne::getCompressionMethods(int method, int level) const
{
    bool small = false;
    return Packer::getDefaultCompressionMethods_8(method, level, small);
}


const int *PackW16Ne::getFilters() const
{
    return NULL;
}


void PackW16Ne::buildLoader(const Filter *ft)
{
    // prepare loader
//    initLoader(nrv_loader,sizeof(nrv_loader));
//    addLoader("...");
    if (ft->id)
    {
        assert(ft->calls > 0);
//        addLoader("...");
    }
//
}


Linker* PackW16Ne::newLinker() const
{
    return new ElfLinkerX86;
}


/*************************************************************************
//
**************************************************************************/

int PackW16Ne::readFileHeader()
{
    // FIXME: identify a win16/ne executable, so that the call
    // for contribution below will get thrown
    return 0;
    //return UPX_F_WIN16_NE;
}


bool PackW16Ne::canPack()
{
    if (!readFileHeader())
        return false;
    throwCantPack("win16/ne is not supported yet; your contribution is welcome");
    return false;
}


/*************************************************************************
//
**************************************************************************/

void PackW16Ne::pack(OutputFile *)
{
    throwCantPack("not yet implemented");
}


/*************************************************************************
//
**************************************************************************/

int PackW16Ne::canUnpack()
{
    if (!readFileHeader())
        return false;
    return false;
}


/*************************************************************************
//
**************************************************************************/

void PackW16Ne::unpack(OutputFile *)
{
    throwCantUnpack("not yet implemented");
}


/*
vi:ts=4:et
*/

