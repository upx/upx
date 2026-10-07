#include <windows.h>

#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

static THREAD_LOCAL volatile int tls_value = 37;
static volatile LONG process_callbacks;
static THREAD_LOCAL volatile LONG thread_callbacks;

static void NTAPI tls_callback(PVOID module, DWORD reason, PVOID reserved) {
    (void)module;
    (void)reserved;
    if (reason == DLL_PROCESS_ATTACH)
        InterlockedIncrement(&process_callbacks);
    else if (reason == DLL_THREAD_ATTACH)
        thread_callbacks++;
}

#if defined(_MSC_VER)
#pragma section(".CRT$XLB", read)
__declspec(allocate(".CRT$XLB")) static PIMAGE_TLS_CALLBACK volatile callback_entry = tls_callback;
#else
static PIMAGE_TLS_CALLBACK volatile callback_entry
    __attribute__((section(".CRT$XLB"), used)) = tls_callback;
#endif

static DWORD WINAPI check_tls(LPVOID argument) {
    (void)argument;
    tls_value += 1;
    return tls_value == 38 && thread_callbacks == 1 ? 0 : 1;
}

int main(void) {
    tls_value += 5;
    if (tls_value != 42 || callback_entry != tls_callback || process_callbacks != 1)
        return 2;
    for (unsigned i = 0; i < 2; i++) {
        HANDLE thread = CreateThread(NULL, 0, check_tls, NULL, 0, NULL);
        if (!thread)
            return 3;
        if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
            return 4;
        DWORD result;
        if (!GetExitCodeThread(thread, &result) || !CloseHandle(thread))
            return 5;
        if (result != 0 || tls_value != 42)
            return 1;
    }
    return 0;
}
