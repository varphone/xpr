#ifndef XPR_GPIO_H
#define XPR_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 2 部分构成：
//     major   主目标, 用来区分 GPIO 组
//     minor   次目标, 用来指定 GPIO 组中特定目标
//
// 例如：
//     PORT = 0x00010000 = 第一个 GPIO 组
//     PORT = 0x00010001 = 第一个 GPIO 组中第一个端口
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 16 个位, 其最大值是 0xFFFF
//
//==============================================================================

///
/// 合成 GPIO 端口号
///
#define XPR_GPIO_PORT(major, minor)     (((major)<<16)|(minor))

///
/// 提取 GPIO 端口号中主目标
///
#define XPR_GPIO_PORT_MAJOR(port)       (((port)>>16) & 0xffff)

///
/// 提取 GPIO 端口号中次目标
///
#define XPR_GPIO_PORT_MINOR(port)       ((port) & 0xffff)

#define XPR_GPIO_PORT_MAJOR_ALL         0xffff
#define XPR_GPIO_PORT_MAJOR_ANY         0xfffe
#define XPR_GPIO_PORT_MAJOR_DEF         0x0001
#define XPR_GPIO_PORT_MAJOR_NUL         0x0000
#define XPR_GPIO_PORT_MAJOR_MIN         0x0001
#define XPR_GPIO_PORT_MAJOR_MAX         0xfffd

#define XPR_GPIO_PORT_MINOR_ALL         0xffff
#define XPR_GPIO_PORT_MINOR_ANY         0xfffe
#define XPR_GPIO_PORT_MINOR_DEF         0x0001
#define XPR_GPIO_PORT_MINOR_NUL         0x0000
#define XPR_GPIO_PORT_MINOR_MIN         0x0001
#define XPR_GPIO_PORT_MINOR_MAX         0xfffd

#ifndef XPR_GPIO_MODE_TYPE_DEFINED
#define XPR_GPIO_MODE_TYPE_DEFINED
///
/// GPIO 工作模式定义
///
typedef enum XPR_GPIO_Mode {
    XPR_GPIO_MODE_INPUT,    ///< 输入
    XPR_GPIO_MODE_OUTPUT,   ///< 输出
    XPR_GPIO_MODE_HW,       ///< 由硬件本身决定
} XPR_GPIO_Mode;
#endif // XPR_GPIO_MODE_TYPE_DEFINED

#ifndef XPR_GPIO_LEVEL_TYPE_DEFINED
#define XPR_GPIO_LEVEL_TYPE_DEFINED
///
/// GPIO 电平定义
///
typedef enum XPP_GPIO_Level {
    XPR_GPIO_LEVEL_LOW = 0,     ///< 低电平
    XPR_GPIO_LEVEL_HIGH = 1,    ///< 高电平
} XPP_GPIO_Level;
#endif // XPR_GPIO_LEVEL_TYPE_DEFINED

///
/// 初始化 XPR_GPIO 模块
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_Init(void);

///
/// 释放 XPR_GPIO 模块已分配的资源
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_Fini(void);

///
/// 获取指定端口值
///
/// @param [in] port        GPIO 端口号 见 [#XPR_GPIO_PORT]
/// @param [in,out] value   保存端口值的地址
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_Get(int port, int* value);

///
/// 设置指定端口值
///
/// @param [in] port        GPIO 端口号, 见 [#XPR_GPIO_PORT]
/// @param [in,out] value   端口值, 见 [#XPR_GPIO_Level]
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_Set(int port, int value);

///
/// 获取指定端口工作模式
///
/// @param [in] port        GPIO 端口号 见 [#XPR_GPIO_PORT]
/// @param [in,out] mode    保存端口工作模式的地址
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_GetMode(int port, int* mode);

///
/// 设置指定端口工作模式
///
/// @param [in] port        GPIO 端口号, 见 [#XPR_GPIO_PORT]
/// @param [in,out] mode    端口工作模式, 见 [#XPR_GPIO_Mode]
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_SetMode(int port, int mode);

#ifdef __cplusplus
}
#endif

#endif // XPR_GPIO_H
