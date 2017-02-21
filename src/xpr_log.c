#include <stdio.h>
#include <stdlib.h>
#include <cstdarg>
#if __GNUC__ >= 4
#  include <locale>
#  include <mutex>
#else
#  include <codecvt>
#endif
#include <ctime>
#include <string>
#ifdef HAVE_STD_THREAD
#  include <thread>
namespace XD {
    typedef std::thread thread;
};
#elif HAVE_BOOST_THREAD
#  include <boost/thread.hpp>
namespace XD {
    typedef boost::thread thread;
};
#endif
#include <boost/filesystem.hpp>
#include "XPR_Common.h"
#include "Common.hpp"

// locale for loging
#if defined(WIN32) || defined(_WIN32)
_locale_t gLogLocale = 0;
#endif

XPR_LogCallback gLogCallback = XPR_LogDefaultCallback;
#if defined(WIN32)
char gLogEOL[256] = {"\r\n"};
#else
char gLogEOL[256] = {"\n"};
#endif
XPR_LogLevel      gLogLevel = XPR_LogWarningLevel;
#if defined(WIN32) || defined(WIN64)
int              gLogOutputFd = 2;
#else
int              gLogOutputFd = STDERR_FILENO;
#endif
FILE*            gLogOutputFp = stderr;
std::string      gLogOutputFile = "stderr";
std::wstring     gLogOutputFileW = L"stderr";
int              gLogOutputFlushable = 0;
XPR_LogOutputFormat gLogOutputFormat = XPR_LogDefaultOutputFormat;
XPR_LogTimeFormat gLogTimeFormat = XPR_LogDefaultTimeFormat;
int              gLogThreadSafe = 1;
#if defined(WIN32) || defined(WIN64)
struct timespec {
    long tv_sec;
    long tv_nsec;
};
struct timespec  gLogFirstTime = {0, 0};
struct timespec  gLogLastTime = {0, 0};
#else
struct timespec  gLogFirstTime = {0, 0};
struct timespec  gLogLastTime = {0, 0};
#endif
char             gAnsiColor[] = "\033[?;??m\0\0\0\0\0\0\0\0";

int gLogAutoArchive = 1;
int gLogMaxArchives = 16;
int gLogAutoRotate = 1;
size_t gLogRotateSize = 1024*1024;

#ifndef HAVE_CLOCK_GETTIME
#  define CLOCK_MONOTONIC 0
typedef int clockid_t;
#  if defined(WIN32)
#    define WIN32_LEAN_AND_MEAN 1
#    include <windows.h>
static int clock_gettime(clockid_t clk_id, struct timespec* tp)
{
    int64_t nsecs = 0;
    FILETIME ft;
    LARGE_INTEGER li;
    GetSystemTimeAsFileTime(&ft);
    li.HighPart = ft.dwHighDateTime;
    li.LowPart = ft.dwLowDateTime;
    nsecs = li.QuadPart * 100;
    if (nsecs) {
        tp->tv_sec = (long)(nsecs / 1000000000);
        tp->tv_nsec = (long)(nsecs % 1000000000);
    }
    return 0;
}
#  endif
#endif

typedef enum ColorType {
    COLOR_BLACK,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MANGENTA,
    COLOR_CYAN,
    COLOR_WHITE,
} ColorType;

typedef enum DisplayType {
    DISPLAY_OFF,
    DISPLAY_LIGHT,
    DISPLAY_UNDERLINE = 4,
    DISPLAY_BRIGHT,
    DISPLAY_INVERSE = 7,
    DISPLAY_INVISIBLE
} DisplayType;

typedef enum SurfaceType {
    SURFACE_FORE = 30,
    SURFACE_BACK = 40,
} SurfaceType;

#if defined(WIN32) || defined(WIN64)
#  include <io.h>
#  define __access _access
#  define __printf(fmt, ...) _fprintf_l(gLogOutputFp, fmt, gLogLocale, __VA_ARGS__)
#  define __vprintf(fmt, vl) _vfprintf_l(gLogOutputFp, fmt, gLogLocale, vl)
#else
#  define __access access
#  define __printf(fmt, args...) fprintf(gLogOutputFp, fmt, ##args)
#  define __vprintf(fmt, vl) vfprintf(gLogOutputFp, fmt, vl)
#endif

