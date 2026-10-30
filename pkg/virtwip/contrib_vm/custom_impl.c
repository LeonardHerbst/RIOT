#include "custom_string.h"

/* TODO: all implementations taken from picolibc */
void *custom_memcpy(void *s1, const void *s2, size_t n)
{
    char *dst = s1;
    const char *src = s2;
    while (n--) {
        *dst++ = *src++;
    }

    return s1;
}

void *custom_memmove(void *s1, const void *s2, size_t n)
{
    char *dst = s1;
    const char *src = s2;

    if (src < dst && dst < src + n) {
        /* Have to copy backwards */
        src += n;
        dst += n;
        while (n--) {
            *--dst = *--src;
        }
    }
    else {
        while (n--) {
            *dst++ = *src++;
        }
    }

    return s1;
}

void *custom_memset(void *s, int c, size_t n)
{
    char *cur = (char *)s;
    while (n--) {
        *cur++ = (char)c;
    }

    return cur;
}

int custom_strncmp(const char *s1, const char *s2, size_t n)
{
    if (n == 0) {
        return 0;
    }

    while (n-- != 0 && *s1 == *s2) {
        if (n == 0 || *s1 == '\0') {
            break;
        }
        s1++;
        s2++;
    }

    return (*(unsigned char *)s1) - (*(unsigned char *)s2);
}

size_t custom_strlen(const char *s)
{
    const char *start = s;
    while (*s) {
        s++;
    }
    return s - start;
}
