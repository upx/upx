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
// FIXME
**************************************************************************/

ACCLIB_PUBLIC(unsigned, acc_get_be16) (const acc_hvoid_p);
ACCLIB_PUBLIC(acc_uint32l_t, acc_get_be24) (const acc_hvoid_p);
ACCLIB_PUBLIC(acc_uint32l_t, acc_get_be32) (const acc_hvoid_p);

ACCLIB_PUBLIC(void, acc_set_be16) (acc_hvoid_p, unsigned v);
ACCLIB_PUBLIC(void, acc_set_be24) (acc_hvoid_p, acc_uint32l_t v);
ACCLIB_PUBLIC(void, acc_set_be32) (acc_hvoid_p, acc_uint32l_t v);

ACCLIB_PUBLIC(unsigned, acc_get_le16) (const acc_hvoid_p);
ACCLIB_PUBLIC(acc_uint32l_t, acc_get_le24) (const acc_hvoid_p);
ACCLIB_PUBLIC(acc_uint32l_t, acc_get_le32) (const acc_hvoid_p);

ACCLIB_PUBLIC(void, acc_set_le16) (acc_hvoid_p, unsigned v);
ACCLIB_PUBLIC(void, acc_set_le24) (acc_hvoid_p, acc_uint32l_t v);
ACCLIB_PUBLIC(void, acc_set_le32) (acc_hvoid_p, acc_uint32l_t v);

#if defined(acc_uint64l_t)
ACCLIB_PUBLIC(acc_uint64l_t, acc_get_be64) (const acc_hvoid_p);
ACCLIB_PUBLIC(void, acc_set_be64) (acc_hvoid_p, acc_uint64l_t v);

ACCLIB_PUBLIC(acc_uint64l_t, acc_get_le64) (const acc_hvoid_p);
ACCLIB_PUBLIC(void, acc_set_le64) (acc_hvoid_p, acc_uint64l_t v);
#endif


/*
vi:ts=4:et
*/