static const char* GetAnsiColor(ColorType clr, DisplayType dsp, SurfaceType srf)
{
    char* p_clr = &gAnsiColor[5];
    char* p_dsp = &gAnsiColor[2];
    char* p_srf = &gAnsiColor[4];
    switch (clr) {
    case COLOR_BLACK: *p_clr = '0'; break;
    case COLOR_RED: *p_clr = '1'; break;
    case COLOR_GREEN: *p_clr = '2'; break;
    case COLOR_YELLOW: *p_clr = '3'; break;
    case COLOR_BLUE: *p_clr = '4'; break;
    case COLOR_MANGENTA: *p_clr = '5'; break;
    case COLOR_CYAN: *p_clr = '6'; break;
    case COLOR_WHITE: *p_clr = '7'; break;
    default: break;
    }
    switch (dsp) {
    case DISPLAY_OFF: *p_dsp = '0'; break;
    case DISPLAY_LIGHT: *p_dsp = '1'; break;
    case DISPLAY_UNDERLINE: *p_dsp = '4'; break;
    case DISPLAY_BRIGHT: *p_dsp = '5'; break;
    case DISPLAY_INVERSE: *p_dsp = '7'; break;
    case DISPLAY_INVISIBLE: *p_dsp = '8'; break;
    default: break;
    }
    switch (srf) {
    case SURFACE_FORE: *p_srf = '3'; break;
    case SURFACE_BACK: *p_srf = '4'; break;
    default: break;
    }
    return gAnsiColor;
}

static const char* GetClearColor(void)
{
    return "\033[0m";
}

static const char* GetLevelColor(int level)
{
    switch (level) {
    case XPR_LogErrorLevel: return GetAnsiColor(COLOR_RED, DISPLAY_LIGHT, SURFACE_FORE);
    case XPR_LogCriticalLevel: return GetAnsiColor(COLOR_RED, DISPLAY_LIGHT, SURFACE_FORE);
    case XPR_LogWarningLevel: return GetAnsiColor(COLOR_YELLOW, DISPLAY_LIGHT, SURFACE_FORE);
    case XPR_LogNoticeLevel: return GetAnsiColor(COLOR_GREEN, DISPLAY_LIGHT, SURFACE_FORE);
    case XPR_LogInformationLevel: return GetAnsiColor(COLOR_BLUE, DISPLAY_LIGHT, SURFACE_FORE);
    case XPR_LogDebugLevel: return GetAnsiColor(COLOR_MANGENTA, DISPLAY_LIGHT, SURFACE_FORE);
    default: return GetAnsiColor(COLOR_WHITE, DISPLAY_LIGHT, SURFACE_FORE);
    }
}

static const char* GetLevelName(int level)
{
    switch (level) {
    case XPR_LogErrorLevel: return "Error";
    case XPR_LogCriticalLevel: return "Critical";
    case XPR_LogWarningLevel: return "Warning";
    case XPR_LogNoticeLevel: return "Notice";
    case XPR_LogInformationLevel: return "Information";
    case XPR_LogDebugLevel: return "Debug";
    default: return "Unknown";
    }
}

#if defined(WIN32) || defined(WIN64)
#include <time.h>
static void GetTime(struct tm* tmp)
{
    time_t t = time(0);
    gmtime_s(tmp, &t);
}
static void GetLocalTime(struct tm* tmp)
{
    time_t t = time(0);
    localtime_s(tmp, &t);
}
#else
static void GetTime(struct tm* tmp)
{
    time_t t = time(0);
    gmtime_r(&t, tmp);
}
static void GetLocalTime(struct tm* tmp)
{
    time_t t = time(0);
    localtime_r(&t, tmp);
}
#endif

