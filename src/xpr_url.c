#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <crtdbg.h>
#include <Windows.h>
#endif
#include <string.h>
#include <xpr/xpr_url.h>
#include <xpr/xpr_utils.h>

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

enum UrlFlags {
    URL_HAVE_NOTHING = 0,
    URL_HAVE_PROTOCOL = 1 << 1,
    URL_HAVE_PROTOCOL_MAJOR = 1 << 2,
    URL_HAVE_PROTOCOL_MINOR = 1 << 3,
    URL_HAVE_USERNAME = 1 << 4,
    URL_HAVE_PASSWORD = 1 << 5,
    URL_HAVE_HOST = 1 << 6,
    URL_HAVE_PORT = 1 << 7,
    URL_HAVE_QUERY = 1 << 8,
    URL_HAVE_PATH = 1 << 9,
    URL_HAVE_FILE_PROTOCOL = 1 << 10,
    URL_HAVE_CHANGED = 1 << 11,
};

enum UrlParsingStage {
    URL_PRS_NULL,
    URL_PRS_FOUND_PROTOCOL,
    URL_PRS_FOUND_PROTOCOL_MAJOR,
    URL_PRS_FOUND_PROTOCOL_MINOR,
    URL_PRS_FOUND_PROTOCOL_DELIM,
    URL_PRS_FOUND_USERNAME,
    URL_PRS_FOUND_PASSWORD,
    URL_PRS_FOUND_UA_DELIM,
    URL_PRS_FOUND_HOST,
    URL_PRS_FOUND_PORT,
    URL_PRS_FOUND_HP_DELIM,
    URL_PRS_FOUND_PATH,
    URL_PRS_FOUND_QUERY,
};

struct XPR_Url {
    char protocol[64];
    char protocolMajor[64];
    char protocolMinor[64];
    char username[64];
    char password[64];
    char host[256];
    int port;
    char path[256];
    char query[4096];
    char* fullString;
    int flags;
};

static const char* strnchr(const char* s, int c, int n)
{
    if (n <= 0)
        n = -1;
    while (*s && n--) {
        if (*s == c)
            return s;
        s++;
    }
    return 0;
}

#if _MSC_VER < 1600
static int strncpy_s(char* strDest, size_t numberOfElements, const char* strSource, size_t count)
{
    size_t n = MIN(numberOfElements-1, count);
    memcpy(strDest, strSource, n);
    strDest[n] = 0;
    return n;
}
#endif

XPR_Url* XPR_UrlNew(void)
{
    XPR_Url* url = (XPR_Url*)calloc(sizeof(XPR_Url), 1);
    if (url)
        url->flags |= URL_HAVE_CHANGED;
    return url;
}

void XPR_UrlDestroy(XPR_Url* url)
{
    if (url) {
        if (url->fullString) {
            free((void*)url->fullString);
            url->fullString = 0;
        }
        free((void*)url);
    }
}

static void ParseHostPart(XPR_Url* u, const char* s, int length)
{
    int n = 0;
    char ctmp = 0;
    char tmp[256] = {0};
    const char* p1 = 0;
    const char* p2 = 0;
    const char* ep = 0;
    if (length <= 0)
        length = strlen(s);
    // Save and Change
    ctmp = *(char*)(s+length);
    *(char*)(s+length) = 0;
    //
    ep = s + length;
    p1 = strnchr(s, '@', length);
    if (p1) {
        // Split username & password
        p2 = strnchr(s, ':', p1 - s);
        if (p2) {
            strncpy_s(u->username, sizeof(u->username), s, p2 - s);
            strncpy_s(u->password, sizeof(u->password), p2 + 1, p1 - p2 -1);
            u->flags |= URL_HAVE_USERNAME|URL_HAVE_PASSWORD;
        } else {
            strncpy_s(u->username, sizeof(u->username), s, p2 - s);
            u->password[0] = 0;
            u->flags |= URL_HAVE_USERNAME;
        }
        // Split host & port
        p1 += 1;
        p2 = strnchr(p1, ':', ep - p1);
        if (p2) {
            strncpy_s(u->host, sizeof(u->host), p1, p2 - p1);
            strncpy_s(tmp, sizeof(tmp), p2+1, ep - p2);
            u->port = strtol(tmp, 0, 10);
            u->flags |= URL_HAVE_HOST | URL_HAVE_PORT;
        } else {
            strncpy_s(u->host, sizeof(u->host), p1, ep - p2);
            u->port = 0;
            u->flags |= URL_HAVE_HOST;
        }
    }
    else {
        // Split host & port
        p2 = strnchr(s, ':', length);
        if (p2) {
            strncpy_s(u->host, sizeof(u->host), s, p2 - s);
            strncpy_s(tmp, sizeof(tmp), p2+1, ep - p2);
            u->port = strtol(tmp, 0, 0);
            u->flags |= URL_HAVE_HOST | URL_HAVE_PORT;
        } else {
            strncpy_s(u->host, sizeof(u->host), s, ep - s);
            u->port = 0;
            u->flags |= URL_HAVE_HOST;
        }
    }
    n = sscanf(u->protocol, "%[^.].%s", u->protocolMajor, u->protocolMinor);
    if (n > 0)
        u->flags |= URL_HAVE_PROTOCOL_MAJOR;
    if (n > 1)
        u->flags |= URL_HAVE_PROTOCOL_MINOR;
    // Restore
    *(char*)(s+length) = ctmp;
}

