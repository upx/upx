/* console.h --

   This file is part of the UPX executable compressor.

   Copyright (C) 1996-2016 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996-2016 Laszlo Molnar
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



/*************************************************************************
//
**************************************************************************/

#undef USE_CONSOLE
#undef USE_ANSI
#undef USE_SCREEN
#undef USE_SCREEN_CURSES
#undef USE_SCREEN_VCSA
#undef USE_SCREEN_WIN32
#undef USE_FRAMES

#if 1 && (WITH_GUI) && !defined(NO_CONSOLE)
#  define NO_CONSOLE 1
#endif

#if 1 && defined(__linux__)
#  define USE_SCREEN 1
#  define USE_SCREEN_VCSA 1
#  if !(HAVE_LINUX_KD_H)
#    undef USE_SCREEN
#    undef USE_SCREEN_VCSA
#  endif
#  if !(HAVE_LINUX_KDEV_T_H) || !(HAVE_LINUX_MAJOR_H)
#    undef USE_SCREEN
#    undef USE_SCREEN_VCSA
#  endif
#endif

#if 1 && defined(__DJGPP__)
#  define USE_SCREEN 1
#elif 1 && (ACC_OS_CYGWIN || ACC_OS_WIN32 || ACC_OS_WIN64)
#  define USE_SCREEN 1
#  define USE_SCREEN_WIN32 1
#elif 1 && (ACC_OS_EMX && defined(__RSXNT__))
#  define USE_SCREEN 1
#  define USE_SCREEN_WIN32 1
#endif


#if 0 || (NO_ANSI)
#  undef USE_ANSI
#endif
#if 0 || (NO_SCREEN)
#  undef USE_SCREEN
#endif
#if 0 || (NO_FRAMES) || !(USE_SCREEN)
#  undef USE_FRAMES
#endif
#if 1
#  undef USE_FRAMES
#endif


#if 0 || (USE_ANSI) || (USE_SCREEN)
#  define USE_CONSOLE 1
#endif

#if 0 || (NO_CONSOLE) || !(USE_CONSOLE)
#  undef USE_CONSOLE
#  undef USE_ANSI
#  undef USE_SCREEN
#  undef USE_SCREEN_VCSA
#  undef USE_SCREEN_CURSES
#  undef USE_FRAMES
#endif


/*************************************************************************
//
**************************************************************************/

enum {
    CON_INIT,
    CON_NONE,
    CON_FILE,
    CON_ANSI_MONO,
    CON_ANSI_COLOR,
    CON_SCREEN,
    CON_UNUSED
};


#if (USE_CONSOLE)

typedef struct
{
    int (*init)(FILE *f, int, int);
    int (*set_fg)(FILE *f, int fg);
    void (*print0)(FILE *f, const char *s);
    bool (*intro)(FILE *f);
}
console_t;


#if defined(__GNUC__)
void __acc_cdecl_va con_fprintf(FILE *f, const char *format, ...)
        __attribute__((__format__(printf,2,3)));
#else
void __acc_cdecl_va con_fprintf(FILE *f, const char *format, ...);
#endif


#define FG_BLACK     0x00
#define FG_BLUE      0x01
#define FG_GREEN     0x02
#define FG_CYAN      0x03
#define FG_RED       0x04
#define FG_VIOLET    0x05
#define FG_ORANGE    0x06
#define FG_LTGRAY    0x07
#define FG_DKGRAY    0x08
#define FG_BRTBLUE   0x09
#define FG_BRTGREEN  0x0a
#define FG_BRTCYAN   0x0b
#define FG_BRTRED    0x0c
#define FG_BRTVIOLET 0x0d
#define FG_YELLOW    0x0e
#define FG_WHITE     0x0f

#define BG_BLACK     0x00
#define BG_BLUE      0x10
#define BG_GREEN     0x20
#define BG_CYAN      0x30
#define BG_RED       0x40
#define BG_VIOLET    0x50
#define BG_ORANGE    0x60
#define BG_WHITE     0x70

#endif /* USE_CONSOLE */


/*************************************************************************
//
**************************************************************************/

extern FILE *con_term;

#if (USE_CONSOLE)

extern int con_mode;
extern console_t *con;

extern console_t console_init;
extern console_t console_none;
extern console_t console_file;
extern console_t console_ansi_mono;
extern console_t console_ansi_color;
extern console_t console_screen;


#define con_fg(f,x)     con->set_fg(f,x)

#else

#define con_fg(f,x)     0
#define con_fprintf     fprintf

#endif /* USE_CONSOLE */

/* vim:set ts=4 sw=4 et: */