static void PrintTime(void)
{
    switch (gLogTimeFormat) {
    case XPR_LogHighPrecisionTime:
        {
            if (gLogFirstTime.tv_sec == 0 || gLogFirstTime.tv_sec > 999999)
                clock_gettime(CLOCK_MONOTONIC, &gLogFirstTime);
            clock_gettime(CLOCK_MONOTONIC, &gLogLastTime);
            long long sec = gLogLastTime.tv_sec - gLogFirstTime.tv_sec;
            long long usec = (gLogLastTime.tv_nsec - gLogFirstTime.tv_nsec) / 1000;
            if (usec < 0) {
                sec--;
                usec += 1000000;
            }
            __printf("[%6lld.%06lld] ", sec, usec);
        }
        break;
    case XPR_LogOnlyTime:
        {
            tm tmp = {0};
            GetTime(&tmp);
            __printf("[%02d:%02d:%02d UTC] ", tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
        }
        break;
    case XPR_LogOnlyLocalTime:
        {
            tm tmp = {0};
            GetLocalTime(&tmp);
            __printf("[%02d:%02d:%02d] ", tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
        }
        break;
    case XPR_LogDateTime:
        {
            tm tmp = {0};
            GetTime(&tmp);
            __printf("[%04d-%02d-%02d %02d:%02d:%02d UTC] ",
                     tmp.tm_year + 1900, tmp.tm_mon + 1, tmp.tm_mday,
                     tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
        }
        break;
    case XPR_LogLocalDateTime:
        {
            tm tmp = {0};
            GetLocalTime(&tmp);
            __printf("[%04d-%02d-%02d %02d:%02d:%02d] ",
                     tmp.tm_year + 1900, tmp.tm_mon + 1, tmp.tm_mday,
                     tmp.tm_hour, tmp.tm_min, tmp.tm_sec);
        }
        break;
    default:
        break;
    }
}

static void ArchiveFile(char* fn)
{
    XPR_FileZip(fn, NULL, 1);
    free(fn);
}

static int OpenOutputFile(void)
{
    if (gLogOutputFile == "stderr") {
        gLogOutputFp = stderr;
        gLogOutputFlushable = 0;
    } else if (gLogOutputFile == "stdout") {
        gLogOutputFp = stdout;
        gLogOutputFlushable = 0;
    } else if ((gLogOutputFile == "null") || (gLogOutputFile == "")) {
        gLogOutputFp = 0;
        gLogOutputFlushable = 0;
    } else {
#if defined(WIN32) || defined(_WIN32)
        gLogOutputFp = _fsopen(gLogOutputFile.c_str(), "a+b", _SH_DENYNO);
#else
        gLogOutputFp = fopen(gLogOutputFile.c_str(), "a+b");
#endif
        if (!gLogOutputFp)
            return -1;
        gLogOutputFlushable = 1;
    }
    return 0;
}

static void Shift(void)
{
    char fn[256];
    char fn2[256];
    const char* fmt = gLogAutoArchive ? "%s.%d.zip" : "%s.%d";
    sprintf(fn, fmt, gLogOutputFile.c_str(), gLogMaxArchives);
    if (__access(fn, 0) == 0)
        unlink(fn);
    for (int i=gLogMaxArchives; i>0; --i) {
        sprintf(fn, fmt, gLogOutputFile.c_str(), i);
        sprintf(fn2, fmt, gLogOutputFile.c_str(), i+1);
        if (__access(fn, 0) == 0)
            rename(fn, fn2);
    }
    sprintf(fn, "%s.1", gLogOutputFile.c_str());
    sprintf(fn2, "%s.1.zip", gLogOutputFile.c_str());
    fflush(gLogOutputFp);
    fclose(gLogOutputFp);
    if (__access(fn, 0) == 0)
        unlink(fn);
    rename(gLogOutputFile.c_str(), fn);
    if (gLogAutoArchive) {
        XD::thread t(ArchiveFile, strdup(fn));
        t.detach();
    }
    gLogOutputFp = fopen(gLogOutputFile.c_str(), "a+b");
}

static void RotateIfNecessary(void)
{
    if (gLogRotateSize > 0 &&
        gLogAutoRotate &&
        gLogOutputFlushable &&
        gLogOutputFp) {
        if (ftell(gLogOutputFp) > (long)gLogRotateSize) {
            Shift();
        }
    }
}

//#if defined(WIN32) || defined(WIN64)
void XPR_LogDefaultCallback(const char* m, int level, const char* fmt, va_list vl)
{
    if (gLogOutputFp) {
        if (gLogOutputFormat == XPR_LogOutputTTYText)
            __printf("%s", GetLevelColor(level));
        if (gLogTimeFormat != XPR_LogNullTime)
            PrintTime();
        if (gLogOutputFormat == XPR_LogOutputTTYText) {
                __printf("%s: ", m);
        } else {
            if (m)
                __printf("%s.%s: ", m, GetLevelName(level));
            else
                __printf("%s: ", GetLevelName(level));
        }
        if (fmt)
            __vprintf(fmt, vl);
        if (gLogOutputFormat == XPR_LogOutputTTYText)
            __printf("%s", GetClearColor());
        __printf("%s", gLogEOL);
    }
}

void XPR_LogFileCallback(const char* m, int level, const char* fmt, va_list vl)
{
    if (gLogOutputFp) {
        if (gLogTimeFormat != XPR_LogNullTime)
            PrintTime();
        if (m)
            __printf("%s.%s: ", m, GetLevelName(level));
        else
            __printf("%s: ", GetLevelName(level));
        if (fmt)
            __vprintf(fmt, vl);
    }
}
#if 0
void XPR_LogDefaultCallback(const char* m, int level, const char* fmt, va_list vl)
{
    dprintf(gLogOutputFd, "%s", GetLevelColor(level));
    if (gLogTimeFormat != XPR_LogNullTime)
        PrintTime();
    if (m)
        dprintf(gLogOutputFd, "%s @ %p: ", getconst char*Name(m), m);
    vdprintf(gLogOutputFd, fmt, vl);
    dprintf(gLogOutputFd, "%s", GetClearColor());
}
#endif

void XPR_Log(const char* m, int level, const char* fmt, ...)
{
    if (gLogCallback && gLogLevel >= level) {
        if (gLogThreadSafe) {
            scoped_lock lock(gMutex);
            if (gLogOutputFlushable && !gLogOutputFp)
                OpenOutputFile();
            va_list vl;
            va_start(vl, fmt);
            gLogCallback(m, level, fmt, vl);
            va_end(vl);
            RotateIfNecessary();
        } else {
            if (gLogOutputFlushable && !gLogOutputFp)
                OpenOutputFile();
            va_list vl;
            va_start(vl, fmt);
            gLogCallback(m, level, fmt, vl);
            va_end(vl);
            RotateIfNecessary();
        }
    }
}

void XPR_Logv(const char* m, int level, const char* fmt, va_list vl)
{
    if (gLogCallback && gLogLevel >= level) {
        if (gLogThreadSafe) {
            scoped_lock lock(gMutex);
            if (gLogOutputFlushable && !gLogOutputFp)
                OpenOutputFile();
            gLogCallback(m, level, fmt, vl);
            RotateIfNecessary();
        } else {
            if (gLogOutputFlushable && !gLogOutputFp)
                OpenOutputFile();
            gLogCallback(m, level, fmt, vl);
            RotateIfNecessary();
        }
    }
}

void XPR_LogSetCallback(XPR_LogCallback cb)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogCallback = cb;
    } else {
        gLogCallback = cb;
    }
}

XPR_LogCallback XPR_LogGetCallback(void)
{
    return gLogCallback;
}

void XPR_LogSetEOL(const char* eol)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        std::strcpy(gLogEOL, eol);
    } else {
        std::strcpy(gLogEOL, eol);
    }
}

