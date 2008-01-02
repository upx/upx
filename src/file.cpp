/* file.cpp --

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
#include "mem.h"


/*************************************************************************
//
**************************************************************************/

void File::chmod(const char *name, int mode)
{
#if defined(HAVE_CHMOD)
    if (::chmod(name,mode) != 0)
        throwIOException(name,errno);
#else
    UNUSED(name); UNUSED(mode);
#endif
}


void File::rename(const char *old_, const char *new_)
{
#if defined(__DJGPP__)
    if (::_rename(old_,new_) != 0)
#else
    if (::rename(old_,new_) != 0)
#endif
        throwIOException("rename error",errno);
}


void File::unlink(const char *name)
{
    if (::unlink(name) != 0)
        throwIOException(name,errno);
}


/*************************************************************************
//
**************************************************************************/

FileBase::FileBase() :
    _fd(-1), _flags(0), _shflags(0), _mode(0), _name(NULL), _offset(0), _length(0)
{
    memset(&st,0,sizeof(st));
}


FileBase::~FileBase()
{
#if 0 && defined(__GNUC__)    // debug
    if (isOpen())
        fprintf(stderr,"%s: %s\n", _name, __PRETTY_FUNCTION__);
#endif

    // FIXME: we should use close() during exception unwinding but
    //        closex() otherwise
    closex();
}


bool FileBase::do_sopen()
{
    if (_shflags < 0)
        _fd = ::open(_name, _flags, _mode);
    else
    {
#if defined(__DJGPP__)
        _fd = ::open(_name,_flags | _shflags, _mode);
#elif defined(__MINT__)
        _fd = ::open(_name,_flags | (_shflags & O_SHMODE), _mode);
#elif defined(SH_DENYRW)
        _fd = ::sopen(_name, _flags, _shflags, _mode);
#else
        throwInternalError("bad usage of do_sopen()");
#endif
    }
    if (_fd < 0)
        return false;
    if (::fstat(_fd, &st) != 0)
        throwIOException(_name, errno);
    _length = st.st_size;
    return true;
}


bool FileBase::close()
{
    bool ok = true;
    if (isOpen() && _fd != STDIN_FILENO && _fd != STDOUT_FILENO && _fd != STDERR_FILENO)
        if (::close(_fd) == -1)
            ok = false;
    _fd = -1;
    _flags = 0;
    _mode = 0;
    _name = NULL;
    _offset = 0;
    _length = 0;
    return ok;
}


void FileBase::closex()
{
    if (!close())
        throwIOException("close failed",errno);
}


int FileBase::read(void *buf, int len)
{
    if (!isOpen() || len < 0)
        throwIOException("bad read");
    errno = 0;
    long l = acc_safe_hread(_fd, buf, len);
    if (errno)
        throwIOException("read error",errno);
    return (int) l;
}


int FileBase::readx(void *buf, int len)
{
    int l = this->read(buf, len);
    if (l != len)
        throwEOFException();
    return l;
}


void FileBase::write(const void *buf, int len)
{
    if (!isOpen() || len < 0)
        throwIOException("bad write");
    errno = 0;
    long l = acc_safe_hwrite(_fd, buf, len);
    if (l != len)
        throwIOException("write error",errno);
}


void FileBase::seek(off_t off, int whence)
{
    if (!isOpen())
        throwIOException("bad seek 1");
    if (whence == SEEK_SET) {
        if (off < 0)
            throwIOException("bad seek 2");
        off += _offset;
    }
    if (whence == SEEK_END) {
        if (off > 0)
            throwIOException("bad seek 3");
        off += _offset + _length;
        whence = SEEK_SET;
    }
    if (::lseek(_fd,off,whence) < 0)
        throwIOException("seek error",errno);
}


off_t FileBase::tell() const
{
    if (!isOpen())
        throwIOException("bad tell");
    off_t l = ::lseek(_fd, 0, SEEK_CUR);
    if (l < 0)
        throwIOException("tell error",errno);
    return l - _offset;
}


void FileBase::set_extent(off_t offset, off_t length)
{
    _offset = offset;
    _length = length;
}

off_t FileBase::st_size() const
{
    return _length;
}


/*************************************************************************
//
**************************************************************************/

InputFile::InputFile()
{
}


InputFile::~InputFile()
{
}


void InputFile::sopen(const char *name, int flags, int shflags)
{
    close();
    _name = name;
    _flags = flags;
    _shflags = shflags;
    _mode = 0;
    _offset = 0;
    _length = 0;
    if (!FileBase::do_sopen())
    {
        if (errno == ENOENT)
            throw FileNotFoundException(_name, errno);
        else if (errno == EEXIST)
            throw FileAlreadyExistsException(_name, errno);
        else
            throwIOException(_name, errno);
    }
}


