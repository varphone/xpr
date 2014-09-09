#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <xpr/xpr_utils.h>

#define MAX_PRINT_SIZE 2048
void dbg_printf(int level, char* format, ...)
{
#ifdef DBG_LEVEL
    char                messagebuf[MAX_PRINT_SIZE]; /* Store message string   */
    va_list             va;                         /* Argument list          */

    if (level <= DBG_LEVEL) {
        memset(messagebuf, 0x00, MAX_PRINT_SIZE);

        /* Format the string                                                  */
        va_start(va, format);
        vsnprintf(messagebuf, MAX_PRINT_SIZE, format, va);
        va_end(va);
        printf("!!!DEBUG!!! : %s", messagebuf);
    }
#endif
}

char* skip_blank(char* s)
{
    while (*s) {
        if (*s != ' ' && *s != '\r' && *s != '\n')
            break;
        s++;
    }
    return s;
}

int split_to_kv(char* line, char** key, char** value)
{
    char*   k    = line;
    char*   v    = "";
    if (!line || !key || !value)
        return -1;
    while (*line) {
        if (*line == ':' || *line == '=') {
            *line = '\0';
            v = ++line;
            break;
        }
        line++;
    }
    while (*v && *v == ' ')
        v++;
    *key    = k;
    *value  = v;
    return 0;
}

#if defined(_MSC_VER)
char* strsep(char** stringp, const char* delim)
{
    char* start = *stringp;
    char* p;

    p = (start != 0) ? strpbrk(start, delim) : 0;

    if (p == 0) {
        *stringp = 0;
    }
    else {
        *p = '\0';
        *stringp = p + strlen(delim);
    }

    return start;
}
#endif

#if !defined(_MSC_VER)
int strcpy_s(char* strDestination, size_t numberOfElements, const char* strSource)
{
    char* p = strDestination;
    size_t available = numberOfElements;
    if (!strDestination || !strSource)
        return EINVAL;
    if (strDestination == strSource)
        return 0;
    while ((*p++ = *strSource++) != 0 && --available > 0) {
        ;;
    }
    if (available == 0)
        strDestination[numberOfElements - 1] = 0;
    else
        *p = 0;
    return 0;
}
#endif

char* trim_all(char* s)
{
    s = skip_blank(s);
    s = trim_tailer(s);
    return s;
}

char* trim_quotes(char* s)
{
    int l = strlen(s);
    if (s[0] == '\"' || s[0] == '\'') {
        s++; l--;
    }
    if (--l > 0 && (s[l] == '\"' || s[l] == '\''))
        s[l] = 0;
    return s;
}

#define isspace(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n')

char* trim_tailer(char* s)
{
    char* back = s + strlen(s) -1;
    while (back > s) {
        if (!isspace(*back)) {
            break;
        }
        *back-- = 0;
    }
    return s;
}

int calc_lines(const char* s)
{
    int lines = 0;
    while (*s) {
        if (*s == '\n')
            lines++;
        s++;
    }
    return lines;
}

const char* get_next_line(const char** sp)
{
    int ch = 0;
    const char* l = *sp;
    const char* s = l;
    while ((ch == *s++)) {
        if (ch == '\n')
            break;
    }
    if (*s == 0)
        return 0;
    *sp = s;
    return l;
}

int XPR_ScanH264Nalus(const uint8_t* data, unsigned int length,
                      XPR_H264NaluInfo nalus[], unsigned int maxNalus)
{
    static const uint8_t kStartCode[4] = { 0x00, 0x00, 0x00, 0x01 };
    int i = 0;
    int l = MIN(length, 1024);
    int n = 0;
    int m = 0;
    for (i = 0; i < l; i++) {
        if (data[i] != kStartCode[m]) {
            m = 0;
            continue;
        }
        if (++m > 3) {
            nalus[n].data = data + i + 1;
            if (n > 0)
                nalus[n-1].length = nalus[n].data - nalus[n-1].data - 4;
            if (++n > (maxNalus-1))
                break;
            m = 0;
        }
    }
    if (n > 0)
        nalus[n-1].length = data+length-nalus[n-1].data;
    return n;
}

XPR_IntRange XPR_IntRangeParse(const char* s)
{
    XPR_IntRange rng = {0, 0};
    sscanf(s, "%d - %d", &rng.min, &rng.max);
    return rng;
}

int XPR_IntRangePrint(XPR_IntRange rng, char* s)
{
    return sprintf(s, "%d-%d", rng.min, rng.max);
}

char* XPR_IntRangeToString(XPR_IntRange rng)
{
    char tmp[128];
    (void)XPR_IntRangePrint(rng, tmp);
#if defined(_MSC_VER)
    return _strdup(tmp);
#else
    return strdup(tmp);
#endif
}