const char* XPR_LogGetEOL(void)
{
    return gLogEOL;
}

void XPR_LogSetLevel(XPR_LogLevel level)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogLevel = level;
    } else {
        gLogLevel = level;
    }
}

XPR_LogLevel XDF_LogGetLevel(void)
{
    return gLogLevel;
}

void XPR_LogSetLgOutputFd(int fd)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogOutputFd = fd;
    } else {
        gLogOutputFd = fd;
    }
}

int XPR_LogGetOutputFd(void)
{
    return gLogOutputFd;
}

static int SetOutputFile(const char* fileName)
{
    gLogOutputFile = fileName;
    boost::filesystem::path p(fileName);
    const boost::filesystem::path& pp = p.parent_path();
    if (!boost::filesystem::exists(pp))
        boost::filesystem::create_directories(pp);
    return OpenOutputFile();
}

static std::wstring A2W(const char* fileName)
{
    std::wstring result;
    try {
#if defined(WIN32) || defined(WIN64)
        std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv(new std::codecvt<wchar_t, char, std::mbstate_t>(""));
        result = conv.from_bytes(fileName);
#else
        //FIXME
        //std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        //result = conv.from_bytes(fileName);
#endif
    } catch (...) {
    }
    return result;
}

static std::string W2A(const wchar_t* fileName)
{
    std::string result;
    try {
#if defined(WIN32) || defined(WIN64)
        std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>> conv(new std::codecvt<wchar_t, char, std::mbstate_t>(""));
        result = conv.to_bytes(fileName);
#else
        //FIXME
        //std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
        //result = conv.to_bytes(fileName);
#endif
    } catch (...) {
    }
    return result;
}

