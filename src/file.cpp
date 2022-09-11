/* file.cpp --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2022 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2022 Laszlo Molnar
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
#include "file.h"

/*************************************************************************
// static functions
**************************************************************************/

void FileBase::chmod(const char *name, int mode) {
#if (HAVE_CHMOD)
    if (::chmod(name, mode) != 0)
        throwIOException(name, errno);
#else
    UNUSED(name);
    UNUSED(mode);
#endif
}

void FileBase::rename(const char *old_, const char *new_) {
#if (ACC_OS_DOS32) && defined(__DJGPP__)
    if (::_rename(old_, new_) != 0)
#else
    if (::rename(old_, new_) != 0)
#endif
        throwIOException("rename error", errno);
}

void FileBase::unlink(const char *name) {
    if (::unlink(name) != 0)
        throwIOException(name, errno);
}

/*************************************************************************
//
**************************************************************************/

FileBase::FileBase()
    : _fd(-1), _flags(0), _shflags(0), _mode(0), _name(nullptr), _offset(0), _length(0) {
    memset(&st, 0, sizeof(st));
}

FileBase::~FileBase() {
#if 0 && defined(__GNUC__) // debug
    if (isOpen())
        fprintf(stderr,"%s: %s\n", _name, __PRETTY_FUNCTION__);
#endif

    // FIXME: we should use close() during exception unwinding but
    //        closex() otherwise
    closex();
}

