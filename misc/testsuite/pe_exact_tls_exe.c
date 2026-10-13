#include <windows.h>

#if defined(_MSC_VER)
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL __thread
#endif

static THREAD_LOCAL volatile int tls_value = 37;

static DWORD WINAPI check_tls(LPVOID argument) {
    (void)argument;
    tls_value += 1;
    return tls_value == 38 ? 0 : 1;
}

int main(void) {
    tls_value += 5;
    if (tls_value != 42)
        return 2;
    HANDLE thread = CreateThread(NULL, 0, check_tls, NULL, 0, NULL);
    if (!thread)
        return 3;
    if (WaitForSingleObject(thread, INFINITE) != WAIT_OBJECT_0)
        return 4;
    DWORD result;
    if (!GetExitCodeThread(thread, &result) || !CloseHandle(thread))
        return 5;
    return result != 0 || tls_value != 42;
}
