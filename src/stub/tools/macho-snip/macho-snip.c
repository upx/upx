// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright 2022 BitWagon Software LLC.  All rights reserved.

/* clang-format off */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#ifdef __APPLE__  //{
// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/mach-o/loader.h
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#else  //}{
    enum { // cmd
        LC_REQ_DYLD      = 0x80000000,  // OR'ed ==> must not ignore
        LC_SEGMENT       = 0x1,
        LC_SYMTAB        = 0x2,
        LC_THREAD        = 0x4,
        LC_UNIXTHREAD    = 0x5,
        LC_DYSYMTAB      = 0xb,
        LC_LOAD_DYLIB    = 0xc,
        LC_ID_DYLIB      = 0xd,
        LC_LOAD_DYLINKER = 0xe,
        LC_ID_DYLINKER   = 0xf,
        LC_ROUTINES      = 0x11,
        LC_TWOLEVEL_HINTS= 0x16,
        LC_LOAD_WEAK_DYLIB= (0x18 | LC_REQ_DYLD),
        LC_SEGMENT_64    = 0x19,
        LC_ROUTINES_64   = 0x1a,
        LC_UUID          = 0x1b,
        LC_RPATH         = 0x1c,
        LC_CODE_SIGNATURE = 0x1d,
        LC_SEGMENT_SPLIT_INFO = 0x1e,
        LC_REEXPORT_DYLIB = (0x1f | LC_REQ_DYLD),
        LC_LAZY_LOAD_DYLIB= 0x20,
        LC_ENCRYPTION_INFO= 0x21,
        LC_DYLD_INFO      = 0x22,  // compressed dyld information (10.6.x)
        LC_DYLD_INFO_ONLY = (0x22|LC_REQ_DYLD),
        LC_VERSION_MIN_MACOSX= 0x24,
        LC_VERSION_MIN_IPHONEOS= 0x25,
        LC_FUNCTION_STARTS= 0x26,
        LC_DYLD_ENVIRONMENT= 0x27,  // string as environment variable
        LC_MAIN           = (0x28|LC_REQ_DYLD),
        LC_DATA_IN_CODE   = 0x29,
        LC_SOURCE_VERSION = 0x2a,
        LC_DYLIB_CODE_SIGN_DRS= 0x2B,
        LC_ENCRYPTION_INFO_64= 0x2C,
        LC_LINKER_OPTIMIZATION_HINT = 0x2E,
        LC_VERSION_MIN_TVOS= 0x2F,
        LC_VERSION_MIN_WATCHOS= 0x30,
        LC_NOTE           = 0x31,
        LC_BUILD_VERSION  = 0x32,
        LC_DYLD_EXPORTS_TRIE   = (0x33|LC_REQ_DYLD),
        LC_DYLD_CHAINED_FIXUPS = (0x34|LC_REQ_DYLD),
        LC_FILESET_ENTRY       = (0x35|LC_REQ_DYLD),
    };

#include <stdint.h>
struct mach_header_64 {
    uint32_t magic;
    uint32_t cputype;
    uint32_t cpusubtype;
    uint32_t filetype;
    uint32_t ncmds;
    uint32_t sizeofcmds;
    uint32_t flags;
    uint32_t reserved;
};
struct load_command {
    uint32_t cmd;
    uint32_t cmdsize;
};
struct segment_command_64 {
    uint32_t cmd;
    uint32_t cmdsize;
    char     segname[16];
    uint64_t vmaddr;
    uint64_t vmsize;
    uint64_t fileoff;
    uint64_t filesize;
    uint32_t maxprot;
    uint32_t initprot;
    uint32_t nsects;
    uint32_t flags;
};
struct linkedit_data_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t dataoff;
    uint32_t datasize;
};
struct symtab_command {
    uint32_t cmd;
    uint32_t cmdsize;
    uint32_t symoff;
    uint32_t nsyms;
    uint32_t stroff;
    uint32_t strsize;
};
struct dysymtab_command {
    uint32_t cmd;
    uint32_t cmdsize;

    uint32_t ilocalsym;
    uint32_t nlocalsym;

    uint32_t iextdefsym;
    uint32_t nextdefsym;

    uint32_t iundefsym;
    uint32_t nundefsym;

    uint32_t tocoff;
    uint32_t ntoc;

    uint32_t modtaboff;
    uint32_t nmodtab;

    uint32_t extrefsymoff;
    uint32_t nextrefsyms;

    uint32_t indirectsymoff;
    uint32_t nindirectsyms;

    uint32_t extreloff;
    uint32_t nextrel;