XPR_Url* XPR_UrlParse(const char* url, int length)
{
    int n = 0;
    XPR_Url* u = 0;
    const char* ep = 0;
    const char* p = 0;
    const char* p2 = 0;
    const char* p3 = 0;
    // Check input parameters
    if (!url)
        return 0;
    // Create an new url context
    u = XPR_UrlNew();
    if (!u)
        return 0;

    if (length < 0)
        length = strlen(url);

    ep = url + length;

    // Find protocol field
    p = strchr(url, ':');
    if (!p)
        return u;

    strncpy_s(u->protocol, sizeof(u->protocol), url, p - url);
    u->flags |= URL_HAVE_PROTOCOL;

    // Find protocol spliter
    p = strstr(p, "//");
    if (!p)
        return u;

    url = p + 2; // Skip the protocol spliter

    // If file protocol, just parse the last part
    if (strcmp(u->protocol, "file") == 0) {
        u->flags |= URL_HAVE_FILE_PROTOCOL;
        goto lastPart;
    }

    p = strchr(url, '/');
    if (!p) {
        ParseHostPart(u, url, -1);
        return u;
    }
    else if (p > (url + 1)) {
        ParseHostPart(u, url, p - url);
        url = p + 1;
    } else {
        u->host[0] = 0;
        url = p;
    }
lastPart:
    p = strchr(url, '?');
    if (p) {
        memcpy(u->path, url, p - url);
        strcpy_s(u->query, sizeof(u->query), p+1);
        u->flags |= URL_HAVE_PATH|URL_HAVE_QUERY;
    } else {
        strcpy_s(u->path, sizeof(u->path), url);
        u->query[0] = 0;
        u->flags |= URL_HAVE_PATH;
    }
    (void)n;
    (void)p2;
    (void)p3;
    (void)ep;
    return u;
}

const char* XPR_UrlGetProtocol(const XPR_Url* url)
{
    return url ? url->protocol : 0;
}

const char* XPR_UrlGetProtocolMajor(const XPR_Url* url)
{
    return url ? url->protocolMajor : 0;
}

const char* XPR_UrlGetProtocolMinor(const XPR_Url* url)
{
    return url ? url->protocolMinor : 0;
}

const char* XPR_UrlGetUsername(const XPR_Url* url)
{
    return url ? url->username : 0;
}

const char* XPR_UrlGetPassword(const XPR_Url* url)
{
    return url ? url->password : 0;
}

const char* XPR_UrlGetHost(const XPR_Url* url)
{
    return url ? url->host : 0;
}

int XPR_UrlGetPort(const XPR_Url* url)
{
    return url ? url->port : 0;
}

const char* XPR_UrlGetPath(const XPR_Url* url)
{
    return url ? url->path : 0;
}

const char* XPR_UrlGetRelPath(const XPR_Url* url)
{
    const char* s = XPR_UrlGetPath(url);
    if (s)
        s = strrchr(s, '/');
    if (s)
        s++;
    return s;
}

const char* XPR_UrlGetQuery(const XPR_Url* url)
{
    return url ? url->query : 0;
}

int XPR_UrlHaveProtocol(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PROTOCOL : 0;
}

int XPR_UrlHaveProtocolMajor(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PROTOCOL_MAJOR : 0;
}

int XPR_UrlHaveProtocolMinor(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PROTOCOL_MINOR : 0;
}

int XPR_UrlHaveUsername(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_USERNAME : 0;
}

int XPR_UrlHavePassword(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PASSWORD : 0;
}

int XPR_UrlHaveHost(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_HOST : 0;
}

int XPR_UrlHavePort(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PORT : 0;
}

int XPR_UrlHavePath(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_PATH : 0;
}

int XPR_UrlHaveQuery(const XPR_Url* url)
{
    return url ? url->flags & URL_HAVE_QUERY : 0;
}