int InputFile::read(void *buf, int len)
{
    return super::read(buf, len);
}

int InputFile::readx(void *buf, int len)
{
    return super::readx(buf, len);
}


int InputFile::read(MemBuffer *buf, int len)
{
    buf->checkState();
    assert((unsigned)len <= buf->getSize());
    return read(buf->getVoidPtr(), len);
}

int InputFile::readx(MemBuffer *buf, int len)
{
    buf->checkState();
    assert((unsigned)len <= buf->getSize());
    return read(buf->getVoidPtr(), len);
}


int InputFile::read(MemBuffer &buf, int len)
{
    return read(&buf, len);
}

int InputFile::readx(MemBuffer &buf, int len)
{
    return readx(&buf, len);
}


void InputFile::seek(off_t off, int whence)
{
    super::seek(off,whence);
}


off_t InputFile::tell() const
{
    return super::tell();
}


/*************************************************************************
//
**************************************************************************/

OutputFile::OutputFile() :
    bytes_written(0)
{
}


OutputFile::~OutputFile()
{
}


void OutputFile::sopen(const char *name, int flags, int shflags, int mode)
{
    close();
    _name = name;
    _flags = flags;
    _shflags = shflags;
    _mode = mode;
    _offset = 0;
    _length = 0;
    if (!FileBase::do_sopen())
    {
#if 0
        // don't throw FileNotFound here -- this is confusing
        if (errno == ENOENT)
            throw FileNotFoundException(_name,errno);
        else
#endif
        if (errno == EEXIST)
            throw FileAlreadyExistsException(_name,errno);
        else
            throwIOException(_name,errno);
    }
}


bool OutputFile::openStdout(int flags, bool force)
{
    close();
    int fd = STDOUT_FILENO;
    if (!force && acc_isatty(fd))
        return false;
    _name = "<stdout>";
    _flags = flags;
    _shflags = -1;
    _mode = 0;
    _offset = 0;
    _length = 0;
    if (flags && acc_set_binmode(fd, 1) == -1)
        throwIOException(_name, errno);
    _fd = fd;
    return true;
}


void OutputFile::write(const void *buf, int len)
{
    super::write(buf, len);
    bytes_written += len;
}

off_t OutputFile::st_size() const
{
    if (opt->to_stdout) {  // might be a pipe ==> .st_size is invalid
        return bytes_written;  // too big if seek()+write() instead of rewrite()
    }
    struct stat my_st;
    if (::fstat(_fd, &my_st) != 0)
        throwIOException(_name, errno);
    return my_st.st_size;
}


void OutputFile::write(const MemBuffer *buf, int len)
{
    buf->checkState();
    assert((unsigned)len <= buf->getSize());
    write(buf->getVoidPtr(), len);
}


void OutputFile::write(const MemBuffer &buf, int len)
{
    write(&buf, len);
}

void OutputFile::rewrite(const void *buf, int len)
{
    assert(!opt->to_stdout);
    write(buf, len);
    bytes_written -= len;       // restore
}

void OutputFile::seek(off_t off, int whence)
{
    assert(!opt->to_stdout);
    super::seek(off,whence);
}

void OutputFile::set_extent(off_t offset, off_t length)
{
    super::set_extent(offset, length);
    bytes_written = 0;
    if (0==offset && (off_t)~0u==length) {
        if (::fstat(_fd, &st) != 0)
            throwIOException(_name, errno);
        _length = st.st_size - offset;
    }
}

off_t OutputFile::unset_extent()
{
    off_t l = ::lseek(_fd, 0, SEEK_END);
    if (l < 0)
        throwIOException("lseek error", errno);
    _offset = 0;
    _length = l;
    bytes_written = _length;
    return _length;
}

void OutputFile::dump(const char *name, const void *buf, int len, int flags)
{
    if (flags < 0)
         flags = O_CREAT | O_BINARY | O_TRUNC;
    flags |= O_WRONLY;
    OutputFile f;
    f.open(name, flags, 0600);
    f.write(buf, len);
    f.closex();
}


/*************************************************************************
//
**************************************************************************/

#if 0

MemoryOutputFile::MemoryOutputFile() :
    b(NULL), b_size(0), b_pos(0), bytes_written(0)
{
}


void MemoryOutputFile::write(const void *buf, int len)
{
    if (!isOpen() || len < 0)
        throwIOException("bad write");
    if (len == 0)
        return;
    if (b_pos + len > b_size)
        throwIOException("write error",ENOSPC);
    memcpy(b + b_pos, buf, len);
    b_pos += len;
    bytes_written += len;
}


#endif /* if 0 */


/*
vi:ts=4:et
*/