    uint32_t locreloff;
    uint32_t nlocrel;
};
struct nlist_64 {
    union {
        uint32_t  n_strx;
    } n_un;
    uint8_t n_type;
    uint8_t n_sect;
    uint16_t n_desc;
    uint64_t n_value;
};
#endif  //}

struct Cmd_names {
    unsigned char val;
    char name[23];
} const cmd_names[] = {
    [ 0x1]  = { 0x1, "LC_SEGMENT"},
    [ 0x2]  = { 0x2, "LC_SYMTAB"},
    [ 0x4]  = { 0x4, "LC_THREAD"},
    [ 0x5]  = { 0x5, "LC_UNIXTHREAD"},
    [ 0xb]  = { 0xb, "LC_DYSYMTAB"},
    [ 0xc]  = { 0xc, "LC_LOAD_DYLIB"},
    [ 0xd]  = { 0xd, "LC_ID_DYLIB"},
    [ 0xe]  = { 0xe, "LC_LOAD_DYLINKER"},
    [ 0xf]  = { 0xf, "LC_ID_DYLINKER"},
    [ 0x11]  = { 0x11, "LC_ROUTINES"},
    [ 0x16]  = { 0x16, "LC_TWOLEVEL_HINTS"},
    [ (0x18 /*|LC_REQ_DYLD*/ )] = {0x18, "LC_LOAD_WEAK_DYLIB"},
    [ 0x19]  = { 0x19, "LC_SEGMENT_64"},
    [ 0x1a]  = { 0x1a, "LC_ROUTINES_64"},
    [ 0x1b]  = { 0x1b, "LC_UUID"},
    [ 0x1c]  = { 0x1c, "LC_RPATH"},
    [ 0x1d]  = { 0x1d, "LC_CODE_SIGNATURE"},
    [ 0x1e]  = { 0x1e, "LC_SEGMENT_SPLIT_INFO"},
    [ (0x1f /*|LC_REQ_DYLD*/ )] = { 0x1f, "LC_REEXPORT_DYLIB"},
    [ 0x20]  = { 0x20, "LC_LAZY_LOAD_DYLIB"},
    [ 0x21]  = { 0x21, "LC_ENCRYPTION_INFO"},
//  [ 0x22]  = { 0x22, "LC_DYLD_INFO"  // compressed dyld information (10.6.x)
    [ (0x22 /*|LC_REQ_DYLD*/ )]  = { 0x22, "LC_DYLD_INFO_ONLY"},
    [ 0x24]  = { 0x24, "LC_VERSION_MIN_MACOSX"},
    [ 0x25]  = { 0x25, "LC_VERSION_MIN_IPHONEOS"},
    [ 0x26]  = { 0x26, "LC_FUNCTION_STARTS"},
    [ 0x27]  = { 0x27, "LC_DYLD_ENVIRONMENT"},  // string as environment variable
    [ (0x28 /*|LC_REQ_DYLD*/ )]  = { 0x28, "LC_MAIN"},
    [ 0x29]  = { 0x29, "LC_DATA_IN_CODE"},
    [ 0x2a]  = { 0x2a, "LC_SOURCE_VERSION"},
    [ 0x2B]  = { 0x2B, "LC_DYLIB_CODE_SIGN_DRS"},
    [ 0x2C]  = { 0x2C, "LC_ENCRYPTION_INFO_64"},
    [ 0x2F]  = { 0x2F, "LC_VERSION_MIN_TVOS"},
    [ 0x30]  = { 0x30, "LC_VERSION_MIN_WATCHOS"},
    [ 0x31]  = { 0x31, "LC_NOTE"},
    [ 0x32]  = { 0x32, "LC_BUILD_VERSION"},
    [(0x33 /*|LC_REQ_DYLD*/ )]  = {0x33, "LC_DYLD_EXPORTS_TRIE"},
    [(0x34 /*|LC_REQ_DYLD*/ )]  = {0x34, "LC_DYLD_CHAINED_FIXUPS"},
    [(0x35 /*|LC_REQ_DYLD*/ )]  = {0x35, "LC_FILESET_ENTRY"},
};

