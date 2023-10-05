/* file.h --

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

/*************************************************************************
//
**************************************************************************/

class FileBase {
protected:
    explicit FileBase() noexcept = default;
    virtual ~FileBase() may_throw;

public:
    bool close_noexcept() noexcept;
    void closex() may_throw;
    bool isOpen() const noexcept { return _fd >= 0; }
    int getFd() const noexcept { return _fd; }
    const char *getName() const noexcept { return _name; }

    virtual upx_off_t seek(upx_off_t off, int whence);
    upx_off_t tell() const;
    virtual upx_off_t st_size() const; // { return _length; }
    virtual void set_extent(upx_off_t offset, upx_off_t length);

public:
    // static file-related util functions; will throw on error
    static void chmod(const char *name, int mode) may_throw;
    static void rename(const char *old_, const char *new_) may_throw;
    static void unlink(const char *name) may_throw;
    static bool unlink_noexcept(const char *name) noexcept;

protected:
    bool do_sopen();
    int _fd = -1;
    int _flags = 0;
    int _shflags = 0;
    int _mode = 0;
    const char *_name = nullptr;
    upx_off_t _offset = 0;
    upx_off_t _length = 0;

public:
    struct stat st = {};
};

/*************************************************************************
//
**************************************************************************/

class InputFile final : public FileBase {
    typedef FileBase super;

public:
    explicit InputFile() noexcept = default;

    void sopen(const char *name, int flags, int shflags);
    void open(const char *name, int flags) { sopen(name, flags, -1); }

    int read(SPAN_P(void) buf, upx_int64_t blen);
    int readx(SPAN_P(void) buf, upx_int64_t blen);

    virtual upx_off_t seek(upx_off_t off, int whence) override;
    upx_off_t st_size_orig() const;

protected:
    upx_off_t _length_orig = 0;
};

/*************************************************************************
//
**************************************************************************/

class OutputFile final : public FileBase {
    typedef FileBase super;

public:
    explicit OutputFile() noexcept = default;

    void sopen(const char *name, int flags, int shflags, int mode);
    void open(const char *name, int flags, int mode) { sopen(name, flags, -1, mode); }
    bool openStdout(int flags = 0, bool force = false);

    // info: allow nullptr if blen == 0
    void write(SPAN_0(const void) buf, upx_int64_t blen);

    virtual upx_off_t seek(upx_off_t off, int whence) override;
    virtual upx_off_t st_size() const override; // { return _length; }
    virtual void set_extent(upx_off_t offset, upx_off_t length) override;
    upx_off_t unset_extent(); // returns actual length

    upx_off_t getBytesWritten() const { return bytes_written; }

    // FIXME - this won't work when using the '--stdout' option
    void rewrite(SPAN_P(const void) buf, int len);

    // util
    static void dump(const char *name, SPAN_P(const void) buf, int len, int flags = -1);

protected:
    upx_off_t bytes_written = 0;
};

/* vim:set ts=4 sw=4 et: */