bool FileBase::do_sopen() {
    if (_shflags < 0)
        _fd = ::open(_name, _flags, _mode);
    else {
#if (ACC_OS_DOS32) && defined(__DJGPP__)
        _fd = ::open(_name, _flags | _shflags, _mode);
#elif defined(__MINT__)
        _fd = ::open(_name, _flags | (_shflags & O_SHMODE), _mode);
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

bool FileBase::close() {
    bool ok = true;
    if (isOpen() && _fd != STDIN_FILENO && _fd != STDOUT_FILENO && _fd != STDERR_FILENO)
        if (::close(_fd) == -1)
            ok = false;
    _fd = -1;
    _flags = 0;
    _mode = 0;
    _name = nullptr;
    _offset = 0;
    _length = 0;
    return ok;
}

void FileBase::closex() {
    if (!close())
        throwIOException("close failed", errno);
}

upx_off_t FileBase::seek(upx_off_t off, int whence) {
    if (!isOpen())
        throwIOException("bad seek 1");
    mem_size_assert(1, off >= 0 ? off : -off); // sanity check
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
    if (::lseek(_fd, off, whence) < 0)
        throwIOException("seek error", errno);
    return off - _offset;
}

upx_off_t FileBase::tell() const {
    if (!isOpen())
        throwIOException("bad tell");
    upx_off_t l = ::lseek(_fd, 0, SEEK_CUR);
    if (l < 0)
        throwIOException("tell error", errno);
    return l - _offset;
}

void FileBase::set_extent(upx_off_t offset, upx_off_t length) {
    _offset = offset;
    _length = length;
}

upx_off_t FileBase::st_size() const { return _length; }

/*************************************************************************
//
**************************************************************************/

InputFile::InputFile() {}

void InputFile::sopen(const char *name, int flags, int shflags) {
    close();
    _name = name;
    _flags = flags;
    _shflags = shflags;
    _mode = 0;
    _offset = 0;
    _length = 0;
    if (!super::do_sopen()) {
        if (errno == ENOENT)
            throw FileNotFoundException(_name, errno);
        else if (errno == EEXIST)
            throw FileAlreadyExistsException(_name, errno);
        else
            throwIOException(_name, errno);
    }
    _length_orig = _length;
}

int InputFile::read(SPAN_P(void) buf, int len) {
    if (!isOpen() || len < 0)
        throwIOException("bad read");
    mem_size_assert(1, len); // sanity check
    errno = 0;
    long l = acc_safe_hread(_fd, raw_bytes(buf, len), len);
    if (errno)
        throwIOException("read error", errno);
    return (int) l;
}

int InputFile::readx(SPAN_P(void) buf, int len) {
    int l = this->read(buf, len);
    if (l != len)
        throwEOFException();
    return l;
}

upx_off_t InputFile::seek(upx_off_t off, int whence) {
    upx_off_t pos = super::seek(off, whence);
    if (_length < pos)
        throwIOException("bad seek 4");
    return pos;
}

upx_off_t InputFile::st_size_orig() const { return _length_orig; }

/*************************************************************************
//
**************************************************************************/

OutputFile::OutputFile() : bytes_written(0) {}

void OutputFile::sopen(const char *name, int flags, int shflags, int mode) {
    close();
    _name = name;
    _flags = flags;
    _shflags = shflags;
    _mode = mode;
    _offset = 0;
    _length = 0;
    if (!super::do_sopen()) {
#if 0
        // don't throw FileNotFound here -- this is confusing
        if (errno == ENOENT)
            throw FileNotFoundException(_name,errno);
        else
#endif
        if (errno == EEXIST)
            throw FileAlreadyExistsException(_name, errno);
        else
            throwIOException(_name, errno);
    }
}

bool OutputFile::openStdout(int flags, bool force) {
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

void OutputFile::write(SPAN_0(const void) buf, int len) {
    if (!isOpen() || len < 0)
        throwIOException("bad write");
    // allow nullptr if len == 0
    if (len == 0)
        return;
    mem_size_assert(1, len); // sanity check
    errno = 0;
#if 0
    fprintf(stderr, "write %p %zd (%p) %d\n", buf.raw_ptr(), buf.raw_size_in_bytes(),
            buf.raw_base(), len);
#endif
    long l = acc_safe_hwrite(_fd, raw_bytes(buf, len), len);
    if (l != len)
        throwIOException("write error", errno);
    bytes_written += len;
}

upx_off_t OutputFile::st_size() const {
    if (opt->to_stdout) {     // might be a pipe ==> .st_size is invalid
        return bytes_written; // too big if seek()+write() instead of rewrite()
    }
    struct stat my_st;
    my_st.st_size = 0;
    if (::fstat(_fd, &my_st) != 0)
        throwIOException(_name, errno);
    return my_st.st_size;
}

void OutputFile::rewrite(SPAN_P(const void) buf, int len) {
    assert(!opt->to_stdout);
    write(buf, len);
    bytes_written -= len; // restore
}

upx_off_t OutputFile::seek(upx_off_t off, int whence) {
    mem_size_assert(1, off >= 0 ? off : -off); // sanity check
    assert(!opt->to_stdout);
    switch (whence) {
    case SEEK_SET: {
        if (bytes_written < off) {
            bytes_written = off;
        }
        _length = bytes_written; // cheap, lazy update; needed?
    } break;
    case SEEK_END: {
        _length = bytes_written; // necessary
    } break;
    }
    return super::seek(off, whence);
}

// WARNING: fsync() does not exist in some Windows environments.
// This trick works only on UNIX-like systems.
// int OutputFile::read(void *buf, int len)
//{
//    fsync(_fd);
//    InputFile infile;
//    infile.open(this->getName(), O_RDONLY | O_BINARY);
//    infile.seek(this->tell(), SEEK_SET);
//    return infile.read(buf, len);
//}

void OutputFile::set_extent(upx_off_t offset, upx_off_t length) {
    super::set_extent(offset, length);
    bytes_written = 0;
    if (0 == offset && (upx_off_t) ~0u == length) {
        if (::fstat(_fd, &st) != 0)
            throwIOException(_name, errno);
        _length = st.st_size - offset;
    }
}

upx_off_t OutputFile::unset_extent() {
    upx_off_t l = ::lseek(_fd, 0, SEEK_END);
    if (l < 0)
        throwIOException("lseek error", errno);
    _offset = 0;
    _length = l;
    bytes_written = _length;
    return _length;
}

void OutputFile::dump(const char *name, SPAN_P(const void) buf, int len, int flags) {
    if (flags < 0)
        flags = O_CREAT | O_TRUNC;
    flags |= O_WRONLY | O_BINARY;
    OutputFile f;
    f.open(name, flags, 0600);
    f.write(raw_bytes(buf, len), len);
    f.closex();
}

/* vim:set ts=4 sw=4 et: */
