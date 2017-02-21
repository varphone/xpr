#ifndef XPR_LOG_H
#define XPR_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

///
/// 日志等级
///
typedef enum XPR_LogLevel {
    XPR_LOG_LEVEL_NULL,        ///< 不记录任何内容
    XPR_LOG_LEVEL_ERROR,       ///< 记录错误消息
    XPR_LOG_LEVEL_CRITICAL,    ///< 记录危急消息
    XPR_LOG_LEVEL_WARNING,     ///< 记录警告消息
    XPR_LOG_LEVEL_NOTICE,      ///< 记录通知消息
    XPR_LOG_LEVEL_INFORMATION, ///< 记录信息消息
    XPR_LOG_LEVEL_DEBUG,       ///< 记录调式信息
    XPR_LOG_LEVEL_DEBUG2,       ///< 记录调式信息
    XPR_LOG_LEVEL_DEBUG3,       ///< 记录调式信息
    XPR_LOG_LEVEL_DEBUG4,       ///< 记录调式信息
    XPR_LOG_LEVEL_MAX,
} XPR_LogLevel;

#define XPR_LOG_LEVEL_DEFAULT XPR_LOG_LEVEL_WARNING

///
/// 日志时间格式
///
typedef enum XPR_LogTimeFormat {
    XPR_LOG_TF_NULL,             ///< 不记录时间
    XPR_LOG_TF_HIGH_PREC,    ///< 高精度时间，精确到1微秒, 格式为： 000000.000000
    XPR_LOG_TF_TIME_ONLY,             ///< 只有时间部分(UTC), 格式为： hh:mm:ss UTC
    XPR_LOG_TF_LOCAL_TIME_ONLY,        ///< 只有时间部分(本地), 格式为： hh:mm:ss
    XPR_LOG_TF_DATETIME,             ///< 有日期及时间(UTC), 格式为： YYYY-MM-DD hh:mm:ss UTC
    XPR_LOG_TF_LOCAL_DATETIME,        ///< 有日期及时间(本地), 格式为： YYYY-MM-DD hh:mm:ss
} XPR_LogTimeFormat;

#define XPR_LOG_TF_DEFAULT  XPR_LOG_TF_HIGH_PREC ///< 库初始化时默认格式

///
/// 日志输出格式
///
typedef enum XPR_LogOutputFormat {
    XPR_LOG_OF_PLAIN_TEXT,      ///< 输出纯文本格式
    XPR_LOG_OF_TTY_TEXT,        ///< 输出TTY类型文本格式， 通过不同的颜色代表不同的日志等级
} XPR_LogOutputFormat;

#define XPR_LOG_OF_DEFAULT  XPR_LOG_OF_PLAIN_TEXT ///< 库初始化时默认格式

///
/// 记录日志消息
///
/// @param [in] m       模块名称, 可以为空
/// @param [in] level   日志等级， 参见 #XPR_LogLevel
/// @param [in] fmt     日志消息格式, 参见 #printf, 可以为空
/// @param [in] ...     日志消息可变参数， 可以为空
/// @return 无返回值
void XPR_Log(const char* m, int level, const char* fmt, ...);

///
/// 记录日志消息(变参)
///
/// @sa XPR_Log()
void XPR_Logv(const char* m, int level, const char* fmt, va_list vl);

///
/// 日志记录回调函数
///
/// @param [in] m       模块名称, 可以为空
/// @param [in] level   日志等级， 参见 #XPR_LogLevel
/// @param [in] fmt     日志消息格式, 参见 #printf
/// @param [in] vl      日志消息变参
/// @return 无返回值
typedef void (*XPR_LogCallback)(const char* m, int level, const char* fmt, va_list vl);

///
/// 默认日志回调函数
///
/// @sa XPR_LogCallback
void XPR_LogDefaultCallback(const char* m, int level, const char* fmt, va_list vl);

///
/// 设置日志回调函数
///
/// @param [in] cb      日志回调函数
/// @return 无返回值
void XPR_LogSetCallback(XPR_LogCallback cb);

///
/// 获取当前日志回调函数
///
/// @return 返回当前设定的回调函数指针
XPR_LogCallback XPR_LogGetCallback(void);

///
/// 设置行结束格式
///
/// @param [in] eol     行结束格式字符串
/// @return 无返回值
/// @note Windows 下默认为: "r\n", Linux 下默认为: "\n"
void XPR_LogSetEOL(const char* eol);

///
/// 获取当前行结束格式
///
/// @return 当前行结束格式字符串地址
const char* XPR_LogGetEOL(void);

///
/// 设置日志记录等级
///
/// @param [in] level   日志等级
/// @return 无返回值
void XPR_LogSetLevel(XPR_LogLevel level);

///
/// 获取当前设定的日志记录等级
///
/// @return 返回当前设定的日志记录等级
XPR_LogLevel XPR_LogGetLevel(void);

///
/// 设定用户输出日志内容的文件描述符
///
/// @param [in] fd      已经打开的文件描述符
/// @return 无返回值
/// @note 此接口只在 Unix 平台上可用
void XPR_LogSetOutputFd(int fd);