int XPR_LogSetOutputFile(const char* fileName)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogOutputFileW = A2W(fileName);
        return SetOutputFile(fileName);
    } else {
        gLogOutputFileW = A2W(fileName);
        return SetOutputFile(fileName);
    }
}

int XPR_LogSetOutputFileW(const wchar_t* fileName)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogOutputFileW = fileName;
        std::string tmp = W2A(fileName);
        return SetOutputFile(tmp.c_str());
    } else {
        gLogOutputFileW = fileName;
        std::string tmp = W2A(fileName);
        return SetOutputFile(tmp.c_str());
    }
}

const char* XPR_LogGetOutputFile(void)
{
    return gLogOutputFile.c_str();
}

const wchar_t* XPR_LogGetOutputFileW(void)
{
    return gLogOutputFileW.c_str();
}

void XPR_LogSetOutputFormat(XPR_LogOutputFormat fmt)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogOutputFormat = fmt;
    } else {
        gLogOutputFormat = fmt;
    }
}

XPR_LogOutputFormat XPR_LogGetOutputFormat(void)
{
    return gLogOutputFormat;
}

void XPR_LogSetTimeFormat(XPR_LogTimeFormat fmt)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogTimeFormat = fmt;
    } else {
        gLogTimeFormat = fmt;
    }
}

XPR_LogTimeFormat XPR_LogGetTimeFormat(void)
{
    return gLogTimeFormat;
}

void XPR_LogSetThreadSafe(int enable)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogThreadSafe = enable;
    } else {
        gLogThreadSafe = enable;
    }
}

int XPR_LogGetThreadSafe(void)
{
    return gLogThreadSafe;
}

void XPR_LogSetAutoArchive(int enable)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogAutoArchive = enable;
    } else {
        gLogAutoArchive = enable;
    }
}

int XPR_LogGetAutoArchive(void)
{
    return gLogAutoArchive;
}

void XPR_LogSetMaxArchives(int num)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogMaxArchives = num;
    } else {
        gLogMaxArchives = num;
    }
}

int XPR_LogSetMaxArchives(void)
{
    return gLogMaxArchives;
}

void XPR_LogSetAutoRotate(int enable)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogAutoRotate = enable;
    } else {
        gLogAutoRotate = enable;
    }
}

int XPR_LogGetAutoRotate(void)
{
    return gLogAutoRotate;
}

void XPR_LogSetRotateSize(size_t size)
{
    if (gLogThreadSafe) {
        scoped_lock lock(gMutex);
        gLogRotateSize = size;
    } else {
        gLogRotateSize = size;
    }
}

size_t XPR_LogGetRotateSize(void)
{
    return gLogRotateSize;
}

void XPR_LogFlush(void)
{
    if (gLogOutputFlushable && gLogOutputFp)
        fflush(gLogOutputFp);
}
