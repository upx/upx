/* packmast.h --

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

class PackerBase;
class InputFile;
class OutputFile;

/*************************************************************************
// dispatch to a concrete subclass of class PackerBase; see work.cpp
**************************************************************************/

class PackMaster final {
public:
    explicit PackMaster(InputFile *f, Options *o = nullptr) noexcept;
    ~PackMaster() noexcept;

    void pack(OutputFile *fo) may_throw;
    void unpack(OutputFile *fo) may_throw;
    void test() may_throw;
    void list() may_throw;
    void fileInfo() may_throw;

    typedef tribool (*visit_func_t)(PackerBase *pb, void *user);
    static PackerBase *visitAllPackers(visit_func_t, InputFile *f, const Options *, void *user)
        may_throw;

private:
    OwningPointer(PackerBase) packer = nullptr; // owner
    InputFile *const fi;                        // reference, required

    static PackerBase *getPacker(InputFile *f) may_throw;
    static PackerBase *getUnpacker(InputFile *f) may_throw;

    // setup local options for each file
    Options local_options;
    Options *saved_opt = nullptr;
};

/* vim:set ts=4 sw=4 et: */
