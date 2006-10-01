/* NOTE: THE FIRST ARGUMENT IS arg1, NOT arg0. */
#define arg1  rdi
#define arg1l  edi
#define arg2  rsi
#define arg2l  esi
#define arg3  rdx
#define arg3l  edx
#define arg4  rcx   /* clobbered by syscall */
#define arg4l  ecx
#define sys4   r10  /* 4th arg to syscall is not in %rcx */
#define sys4l  r10d
#define arg5  r8
#define arg5l  r8d
#define arg5b  r8b
#define arg6  r9
#define arg6l  r9d
