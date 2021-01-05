#include "conf.h"

/*************************************************************************
// UPX version of string functions, with assertions
**************************************************************************/

int upx_vsnprintf(char *str, upx_rsize_t max_size, const char *format, va_list ap) {
    size_t size;

    // preconditions
    assert(max_size <= UPX_RSIZE_MAX_STR);
    if (str != nullptr)
        assert(max_size > 0);
    else
        assert(max_size == 0);

    long long n = vsnprintf(str, max_size, format, ap);
    assert(n >= 0);
    assert(n < UPX_RSIZE_MAX_STR);
    size = (size_t)n + 1;

    // postconditions
    assert(size > 0);
    assert(size <= UPX_RSIZE_MAX_STR);
    if (str != nullptr) {
        assert(size <= max_size);
        assert(str[size - 1] == '\0');
    }

    return ACC_ICONV(int, size - 1); // snprintf() returns length, not size
}

int __acc_cdecl_va upx_snprintf(char *str, upx_rsize_t max_size, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_vsnprintf(str, max_size, format, ap);
    va_end(ap);
    return len;
}

int upx_vasprintf(char **ptr, const char *format, va_list ap) {
    int len;

    assert(ptr != nullptr);
    *ptr = nullptr;

    va_list ap_copy;
    va_copy(ap_copy, ap);
    len = upx_vsnprintf(nullptr, 0, format, ap_copy);
    va_end(ap_copy);

    if (len >= 0) {
        *ptr = (char *) malloc(len + 1);
        assert(*ptr != nullptr);
        if (*ptr == nullptr)
            return -1;
        int len2 = upx_vsnprintf(*ptr, len + 1, format, ap);
        assert(len2 == len);
    }
    return len;
}

int __acc_cdecl_va upx_asprintf(char **ptr, const char *format, ...) {
    va_list ap;
    int len;

    va_start(ap, format);
    len = upx_vasprintf(ptr, format, ap);
    va_end(ap);
    return len;
}

#undef strlen
upx_rsize_t upx_strlen(const char *s) {
    assert(s != nullptr);
    size_t len = strlen(s);
    assert(len < UPX_RSIZE_MAX_STR);
    return len;
}


/* vim:set ts=4 sw=4 et: */
