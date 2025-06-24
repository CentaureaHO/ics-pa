#include <klib.h>
#include <stdarg.h>

static int _vsnprintf(char* out, size_t n, const char* fmt, va_list ap)
{
    char*        str   = out;
    size_t       count = 0;
    const char*  s;
    char         temp_buf[20];
    char*        p;
    int          i_val;
    unsigned int u_val;

    while (*fmt && count < n - 1)
    {
        if (*fmt != '%')
        {
            *str++ = *fmt++;
            count++;
            continue;
        }

        fmt++;  // skip '%'

        switch (*fmt)
        {
            case 's':
                s = va_arg(ap, const char*);
                if (!s) s = "(null)";
                while (*s && count < n - 1)
                {
                    *str++ = *s++;
                    count++;
                }
                break;
            case 'd':
                i_val = va_arg(ap, int);
                p     = temp_buf;
                if (i_val < 0)
                {
                    if (count < n - 1)
                    {
                        *str++ = '-';
                        count++;
                    }
                    i_val = -i_val;
                }
                if (i_val == 0) { *p++ = '0'; }
                else
                {
                    while (i_val > 0)
                    {
                        *p++ = (i_val % 10) + '0';
                        i_val /= 10;
                    }
                }
                while (p > temp_buf && count < n - 1)
                {
                    *str++ = *--p;
                    count++;
                }
                break;
            case 'x':
            case 'p':
                u_val = va_arg(ap, unsigned int);
                p     = temp_buf;
                if (u_val == 0) { *p++ = '0'; }
                else
                {
                    while (u_val > 0)
                    {
                        int digit = u_val % 16;
                        *p++      = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
                        u_val /= 16;
                    }
                }
                while (p > temp_buf && count < n - 1)
                {
                    *str++ = *--p;
                    count++;
                }
                break;
            case 'c':
                i_val = va_arg(ap, int);
                if (count < n - 1)
                {
                    *str++ = (char)i_val;
                    count++;
                }
                break;
            case '%':
                if (count < n - 1)
                {
                    *str++ = '%';
                    count++;
                }
                break;
            default:
                // unsupported format specifier, just print it
                if (count < n - 1)
                {
                    *str++ = *fmt;
                    count++;
                }
                break;
        }
        fmt++;
    }

    *str = '\0';
    return count;
}

int printf(const char* fmt, ...)
{
    char    buf[1024];
    va_list args;
    va_start(args, fmt);
    int len = _vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    for (int i = 0; i < len; i++) { _putc(buf[i]); }
    return len;
}

int sprintf(char* out, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int len = _vsnprintf(out, (size_t)-1, fmt, args);
    va_end(args);
    return len;
}

int snprintf(char* out, size_t n, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = _vsnprintf(out, n, fmt, args);
    va_end(args);
    return len;
}

int vsprintf(char* out, const char* fmt, va_list ap) { return _vsnprintf(out, (size_t)-1, fmt, ap); }

int vsnprintf(char* out, size_t n, const char* fmt, va_list ap) { return _vsnprintf(out, n, fmt, ap); }
