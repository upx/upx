#include <windows.h>
#include <string.h>

typedef int (*status_fn_t)(void);
typedef int (*tls_add_fn_t)(int);

static DWORD WINAPI check_thread_tls(LPVOID argument) {
    tls_add_fn_t tls_add = (tls_add_fn_t) argument;
    return tls_add(1) == 38 ? 0 : 1;
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3)
        return 2;
    const int no_tls = argc == 3;
    if (no_tls && strcmp(argv[2], "no-tls") != 0)
        return 2;
    HMODULE module = LoadLibraryA(argv[1]);
    if (!module)
        return 3;
    status_fn_t status = (status_fn_t) (void (*)(void)) GetProcAddress(module, "pe_exact_dll_status");
    if (!status || !status())
        return 4;
    if (no_tls)
        return FreeLibrary(module) ? 0 : 7;
    tls_add_fn_t tls_add = (tls_add_fn_t) (void (*)(void)) GetProcAddress(module, "pe_exact_dll_tls_add");
    if (!tls_add || tls_add(5) != 42)
        return 4;
    HANDLE thread = CreateThread(NULL, 0, check_thread_tls, (LPVOID) tls_add, 0, NULL);
    if (!thread)
        return 5;
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
        return 6;
    DWORD thread_result;
    if (!GetExitCodeThread(thread, &thread_result) || !CloseHandle(thread) || thread_result != 0)
        return 6;
    if (!FreeLibrary(module))
        return 7;
    return 0;
}
