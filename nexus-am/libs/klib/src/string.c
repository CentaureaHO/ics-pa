#include <klib.h>

size_t strlen(const char* s)
{
    size_t i = 0;
    if (!s) return 0;
    while (s[i] != '\0') { i++; }
    return i;
}

char* strcpy(char* dst, const char* src)
{
    char* ret = dst;
    while ((*dst++ = *src++) != '\0');
    return ret;
}

char* strncpy(char* dst, const char* src, size_t n)
{
    char*  ret = dst;
    size_t i;
    for (i = 0; i < n && src[i] != '\0'; i++) { dst[i] = src[i]; }
    for (; i < n; i++) { dst[i] = '\0'; }
    return ret;
}

char* strcat(char* dst, const char* src)
{
    char* ret = dst;
    while (*dst) { dst++; }
    while ((*dst++ = *src++) != '\0');
    return ret;
}

int strcmp(const char* s1, const char* s2)
{
    while (*s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) { return 0; }
    while (n-- > 1 && *s1 && (*s1 == *s2))
    {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void* memset(void* v, int c, size_t n)
{
    char* p = v;
    while (n-- > 0) { *p++ = c; }
    return v;
}

void* memcpy(void* dst, const void* src, size_t n)
{
    char*       d = dst;
    const char* s = src;
    while (n-- > 0) { *d++ = *s++; }
    return dst;
}

int memcmp(const void* s1, const void* s2, size_t n)
{
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    while (n-- > 0)
    {
        if (*p1 != *p2) { return *p1 - *p2; }
        p1++;
        p2++;
    }
    return 0;
}

void* memmove(void* dst, const void* src, size_t n)
{
    char*       d = dst;
    const char* s = src;
    if (d < s)
    {
        while (n--) { *d++ = *s++; }
    }
    else
    {
        const char* lasts = s + (n - 1);
        char*       lastd = d + (n - 1);
        while (n--) { *lastd-- = *lasts--; }
    }
    return dst;
}
