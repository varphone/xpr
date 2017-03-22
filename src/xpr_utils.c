#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <xpr/xpr_utils.h>

#define DBG_MAX_PRINT_SIZE 2048
void dbg_printf(int level, char* format, ...)
{
#ifdef DBG_LEVEL
    char tmp[DBG_MAX_PRINT_SIZE] = {0};
    va_list va;

    if (level <= DBG_LEVEL) {
        va_start(va, format);
        vsnprintf(tmp, DBG_MAX_PRINT_SIZE, format, va);
        va_end(va);
        fprintf(stderr, "!!!DEBUG!!! : %s", tmp);
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

int get_resolution_from_string(const char *str, int *width, int *height)
{
    if(!str || !width || !height)
        return -1;
    
    if(sscanf(str, "%dx%d", width, height) != 2) {
        if(sscanf(str, "%dX%d", width, height) !=2)
            return -1;
    }
        
    return 0;
}
#if 0
int get_action_from_key(const char *key, char *name, int len)
{
	int l, s;

	if(!key || !name)
		return -1;

    l = strlen(key);
    s = l;
    if(key[l-1] == '/')
    	return -1;

    while(l > 0 && key[l-1] != '/') {
        --l;
    }

    s = s - l;
    if(s + 1 > len)
    	return -1;

    strncpy(name, key + l, s);
    name[s+1] = '\0';
	return 0;
}
#else
int get_action_from_key(const char *key, char *name, int len)
{
	int l, s;

	if(!key || !name)
		return -1;

    l = strlen(key);
    s = l;

    if(key[l-1] == '/')
    	return -1;

    while(l > 0 && key[l-1] != '/') {
        --l;
    }

    strcpy_s(name, len, key + l);
    name[s+1] = '\0';
	return 0;
}

int get_ether_name_from_key(const char *key, char *name, int len)
{
    int l, s;
    const char* str;
    if(!key || !name)
        return -1; 

    if(len <5) 
        return -1; 

    str = strstr(key, "eth");
    if(!str)
        return -1; 

    strncpy(name, str, 4); 
    name[4] = '\0';
    return 0;
}


#endif
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
int strcpy_s(char* strDestination, int numberOfElements, const char* strSource)
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

#ifndef isspace
#define isspace(x) ((x) == ' ' || (x) == '\t' || (x) == '\r' || (x) == '\n')
#endif

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

/// @brief PackBits encode
/// @param [in] data    Original data
/// @param [in] length  Original data length
/// @param [in] buffer  Buffer to save packed data
/// @param [in] size    Buffer size
/// @return bytes of packed data
int XPR_PackBitse(unsigned char* data, int length,
                  unsigned char* buffer, int size)
{
    unsigned char* p, *q, *run, *dataend;
    int count, maxrun;
    dataend = data + length;

    for (run = data, q = buffer; length > 0; run = p, length -= count) {
        // A run cannot be longer than 128 bytes.
        maxrun = length < 128 ? length : 128;

        if (run <= (dataend - 3) && run[1] == run[0] && run[2] == run[0]) {
            // 'run' points to at least three duplicated values.
            // Step forward until run length limit, end of input,
            // or a non matching byte:
            for (p = run + 3; p < (run + maxrun) && *p == run[0];)
                ++p;

            count = p - run;
            // replace this run in output with two bytes:
            *q++ = 1 + 256 - count; /* flag byte, which encodes count (129..254) */
            *q++ = run[0];      /* byte value that is duplicated */
        } else {
            // If the input doesn't begin with at least 3 duplicated values,
            // then copy the input block, up to the run length limit,
            // end of input, or until we see three duplicated values:
            for (p = run; p < (run + maxrun);)
                if (p <= (dataend - 3) && p[1] == p[0] && p[2] == p[0])
                    break; // 3 bytes repeated end verbatim run
                else
                    ++p;

            count = p - run;
            *q++ = count - 1;      /* flag byte, which encodes count (0..127) */
            memcpy(q, run, count); /* followed by the bytes in the run */
            q += count;
        }
    }

    return q - buffer;
}

/// @brief PackBits decode
/// @param [in] data    Packed data
/// @param [in] length  Packed data length
/// @param [in] buffer  Buffer to save unpacked data
/// @param [in] size    Buffer size
/// @return bytes of unpacked data
int XPR_UnPackBits(unsigned char* data, int length,
                   unsigned char* buffer, int size)
{
    unsigned int i, len;
    int val;

    /* i counts output bytes; outlen = expected output size */
    for (i = 0; length > 1 && i < size;) {
        /* get flag byte */
        len = *data++;
        --length;

        if (len == 128) /* ignore this flag value */
            ; // warn_msg("RLE flag byte=128 ignored");
        else {
            if (len > 128) {
                len = 1 + 256 - len;
                /* get value to repeat */
                val = *data++;
                --length;

                if ((i + len) <= size)
                    memset(buffer, val, len);
                else {
                    memset(buffer, val, size - i); // fill enough to complete row
                    //printf("unpacked RLE data would overflow row (run)\n");
                    len = 0; // effectively ignore this run, probably corrupt flag byte
                }
            } else {
                ++len;

                if ((i + len) <= size) {
                    if (len > length)
                        break; // abort - ran out of input data

                    /* copy verbatim run */
                    memcpy(buffer, data, len);
                    data += len;
                    length -= len;
                } else {
                    memcpy(buffer, data, size - i); // copy enough to complete row
                    //printf("unpacked RLE data would overflow row (copy)\n");
                    len = 0; // effectively ignore
                }
            }

            buffer += len;
            i += len;
        }
    }

    if (i < size) {
        //printf("not enough RLE data for row\n");
    }

    return i;
}

// Uri
//==============================================================================
static const uint8_t uri_unescape_chars[] = 
{
    0, 0, 0, 0, 0, 0, 0, 0, // 0 - 7
    0, 0, 0, 0, 0, 0, 0, 0, // 8 - 15
    0, 0, 0, 0, 0, 0, 0, 0, // 16 - 23
    0, 0, 0, 0, 0, 0, 0, 0, // 24 - 31
    0, 0, 0, 0, 0, 0, 0, 0, // 32 - 39
    0, 0, 0, 0, 0, 0, 1, 1, // 40 - 47
    1, 1, 1, 1, 1, 1, 1, 1, // 48 - 55
    1, 1, 1, 0, 0, 0, 0, 0, // 56 - 63
    0, 1, 1, 1, 1, 1, 1, 1, // 64 - 71
    1, 1, 1, 1, 1, 1, 1, 1, // 72 - 79
    1, 1, 1, 1, 1, 1, 1, 1, // 80 - 87
    1, 1, 1, 0, 0, 0, 0, 0, // 88 - 95
    1, 1, 1, 1, 1, 1, 1, 1, // 96 - 103
    1, 1, 1, 1, 1, 1, 1, 1, // 104 - 111
    1, 1, 1, 1, 1, 1, 1, 1, // 112 - 119
    1, 1, 1, 0, 0, 0, 0, 0, // 120 - 127
    0, 0, 0, 0, 0, 0, 0, 0, // 128 - 135
    0, 0, 0, 0, 0, 0, 0, 0, // 136 - 143
    0, 0, 0, 0, 0, 0, 0, 0, // 144 - 151
    0, 0, 0, 0, 0, 0, 0, 0, // 152 - 159
    0, 0, 0, 0, 0, 0, 0, 0, // 160 - 167
    0, 0, 0, 0, 0, 0, 0, 0, // 168 - 175
    0, 0, 0, 0, 0, 0, 0, 0, // 176 - 183
    0, 0, 0, 0, 0, 0, 0, 0, // 184 - 191
    0, 0, 0, 0, 0, 0, 0, 0, // 192 - 199
    0, 0, 0, 0, 0, 0, 0, 0, // 200 - 207
    0, 0, 0, 0, 0, 0, 0, 0, // 208 - 215
    0, 0, 0, 0, 0, 0, 0, 0, // 216 - 223
    0, 0, 0, 0, 0, 0, 0, 0, // 224 - 231
    0, 0, 0, 0, 0, 0, 0, 0, // 232 - 239
    0, 0, 0, 0, 0, 0, 0, 0, // 240 - 247
    0, 0, 0, 0, 0, 0, 0, 0, // 248 - 255
    0, 0, 0, 0, 0, 0, 0, 0, // 256 - 263
};

static int htoi(int8_t hc)
{
    if (hc >= '0' && hc <= '9')
        return hc - '0';
    if (hc >= 'a' && hc <= 'f')
        return 10 + (hc - 'a');
    if (hc >= 'A' && hc <= 'F')
        return 10 + (hc - 'A');
    return 0;
}

int8_t* XPR_UriEncode(const uint8_t* uri, int length, int8_t* buffer, int* size)
{
    static int8_t hexTable[] = "0123456789ABCDEF";
    int i = 0;
    int c = 0;
    int ulen = 0;
    int bsize = 0;
    int8_t* ptr = 0;
    if (length < 0)
        length = strlen((const char*)uri);
    if (!size)
        bsize = length * 3;
    else
        bsize = *size;
    if (!buffer) {
        buffer = malloc(bsize);
        if (!buffer) {
            if (size)
                *size = 0;
            return 0;
        }
    }
    //
    ptr = buffer;
    //
    for (i = 0; i < length; i++) {
        if (bsize < 4)
            break;
        c = uri[i];
        if (uri_unescape_chars[c]) {
            *ptr++ = c;
            bsize--;
        }
        else {
            *ptr++ = '%';
            *ptr++ = hexTable[c / 16];
            *ptr++ = hexTable[c % 16];
            bsize -= 3;
        }
    }
    //
    *ptr = 0;
    //
    if (size)
        *size = ptr - buffer;
    //
    return buffer;
}

uint8_t* XPR_UriDecode(const int8_t* uri, int length, uint8_t* buffer, int* size)
{
    int i = 0;
    int c = 0;
    int ulen = 0;
    int bsize = 0;
    uint8_t* ptr = 0;
    const int8_t* uriep = 0;
    uint8_t* bufep = 0;
    if (length < 0)
        length = strlen((const char*)uri);
    if (!size)
        bsize = length;
    else
        bsize = *size;
    if (!buffer) {
        buffer = malloc(bsize);
        if (!buffer) {
            if (size)
                *size = 0;
            return 0;
        }
    }
    //
    ptr = buffer;
    uriep = uri + length + 1;
    bufep = buffer + bsize;
    //
    while (uri < uriep && ptr < bufep) {
        c = *uri;
        if (c == 0)
            break;
        if (c == '%') {
            *ptr++ = htoi(uri[1]) << 4 | htoi(uri[2]);
            uri += 2;
        }
        else {
            *ptr++ = c;
        }
        uri++;
    }
    //
    *ptr = 0;
    //
    if (size)
        *size = ptr - buffer;
    //
    return buffer;
}

// Unicode
//==============================================================================
/*
Bits | First     | Last       | Bytes | Byte 1   | Byte 2   | Byte 3   | Byte 4   | Byte 5   | Byte 6   
-----|-----------|------------|-------|----------|----------|----------|----------|----------|----------
  7  | U+0000    | U+007F     | 1     | 0xxxxxxx |          |          |          |          |          
 11  | U+0080    | U+07FF     | 2     | 110xxxxx | 10xxxxxx |          |          |          |          
 16  | U+0800    | U+FFFF     | 3     | 1110xxxx | 10xxxxxx | 10xxxxxx |          |          |          
 21  | U+10000   | U+1FFFFF   | 4     | 11110xxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |          |          
 26  | U+200000  | U+3FFFFFF  | 5     | 111110xx | 10xxxxxx | 10xxxxxx | 10xxxxxx | 10xxxxxx |          
 31  | U+4000000 | U+7FFFFFFF | 6     | 1111110x | 10xxxxxx | 10xxxxxx | 10xxxxxx | 10xxxxxx | 10xxxxxx 
*/

uint8_t XPR_UTF8_BOM[] = {0xEF, 0xBB, 0xBF};
uint8_t XPR_UTF16BE_BOM[] = {0xFE, 0xFF};
uint8_t XPR_UTF16LE_BOM[] = {0xFF, 0xFE};
uint8_t XPR_UTF32BE_BOM[] = {0x00, 0x00, 0xFE, 0xFF};
uint8_t XPR_UTF32LE_BOM[] = {0xFF, 0xFE, 0x00, 0x00};

static int UTF8C_UTF16C(const uint8_t* utf8, int length, uint16_t* utf16, int size)
{
    uint32_t v = 0;
    int retval = 1;
    switch (length) {
    case 1:
        *utf16 = *utf8 & 0x7f;
        break;
    case 2:
        v = utf8[0] & 0x1f;
        v <<= 6;
        v |= utf8[1] & 0x3f;
        *utf16 = v;
        break;
    case 3:
        v   = utf8[0] & 0x0f;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        *utf16 = v;
        break;
    case 4:
        v   = utf8[0] & 0x07;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v  -= 0x10000;
        *utf16++ = 0xD800 | ((v >> 10) & 0x3ff);
        *utf16++ = 0xDC00 | (v & 0x3ff);
        retval = 2;
        break;
    case 5:
        v   = utf8[0] & 0x03;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v  -= 0x10000;
        *utf16++ = 0xD800 | ((v >> 10) & 0x3ff);
        *utf16++ = 0xDC00 | (v & 0x3ff);
        retval = 2;
        break;
    case 6:
        v   = utf8[0] & 0x01;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v <<= 6;
        v  |= utf8[5] & 0x3f;
        v  -= 0x10000;
        *utf16++ = 0xD800 + ((v >> 10) & 0x3ff);
        *utf16++ = 0xDC00 + (v & 0x3ff);
        retval = 2;
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}

static int UTF8C_UTF16BEC(const uint8_t* utf8, int length, uint16_t* utf16, int size)
{
    uint32_t v = 0;
    int retval = 0;
    switch (length) {
    case 1:
        *utf16 = swap16(*utf8 & 0x7f);
        retval = 1;
        break;
    case 2:
        v = utf8[0] & 0x1f;
        v <<= 6;
        v |= utf8[1] & 0x3f;
        *utf16 = swap16(v);
        retval = 1;
        break;
    case 3:
        v   = utf8[0] & 0x0f;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        *utf16 = swap16(v);
        retval = 1;
        break;
    case 4:
        v   = utf8[0] & 0x07;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v  -= 0x10000;
        *utf16++ = swap16(0xD800 | ((v >> 10) & 0x3ff));
        *utf16++ = swap16(0xDC00 | (v & 0x3ff));
        retval = 2;
        break;
    case 5:
        v   = utf8[0] & 0x03;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v  -= 0x10000;
        *utf16++ = swap16(0xD800 | ((v >> 10) & 0x3ff));
        *utf16++ = swap16(0xDC00 | (v & 0x3ff));
        retval = 2;
        break;
    case 6:
        v   = utf8[0] & 0x01;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v <<= 6;
        v  |= utf8[5] & 0x3f;
        v  -= 0x10000;
        *utf16++ = swap16(0xD800 + ((v >> 10) & 0x3ff));
        *utf16++ = swap16(0xDC00 + (v & 0x3ff));
        retval = 2;
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}

static int UTF8C_UTF32C(const uint8_t* utf8, int length, uint32_t* utf32, int size)
{
    uint32_t v = 0;
    int retval = 1;
    switch (length) {
    case 1:
        *utf32 = *utf8 & 0x7f;
        break;
    case 2:
        v = utf8[0] & 0x1f;
        v <<= 6;
        v |= utf8[1] & 0x3f;
        *utf32 = v;
        break;
    case 3:
        v   = utf8[0] & 0x0f;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        *utf32 = v;
        break;
    case 4:
        v   = utf8[0] & 0x07;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        *utf32 = v;
        break;
    case 5:
        v   = utf8[0] & 0x03;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        //v  -= 0x10000;
        *utf32 = v;
        break;
    case 6:
        v   = utf8[0] & 0x01;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v <<= 6;
        v  |= utf8[5] & 0x3f;
        //v  -= 0x10000;
        *utf32 = v;
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}

static int UTF8C_UTF32BEC(const uint8_t* utf8, int length, uint32_t* utf32, int size)
{
    uint32_t v = 0;
    int retval = 1;
    switch (length) {
    case 1:
        *utf32 = swap32(*utf8 & 0x7f);
        break;
    case 2:
        v = utf8[0] & 0x1f;
        v <<= 6;
        v |= utf8[1] & 0x3f;
        *utf32 = swap32(v);
        break;
    case 3:
        v   = utf8[0] & 0x0f;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        *utf32 = swap32(v);
        break;
    case 4:
        v   = utf8[0] & 0x07;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        *utf32 = swap32(v);
        break;
    case 5:
        v   = utf8[0] & 0x03;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        //v  -= 0x10000;
        *utf32 = swap32(v);
        break;
    case 6:
        v   = utf8[0] & 0x01;
        v <<= 6;
        v  |= utf8[1] & 0x3f;
        v <<= 6;
        v  |= utf8[2] & 0x3f;
        v <<= 6;
        v  |= utf8[3] & 0x3f;
        v <<= 6;
        v  |= utf8[4] & 0x3f;
        v <<= 6;
        v  |= utf8[5] & 0x3f;
        //v  -= 0x10000;
        *utf32 = swap32(v);
        break;
    default:
        retval = 0;
        break;
    }
    return retval;
}

static int UTF16C_UTF8C(const uint16_t* utf16, int length, uint8_t* utf8, int size)
{
    uint32_t v = 0;
    int retval = 0;
    if (length == 2) {
        v   = (utf16[0] - 0xD800) & 0x3ff;
        v <<= 10;
        v  |= (utf16[1] - 0xDC00) & 0x3ff;
        v  += 10000;
    }
    else {
        v = *utf16;
    }
    if (v <= 0x7F) {
        *utf8 = v;
        retval = 1;
    }
    else if (v <= 0x7FF) {
        utf8[0] = 0xC0 | ((v >> 6) & 0x1F);
        utf8[1] = 0x80 | (v & 0x3F);
        retval = 2;
    }
    else if (v <= 0xFFFF) {
        utf8[0] = 0xE0 | ((v >> 12) & 0x0F);
        utf8[1] = 0x80 | ((v >> 6) & 0x3F);
        utf8[2] = 0x80 | (v & 0x3F);
        retval = 3;
    }
    else if (v <= 0x1FFFF) {
        utf8[0] = 0xF0 | ((v >> 18) & 0x07);
        utf8[1] = 0x80 | ((v >> 12) & 0x3F);
        utf8[2] = 0x80 | ((v >> 6) & 0x3F);
        utf8[3] = 0x80 | (v & 0x3F);
        retval = 4;
    }
    else if (v <= 0x3FFFFFF) {
        utf8[0] = 0xF8 | ((v >> 24) & 0x03);
        utf8[1] = 0x80 | ((v >> 18) & 0x3F);
        utf8[2] = 0x80 | ((v >> 12) & 0x3F);
        utf8[3] = 0x80 | ((v >> 6) & 0x3F);
        utf8[4] = 0x80 | (v & 0x3F);
        retval = 5;
    }
    else if (v <= 0x7FFFFFFF) {
        utf8[0] = 0xFC | ((v >> 30) & 0x01);
        utf8[1] = 0x80 | ((v >> 24) & 0x3F);
        utf8[2] = 0x80 | ((v >> 18) & 0x3F);
        utf8[3] = 0x80 | ((v >> 12) & 0x3F);
        utf8[4] = 0x80 | ((v >> 6) & 0x3F);
        utf8[5] = 0x80 | (v & 0x3F);
        retval = 6;
    }
    return retval;
}

uint16_t* XPR_UTF8_UTF16(const uint8_t* utf8, int length, uint16_t* utf16, int* size)
{
    int n8 = 0;
    int n16 = 0;
    int u16size = 0;
    int u16len = 0;
    uint8_t c = 0;
    uint16_t* u16 = 0;
    //
    if (length < 0)
        length = strlen((const char*)utf8);
    //
    if (!utf16) {
        u16size = length+1;
        utf16 = malloc(sizeof(uint16_t) * u16size);
    }
    else {
        if (size)
            u16size = *size;
    }
    //
    u16 = utf16;
    if (!u16 || u16size < 1)
        return 0;
    //
    while (length > 0) {
        c = *utf8;
        if (c == 0x00)
            break;
        if ((c & 0xC0) == 0x80) {
            n8 = 1;
            goto next;
        }
        if (c <= 0x7F) {
            n8 = 1;
            n16 = UTF8C_UTF16C(utf8, 1, utf16, u16size);
        }
        else if ((c & 0xE0) == 0xC0) {
            n8 = 2;
            n16 = UTF8C_UTF16C(utf8, 2, utf16, u16size);
        }
        else if ((c & 0xF0) == 0xE0) {
            n8 = 3;
            n16 = UTF8C_UTF16C(utf8, 3, utf16, u16size);
        }
        else if ((c & 0xF8) == 0xF0) {
            n8 = 4;
            n16 = UTF8C_UTF16C(utf8, 4, utf16, u16size);
        }
        else if ((c & 0xFC) == 0xF8) {
            n8 = 5;
            n16 = UTF8C_UTF16C(utf8, 5, utf16, u16size);
        }
        else if ((c & 0xFE) == 0xFC) {
            n8 = 6;
            n16 = UTF8C_UTF16C(utf8, 6, utf16, u16size);
        }
        //
        utf16   += n16;
        u16len  += n16;
        u16size -= n16;
next:
        utf8   += n8;
        length -= n8;
    }
    // Make NULL terminated
    *utf16 = 0;
    //
    if (size)
        *size = u16len;
    //
    return u16;
}

uint16_t* XPR_UTF8_UTF16BE(const uint8_t* utf8, int length, uint16_t* utf16, int* size)
{
    int n8 = 0;
    int n16 = 0;
    int u16len = 0;
    int u16size = 0;
    uint8_t c = 0;
    uint16_t* u16 = 0;
    //
    if (length < 0)
        length = strlen((const char*)utf8);
    //
    if (!utf16) {
        u16size = length+1;
        utf16 = malloc(sizeof(uint16_t) * u16size);
    }
    else {
        if (size)
            u16size = *size;
    }
    //
    u16 = utf16;
    //
    while (length > 0) {
        c = *utf8;
        if (c == 0x00)
            break;
        if ((c & 0xC0) == 0x80) {
            n8 = 1;
            goto next;
        }
        if (c <= 0x7F) {
            n8 = 1;
            n16 = UTF8C_UTF16BEC(utf8, 1, utf16, u16size);
        }
        else if ((c & 0xE0) == 0xC0) {
            n8 = 2;
            n16 = UTF8C_UTF16BEC(utf8, 2, utf16, u16size);
        }
        else if ((c & 0xF0) == 0xE0) {
            n8 = 3;
            n16 = UTF8C_UTF16BEC(utf8, 3, utf16, u16size);
        }
        else if ((c & 0xF8) == 0xF0) {
            n8 = 4;
            n16 = UTF8C_UTF16BEC(utf8, 4, utf16, u16size);
        }
        else if ((c & 0xFC) == 0xF8) {
            n8 = 5;
            n16 = UTF8C_UTF16BEC(utf8, 5, utf16, u16size);
        }
        else if ((c & 0xFE) == 0xFC) {
            n8 = 6;
            n16 = UTF8C_UTF16BEC(utf8, 6, utf16, u16size);
        }
        //
        utf16   += n16;
        u16len  += n16;
        u16size -= n16;
next:
        utf8   += n8;
        length -= n8;
    }
    //
    *utf16 = 0;
    //
    if (size)
        *size = u16len;
    //
    return u16;
}

uint16_t* XPR_UTF8_UTF16LE(const uint8_t* utf8, int length, uint16_t* utf16, int* size)
{
    return XPR_UTF8_UTF16(utf8, length, utf16, size);
}

uint32_t* XPR_UTF8_UTF32(const uint8_t* utf8, int length, uint32_t* utf32, int* size)
{
    int n8 = 0;
    int n32 = 0;
    int u32len = 0;
    int u32size = 0;
    uint8_t c = 0;
    uint32_t* u32 = 0;
    //
    if (length < 0)
        length = strlen((const char*)utf8);
    //
    if (!utf32) {
        u32size = length+1;
        utf32 = malloc(sizeof(uint32_t) * u32size);
    }
    else {
        if (size)
            u32size = *size;
    }
    //
    u32 = utf32;
    //
    while (length > 0) {
        c = *utf8;
        if (c == 0x00)
            break;
        if ((c & 0xC0) == 0x80) {
            n8 = 1;
            goto next;
        }
        if (c <= 0x7F) {
            n8 = 1;
            n32 = UTF8C_UTF32C(utf8, 1, utf32, u32size);
        }
        else if ((c & 0xE0) == 0xC0) {
            n8 = 2;
            n32 = UTF8C_UTF32C(utf8, 2, utf32, u32size);
        }
        else if ((c & 0xF0) == 0xE0) {
            n8 = 3;
            n32 = UTF8C_UTF32C(utf8, 3, utf32, u32size);
        }
        else if ((c & 0xF8) == 0xF0) {
            n8 = 4;
            n32 = UTF8C_UTF32C(utf8, 4, utf32, u32size);
        }
        else if ((c & 0xFC) == 0xF8) {
            n8 = 5;
            n32 = UTF8C_UTF32C(utf8, 5, utf32, u32size);
        }
        else if ((c & 0xFE) == 0xFC) {
            n8 = 6;
            n32 = UTF8C_UTF32C(utf8, 6, utf32, u32size);
        }
        //
        utf32   += n32;
        u32len  += n32;
        u32size -= n32;
next:
        utf8   += n8;
        length -= n8;
    }
    //
    *utf32 = 0;
    //
    if (size)
        *size = u32len;
    //
    return u32;
}

uint32_t* XPR_UTF8_UTF32BE(const uint8_t* utf8, int length, uint32_t* utf32, int* size)
{
    int n8 = 0;
    int n32 = 0;
    int u32len = 0;
    int u32size = 0;
    uint8_t c = 0;
    uint32_t* u32 = 0;
    //
    if (length < 0)
        length = strlen((const char*)utf8);
    //
    if (!utf32) {
        u32size = length+1;
        utf32 = malloc(sizeof(uint32_t) * u32size);
    }
    else {
        if (size)
            u32size = *size;
    }
    //
    u32 = utf32;
    //
    while (length > 0) {
        c = *utf8;
        if (c == 0x00)
            break;
        if ((c & 0xC0) == 0x80) {
            n8 = 1;
            goto next;
        }
        if (c <= 0x7F) {
            n8 = 1;
            n32 = UTF8C_UTF32BEC(utf8, 1, utf32, u32size);
        }
        else if ((c & 0xE0) == 0xC0) {
            n8 = 2;
            n32 = UTF8C_UTF32BEC(utf8, 2, utf32, u32size);
        }
        else if ((c & 0xF0) == 0xE0) {
            n8 = 3;
            n32 = UTF8C_UTF32BEC(utf8, 3, utf32, u32size);
        }
        else if ((c & 0xF8) == 0xF0) {
            n8 = 4;
            n32 = UTF8C_UTF32BEC(utf8, 4, utf32, u32size);
        }
        else if ((c & 0xFC) == 0xF8) {
            n8 = 5;
            n32 = UTF8C_UTF32BEC(utf8, 5, utf32, u32size);
        }
        else if ((c & 0xFE) == 0xFC) {
            n8 = 6;
            n32 = UTF8C_UTF32BEC(utf8, 6, utf32, u32size);
        }
        //
        utf32   += n32;
        u32len  += n32;
        u32size -= n32;
next:
        utf8   += n8;
        length -= n8;
    }
    //
    *utf32 = 0;
    //
    if (size)
        *size = u32len;
    //
    return u32;
}

uint32_t* XPR_UTF8_UTF32LE(const uint8_t* utf8, int length, uint32_t* utf32, int* size)
{
    return XPR_UTF8_UTF32(utf8, length, utf32, size);
}

int XPR_UTF8_GetChars(const uint8_t* utf8, int length)
{
    int chars = 0;
    uint8_t c = 0;
    if (length > 0) {
        while (length-- > 0) {
            c = *utf8++;
            if (c == 0)
                break;
            if ((c & 0xC0) != 0x80)
                chars++;
        }
    }
    else {
        while (1) {
            c = *utf8++;
            if (c == 0)
                break;
            if ((c & 0xC0) != 0x80)
                chars++;
        }
    }
    return chars;
}

wchar_t* XPR_UTF8_Unicode(const uint8_t* utf8, int length, wchar_t* wcs, int* size)
{
    if (sizeof(wchar_t) == sizeof(uint16_t))
        return (wchar_t*)XPR_UTF8_UTF16(utf8, length, (uint16_t*)wcs, size);
    else if (sizeof(wchar_t) == sizeof(uint32_t))
        return (wchar_t*)XPR_UTF8_UTF32(utf8, length, (uint32_t*)wcs, size);
    return 0;
}

uint8_t* XPR_UTF16_UTF8(const uint16_t* utf16, int length, uint8_t* utf8, int size)
{
    return 0;
}

uint8_t* XPR_UTF32_UTF8(const uint32_t* utf32, int length, uint8_t* utf8, int size)
{
    return 0;
}

uint8_t* XPR_Unicode_UTF8(const wchar_t* wcs, int length, uint8_t* utf8, int size)
{
    return 0;
}

struct XPR_Template
{
    XPR_TemplateHandler handler;
    void* opaque;
    char* src_buffer;
    int src_buffer_size;
    int src_data_size;
    char* dst_buffer;
    int dst_buffer_size;
    int dst_data_size;
};

#define XPR_TEMPLATE_BUFFER_SIZE    32768

XPR_Template* XPR_TemplateNew(void)
{
    XPR_Template* tmpl = (XPR_Template*)calloc(sizeof(*tmpl), 1);
    if (tmpl) {
        tmpl->src_buffer = (char*)malloc(XPR_TEMPLATE_BUFFER_SIZE);
        tmpl->src_buffer_size = XPR_TEMPLATE_BUFFER_SIZE;
        tmpl->dst_buffer = (char*)malloc(XPR_TEMPLATE_BUFFER_SIZE);
        tmpl->dst_buffer_size = XPR_TEMPLATE_BUFFER_SIZE;
    }
    return tmpl;
}

int XPR_TemplateDestroy(XPR_Template* tmpl)
{
    if (tmpl) {
        if (tmpl->dst_buffer) {
            free((void*)tmpl->dst_buffer);
            tmpl->dst_buffer = 0;
        }
        if (tmpl->src_buffer) {
            free((void*)tmpl->src_buffer);
            tmpl->src_buffer = 0;
        }
        free((void*)tmpl);
    }
    return 0;
}

int XPR_TemplateBuild(XPR_Template* tmpl)
{
    int l1 = 0;
    int l2 = 0;
    char* p1 = 0;
    char* p2 = 0;
    char* src_data = tmpl->src_buffer;
    int src_data_size = tmpl->src_data_size;
    char* dst_data = tmpl->dst_buffer;
    int dst_data_size = 0;
    if (src_data_size <= 0)
        return -1;
    while (src_data_size > 0) {
        p1 = strstr(src_data, "${{");
        if (!p1) {
            XPR_TemplatePutData(tmpl, src_data, src_data_size);
            src_data += src_data_size;
            src_data_size -= src_data_size;
        }
        else {
            p2 = strstr(p1, "}}");
            if (!p2) {
                XPR_TemplatePutData(tmpl, src_data, src_data_size);
                src_data += src_data_size;
                src_data_size -= src_data_size;
            }
            else {
                p1[2] = 0;
                p2[0] = 0;
                l1 = p1 - src_data;
                XPR_TemplatePutData(tmpl, src_data, l1);
                src_data += l1;
                src_data_size -= l1;
                if (tmpl->handler)
                    l2 = tmpl->handler(tmpl, p1 + 3);
                else
                    l2 = 0;
                if (l2 > 0) {
                    tmpl->dst_data_size += l2;
                }
                p1[2] = '{';
                p2[0] = '}';
                l1 = p2 - p1 + 2;
                src_data += l1;
                src_data_size -= l1;
            }
        }
    }
    return 0;
}

int XPR_TemplateLoad(XPR_Template* tmpl, const char* file)
{
    int fd = -1;
    fd = open(file, O_RDONLY);
    if (fd < 0)
        return -1;
    tmpl->src_data_size = read(fd, tmpl->src_buffer, tmpl->src_buffer_size - 4);
    close(fd);
    if (tmpl->src_data_size <= 0)
        return -1;
    tmpl->src_buffer[tmpl->src_data_size] = 0;
    return 0;
}

int XPR_TemplateSetHandler(XPR_Template* tmpl, XPR_TemplateHandler cb)
{
    tmpl->handler = cb;
    return 0;
}

XPR_TemplateHandler XPR_TemplateGetHandler(XPR_Template* tmpl)
{
    return tmpl->handler;
}

int XPR_TemplateSetOpaque(XPR_Template* tmpl, void* opaque)
{
    tmpl->opaque = opaque;
    return 0;
}

void* XPR_TemplateGetOpaque(XPR_Template* tmpl)
{
    return tmpl->opaque;
}

char* XPR_TemplateGetData(XPR_Template* tmpl)
{
    return (char*)tmpl->dst_buffer;
}

int XPR_TemplateGetDataSize(XPR_Template* tmpl)
{
    return tmpl->dst_data_size;
}

int XPR_TemplatePutf(XPR_Template* tmpl, const char* fmt, ...)
{
    int l = 0;
    va_list ap;
    va_start(ap, fmt);
    l = vsnprintf(XPR_TemplateGetSpace(tmpl), XPR_TemplateGetSpaceSize(tmpl),
                  fmt, ap);
    va_end(ap);
    if (l < 0)
        return -1;
    tmpl->dst_data_size += l;
    return l;
}

int XPR_TemplatePutData(XPR_Template* tmpl, char* data, int length)
{
    if (length == -1)
        length = strlen(data);
    if (tmpl->dst_data_size + length > tmpl->dst_buffer_size)
        return -1;
    memcpy(tmpl->dst_buffer + tmpl->dst_data_size, data, length);
    tmpl->dst_data_size += length;
    return length;
}

char* XPR_TemplateGetSpace(XPR_Template* tmpl)
{
    return (char*)(tmpl->dst_buffer + tmpl->dst_data_size);
}

int XPR_TemplateGetSpaceSize(XPR_Template* tmpl)
{
    return tmpl->dst_buffer_size - tmpl->dst_data_size;
}

int XPR_System(const char* cmdstring)
{
	pid_t pid;
	int status;
	struct sigaction ignore, saveintr,savequit;
	sigset_t chldmask ,savemask;

	if(cmdstring ==NULL)
		return 1;

	ignore.sa_handler = SIG_IGN;
	sigemptyset(&ignore.sa_mask);
	ignore.sa_flags = 0;
	if(sigaction(SIGINT, &ignore, &saveintr) <0)
		return -1;
	if(sigaction(SIGQUIT, &ignore, &savequit)<0)
		return -1;
	sigemptyset(&chldmask);
	sigaddset(&chldmask, SIGCHLD);
	if(sigprocmask(SIG_BLOCK, &chldmask, &savemask) <0)
		return -1;

	if((pid=vfork())<0) {
		status = -1;
	}
	else if(pid == 0) {
		sigaction(SIGINT, &saveintr, NULL);
		sigaction(SIGQUIT, &savequit, NULL);
		sigaction(SIG_SETMASK, &savemask, NULL);
		execl("/bin/sh", "sh", "-c", cmdstring, (char*)0);
		_exit(127);
	}
	else {
		while(waitpid(pid, &status, 0)<0) {
			if(errno != EINTR) {
				status = -1;
				break;
			}
		}
	}
	if(sigaction(SIGINT, &saveintr, NULL) <0)
		return -1;
	if(sigaction(SIGQUIT, &savequit, NULL) <0)
		return -1;
	if(sigprocmask(SIG_SETMASK, &savemask, NULL) <0)
		return -1;

	return status;
}

