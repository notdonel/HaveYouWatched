#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void util_trim(char *value);
void util_to_lower(char *dest, size_t dest_size, const char *src);
int util_streq_ignore_case(const char *a, const char *b);
int util_contains_ignore_case(const char *haystack, const char *needle);
void util_now_iso8601(char *buffer, size_t buffer_size);
void util_copy_string(char *dest, size_t dest_size, const char *src);

#endif
