/* file.h --

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


#ifndef __UPX_FILE_H
#define __UPX_FILE_H

class MemBuffer;


/*************************************************************************
//
**************************************************************************/

class File
{
protected:
    File() { }
    virtual ~File() { }
public:
    static void chmod(const char *name, int mode);
    static void rename(const char *old_, const char *new_);
    static void unlink(const char *name);
};


class FileBase : public File
{
protected:
    FileBase();
    virtual ~FileBase();
public:
    virtual bool close();
    virtual void closex();
    virtual bool isOpen() const { return _fd >= 0; }
    int getFd() const { return _fd; }
    const char *getName() const { return _name; }
    virtual off_t st_size() const;  // { return _length; }
    virtual void set_extent(off_t offset, off_t length);

protected:
    bool do_sopen();
    virtual int read(void *buf, int len);
    virtual int readx(void *buf, int len);
    virtual void write(const void *buf, int len);
    virtual void seek(off_t off, int whence);
    virtual off_t tell() const;

    int _fd;
    int _flags;
    int _shflags;
    int _mode;
    const char *_name;
    off_t _offset;
    off_t _length;
public:
    struct stat st;
};


/*************************************************************************
//
**************************************************************************/

class InputFile : public FileBase
{
    typedef FileBase super;
public:
    InputFile();
    virtual ~InputFile();

    virtual void sopen(const char *name, int flags, int shflags);
    virtual void open(const char *name, int flags)
    {
        sopen(name, flags, -1);
    }

    virtual int read(void *buf, int len);
    virtual int readx(void *buf, int len);
    virtual int read(MemBuffer *buf, int len);
    virtual int readx(MemBuffer *buf, int len);
    virtual int read(MemBuffer &buf, int len);
    virtual int readx(MemBuffer &buf, int len);

    virtual void seek(off_t off, int whence);
    virtual off_t tell() const;
};


/*************************************************************************
//
**************************************************************************/

class OutputFile : public FileBase
{
    typedef FileBase super;
public:
    OutputFile();
    virtual ~OutputFile();

    virtual void sopen(const char *name, int flags, int shflags, int mode);
    virtual void open(const char *name, int flags, int mode)
    {
        sopen(name, flags, -1, mode);
    }
    virtual bool openStdout(int flags=0, bool force=false);

    virtual void write(const void *buf, int len);
    virtual void write(const MemBuffer *buf, int len);
    virtual void write(const MemBuffer &buf, int len);
    virtual void set_extent(off_t offset, off_t length);
    virtual off_t unset_extent();  // returns actual length

    off_t getBytesWritten() const { return bytes_written; }
    virtual off_t st_size() const;  // { return _length; }

    // FIXME - these won't work when using the '--stdout' option
    virtual void seek(off_t off, int whence);
    virtual void rewrite(const void *buf, int len);

    // util
    static void dump(const char *name, const void *buf, int len, int flags=-1);

protected:
    off_t bytes_written;
};


/*************************************************************************
//
**************************************************************************/

#if 0 /* NOT USED */
class MemoryOutputFile : public FileBase
{
    typedef FileBase super;
public:
    MemoryOutputFile();
    virtual ~MemoryOutputFile() { b = NULL; }

    virtual bool close() { b = NULL; return true; }
    virtual bool isOpen() const { return b != NULL; }
    virtual void open(void *buf, unsigned size)
        { b = (upx_bytep) buf; b_size = size; }

    virtual void write(const void *buf, int len);

    off_t getBytesWritten() const { return bytes_written; }

protected:
    upx_bytep b;
    unsigned b_size;
    unsigned b_pos;
    off_t bytes_written;
};
#endif /* if 0 */


#endif /* already included */


/*
vi:ts=4:et
*/

