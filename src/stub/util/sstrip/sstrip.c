/* sstrip, version 1.0: Copyright (C) 1999 by Brian Raiter, under the
 * GNU General Public License. No warranty. See COPYING for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <unistd.h>
#include <linux/elf.h>

#ifndef TRUE
#define TRUE            1
#define FALSE           0
#endif

/* The memory-allocation macro.
 */
#define alloc(p, n)     (((p) = realloc(p, n))                          \
                            || (fputs("Out of memory.\n", stderr),      \
                                exit(EXIT_FAILURE), 0))

static char const      *thefilename;    /* the current file name        */
static FILE            *thefile;        /* the current file handle      */

static Elf32_Ehdr       elfhdr;         /* original ELF header          */
static Elf32_Phdr      *phdrs = NULL;   /* original program header tbl  */
static unsigned long    phdrsize;       /* size of program header tbl   */
static unsigned long    newsize;        /* size of the new file         */

/* An error-handling function. The given error message is used only
 * when errno is not set.
 */
static int err(char const *errmsg)
{
    if (errno)
        perror(thefilename);
    else
        fprintf(stderr, "%s: %s\n", thefilename, errmsg);
    return FALSE;
}

/* readheaders() reads the ELF header and the program header table,
 * and checks to make sure that this is in fact a file that we should
 * be munging.
 */
static int readheaders(void)
{
    int         bigend;

    errno = 0;
    if (fread(&elfhdr, sizeof elfhdr, 1, thefile) != 1)
        return err("not an ELF file.");
    if (elfhdr.e_ident[EI_MAG0] != ELFMAG0
                || elfhdr.e_ident[EI_MAG1] != ELFMAG1
                || elfhdr.e_ident[EI_MAG2] != ELFMAG2
                || elfhdr.e_ident[EI_MAG3] != ELFMAG3)
        return err("not an ELF file.");

    bigend = TRUE;
    *(char*)&bigend = 0;
    if (elfhdr.e_ident[EI_DATA] != (bigend ? ELFDATA2MSB : ELFDATA2LSB)) {
        fprintf(stderr, "%s: not %s-endian.\n",
                        thefilename, bigend ? "big" : "little");
        return FALSE;
    }
    if (elfhdr.e_ehsize != sizeof(Elf32_Ehdr)) {
        fprintf(stderr, "%s: unrecognized ELF header size "
                        "(size = %u instead of %u).\n",
                        thefilename, elfhdr.e_ehsize, sizeof(Elf32_Ehdr));
        return FALSE;
    }
    if (!elfhdr.e_phoff)
        return err("no program header table.");
    if (elfhdr.e_phentsize != sizeof(Elf32_Phdr)) {
        fprintf(stderr, "%s: unrecognized program header size "
                        "(size = %u instead of %u).\n",
                        thefilename, elfhdr.e_phentsize, sizeof(Elf32_Ehdr));
        return FALSE;
    }

    phdrsize = elfhdr.e_phnum * elfhdr.e_phentsize;
    alloc(phdrs, phdrsize);
    errno = 0;
    if (fread(phdrs, phdrsize, 1, thefile) != 1)
        return err("invalid program header table.");

    return TRUE;
}

/* getloadsize() determines the offset of the last byte of the file
 * that is actually loaded into memory. Anything after this point can
 * be safely discarded.
 */
static int getloadsize(void)
{
    Elf32_Phdr         *phdr;
    unsigned long       n;
    int                 i;

    newsize = elfhdr.e_phoff + phdrsize;
    phdr = phdrs;
    for (i = 0 ; i < elfhdr.e_phnum ; ++i) {
        if (phdr->p_type == PT_NULL || phdr->p_type == PT_NOTE)
            continue;
        n = phdr->p_offset + phdr->p_filesz;
        if (n > newsize)
            newsize = n;
        phdr = (Elf32_Phdr*)((char*)phdr + elfhdr.e_phentsize);
    }

    for (i = 0 ; i < elfhdr.e_phnum ; ++i)
        if (phdr->p_filesz > 0 && phdr->p_offset >= newsize)
            memset(phdr, 0, elfhdr.e_phentsize);

    return TRUE;
}

/* truncatezeros() examines the bytes at the end of the file's
 * size-to-be, and reduces the size to exclude trailing zero bytes.
 */
static int truncatezeros(void)
{
    char                contents[1024];
    unsigned long       n;

    do {
        n = sizeof contents;
        if (n > newsize)
            n = newsize;
        if (fseek(thefile, newsize - n, SEEK_SET)
                || fread(contents, n, 1, thefile) != 1)
            return err("cannot read file contents");
        while (n && !contents[--n])
            --newsize;
    } while (newsize && !n);

    return TRUE;
}

/* modifyheaders() removes references to the section header table if
 * it was removed, and reduces program header table entries that
 * included truncated bytes at the end of the file.
 */
static int modifyheaders(void)
{
    Elf32_Phdr *phdr;
    int         i;

    if (elfhdr.e_shoff >= newsize) {
        elfhdr.e_shoff = 0;
        elfhdr.e_shnum = 0;
        elfhdr.e_shentsize = 0;
        elfhdr.e_shstrndx = 0;
    }

    phdr = phdrs;
    for (i = 0 ; i < elfhdr.e_phnum ; ++i) {
        if (phdr->p_offset + phdr->p_filesz > newsize) {
            if (phdr->p_offset >= newsize)
                phdr->p_filesz = 0;
            else
                phdr->p_filesz = newsize - phdr->p_offset;
        }
        phdr = (Elf32_Phdr*)((char*)phdr + elfhdr.e_phentsize);
    }

    return TRUE;
}

/* savestripped() writes the new headers back to the original file
 * and sets the new file size.
 */
static int savestripped(void)
{
    rewind(thefile);

    errno = 0;
    if (fwrite(&elfhdr, sizeof elfhdr, 1, thefile) != 1
                || fwrite(phdrs, phdrsize, 1, thefile) != 1
                || ftruncate(fileno(thefile), newsize)) {
        err("could not write contents");
        fprintf(stderr, "WARNING: %s may be corrupted!\n", thefilename);
        return FALSE;
    }

    return TRUE;
}

/* main() loops over the cmdline arguments, leaving all the real work
 * to the other functions.
 */
int main(int argc, char *argv[])
{
    char              **arg;
    int                 ret = 0;

    if (argc < 2 || !strcmp(argv[1], "-h")) {
        printf("sstrip, version 2.0: Copyright (C) 1999 Brian Raiter\n"
               "Usage: sstrip FILE...\n");
        return 0;
    }

    for (arg = argv + 1 ; (thefilename = *arg) != NULL ; ++arg) {
        if (!(thefile = fopen(thefilename, "rb+"))) {
            err("unable to open.");
            ++ret;
            continue;
        }
        if (!readheaders() || !getloadsize() || !truncatezeros()
                           || !modifyheaders() || !savestripped())
            ++ret;
        fclose(thefile);
    }

    return ret;
}


/*
vi:ts=8:et:nowrap
*/

