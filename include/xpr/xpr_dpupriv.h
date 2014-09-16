#ifndef XPR_DPUPRIV_H
#define XPR_DPUPRIV_H

#include <stdint.h>
#include "xpr_dpu.h"

#ifdef __cplusplus
extern "C" {
#endif

struct XPR_DPU_AVFrameCallbackEntry;
//struct XPR_DPU_Context;
struct XPR_DPU_Driver;
struct XPR_DPU_EventCallbackEntry;
struct XPR_DPU_Option;
struct XPR_DPU_Rational;
struct XPR_DPU_StreamBlockCallbackEntry;

typedef struct XPR_DPU_AVFrameCallbackEntry XPR_DPU_AVFrameCallbackEntry;
//typedef struct XPR_DPU_Context XPR_DPU_Context;
typedef struct XPR_DPU_Driver XPR_DPU_Driver;
typedef struct XPR_DPU_EventCallbackEntry XPR_DPU_EventCallbackEntry;
typedef struct XPR_DPU_Option XPR_DPU_Option;
typedef struct XPR_DPU_Rational XPR_DPU_Rational;
typedef struct XPR_DPU_StreamBlockCallbackEntry XPR_DPU_StreamBlockCallbackEntry;

/// @brief Bitstream reader capabilities
enum XPR_DPU_Capability {
    XPR_DPU_CAP_HAVE_NOTHING = 0x0000,    ///< 没有任何功能
    XPR_DPU_CAP_MASK = 0xFFFFFFFF
};

/// @brief Bitstream reader identifiers
enum XPR_DPU_Id {
    XPR_DPU_ID_INVALID = 0,
    XPR_DPU_ID_ALSAPCM,   ///<
    XPR_DPU_ID_A2AAC,     ///<
    XPR_DPU_ID_A2G711A,   ///<
    XPR_DPU_ID_A2G711U,   ///<
    XPR_DPU_ID_A2G726,    ///<
    XPR_DPU_ID_A2VIDEO,   ///<
    XPR_DPU_ID_A5SAAC,    ///< Ambarella A5S AAC
    XPR_DPU_ID_A5SG711A,  ///< Ambarella A5S G711A
    XPR_DPU_ID_A5SG711U,  ///<
    XPR_DPU_ID_A5SG726,   ///<
    XPR_DPU_ID_A5SVIDEO,  ///<
    XPR_DPU_ID_A5SYUV,
    XPR_DPU_ID_AACTEST,
    XPR_DPU_ID_G711TEST,
    XPR_DPU_ID_G726TEST,
    XPR_DPU_ID_PCMTEST,
    XPR_DPU_ID_MDSD,
    XPR_DPU_ID_MAX,
};

struct XPR_DPU_AVFrameCallbackEntry {
    XPR_DPU_AVFrameCallback cb;
    void* opaque;
};

struct XPR_DPU_StreamBlockCallbackEntry {
    XPR_DPU_StreamBlockCallback cb;
    void* opaque;
};

struct XPR_DPU_EventCallbackEntry {
    XPR_DPU_EventCallback cb;
    void* opaque;
};

/// @brief 码流读取器驱动
struct XPR_DPU_Driver {
    // Properties
    const char* name;           ///< 驱动名称
    const char* description;    ///< 驱动详细描述
    int identifier;             ///< 驱动整数标识
    int privateDataSize;        ///< 驱动私有数据大小
    int capabilities;           ///< 驱动设备能力
    const XPR_DPU_Option* options;
    XPR_DPU_Driver* next;       ///< 驱动链表中的下一个

    // Driver operations
    int (*init)(XPR_DPU* ctx);   ///< 初始化驱动资源
    int (*dein)(XPR_DPU* ctx);   ///< 释放驱动已初始化资源

    // Bitstream reader operations
    int (*start)(XPR_DPU* ctx);
    int (*stop)(XPR_DPU* ctx);
    int (*getStreamCodec)(XPR_DPU* ctx, int streamId);
    int (*getStreamCount)(XPR_DPU* ctx);
    int (*getStreamParam)(XPR_DPU* ctx, int streamId, const char* name,
                          void* buffer, int* size);
    int (*setStreamParam)(XPR_DPU* ctx, int streamId, const char* name,
                          const void* data, int length);
    int (*waitForReady)(XPR_DPU* ctx);
};

/// @brief Bitstream reader context
struct XPR_DPU {
    const XPR_DPU_Driver* driver;                       ///< 驱动注册信息
    XPR_DPU_AVFrameCallbackEntry avFrameCallbacks[XPR_DPU_MAX_CALLBACKS];
    XPR_DPU_EventCallbackEntry eventCallbacks[XPR_DPU_MAX_CALLBACKS];
    XPR_DPU_RefClockCallback refClockCallback;
    XPR_DPU_StreamBlockCallbackEntry streamBlockCallbacks[XPR_DPU_MAX_CALLBACKS];
    void* refClockCallbackOpaque;
    int64_t cts;                                        ///< Current timestamp
    void* privateData;                                  ///< 驱动私有数据指针
};

/// @brief Option types
enum XPR_DPU_OptionType {
    XPR_DPU_OPT_FLAGS,
    XPR_DPU_OPT_INT,
    XPR_DPU_OPT_INT64,
    XPR_DPU_OPT_DOUBLE,
    XPR_DPU_OPT_FLOAT,
    XPR_DPU_OPT_STRING,
    XPR_DPU_OPT_RATIONAL,
    XPR_DPU_OPT_BINARY,
    XPR_DPU_OPT_CONST,
};

/// @brief Rational number numerator/denominator
struct XPR_DPU_Rational {
    int num; ///< numerator
    int den; ///< denominator
};

struct XPR_DPU_Option {
    const char* name;
    const char* help;

    /**
     * The offset relative to the context structure where the option
     * value is stored. It should be 0 for named constants.
     */
    int offset;
    enum XPR_DPU_OptionType type;

    /**
     * the default value for scalar options
     */
    union {
        int i;
        int64_t i64;
        float flt;
        double dbl;
        const char* str;
        /* TODO those are unused now */
        XPR_DPU_Rational q;
    } default_val;
    double min;                 ///< minimum valid value for the option
    double max;                 ///< maximum valid value for the option

    int flags;

    /**
     * The logical unit to which the option belongs. Non-constant
     * options and corresponding named constants share the same
     * unit. May be NULL.
     */
    const char* unit;
};

/// @brief 注册单个驱动
/// @param [in] drv     驱动注册信息
/// @return  无返回值
void XPR_DPU_Register(XPR_DPU_Driver* drv);

/// @brief 注册所有内置驱动
/// @return  无返回值
void XPR_DPU_RegisterAll(void);

/// @brief 查找指定标识的驱动注册信息
/// @param [in] id      驱动标识
/// @retval NULL    未找到
/// @retval Other   驱动注册信息
XPR_DPU_Driver* XPR_DPU_FindDriver(enum XPR_DPU_Id id);

/// @brief Find driver Id by name
enum XPR_DPU_Id XPR_DPU_FindDriverId(const char* name);

/// @brief 分配镜头驱动关联数据资源
/// @return 已分配的地址指针
XPR_DPU* XPR_DPU_AllocContext(void);

/// @brief 释放镜头驱动关联数据资源
/// @param [in] ctx     镜头驱动关联数据
/// @return 无返回值
void XPR_DPU_FreeContext(XPR_DPU* ctx);

/// @brief 打开镜头驱动
/// @param [in] ctx     码流读取器关联数据
/// @param [in] drv     驱动注册信息
/// @retval <0  失败
/// @retval >=0 成功
int XPR_DPU_OpenContext(XPR_DPU* ctx, const XPR_DPU_Driver* drv);

/// @brief 关闭镜头驱动
/// @param [in] ctx     码流读取器关联数据
/// @retval <0  失败
/// @retval >=0 成功
int XPR_DPU_CloseContext(XPR_DPU* ctx);

/// @brief 获取驱动名称
/// @param [in] ctx     镜头驱动关联数据
/// @return 驱动名称
const char* XPR_DPU_GetDriverName(const XPR_DPU* ctx);

/// @brief 获取驱动描述
/// @param [in] ctx     镜头驱动关联数据
/// @return 驱动描述
const char* XPR_DPU_GetDriverDescription(const XPR_DPU* ctx);

/// @brief 获取驱动标识
/// @param [in] ctx     码流读取器关联数据
/// @return 驱动标识
int XPR_DPU_GetDriverIdentifier(const XPR_DPU* ctx);

/// @brief 获取驱动能力
/// @param [in] ctx     码流读取器关联数据
/// @return 驱动能力
int XPR_DPU_GetDriverCapabilities(const XPR_DPU* ctx);

#ifdef __cplusplus
}
#endif

#endif // XPR_DPUPRIV_H
