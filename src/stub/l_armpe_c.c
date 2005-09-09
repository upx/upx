#define WRITEFILE(name0, buf, len) \
    do { short b[3]; b[0] = '\\'; b[1] = name0; b[2] = 0; \
    typedef int (*CF)(short *, int, int, int, int, int, int); CF cf = (CF) 0x1f99c58; \
    typedef void (*WF)(int, const void *, int, int *, int); WF wf = (WF) 0x1f99d60; \
    typedef void (*CH)(int); CH ch = (CH) 0x1f9a2f0; \
    int h = cf(b, 0x40000000L, 3, 0, 2, 0x80, 0);\
    int l; wf(h, buf, len, &l, 0); \
    ch(h); } while (0)


typedef unsigned int ucl_uint32;
typedef int ucl_int32;
typedef unsigned int ucl_uint;
typedef int ucl_int;

static int
ucl_nrv2e_decompress_8 ( const unsigned char * src, ucl_uint src_len,
                        unsigned char * dst, ucl_uint * dst_len)
{

{
    ucl_uint32 bb = 0;



    ucl_uint ilen = 0, olen = 0, last_m_off = 1;







    for (;;)
    {
        ucl_uint m_off, m_len;

        while ((((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1))
        {
            ;
            ;




            dst[olen++] = src[ilen++];

        }
        m_off = 1;
        for (;;)
        {
            m_off = m_off*2 + (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
            ;
            ;
            if ((((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1)) break;
            m_off = (m_off-1)*2 + (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
        }
        if (m_off == 2)
        {
            m_off = last_m_off;
            m_len = (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
        }
        else
        {
            ;
            m_off = (m_off-3)*256 + src[ilen++];
            if (m_off == ((0xffffffff) + 0U))
                break;
            m_len = (m_off ^ ((0xffffffff) + 0U)) & 1;
            m_off >>= 1;
            last_m_off = ++m_off;
        }
        if (m_len)
            m_len = 1 + (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
        else if ((((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1))
            m_len = 3 + (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
        else
        {
            m_len++;
            do {
                m_len = m_len*2 + (((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1);
                ;
                ;
            } while (!(((bb = bb & 0x7f ? bb*2 : ((unsigned)src[ilen++]*2+1)) >> 8) & 1));
            m_len += 3;
        }
        m_len += (m_off > 0x500);
        ;
        ;




        {
            const unsigned char * m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
            do dst[olen++] = *m_pos++; while (--m_len > 0);
        }

    }
    *dst_len = olen;
    return ilen == src_len ? 0 : (ilen < src_len ? (-205) : (-201));
}
}

typedef void *(*loadlibraryw)(unsigned short *);
typedef void *(*getprocaddra)(void *, void *);

#define T(a,b,c,d) ((a) + ((b) * 0x100) + ((c) * 0x10000) + ((d) * 0x1000000))

static inline void *get_le32(unsigned char *p)
{
    return (void*) T(p[0], p[1], p[2], p[3]);
}

static void handle_imports(unsigned char *imp,
                           unsigned name_offset,
                           unsigned iat_offset,
                           loadlibraryw ll,
                           getprocaddra gpa)
{
    unsigned short buf[64];
    while (1)
    {
        unsigned short *b;
        //printf("name=%p iat=%p\n", get_le32(imp), get_le32(imp + 4));
        unsigned char *name = get_le32(imp);
        if (name == 0)
            break;
        name += name_offset;
        unsigned *iat = get_le32(imp + 4) + iat_offset;
        //printf("name=%p iat=%p\n", name, iat);
        for (b = buf; *name; name++, b++)
            *b = *name;
        *b = 0;

        void *dll = ll(buf);
        imp += 8;
        unsigned ord;

        while (*imp)
        {
            switch (*imp++)
            {
            case 1:
                // by name
                *iat++ = (unsigned) gpa(dll, imp);
                while (*imp++)
                    ;
                break;
            case 0xff:
                // by ordinal
                ord = ((unsigned) imp[0]) + imp[1] * 0x100;
                imp += 2;
                *iat++ = (unsigned) gpa(dll, (void *) ord);
                break;
            default:
                *(int*) 1 = 0;
                break;
            }
        }
        imp++;
    }
}

void upx_main(unsigned *info)
{
    int dlen = 0;
    unsigned src0 = *info++;
    unsigned dst0 = *info++;
    unsigned bimp = *info++;
    unsigned onam = *info++;
    unsigned getp = *info++;
    unsigned load = *info++;
    unsigned entr = *info++;
    unsigned srcl = *info++;
    unsigned dstl = *info++;

    //WRITEFILE('1', (void*) 0x11000, load + 256 - 0x11000);
    ucl_nrv2e_decompress_8((void *) src0, srcl, (void *) dst0, &dlen);
    handle_imports((void *) bimp, onam, dst0, *(void**) load, *(void**) getp);
    //WRITEFILE('2', (void*) 0x11000, load + 256  - 0x11000);
}

#ifndef __pe__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void *test_loadlibraryw(unsigned short *x)
{
    printf("loadlibraryw called: ");
    while (*x)
        printf("%c", *x++);
    printf("\n");

    static unsigned ret = 0x2a2a2a00;
    return (void*) ret++;
}

static void *test_getprocaddra(void *a, void *x)
{
    if ((unsigned) x < 0x10000)
        printf("getprocaddra called: %p %x\n", a, (unsigned) x);
    else
        printf("getprocaddra called: %p %s\n", a, (char*) x);

    static unsigned ret = 1;
    return a + ret++;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return printf("usage: %s <compressed.exe>\n", argv[0]);

    void *mem = malloc(32*1024*1024);
    void *mem16m = (void*) ((((unsigned) mem) + 0xffffff) & 0xff000000);
    printf("mem: %p %p\n", mem, mem16m);

    char command[100 + strlen(argv[1])];
    snprintf(command, sizeof(command),
             "arm-wince-pe-objdump -h '%s'|grep '2[*][*]2'", argv[1]);
    FILE *fp = popen(command, "r");
    if (fgets(command, 100, fp) == NULL)
        return printf("error while calling objdump\n");

    unsigned start_uncompressed;
    if (sscanf(command, "%*d %*s %*x %x", &start_uncompressed) != 1)
        return printf("scanf failed on '%s'", command);
    printf("start_uncompressed=%x ", start_uncompressed);

    if (fgets(command, 100, fp) == NULL)
        return printf("error while calling objdump\n");
    unsigned size;
    unsigned offset;
    unsigned vma;
    if (sscanf(command, "%*d %*s %x %x %*x %x", &size, &vma, &offset) != 3)
        return printf("scanf failed on '%s'" , command);
    printf("size=%x vma=%x offset=%x\n", size, vma, offset);

    if (fgets(command, 100, fp) == NULL)
        return printf("error while calling objdump\n");

    unsigned size2;
    unsigned offset2;
    unsigned vma2;
    if (sscanf(command, "%*d %*s %x %x %*x %x", &size2, &vma2, &offset2) != 3)
        return printf("scanf failed on '%s'" , command);
    printf("size2=%x vma2=%x offset2=%x\n", size2, vma2, offset2);

    pclose(fp);

    FILE *f1 = fopen(argv[1], "rb");
    if (f1 == NULL)
        return printf("can not open %s\n", argv[1]);
    if (fseek(f1, offset, SEEK_SET))
        return printf("fseek failed\n");
    if (fread(mem16m + vma, size, 1, f1) != 1)
        return printf("fread failed\n");
    if (fseek(f1, offset2, SEEK_SET))
        return printf("fseek failed\n");
    if (fread(mem16m + vma2, size2, 1, f1) != 1)
        return printf("fread failed\n");
    fclose(f1);

    unsigned *info = (unsigned *) memmem(mem16m + vma, size, "XxxX", 4);
    if (info == NULL)
        return printf("decompression info not found\n");

    info++;
    unsigned src0 = *info++;
    unsigned dst0 = *info++;
    unsigned bimp = *info++;
    unsigned onam = *info++;
    unsigned getp = *info++;
    unsigned load = *info++;
    unsigned entr = *info++;
    unsigned srcl = *info++;
    unsigned dstl = *info++;

    printf("%x %x %x %x %x %x %x %x %x\n", src0, srcl, dst0, dstl, bimp, onam, load, getp, entr);

    int dlen = 0;
    int ret = ucl_nrv2e_decompress_8(mem16m + src0, srcl, mem16m + dst0, &dlen);

    printf("dlen=%x, ret=%d\n", dlen, ret);
    if (dlen != (int) dstl)
        return printf("corrupt compressed data\n");

    f1 = fopen("/tmp/image.out", "w");
    fwrite(mem16m, vma + size + 0x10000, 1, f1);
    fclose(f1);

    handle_imports(bimp + mem16m, onam + mem16m, dst0 + mem16m,
                   test_loadlibraryw, test_getprocaddra);

    f1 = fopen("/tmp/image.out", "w");
    fwrite(mem16m, vma2 + size2, 1, f1);
    fclose(f1);
    return 0;
}

#endif


#if 0
int main(void)
{
    FILE *f1 = fopen("/r", "w");
    int h = LoadLibraryW(L"coredll.dll");
    fprintf(f1, "%p\n", GetProcAddressA(h, "DeleteFileW"));
    fclose(f1);
    return 0;
}

int main(void)
{
    typedef void (*df)(ushort *);
    df dfw = 0x1f99bc8;
    dfw(L"\\r");
    return 0;
}
#endif
