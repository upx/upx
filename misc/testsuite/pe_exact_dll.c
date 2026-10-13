#include <windows.h>

#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

static LONG process_attach_count;
static THREAD_LOCAL volatile int tls_value = 37;

extern BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved);
extern __declspec(dllexport) int pe_exact_dll_status(void);
extern __declspec(dllexport) int pe_exact_dll_tls_add(int value);

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
    (void) instance;
    (void) reserved;
    if (reason == DLL_PROCESS_ATTACH)
        InterlockedIncrement(&process_attach_count);
    return TRUE;
}

__declspec(dllexport) int pe_exact_dll_status(void) {
    return process_attach_count == 1;
}

__declspec(dllexport) int pe_exact_dll_tls_add(int value) {
    tls_value += value;
    return tls_value;
}