///
/// 获取输出日志内容的文件描述符
///
/// @return 返回当前输入日志内容的文件描述符
/// @note 此接口只在 Unix 平台上可用
int XPR_LogGetOutputFd(void);

///
/// 设置输出文件名(窄字符版)
///
/// @param [in] fn      文件路径名, 可以是相当或绝对路径，也可以是其他特定文件，比如: stderr, stdout
/// @retval -1  失败
/// @retval 0   成功
/// @note 在 Windows 平台为 ACP 编码，在 Linux 平台为 UTF8 编码
/// @sa XPR_LogSetOutputFileW()
int XPR_LogSetOutputFile(const char* fn);

///
/// 设置输出文件名(宽字符版)
///
int XPR_LogSetOutputFileW(const wchar_t* fn);

///
/// 获取当前日志输出文件名(窄字符版)
///
/// @note 在 Windows 平台为 ACP 编码，在 Linux 平台为 UTF8 编码
/// @sa XPR_LogGetOutputFileW(), XPR_LogGetOutputFileU8()
const char* XPR_LogGetOutputFile(void);

///
/// 获取当前日志输出文件名(宽字符版)
///
const wchar_t* XPR_LogGetOutputFileW(void);

///
/// 设置输出文本格式
///
/// @param [in] fmt     文本格式
/// @return 无返回值
void XPR_LogSetOutputFormat(XPR_LogOutputFormat fmt);

///
/// 获取输出文本格式
///
/// @return 返回当前设定的输出文本格式
/// @sa XPR_LogSetOutputFormat()
XPR_LogOutputFormat XPR_LogGetOutputFormat(void);

///
/// 设定日志时间格式
///
/// @param [in] fmt     时间格式
/// @return 无返回值
void XPR_LogSetTimeFormat(XPR_LogTimeFormat fmt);

///
/// 获取当前日志时间格式
///
/// @return 返回当前日志时间格式
/// @sa XPR_LogSetTimeFormat()
XPR_LogTimeFormat XPR_LogGetTimeFormat(void);

///
/// 设定是否开启线程安全
///
/// @param [in] enable  线程安全开启开关， 0: 禁用, 1: 启用, 默认为： 1
/// @return 无返回值
void XPR_LogSetThreadSafe(int enable);

///
/// 获取当前线程安全是否开启
///
/// @retval 0   禁用
/// @retval 1   启用
int XPR_LogGetThreadSafe(void);

///
/// 设定是否开启自动压缩
///
/// @param [in] enable  日志自动压缩开关， 0: 禁用， 1: 启用, 默认为： 1
/// @return 无返回值
void XPR_LogSetAutoArchive(int enable);

///
/// 获取当前自动压缩是否开启
///
/// @retval 0   禁用
/// @retval 1   启用
int XPR_LogGetAutoArchive(void);

///
/// 设定最大存档数目
///
/// @param [in] num     最大存档数目, 默认为 16
/// @return 无返回值
void XPR_LogSetMaxArchives(int num);

///
/// 设定最大存档数目
///
/// return 当前设定的存档数目
int XPR_LogGetMaxArchives(void);

///
/// 设定是否开启自动轮转
///
/// @param [in] enable  日志自动轮转开关， 0: 禁用， 1: 启用, 默认为： 1
/// @return 无返回值
void XPR_LogSetAutoRotate(int enable);

///
/// 获取当前自动轮转是否开启
///
/// @retval 0   禁用
/// @retval 1   启用
int XPR_LogGetAutoRotate(void);

///
/// 设定单个日志文件轮转阈值
///
/// @param [in] size    文件大小， 默认为 1MB
/// @return 无返回值
void XPR_LogSetRotateSize(size_t size);

///
/// 获取当前单个日志文件轮转阈值
///
/// @return 当前设定的单个日志文件轮转阈值
size_t XPR_LogGetRotateSize(void);

///
/// 立即将缓存中的日志数据写入文件
///
/// @return 无返回值
void XPR_LogFlush(void);

///
/// 过滤到的日志消息回调函数
///
/// @param [in] opaque  用户关联数据
/// @param [in] m       模块名称, 可以为空
/// @param [in] level   日志等级， 参见 #XPR_LogLevel
/// @param [in] fmt     日志消息格式, 参见 #printf
/// @param [in] vl      日志消息变参
/// @return 无返回值
typedef void (*XPR_LogFilter)(void* opaque, const char* m, int level, const char* fmt, va_list vl);

///
/// 添加日志过滤器
///
/// @param [in] m       要过滤的模块名称, 可以为空，若为空则表示接收所有模块的消息
/// @param [in] level   要过滤的日志等级, 
/// @param [in] cb      过滤到的日志消息回调函数
/// @param [in] opaque  用户关联数据
/// @retval -1  失败
/// @retval 0   成功
int XPR_LogAddFilter(const char* m, int level, XPR_LogFilter cb, void* opaque);

#ifdef __cplusplus
}
#endif

#endif // IPC_SYS_H
