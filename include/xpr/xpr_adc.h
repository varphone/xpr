#ifndef XPR_ADC_H
#define XPR_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 2 部分构成：
//     major   主目标, 用来区分 ADC 类型, 1 = 普通数模转换
//     minor   次目标, 用来指定特定通道
//
// 例如：
//     PORT = 0x00010000 = 普通数模转换对象
//     PORT = 0x00010001 = 普通数模转换对象的第一个通道
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 16 个位, 其最大值是 0xFFFF
//
//==============================================================================

///
/// 合成 XPR_ADC 端口句柄
///
/// @param [in] major       主目标编号
/// @param [in] minor       次目标编号
/// @return ADC 对象句柄
#define XPR_ADC_PORT(major, minor)  (((major) << 16) | (minor))

#define XPR_ADC_PORT_MAJOR(a)       ((a)>>8 & 0xffff)
#define XPR_ADC_PORT_MINOR(a)       ((a) & 0xffff)
#define XPR_ADC_PORT_MAJOR_MASK     0xffff0000
#define XPR_ADC_PORT_MINOR_MASK     0x0000ffff

#define XPR_ADC_PORT_MAJOR_ALL      0xffff
#define XPR_ADC_PORT_MAJOR_ANY      0xfffe
#define XPR_ADC_PORT_MAJOR_DEF      0x0001
#define XPR_ADC_PORT_MAJOR_MIN      0x0001
#define XPR_ADC_PORT_MAJOR_MAX      0x000f
#define XPR_ADC_PORT_MAJOR_NUL      0x0000

#define XPR_ADC_PORT_MINOR_ALL      0xffff
#define XPR_ADC_PORT_MINOR_ANY      0xfffe
#define XPR_ADC_PORT_MINOR_DEF      0x0001
#define XPR_ADC_PORT_MINOR_MIN      0x0001
#define XPR_ADC_PORT_MINOR_MAX      0x000f
#define XPR_ADC_PORT_MINOR_NUL      0x0000

// 预定义通道端口号
//==============================================================================
#define XPR_ADC_PORT_CH1            0x00010001
#define XPR_ADC_PORT_CH2            0x00010002
#define XPR_ADC_PORT_CH3            0x00010003
#define XPR_ADC_PORT_CH4            0x00010004

#ifndef XPR_ADC_CONFIGTYPE_TYPE_DEFINED
#define XPR_ADC_CONFIGTYPE_TYPE_DEFINED
///
/// XPR_ADC 配置参数类型定义
///
typedef enum XPR_ADC_ConfigType {
    XPR_ADC_CFG_UNKNOWN, ///< 未知参数类型
    XPR_ADC_CFG_MAX,
} XPR_ADC_ConfigType;
#endif // XPR_ADC_CONFIGTYPE_TYPE_DEFINED

///
/// 配置 XPR_ADC 模块
///
/// @param [in] cfg             配置参数类型
/// @param [in] data            配置参数数据
/// @param [in] length          配置参数数据长度
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
/// @note 此接口必须要在 XPR_ADC_Init() 之前调用方可生效
int XPR_ADC_Config(int cfg, const void* data, int length);

///
/// 初始化 XPR_ADC 模块
///
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
/// @retval XPR_ERR_ADC_BUSY    已经初始化过了
int XPR_ADC_Init(void);

///
/// 释放 XPR_ADC 模块内已分配的资源
///
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
int XPR_ADC_Fini(void);

///
/// 检测端口是否准备就绪
///
/// @param [in] port            端口句柄
/// @return XPR_TRUE            已就绪
/// @return XPR_FALSE           未就绪
int XPR_ADC_IsPortReady(int port);

///
/// 检测端口是否可用
///
/// @param [in] port            端口句柄
/// @retval XPR_TRUE            端口可用
/// @retval XPR_FALSE           端口不可用
int XPR_ADC_IsPortValid(int port);

///
/// 获取指定端口的采样值
///
/// @param [in] port            端口句柄
/// @param [in,out] buffer      保存端口当前采样值的缓冲区
/// @param [in,out] size        保存端口当前采样值的缓冲区容量, 并返回实际数据长度
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
int XPR_ADC_GetValue(int port, void* buffer, int* size);

///
/// 设置端口参数
///
/// @param [in] port            端口句柄
/// @param [in] data            要设置的参数数据地址
/// @param [in] length          要设置的参数数据长度
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
int XPR_ADC_SetParam(int port, const void* data, int length);

///
/// 获取端口参数
///
/// @param [in] port            端口句柄
/// @param [in] data            要设置的参数数据地址
/// @param [in] length          要设置的参数数据长度
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
int XPR_ADC_GetParam(int port, void* buffer, int* size);

#ifdef __cplusplus
}
#endif

#endif // XPR_ADC_H

