/* ACC -- Automatic Compiler Configuration

   Copyright (C) 1996-2003 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   This software is a copyrighted work licensed under the terms of
   the GNU General Public License. Please consult the file "ACC_LICENSE"
   for details.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
   http://www.oberhumer.com/
 */


#if !defined(ACCLIB_PUBLIC)
#  define ACCLIB_PUBLIC(r,f)    r __ACCLIB_FUNCNAME(f)
#endif


/*************************************************************************
//
**************************************************************************/

ACCLIB_PUBLIC(unsigned, acc_get_be16) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((unsigned)b[1]) | ((unsigned)b[0] << 8);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_get_be24) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[2]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[0] << 16);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_get_be32) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[3]) | ((acc_uint32l_t)b[2] << 8) | ((acc_uint32l_t)b[1] << 16) | ((acc_uint32l_t)b[0] << 24);
}



ACCLIB_PUBLIC(void, acc_set_be16) (acc_hvoid_p p, unsigned v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[1] = (unsigned char) ((v >>  0) & 0xff);
    b[0] = (unsigned char) ((v >>  8) & 0xff);
}

ACCLIB_PUBLIC(void, acc_set_be24) (acc_hvoid_p p, acc_uint32l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[2] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[0] = (unsigned char) ((v >> 16) & 0xff);
}

ACCLIB_PUBLIC(void, acc_set_be32) (acc_hvoid_p p, acc_uint32l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[3] = (unsigned char) ((v >>  0) & 0xff);
    b[2] = (unsigned char) ((v >>  8) & 0xff);
    b[1] = (unsigned char) ((v >> 16) & 0xff);
    b[0] = (unsigned char) ((v >> 24) & 0xff);
}



ACCLIB_PUBLIC(unsigned, acc_get_le16) (const acc_hvoid_p p)
{
#if (ACC_ARCH_IA32)
    return (* (const unsigned short *) (p));
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((unsigned)b[0]) | ((unsigned)b[1] << 8);
#endif
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_get_le24) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16);
}

ACCLIB_PUBLIC(acc_uint32l_t, acc_get_le32) (const acc_hvoid_p p)
{
#if (ACC_ARCH_IA32)
    return (* (const unsigned int *) (p));
#else
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint32l_t)b[0]) | ((acc_uint32l_t)b[1] << 8) | ((acc_uint32l_t)b[2] << 16) | ((acc_uint32l_t)b[3] << 24);
#endif
}



ACCLIB_PUBLIC(void, acc_set_le16) (acc_hvoid_p p, unsigned v)
{
#if (ACC_ARCH_IA32)
    (* (unsigned short *) (p) = (unsigned short) (v));
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
#endif
}

ACCLIB_PUBLIC(void, acc_set_le24) (acc_hvoid_p p, acc_uint32l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[2] = (unsigned char) ((v >> 16) & 0xff);
}

ACCLIB_PUBLIC(void, acc_set_le32) (acc_hvoid_p p, acc_uint32l_t v)
{
#if (ACC_ARCH_IA32)
    (* (unsigned int *) (p) = (unsigned int) (v));
#else
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[2] = (unsigned char) ((v >> 16) & 0xff);
    b[3] = (unsigned char) ((v >> 24) & 0xff);
#endif
}



#if defined(acc_uint64l_t)

ACCLIB_PUBLIC(acc_uint64l_t, acc_get_be64) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint64l_t)b[0]) | ((acc_uint64l_t)b[1] << 8) | ((acc_uint64l_t)b[2] << 16) | ((acc_uint64l_t)b[3] << 24) | ((acc_uint64l_t)b[4] << 32) | ((acc_uint64l_t)b[5] << 40) | ((acc_uint64l_t)b[6] << 48) | ((acc_uint64l_t)b[7] << 56);
}

ACCLIB_PUBLIC(void, acc_set_be64) (acc_hvoid_p p, acc_uint64l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[7] = (unsigned char) ((v >>  0) & 0xff);
    b[6] = (unsigned char) ((v >>  8) & 0xff);
    b[5] = (unsigned char) ((v >> 16) & 0xff);
    b[4] = (unsigned char) ((v >> 24) & 0xff);
    b[3] = (unsigned char) ((v >> 32) & 0xff);
    b[2] = (unsigned char) ((v >> 40) & 0xff);
    b[1] = (unsigned char) ((v >> 48) & 0xff);
    b[0] = (unsigned char) ((v >> 56) & 0xff);
}



ACCLIB_PUBLIC(acc_uint64l_t, acc_get_le64) (const acc_hvoid_p p)
{
    const acc_hbyte_p b = (const acc_hbyte_p) p;
    return ((acc_uint64l_t)b[7]) | ((acc_uint64l_t)b[6] << 8) | ((acc_uint64l_t)b[5] << 16) | ((acc_uint64l_t)b[4] << 24) | ((acc_uint64l_t)b[3] << 32) | ((acc_uint64l_t)b[2] << 40) | ((acc_uint64l_t)b[1] << 48) | ((acc_uint64l_t)b[0] << 56);
}

ACCLIB_PUBLIC(void, acc_set_le64) (acc_hvoid_p p, acc_uint64l_t v)
{
    acc_hbyte_p b = (acc_hbyte_p) p;
    b[0] = (unsigned char) ((v >>  0) & 0xff);
    b[1] = (unsigned char) ((v >>  8) & 0xff);
    b[2] = (unsigned char) ((v >> 16) & 0xff);
    b[3] = (unsigned char) ((v >> 24) & 0xff);
    b[4] = (unsigned char) ((v >> 32) & 0xff);
    b[5] = (unsigned char) ((v >> 40) & 0xff);
    b[6] = (unsigned char) ((v >> 48) & 0xff);
    b[7] = (unsigned char) ((v >> 56) & 0xff);
}


#endif /* acc_uint64l_t */


/*
vi:ts=4:et
*/
