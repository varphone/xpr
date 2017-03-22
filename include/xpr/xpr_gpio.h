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

#define XPR_GPIO0_BASE              0x20140000
#define XPR_GPIO1_BASE              0x20150000
#define XPR_GPIO2_BASE              0x20160000
#define XPR_GPIO3_BASE              0x20170000
#define XPR_GPIO4_BASE              0x20180000
#define XPR_GPIO5_BASE              0x20190000
#define XPR_GPIO6_BASE              0x201A0000
#define XPR_GPIO7_BASE              0x201B0000
#define XPR_GPIO8_BASE              0x201C0000
#define XPR_GPIO9_BASE              0x201D0000
#define XPR_GPIO10_BASE             0x201E0000
#define XPR_GPIO11_BASE             0x201F0000
int XPR_GPIOX_BASE[12] = {XPR_GPIO0_BASE, XPR_GPIO1_BASE, XPR_GPIO2_BASE, XPR_GPIO3_BASE, XPR_GPIO4_BASE, XPR_GPIO5_BASE, XPR_GPIO6_BASE, XPR_GPIO7_BASE, XPR_GPIO8_BASE, XPR_GPIO9_BASE, XPR_GPIO10_BASE, XPR_GPIO11_BASE};

#define XPR_GPIO_DIR              0x400
#define XPR_GPIO_IS               0x404
#define XPR_GPIO_IBE              0x408
#define XPR_GPIO_IEV              0x40C
#define XPR_GPIO_IE               0x410
#define XPR_GPIO_RIS              0x414
#define XPR_GPIO_MIS              0x418
#define XPR_GPIO_IC               0x41C
int XPR_GPIO_OFFSET[8] = {XPR_GPIO_DIR, XPR_GPIO_IS, XPR_GPIO_IBE, XPR_GPIO_IEV, XPR_GPIO_IE, XPR_GPIO_RIS, XPR_GPIO_MIS, XPR_GPIO_IC};

#define GPIO0_6_ENABLE  0x200F0138
#define GPIO5_2_ENABLE  0x200F00BC
#define GPIO5_3_ENABLE  0x200F00C0


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
/// GPIO 管脚复用
///
typedef enum XPP_GPIO_Mux {
    XPR_GPIO_MUX_ENABLED  = 0,    ///< 开启
    XPR_GPIO_MUX_DISABLED = 1,    ///< 关闭
} XPP_GPIO_Mux;
#endif // XPR_GPIO_MUX_TYPE_DEFINED

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

///
/// 开启/关闭指定端口复用
///
/// @param [in] port        GPIO 端口号, 见 [#XPR_GPIO_PORT]
/// @param [in,out] value   端口值, 见 [#XPR_GPIO_Mux]
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_GPIO_Enable(int port, int value);

#ifdef __cplusplus
}
#endif