void XPR_UrlReplaceProtocol(XPR_Url* url, const char* value)
{
    int n = 0;
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->protocol, sizeof(url->protocol), value);
        n = sscanf(url->protocol, "%[^.].%s", url->protocolMajor, url->protocolMinor);
        url->flags |= URL_HAVE_PROTOCOL;
        url->flags &= ~URL_HAVE_PROTOCOL_MINOR;
        if (n > 0)
            url->flags |= URL_HAVE_PROTOCOL_MAJOR;
        if (n > 1)
            url->flags |= URL_HAVE_PROTOCOL_MINOR;
    }
    else
        url->flags &= ~(URL_HAVE_PROTOCOL|URL_HAVE_PROTOCOL_MAJOR|URL_HAVE_PROTOCOL_MINOR);
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplaceUsername(XPR_Url* url, const char* value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->username, sizeof(url->username), value);
        url->flags |= URL_HAVE_USERNAME;
    }
    else
        url->flags &= ~URL_HAVE_USERNAME;
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplacePassword(XPR_Url* url, const char* value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->password, sizeof(url->password), value);
        url->flags |= URL_HAVE_PASSWORD;
    }
    else
        url->flags &= ~URL_HAVE_PASSWORD;
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplaceHost(XPR_Url* url, const char* value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->host, sizeof(url->host), value);
        url->flags |= URL_HAVE_HOST;
    }
    else
        url->flags &= ~URL_HAVE_HOST;
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplacePort(XPR_Url* url, int value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value) {
        url->port = value;
        url->flags |= URL_HAVE_PORT;
    }
    else
        url->flags &= ~URL_HAVE_PORT;
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplacePath(XPR_Url* url, const char* value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->path, sizeof(url->path), value);
        url->flags |= URL_HAVE_PATH;
    }
    else
        url->flags &= ~URL_HAVE_PATH;
    url->flags |= URL_HAVE_CHANGED;
}

void XPR_UrlReplaceQuery(XPR_Url* url, const char* value)
{
    ASSERT(url);
    if (!url)
        return;
    if (value && value[0]) {
        strcpy_s(url->query, sizeof(url->query), value);
        url->flags |= URL_HAVE_QUERY;
    }
    else
        url->flags &= ~URL_HAVE_QUERY;
    url->flags |= URL_HAVE_CHANGED;
}

const char* XPR_UrlGetFullString(XPR_Url* url)
{
    int size = 64;
    char* p = 0;
    int bufferSize = 0;

    ASSERT(url);
    if (!url)
        return 0;

    if (url->fullString) {
        if (!(url->flags & URL_HAVE_CHANGED))
            return url->fullString;
        free((void*)url->fullString);
    }

    if (url->flags & URL_HAVE_PROTOCOL_MINOR)
        size += strlen(url->protocolMajor) + 1 + strlen(url->protocolMinor);
    else if (url->flags & URL_HAVE_PROTOCOL_MAJOR)
        size += strlen(url->protocolMajor);
    if (url->flags & URL_HAVE_USERNAME)
        size += strlen(url->username);
    if (url->flags & URL_HAVE_PASSWORD)
        size += strlen(url->password);
    if (XPR_UrlHaveHost(url))
        size += strlen(url->host);
    if (XPR_UrlHavePort(url))
        size += 5;
    if (XPR_UrlHavePath(url))
        size += strlen(url->path);
    if (XPR_UrlHaveQuery(url))
        size += strlen(url->query);
    p = url->fullString = (char*)malloc(size+32);
    bufferSize = size+32;
    if (XPR_UrlHaveProtocolMinor(url)) {
        size = snprintf(p, bufferSize, "%s.%s://",
                        XPR_UrlGetProtocolMajor(url),
                        XPR_UrlGetProtocolMinor(url));
        p += size; bufferSize -= size;
    }
    else {
        size = snprintf(p, bufferSize, "%s://", XPR_UrlGetProtocolMajor(url));
        p += size; bufferSize -= size;
    }
    if (XPR_UrlHaveUsername(url) || XPR_UrlHavePassword(url)) {
        size = snprintf(p, bufferSize, "%s:%s@", XPR_UrlGetUsername(url), XPR_UrlGetPassword(url));
        p += size; bufferSize -= size;
    }
    if (XPR_UrlHaveHost(url)) {
        size = snprintf(p, bufferSize, "%s", XPR_UrlGetHost(url));
        p += size; bufferSize -= size;
    }
    if (XPR_UrlHavePort(url)) {
        size = snprintf(p, bufferSize, ":%d", XPR_UrlGetPort(url));
        p += size; bufferSize -= size;
    }
    if (XPR_UrlHavePath(url)) {
        size = snprintf(p, bufferSize, "%s%s", url->flags & URL_HAVE_FILE_PROTOCOL ? "" : "/", url->path);
        p += size; bufferSize -= size;
    }
    else {
        *p++ = '/';
        *p = 0;
        bufferSize--;
    }
    if (XPR_UrlHaveQuery(url)) {
        size = snprintf(p, bufferSize, "?%s", url->query);
        p += size; bufferSize -= size;
    }

    url->flags &= ~URL_HAVE_CHANGED;

    return url->fullString;
}