// Remove (cut out, "snip") named loader_commands from a Macho-O file.
// Try to enable success of running "codesign -s - file" afterwards.
// Note that LC_CODE_SIGNATURE should be removed before snipping:
//   codesign --remove-signature file
//
// This is EXPERIMENTAL to aid in finding a "minimal" executable
// on Apple MacOS Big Sur, particularly Apple M1 hardware (aarch64).
int
main(int argc, char const * /*const*/ *const argv, char const *const *const envp)
{
    struct stat st;
    int fd;
    int err1;
    int res2;
    int err2;
    int prot = PROT_READ | PROT_WRITE;
    int flags = MAP_FIXED | MAP_SHARED;
    void *const awant = (void *)(0x18L << 28);  // above user, below dylibs
    char *addr;

    if (argc < 3) {
        fprintf(stderr, "Usage: macho-snip file loader_cmd...\n");
        exit(1);
    }
    fd = open(argv[1], O_RDWR, 0);
    err1 = errno;
    if (fd < 0) {
        perror(argv[1]);
        fprintf(stderr, "Trying readonly...\n");
        flags = MAP_FIXED | MAP_PRIVATE;
        fd = open(argv[1], O_RDONLY, 0);
    }
    res2 = fstat(fd, &st);
    err2 = errno;

    if (fd < 0) {
        errno = err1;
        perror(argv[1]);
        exit(1);
    }
    if (0!=res2) {
        errno = err2;
        perror(argv[1]);
        exit(1);
    }
    addr = mmap(awant, st.st_size, prot, flags, fd, 0);
    if (awant!=addr) {
        perror(argv[1]);
        exit(1);
    }
    fprintf(stderr,"%zd (%#zx) bytes at %p\n", (long)st.st_size, (long)st.st_size, addr);

    unsigned long argv_done = 0;  // set of bits
    struct segment_command_64 *linkedit = 0;
    struct mach_header_64 *const mhdr = (struct mach_header_64 *)addr;
    unsigned ncmds = mhdr->ncmds;
    unsigned headway = mhdr->sizeofcmds;
    struct load_command *cmd = (struct load_command *)(1+ mhdr);
    struct load_command *cmd_next;
    unsigned delta_dataoff = 0;
    for (; ncmds; --ncmds, cmd = cmd_next) {
        unsigned end_dataoff = 0;
        unsigned end_datasize = 0;
again: ;
        fprintf(stderr, "cmd@%p %s %d(%#x)\n",
            cmd, cmd_names[cmd->cmd&0xFF].name, cmd->cmd&0xFF, cmd->cmd);
        unsigned const cmdsize = cmd->cmdsize;
        if (headway < cmdsize ) {
        }
        else {
            headway -= cmdsize;
            cmd_next = (struct load_command *)(cmdsize + (void *)cmd);
        }
        switch (cmd->cmd &~ LC_REQ_DYLD) {
            int jargv;

        case LC_SEGMENT_64: {
            struct segment_command_64 *seg = (struct segment_command_64 *)cmd;
            if (!strcmp("__LINKEDIT", seg->segname)) {
                linkedit = seg;
            }
        } break;
        case LC_CODE_SIGNATURE: {
            fprintf(stderr, "macho-snip: use 'codesign --remove-signature' to remove LC_CODE_SIGNATURE\n");
        } break;

//struct nlist_64 {
//    union {
//        uint32_t  n_strx; /* index into the string table */
//    } n_un;
//    uint8_t n_type;        /* type flag, see below */
//    uint8_t n_sect;        /* section number or NO_SECT */
//    uint16_t n_desc;       /* see <mach-o/stab.h> */
//    uint64_t n_value;      /* value of this symbol (or stab offset) */
//};
//
// The string table has an extra entry at the front: " " (one space)
// so that the first actual string has an index of 2.
// See the comment which follows the definition of struct nlist_64 in
// /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/mach-o/nlist.h
//
// The string table must be zero-padded to a multiple of 8 bytes.
// codesign requires that the string table must be last in __LINKEDIT:
// (__LINKEDIT.filesize + .fileoff) == (LC_SYMTAB.strsize + .stroff)
// [The LC_CODE_SIGNATURE.dataoff also aligns itself to (0 mod 16),
// which is peculiar because the data array of sha256 values (one 32-byte
// value per 4KB page) has offset (4 mod 16) instead of (0 mod 16).]

        case LC_SYMTAB: {
            fprintf(stderr, "macho-snip: LC_SYMTAB skipped\n");
            continue;
            struct symtab_command *symcmd = (struct symtab_command *)cmd;
            if ((  symcmd->strsize +    symcmd->stroff)
            != (linkedit->filesize + linkedit->fileoff)) {
                fprintf(stderr,"macho-snip: bad LC_SYMTAB string table\n");
            }
            // find beginning of last name string
            unsigned j;
            struct nlist_64 *const symp = (struct nlist_64 *)(symcmd->symoff + addr);
            char *const namp0 = symcmd->stroff + addr;
            char *namp;
            for (j=0, namp = namp0; j < symcmd->nsyms; ++j) {
                namp += 1+ strlen(namp);
                if (symp[j].n_un.n_strx != (namp - namp0)) {
                    fprintf(stderr, "macho-snip: bad .n_strx\n");
                }
            }
            unsigned pad = 7& -(unsigned long)namp;
            memset(namp, 0, pad); namp += pad;  // zero pad to (0 mod 8)
            symcmd->strsize = namp - namp0;
            linkedit->filesize = (namp - addr) - linkedit->fileoff;
            symcmd->nsyms -= 1;  // lop last symbol  FIXME: generalize
        } break;
        case LC_DYSYMTAB: {
            fprintf(stderr, "macho-snip: LD_DYSYMTAB skipped\n");
            continue;
            struct dysymtab_command *dysym = (struct dysymtab_command *)cmd;
            if (0==(dysym->nundefsym -= 1)) { // FIXME: generalize
                dysym->iundefsym = 0;
            }
        } break;

        case LC_BUILD_VERSION:
        case LC_DYLD_INFO:  // also LC_DYLD_INFO_ONLY because low 8 bits
        case LC_LOAD_DYLIB:
        case LC_LOAD_DYLINKER:
        case LC_MAIN:
        case LC_SOURCE_VERSION:
        case LC_UUID:
        {
            for (jargv = 2; jargv < argc; ++jargv) {
                if (argv[jargv] && !strcmp(cmd_names[cmd->cmd & 0xFF].name, argv[jargv])) {
                    argv_done |= 1uL << jargv;
                    fprintf(stderr, "macho-snip: %#x, %s\n",
                        cmd_names[cmd->cmd & 0xFF].val, cmd_names[cmd->cmd & 0xFF].name);
                    // EXPERIMENT:
                    if (cmd->cmd == LC_DYLD_INFO_ONLY) { // the "must process" case
                        struct dyld_info_command *dyldcmd = (struct dyld_info_command *)cmd;
                        dyldcmd->export_off = 0;
                        dyldcmd->export_size = 0;
                        goto next;  // EXPERIMENT
                    }
                    goto snip;
                }
            }
        } break;

        case LC_DATA_IN_CODE: {
        case LC_DYLD_EXPORTS_TRIE:
        case LC_DYLD_CHAINED_FIXUPS:
        case LC_DYLIB_CODE_SIGN_DRS:
        case LC_FUNCTION_STARTS:
        case LC_LINKER_OPTIMIZATION_HINT:
        case LC_SEGMENT_SPLIT_INFO: {
            for (jargv = 2; jargv < argc; ++jargv) {
                if (argv[jargv] && !strcmp(cmd_names[cmd->cmd & 0xFF].name, argv[jargv])) {
                    argv_done |= 1uL << jargv;
                    fprintf(stderr, "macho-snip: %#x, %s\n",
                        cmd_names[cmd->cmd & 0xFF].val, cmd_names[cmd->cmd & 0xFF].name);
                    goto snip_linkedit_data_command;
                }
            }
        } break;
        }
        continue;  // no changes ==> advance
snip_linkedit_data_command: ;
        struct linkedit_data_command *ldc = (struct linkedit_data_command *)cmd;
        end_datasize = ldc->datasize;
        end_dataoff  = ldc->datasize + ldc->dataoff;
        memset(addr + ldc->dataoff, 0, end_datasize);  // the linkedit_data
        if ((linkedit->fileoff + linkedit->filesize) == end_dataoff) {
            linkedit->filesize -= end_datasize;  // trim
        }
snip: ;
        memmove(cmd, cmd_next, headway);
        memset(headway + (char *)cmd, 0, cmdsize);  // space that was vacated
        cmd_next = cmd;  // we moved tail at *cmd_next to *cmd
        mhdr->sizeofcmds -= cmdsize;
        mhdr->ncmds -= 1;
        argv[jargv] = 0;  // snip only once per argv[]
        }  // switch
next: ;
    }  // ncmds
    argv_done |= (1<<1) | (1<<0);  // argv[0,1] do not name linker_commands
    if (~(~0uL << argc) != argv_done) {
        int j;
        for (j=2; j < argc; ++j) {
            if (!((1uL << j) & argv_done)) {
                fprintf(stderr, "macho-snip warning: %s not processed\n", argv[j]);
            }
        }
    }
    if (!(MAP_SHARED & flags)) {
        write(1, addr, st.st_size);
    }
    return 0;  // success
}

/* vim:set ts=4 sw=4 et: */
