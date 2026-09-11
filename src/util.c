#include "util.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

void util_trim(char *value) {
    size_t start;
    size_t end;
    size_t length;

    if (value == NULL) {
        return;
    }

    length = strlen(value);
    if (length == 0) {
        return;
    }

    start = 0;
    while (value[start] != '\0' && isspace((unsigned char)value[start])) {
        start++;
    }

    if (start == length) {
        value[0] = '\0';
        return;
    }

    end = length - 1;
    while (end > start && isspace((unsigned char)value[end])) {
        end--;
    }

    if (start > 0) {
        memmove(value, value + start, end - start + 1);
    }

    value[end - start + 1] = '\0';
}

void util_to_lower(char *dest, size_t dest_size, const char *src) {
    size_t i;

    if (dest == NULL || dest_size == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

    for (i = 0; src[i] != '\0' && i + 1 < dest_size; i++) {
        dest[i] = (char)tolower((unsigned char)src[i]);
    }

    dest[i] = '\0';
}

int util_streq_ignore_case(const char *a, const char *b) {
    if (a == NULL || b == NULL) {
        return a == b;
    }

    while (*a != '\0' && *b != '\0') {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        a++;
        b++;
    }

    return *a == *b;
}

int util_contains_ignore_case(const char *haystack, const char *needle) {
    size_t haystack_len;
    size_t needle_len;
    size_t i;

    if (haystack == NULL || needle == NULL) {
        return 0;
    }

    needle_len = strlen(needle);
    if (needle_len == 0) {
        return 1;
    }

    haystack_len = strlen(haystack);
    if (needle_len > haystack_len) {
        return 0;
    }

    for (i = 0; i + needle_len <= haystack_len; i++) {
        size_t j;
        int match = 1;

        for (j = 0; j < needle_len; j++) {
            if (tolower((unsigned char)haystack[i + j]) !=
                tolower((unsigned char)needle[j])) {
                match = 0;
                break;
            }
        }

        if (match) {
            return 1;
        }
    }

    return 0;
}

void util_now_iso8601(char *buffer, size_t buffer_size) {
    time_t now;
    struct tm utc;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    time(&now);
#if defined(_WIN32)
    gmtime_s(&utc, &now);
#else
    gmtime_r(&now, &utc);
#endif

    if (strftime(buffer, buffer_size, "%Y-%m-%dT%H:%M:%SZ", &utc) == 0) {
        buffer[0] = '\0';
    }
}

void util_copy_string(char *dest, size_t dest_size, const char *src) {
    if (dest == NULL || dest_size == 0) {
        return;
    }

    if (src == NULL) {
        dest[0] = '\0';
        return;
    }

#if defined(_MSC_VER)
    strncpy_s(dest, dest_size, src, _TRUNCATE);
#else
    snprintf(dest, dest_size, "%s", src);
#endif
}
