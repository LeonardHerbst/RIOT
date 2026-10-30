#pragma once

#include <stddef.h>

void *custom_memcpy(void *, const void *, size_t);
void *custom_memmove(void *s1, const void *s2, size_t n);
void *custom_memset(void *s, int c, size_t n);
int custom_strncmp(const char *s1, const char *s2, size_t n);
size_t custom_strlen(const char *s);
